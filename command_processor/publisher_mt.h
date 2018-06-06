// publisher.h in Otus homework#7 project

#pragma once

#include "listeners.h"
#include "smart_buffer_mt.h"
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <condition_variable>

class Publisher : public NotificationListener,
                  public MessageListener,
                  public MessageBroadcaster,
                  public std::enable_shared_from_this<NotificationListener>
{
public:

  Publisher(const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>>& newBuffer,
            std::ostream& newOutput, std::mutex& newOutpuLock,
            std::ostream& newErrorOut = std::cerr, std::ostream& newMetricksOut = std::cout);

  ~Publisher();

  void start();

  void reactNotification(NotificationBroadcaster* sender) override;

  void reactMessage(MessageBroadcaster* sender, Message message) override;

private:

  struct MetricsRecord
  {
    size_t totalBulksCount{};
    size_t totalCommandsCount{};
  };

  void run();

  bool publish();

  using DataType = std::pair<size_t, std::string>;

  std::shared_ptr<SmartBuffer<DataType>> buffer;
  std::ostream& output;
  std::mutex& outputLock;  

  bool shouldExit;
  std::atomic<size_t> notificationsCount;
  std::condition_variable threadNotifier{};
  std::mutex notifierLock;

  std::thread workingThread;
  MetricsRecord threadMetrics;
  std::ostream& errorOut;
  std::ostream& metricsOut;
};
