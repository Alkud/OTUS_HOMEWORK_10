// async_worker.h in Otus homework#11 project

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <future>
#include <cassert>
#include "worker_state.h"

template<size_t workingThreadsCount = 1u>
class AsyncWorker
{
public:
  AsyncWorker() = delete;

  AsyncWorker(const std::string& newWorkerName) :
    shouldExit{false}, noMoreData{false}, isStopped{true}, notificationCount{},
    threadFinished{}, workerName{newWorkerName}, state{WorkerState::NotStarted}
  {
    futureResults.reserve(workingThreadsCount);
    threadFinished.resize(workingThreadsCount, false);
  }

  virtual ~AsyncWorker()
  {
    #ifdef _DEBUG
      std::cout << "\n                    " << workerName << " destructor, shouldExit = " << shouldExit << "\n";
    #endif

    assert(isStopped == true);
  }

  virtual void start()
  {
    startWorkingThreads();
  }

  virtual bool startAndWait()
  {
    startWorkingThreads();
    if (futureResults.empty() != true)
    {
      return waitForThreadTermination();
    }
    else
    {
      return false;
    }
  }

  void stop()
  {
    if (true == isStopped)
    {
      return;
    }

    #ifdef _DEBUG
      std::cout << "\n                    " << workerName << " trying to stop\n";
    #endif

    shouldExit = true;
    threadNotifier.notify_all();

    for (auto& result : futureResults)
    {
      while (result.wait_for(std::chrono::milliseconds(0))
          != std::future_status::ready)
      {
        shouldExit = true;
        threadNotifier.notify_all();
        result.wait_for(std::chrono::milliseconds(150));
      }
    }

    if (state != WorkerState::Finished)
    {
      state = WorkerState::Finished;
    }

    isStopped = true;
  }

  WorkerState getWorkerState()
  {
    return state;
  }

protected:

  void startWorkingThreads()
  {
    if (futureResults.empty() != true)
    {
      return;
    }

    /* start working threads */
    for (size_t threadIndex{0}; threadIndex < workingThreadsCount; ++threadIndex)
    {
      futureResults.push_back(
        std::async(
          std::launch::async,
          &AsyncWorker<workingThreadsCount>::run,
          this, threadIndex
        )
      );
    }
    isStopped = false;
    state = WorkerState::Started;
  }

  bool waitForThreadTermination()
  {
    /* wait for working threads results */
    bool workSuccess{true};
    for (auto& result : futureResults)
    {
      workSuccess = workSuccess && result.get();
    }

    return workSuccess;
  }

  virtual bool run(const size_t threadIndex)
  {
    try
    {
      while(this->shouldExit != true
            && (this->noMoreData != true || this->notificationCount > 0))
      {
        std::unique_lock<std::mutex> lockNotifier{this->notifierLock};

        if (notificationCount.load() > 0)
        {
          #ifdef _DEBUG
            std::cout << this->workerName << " decrement notificationCount\n";
          #endif

          --notificationCount;
          lockNotifier.unlock();
          threadProcess(threadIndex);

          #ifdef _DEBUG
            std::cout << this->workerName << " threadProcess success\n";
          #endif
        }
        else
        {
          std::unique_lock<std::mutex> lockControl{controlLock};
          if (shouldExit != true || noMoreData != true)
          {
            lockControl.unlock();

            #ifdef _DEBUG
              std::cout << "\n                     " << this->workerName<< " waiting. shouldExit="<< shouldExit << ", noMoreData=" << noMoreData << "\n";
            #endif

            threadNotifier.wait_for(lockNotifier, std::chrono::seconds(1), [this]()
            {
              return this->noMoreData || this->notificationCount.load() > 0 || this->shouldExit;
            });
          }
          else
          {
            lockControl.unlock();
          }
          lockNotifier.unlock();
        }
      }

      /*check if this thread is the only active one */
      std::unique_lock<std::mutex> lockTermination{terminationLock};

      size_t activeThreadCount{};
      for (size_t idx{0}; idx < workingThreadsCount; ++idx)
      {
        if (idx != threadIndex
            && threadFinished[idx] != true)
        {
          ++activeThreadCount;
        }
      }

      #ifdef _DEBUG
        std::cout << "\n                     " << this->workerName<< " activeThreadCount=" << activeThreadCount << "\n";
      #endif

      threadFinished[threadIndex] = true;

      lockTermination.unlock();

      if (0 == activeThreadCount)
      {
        #ifdef _DEBUG
          std::cout << "\n                     " << this->workerName<< " finishing\n";
        #endif

        if (shouldExit != true)
        {
          onTermination(threadIndex);
        }
        state = WorkerState::Finished;
      }

      #ifdef _DEBUG
        std::cout << "\n                     " << this->workerName<< " finished\n";
      #endif

      return true;
    }
    catch (const std::exception& ex)
    {
      onThreadException(ex, threadIndex);
      state = WorkerState::Finished;
      return false;
    }
  }

  virtual bool threadProcess(const size_t threadIndex) = 0;

  virtual void onThreadException(const std::exception& ex, const size_t threadIndex) = 0;

  virtual void onTermination(const size_t threadIndex) = 0;

  std::vector<std::future<bool>> futureResults{};
  std::mutex controlLock;
  bool shouldExit;
  bool noMoreData;
  bool isStopped;

  std::atomic<size_t> notificationCount;
  std::condition_variable threadNotifier{};
  std::mutex notifierLock;

  std::vector<bool> threadFinished;
  std::mutex terminationLock;

  const std::string workerName;

  WorkerState state;
};
