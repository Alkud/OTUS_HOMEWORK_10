// publisher.h in Otus homework#11 project

#pragma once

#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <condition_variable>
#include "listeners.h"
#include "smart_buffer_mt.h"
#include "thread_metrics.h"


class Publisher : public NotificationListener,
                  public MessageListener,
                  public MessageBroadcaster,
                  public std::enable_shared_from_this<NotificationListener>,
                  public AsyncWorker<1>
{
public:

  Publisher(const std::string& newWorkerName,
            const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>>& newBuffer,
            bool& newTerminationFlag, bool& newAbortFlag,
            std::condition_variable& newTerminationNotifier,
            std::ostream& newOutput, std::mutex& newOutpuLock,
            std::ostream& newErrorOut = std::cerr);

  ~Publisher();

  void reactNotification(NotificationBroadcaster* sender) override;

  void reactMessage(MessageBroadcaster* sender, Message message) override;

  const SharedMetrics getMetrics();

private:

  bool threadProcess(const size_t threadIndex) override;

  void onThreadException(const std::exception& ex, const size_t threadIndex) override;

  void onTermination(const size_t threadIndex) override;

  using DataType = std::pair<size_t, std::string>;

  std::shared_ptr<SmartBuffer<DataType>> buffer;
  std::ostream& output;
  std::mutex& outputLock;  

  SharedMetrics threadMetrics;
  std::ostream& errorOut;

  bool& terminationFlag;
  bool& abortFlag;
  std::condition_variable& terminationNotifier;
};
