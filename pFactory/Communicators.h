#ifndef communicators_H
#define communicators_H

#include "Parallel.h"
namespace pFactory {
struct BaseCommunicator { };

/*
 * To communicate between threads some information by copies.
 */
template <class T = int>
class Communicator : public BaseCommunicator {
   public:
    explicit Communicator(Group* g, T pemptyValue) : group(g), emptyValue(pemptyValue), oneWatchs(false) {};

    virtual ~Communicator() { }

    virtual void send(T data)                                                   = 0;
    virtual T    recv(bool& isLast)                                             = 0;
    virtual T    recv()                                                         = 0;
    virtual bool isEmpty()                                                      = 0;
    virtual void recvAll(std::vector<T>& dataNotLast, std::vector<T>& dataLast) = 0;
    virtual void recvAll(std::vector<T>& data)                                  = 0;

    virtual unsigned int getNbSend() = 0;
    virtual unsigned int getNbRecv() = 0;

    inline T getEmptyValue() { return emptyValue; }

    inline bool isOneWatchs() { return oneWatchs; }
    inline void setOneWatchs(bool p) { oneWatchs = p; }

   protected:
    Group* group;
    T      emptyValue;   // Value to represent that the receive operation return nothing
    bool   oneWatchs;
};


class OrderPointer {
   public:
    explicit OrderPointer() : next(NULL), previous(NULL), idThread(-1) {};
    explicit OrderPointer(int pidThread) : next(NULL), previous(NULL), idThread(pidThread) {};


    OrderPointer* next;
    OrderPointer* previous;
    int           idThread;
};

template <class T = int>
class MultipleQueuesCommunicator : public Communicator<T> {
   public:
    explicit MultipleQueuesCommunicator(Group* g, T pemptyValue)
        : Communicator<T>(g, pemptyValue),
          nbThreads(g->getNbThreads()),
          threadMutexs(g->getNbThreads()),
          vectorOfQueues(g->getNbThreads()),

          threadQueuesPointer(g->getNbThreads(), std::vector<unsigned int>(g->getNbThreads())),
          threadOrdersPointer(g->getNbThreads(), std::vector<OrderPointer*>(g->getNbThreads(), NULL)),
          threadOrdersPointerStart(g->getNbThreads(), NULL),
          threadOrdersPointerEnd(g->getNbThreads(), NULL),

          minQueuesPointer(g->getNbThreads()),
          minSecondQueuesPointer(g->getNbThreads()),

          nbSend(g->getNbThreads()),
          nbRecv(g->getNbThreads()),
          nbRecvAll(g->getNbThreads()) {
        for(unsigned int i = 0; i < nbThreads; i++) {
            std::vector<OrderPointer*>& ordersPointer = threadOrdersPointer[i];
            // Create OrderPointers and set the idThread
            threadOrdersPointerStart[i] = new OrderPointer();
            for(unsigned int j = 0; j < nbThreads; j++) ordersPointer[j] = new OrderPointer(j);
            threadOrdersPointerEnd[i] = new OrderPointer();

            // Set the next pointers
            threadOrdersPointerStart[i]->next = ordersPointer[0];
            for(unsigned int j = 0; j < nbThreads - 1; j++) ordersPointer[j]->next = ordersPointer[j + 1];
            ordersPointer[nbThreads - 1]->next = threadOrdersPointerEnd[i];

            // Set the previous pointers
            threadOrdersPointerEnd[i]->previous = ordersPointer[nbThreads - 1];
            for(unsigned int j = nbThreads - 1; j > 0; j--) ordersPointer[j]->previous = ordersPointer[j - 1];
            ordersPointer[0]->previous = threadOrdersPointerStart[i];

            // Delete the current pointeur (swap and delete)
            ordersPointer[i]->next->previous = ordersPointer[i]->previous;
            ordersPointer[i]->previous->next = ordersPointer[i]->next;
            delete ordersPointer[i];
            ordersPointer[i] = NULL;
        }
    };

    /*Send a data to others threads
    \param data Data to send
    Warning : user may pass a copŷ
    */
    inline void send(T data) {
        const unsigned int threadId = this->group->getThreadId();
        threadMutexs[threadId].lock();
        vectorOfQueues[threadId].push_back(data);
        // printf("send data of %d\n",threadId);
        nbSend[threadId] += (nbThreads - 1);
        threadMutexs[threadId].unlock();
    }

    /* Say if there are data to recuperate
     */
    inline bool isEmpty() {
        const unsigned int threadId = this->group->getThreadId();
        for(unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++) {
            // Browse all queue except the queue of this thread
            if(threadIdQueue != threadId) {
                // These adresses don't move, so no mutex here !
                std::deque<T>&             deque        = vectorOfQueues[threadIdQueue];
                std::vector<unsigned int>& queuePointer = threadQueuesPointer[threadIdQueue];
                if(deque.empty())
                    continue;
                else if(queuePointer[threadId] == deque.size())
                    continue;
                else
                    return false;
            }
        }
        return true;
    }
    /* Receive all elements from the communicator.
    \param data: Received elements
    Remark2: When no data is found, nothing is added in these two parameters
    */
    inline void recvAll(std::vector<T>& data) {
        const unsigned int threadId = this->group->getThreadId();
        for(unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++) {
            // Browse all queue except the queue of this thread
            if(threadIdQueue != threadId) {
                // These adresses don't move, so no mutex here !
                std::deque<T>&             deque        = vectorOfQueues[threadIdQueue];
                std::vector<unsigned int>& queuePointer = threadQueuesPointer[threadIdQueue];

                // Special issue if the watch of thread is at the end (the vector is empty)
                if(deque.empty() || queuePointer[threadId] == deque.size()) {
                    // mutex.unlock();
                    continue;
                }
                std::mutex& mutex = threadMutexs[threadIdQueue];

                unsigned int&               minQueuePointer = minQueuesPointer[threadIdQueue];
                std::vector<OrderPointer*>& ordersPointer   = threadOrdersPointer[threadIdQueue];
                mutex.lock();


                // Verify the queue of pointers
                assert(threadOrdersPointerStart[threadIdQueue]->next != NULL);
                assert(threadOrdersPointerStart[threadIdQueue]->next->next != NULL);
                assert(threadOrdersPointerStart[threadIdQueue]->previous == NULL);

                assert(threadOrdersPointerEnd[threadIdQueue]->next == NULL);
                assert(threadOrdersPointerEnd[threadIdQueue]->previous != NULL);

                // Get the minimum and the second minimum !
                minQueuePointer = queuePointer[threadOrdersPointerStart[threadIdQueue]->next->idThread];

                // Now, recuperate clauses that I have to copy
                while(queuePointer[threadId] != deque.size()) {
                    data.push_back(deque[queuePointer[threadId]++]);
                    nbRecv[threadId]++;
                }
                // At this time, this assertion have to be verify (no more clauses to recuperate)
                assert(queuePointer[threadId] == deque.size());

                // Update the ordersPointer (put the current thread in the end of the queue) (usefull to calculate the minimum and
                // the second minimum faster)
                ordersPointer[threadId]->next->previous = ordersPointer[threadId]->previous;   // Removing in the queue
                ordersPointer[threadId]->previous->next = ordersPointer[threadId]->next;       // Removing in the queue

                ordersPointer[threadId]->next = threadOrdersPointerEnd[threadIdQueue];                 // Push back in the queue
                ordersPointer[threadId]->previous = threadOrdersPointerEnd[threadIdQueue]->previous;   // Push back in the queue
                threadOrdersPointerEnd[threadIdQueue]->previous->next = ordersPointer[threadId];       // Push back in the queue
                threadOrdersPointerEnd[threadIdQueue]->previous = ordersPointer[threadId];             // Push back in the queue


                // pop data already recuperate by all threads
                if(minQueuePointer > 1000) {
                    for(unsigned int i = 0; i < minQueuePointer; i++) {
                        for(unsigned int j = 0; j < nbThreads; j++) {
                            if(j != threadIdQueue)
                                queuePointer[j]--;
                        }
                        deque.pop_front();
                    }
                }

                mutex.unlock();
            }
        }
        nbRecvAll[threadId]++;
    }

    /* Receive all elements from the communicator.
    \param dataNotLast Elements which have not been received by all threads
    \param dataLast Elements which have been received by all threads (others threads have already received the element)
    Remark1: dataNotLast and dataLast are useful to deal with copy
    Remark2: When no data is found, nothing is added in these two parameters
    */
    inline void recvAll(std::vector<T>& dataNotLast, std::vector<T>& dataLast) {
        const unsigned int threadId = this->group->getThreadId();
        for(unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++) {
            // Browse all queue except the queue of this thread
            if(threadIdQueue != threadId) {
                // These adresses don't move, so no mutex here !
                std::deque<T>&             deque        = vectorOfQueues[threadIdQueue];
                std::vector<unsigned int>& queuePointer = threadQueuesPointer[threadIdQueue];
                // Special issue if the watch of thread is at the end or that the vector is empty (no clause to recuperate)
                if(deque.empty() || queuePointer[threadId] == deque.size()) {
                    continue;
                }

                std::mutex&                 mutex                 = threadMutexs[threadIdQueue];
                unsigned int&               minQueuePointer       = minQueuesPointer[threadIdQueue];
                unsigned int&               minSecondQueuePointer = minSecondQueuesPointer[threadIdQueue];
                std::vector<OrderPointer*>& ordersPointer         = threadOrdersPointer[threadIdQueue];

                mutex.lock();

                // Verify the queue of pointers
                assert(threadOrdersPointerStart[threadIdQueue]->next != NULL);
                assert(threadOrdersPointerStart[threadIdQueue]->next->next != NULL);
                assert(threadOrdersPointerStart[threadIdQueue]->previous == NULL);

                assert(threadOrdersPointerEnd[threadIdQueue]->next == NULL);
                assert(threadOrdersPointerEnd[threadIdQueue]->previous != NULL);

                // Get the minimum and the second minimum !
                minQueuePointer          = queuePointer[threadOrdersPointerStart[threadIdQueue]->next->idThread];
                int idSecondQueuePointer = threadOrdersPointerStart[threadIdQueue]->next->next->idThread;
                minSecondQueuePointer    = (idSecondQueuePointer == -1) ? deque.size() : queuePointer[idSecondQueuePointer];

                // Recuperate clauses that I have to no copy : it is the dataLast thread that take these clauses
                if(minQueuePointer ==
                   queuePointer[threadId]) {   // I am a minumum : there are may be dataLast clauses with no copy
                    while(queuePointer[threadId] !=
                          minSecondQueuePointer) {   // warning, that can be equals (severals minimums equals)!
                        dataLast.push_back(deque[queuePointer[threadId]++]);
                        nbRecv[threadId]++;
                    }
                }
                // Now, recuperate clauses that I have to copy
                while(queuePointer[threadId] != deque.size()) {
                    dataNotLast.push_back(deque[queuePointer[threadId]++]);
                    nbRecv[threadId]++;
                }
                // At this time, this assertion have to be verify (no more clauses to recuperate)
                assert(queuePointer[threadId] == deque.size());

                // Update the ordersPointer (put the current thread in the end of the queue) (usefull to calculate the minimum and
                // the second minimum faster)
                ordersPointer[threadId]->next->previous = ordersPointer[threadId]->previous;   // Removing in the queue
                ordersPointer[threadId]->previous->next = ordersPointer[threadId]->next;       // Removing in the queue

                ordersPointer[threadId]->next = threadOrdersPointerEnd[threadIdQueue];                 // Push back in the queue
                ordersPointer[threadId]->previous = threadOrdersPointerEnd[threadIdQueue]->previous;   // Push back in the queue
                threadOrdersPointerEnd[threadIdQueue]->previous->next = ordersPointer[threadId];       // Push back in the queue
                threadOrdersPointerEnd[threadIdQueue]->previous = ordersPointer[threadId];             // Push back in the queue


                // pop data already recuperate by all threads
                if(minQueuePointer > 1000) {
                    for(unsigned int i = 0; i < minQueuePointer; i++) {
                        for(unsigned int j = 0; j < nbThreads; j++) {
                            if(j != threadIdQueue)
                                queuePointer[j]--;
                        }
                        deque.pop_front();
                    }
                }

                mutex.unlock();
            }
        }
        nbRecvAll[threadId]++;
    }

    /* Receive only one element
    \return The element or emptyValue if no element is found
    */
    inline T recv() {
        bool nothing = false;
        return recv(nothing);
    }


    /* Receive only one element
    \param isLast true if it is the dataLast thread receiving the element (ie. others have already received the element)
    \return The element or emptyValue if no element is found
    */
    inline T recv(bool& isLast) {
        const unsigned int threadId = this->group->getThreadId();
        for(unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++) {
            // printf("Browse:%d\n",threadIdQueue);
            if(threadIdQueue != threadId) {
                // Browse all queue except the queue of this thread
                unsigned int               i               = 0;
                std::deque<T>&             deque           = vectorOfQueues[threadIdQueue];
                std::vector<unsigned int>& queuePointer    = threadQueuesPointer[threadIdQueue];
                std::mutex&                mutex           = threadMutexs[threadIdQueue];
                unsigned int&              minQueuePointer = minQueuesPointer[threadIdQueue];

                mutex.lock();
                // Special issue if the watch of thread is at the end or the vector is empty : no data
                if(deque.empty() || queuePointer[threadId] == deque.size()) {
                    mutex.unlock();
                    continue;
                }
                // Recuperate the data and increment the queuePointer of the thread
                unsigned int positionRet = queuePointer[threadId];
                T            ret         = deque[queuePointer[threadId]++];
                // Update the minWatch (usefull to know data to pop)
                minQueuePointer = INT_MAX;
                for(; i < nbThreads; i++)
                    if(i != threadIdQueue && minQueuePointer > queuePointer[i])
                        minQueuePointer = queuePointer[i];
                // Find if it is the dataLast or not
                isLast = (positionRet < minQueuePointer) ? true : false;
                // pop data already recuperate by all threads
                if(minQueuePointer > 1000) {
                    for(i = 0; i < minQueuePointer; i++) {
                        for(unsigned int j = 0; j < nbThreads; j++) {
                            if(j != threadIdQueue)
                                queuePointer[j]--;
                        }
                        deque.pop_front();
                    }
                }
                // printf("Recuperate %d from %d\n",ret,threadId);
                nbRecv[threadId]++;
                mutex.unlock();
                return ret;
            }
        }
        return this->emptyValue;
    }

    inline unsigned int getNbSend() {
        unsigned int ret = 0;
        for(unsigned int threadId = 0; threadId < nbThreads; threadId++) {
            threadMutexs[threadId].lock();
            ret += nbSend[threadId];
            threadMutexs[threadId].unlock();
        }
        return ret;
    };
    inline unsigned int getNbRecv() {
        unsigned int ret = 0;
        for(unsigned int threadId = 0; threadId < nbThreads; threadId++) {
            threadMutexs[threadId].lock();
            ret += nbRecv[threadId];
            threadMutexs[threadId].unlock();
        }
        return ret;
    };

   private:
    const unsigned int      nbThreads;
    std::vector<std::mutex> threadMutexs;

    std::vector<std::deque<T>>              vectorOfQueues;
    std::vector<std::vector<unsigned int>>  threadQueuesPointer;
    std::vector<std::vector<OrderPointer*>> threadOrdersPointer;
    std::vector<OrderPointer*>              threadOrdersPointerStart;
    std::vector<OrderPointer*>              threadOrdersPointerEnd;

    std::vector<unsigned int> minQueuesPointer;
    std::vector<unsigned int> minSecondQueuesPointer;

    std::vector<unsigned int> nbSend;
    std::vector<unsigned int> nbRecv;
    std::vector<unsigned int> nbRecvAll;
};


}   // namespace pFactory
#endif
