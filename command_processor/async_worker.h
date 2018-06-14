// async_worker.h in Otus homework#11 project

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <future>
#include <cassert>

template<size_t workingThreadsCount = 1u>
class AsyncWorker
{
public:
  AsyncWorker() = delete;

  AsyncWorker(const std::string& newWorkerName) :
    shouldExit{false}, noMoreData{false}, isStopped{true}, notificationCount{},
    workerName{newWorkerName}
  {
    futureResults.reserve(workingThreadsCount);
  }

  virtual ~AsyncWorker()
  {
    //std::cout << "\n                    " << workerName << " destructor\n";
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

    shouldExit = true;
    threadNotifier.notify_all();

    for (auto& result : futureResults)
    {
      if (result.wait_for(std::chrono::milliseconds(0))
          != std::future_status::ready)
      {
        result.wait_for(std::chrono::milliseconds(150));
      }
    }
    isStopped = true;
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

  virtual bool run(const size_t threadIndex) = 0;

  std::vector<std::future<bool>> futureResults{};
  bool shouldExit;
  bool noMoreData;
  bool isStopped;

  std::atomic<size_t> notificationCount;
  std::condition_variable threadNotifier{};
  std::mutex notifierLock;

  const std::string workerName;
};
