// publisher.h in Otus homework#7 project

#pragma once

#include "listeners.h"
#include "smart_buffer_mt.h"
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>

class Publisher : public NotificationListener,
                  public std::enable_shared_from_this<Publisher>
{
public:

  Publisher(const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>>& newBuffer,
            std::ostream& newOutput, std::mutex& newOutpuLock,
            std::ostream& newErrorOut = std::cerr, std::ostream& newMetricksOut = std::cout);

  ~Publisher();

  void reactNotification(NotificationBroadcaster* sender) override;

private:

  struct MetricsRecord
  {
    size_t totalBulksCount{};
    size_t totalCommandsCount{};
  };

  void run();

  void publish();

  std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>> buffer;
  std::ostream& output;
  std::mutex& outputLock;  

  bool shouldExit;
  std::atomic<size_t> notificationsCount;

  std::thread workingThread;
  MetricsRecord threadMetrics;
  std::ostream& errorOut;
  std::ostream& metricsOut;
};
