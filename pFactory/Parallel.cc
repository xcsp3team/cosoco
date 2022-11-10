
#include "Parallel.h"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#include "Barrier.h"

namespace pFactory {

int Group::groupCount = 0;

unsigned int getNbCores() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    return (!std::thread::hardware_concurrency()) ? (std::count(std::istream_iterator<std::string>(cpuinfo),
                                                                std::istream_iterator<std::string>(), std::string("processor")))
                                                  : std::thread::hardware_concurrency();
}


Group::Group(unsigned int pnbThreads)
    : testStop(false),
      idGroup(Group::groupCount++),
      nbThreads(pnbThreads),
      nbLaunchedTasks(0),
      concurrent(false),
      winnerConcurrentThreads(UINT_MAX),
      winnerConcurrentReturnCode(UINT_MAX),
      winnerConcurrentTask(UINT_MAX) {
    startedBarrier = new Barrier(pnbThreads + 1);
    for(unsigned int i = 0; i < pnbThreads; i++) threads.push_back(new std::thread(&Group::wrapperFunction, this, i));
    printf("c [pFactory][Group N°%d] created (threads:%d).\n", idGroup, pnbThreads);
}

void Group::reload() {
    testStop                   = false;
    nbLaunchedTasks            = 0;
    concurrent                 = false;
    winnerConcurrentThreads    = UINT_MAX;
    winnerConcurrentReturnCode = UINT_MAX;
    winnerConcurrentTask       = UINT_MAX;
    // The barrier
    delete startedBarrier;
    startedBarrier = new Barrier(nbThreads + 1);
    // Threads
    threads.clear();
    for(unsigned int i = 0; i < nbThreads; i++) threads.push_back(new std::thread(&Group::wrapperFunction, this, i));
    // Tasks
    tasks.clear();
    for(auto task : tasksSave) tasks.push_back(task);
    printf("c [pFactory][Group N°%d] reload (threads:%d).\n", idGroup, nbThreads);
}


void Group::start(bool pconcurrent) {
    concurrent = pconcurrent;
    printf("c [pFactory][Group N°%d] concurrent mode: %s.\n", idGroup, concurrent ? "enabled" : "disabled");
    printf("c [pFactory][Group N°%d] computations in progress (threads:%d - tasks:%d).\n", idGroup, nbThreads, (int)tasks.size());
    startedBarrier->wait();
}

void Group::add(std::function<int()> function) {
    tasks.push_back(function);
    returnCodes.push_back(-1);
    tasksSave.push_back(function);
    printf("c [pFactory][Group N°%d] new task added (threads:%d - tasks:%d).\n", idGroup, nbThreads, (int)tasks.size());
}

int Group::wait() {
    for(unsigned int i = 0; i < nbThreads; i++) {
        if(threads[i]->joinable()) {
            threads[i]->join();
            printf("c [pFactory][Group N°%d] Thread N°%d is joined.\n", idGroup, i);
        }
    }
    if(concurrent) {
        printf("c [pFactory][Group N°%d] Return Code of the winner:%d (Thread N°%d)\n", idGroup, winnerConcurrentReturnCode,
               winnerConcurrentThreads);
        return winnerConcurrentReturnCode;
    }
    return 0;
}

int Group::kill() {
    stop();
    return wait();
}

void Group::wrapperWaitting(unsigned int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    stop();
}

int Group::wait(unsigned int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::unique_lock<std::mutex> tasksLock(tasksMutex);
    if(concurrent && winnerConcurrentThreads != UINT_MAX) {
        tasksLock.unlock();
        return wait();
    }
    return -1;
}

int Group::waitAndKill(unsigned int seconds) {
    waitingThreads = new std::thread(&Group::wrapperWaitting, this, seconds);
    return wait();
}


void Group::wrapperFunction(unsigned int num) {
    // Create a wrapper unique lock for the mutex
    std::unique_lock<std::mutex> tasksLock(tasksMutex, std::defer_lock);
    // wait that the user calls start() para:
    startedBarrier->wait();
    int idTask = 0;
    // Take a task
    while(true) {
        tasksLock.lock();
        // if there are no more tasks
        if(!tasks.size())
            return;
        // Get a task
        std::function<int()> function = tasks.back();
        idTask                        = tasks.size() - 1;
        tasks.pop_back();
        nbLaunchedTasks++;
        tasksLock.unlock();
        // Launch a task
        printf("c [pFactory][Group N°%d] task %d launched on thread %d.\n", idGroup, idTask, num);
        int returnCode      = function();
        returnCodes[idTask] = returnCode;
        // We have kill all others threads !
        tasksLock.lock();
        if(concurrent && winnerConcurrentThreads == UINT_MAX) {
            winnerConcurrentThreads    = num;
            winnerConcurrentReturnCode = returnCode;
            winnerConcurrentTask       = idTask;
            stop();
            printf("c [pFactory][Group N°%d] concurent mode: thread %d has won with the task %d.\n", idGroup, num, idTask);
            return;
        }
        tasksLock.unlock();
    }
}
}   // namespace pFactory
