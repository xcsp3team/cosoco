#ifndef para_H
#define para_H

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <algorithm>
#include <climits>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <mutex>
#include <thread>
#include <vector>

#include "Barrier.h"

namespace pFactory {
class Group;   // Say Group exists without defining it.

unsigned int getNbCores();
/* An instance of the class group represent :
    - a set of threads (std::thread*)
    - a set of tasks (std::function<int())
*/
class Group {
    static int groupCount;   // To get the id of a group

   public:
    /* The constructor of a group
    \param pnbThreads the number of threads
    \return An instance of group
    */
    Group(unsigned int pnbThreads);


    /* Add a task to this group of threads
    \param function the task using C++11 lambdas
    */
    void add(std::function<int()> function);

    /* Start the execution of tasks by the threads of the group
    A task is considered as completed when its associated lambda function (given in add()) return
    \param concurrent True to kill all tasks as soon as one task is terminated ()
    */
    void start(bool concurrent = false);

    /* Wait that all tasks are completed (only one in concurrent mode)
    and join all threads
    \return The return code of the winner in concurrent mode
    */
    int wait();


    /* Kill all tasks and join all threads
    \return The return code of the winner in concurrent mode
    */
    int kill();

    /* Wait some seconds and verify if the a task incompleted in concurrent mode.
    \param seconds Seconds to wait
    \return -1 if no task has won else the return code of the winner
    */
    int wait(unsigned int seconds);

    /* Wait some seconds and kill all tasks
    \param seconds Seconds to wait
    \return The return code of the winner in concurrent mode
    */
    int waitAndKill(unsigned int seconds);


    /* Reload/Reinit threads and tasks as a constructor call
     */
    void reload();

    /* For a group of N threads, this method return the thread id.
     * The thread id is between 0 and N-1.
     */
    inline unsigned int getThreadId() {
        thread_local static unsigned int threadId = UINT_MAX;
        // If the id is already calculate, it is ok
        if(threadId != UINT_MAX)
            return threadId;
        // Else loop on the threads to calculate the thread id.
        for(unsigned int i = 0; i < threads.size(); i++) {
            if(threads[i]->get_id() == std::this_thread::get_id()) {
                threadId = i;
                return threadId;
            }
        }
        return threadId;
    }


    // Inline getter/setter
    inline unsigned int getId() { return idGroup; }
    inline unsigned int getNbThreads() { return nbThreads; }
    inline unsigned int getNbLaunchedTasks() { return nbLaunchedTasks; }
    inline unsigned int getWinnerConcurrentThreads() { return winnerConcurrentThreads; }
    inline unsigned int getwinnerConcurrentReturnCode() { return winnerConcurrentReturnCode; }
    inline unsigned int getWinnerConcurrentTask() { return winnerConcurrentTask; }

    inline std::vector<int>& getReturnCodes() { return returnCodes; }

    // To stop tasks
    inline void stop() { testStop = true; }
    inline bool isStopped() { return testStop; }

   private:
    void wrapperFunction(unsigned int num);
    void wrapperWaitting(unsigned int seconds);

    // General variables for a group
    std::vector<std::thread*>         threads;
    std::vector<std::function<int()>> tasks;
    std::vector<std::function<int()>> tasksSave;
    std::vector<int>                  returnCodes;

    volatile bool testStop;

    unsigned int idGroup;
    unsigned int nbThreads;
    unsigned int nbLaunchedTasks;

    // For the concurrent mode
    bool         concurrent;
    unsigned int winnerConcurrentThreads;
    unsigned int winnerConcurrentReturnCode;
    unsigned int winnerConcurrentTask;

    // For the mutual exclusions
    Barrier*   startedBarrier;
    std::mutex tasksMutex;

    // For wait with seconds
    std::thread* waitingThreads;
};

template <class T = int>
class SafeQueue {
   public:
    explicit SafeQueue() {};
    inline void push_back(T& ele) {
        mutex.lock();
        queue.push_back(ele);
        mutex.unlock();
    };

    /* Calling this function on an empty container causes undefined behavior. */
    inline T& pop_back() {
        mutex.lock();
        T& ele = queue.back();
        queue.pop_back();
        mutex.unlock();
        return ele;
    };

   private:
    std::mutex    mutex;
    std::deque<T> queue;
};
}   // namespace pFactory

#endif
