// command_processor.h in Otus homework#10 project

#pragma once

#include <memory>
#include <mutex>
#include "input_reader.h"
#include "input_processor.h"
#include "smart_buffer_mt.h"
#include "publisher_mt.h"
#include "logger_mt.h"

template <size_t loggingThreadsCount = 2u>
class CommandProcessor : public MessageBroadcaster
{
public:

  CommandProcessor
  (
    std::istream& inputStream,
    std::ostream& outputStream,
    std::ostream& errorStream,
    std::ostream& metricsStream,
    const size_t& bulkSize,
    const char& bulkOpenDelimiter,
    const char& bulkCloseDelimiter
   ) :
    /* creating buffers */
    inputBuffer{std::make_shared<SmartBuffer<std::string>>("command buffer")},
    outputBuffer{std::make_shared<SmartBuffer<std::pair<size_t, std::string>>>("bulk buffer")},
    /* creating command reader */
    inputReader{std::make_shared<InputReader>(inputStream, inputStreamLock, inputBuffer)},
    /* creating logger */
    logger{std::make_shared<Logger<loggingThreadsCount>>(
           "logger", outputBuffer, dataLogged, shouldExit, terminationNotifier, "", errorStream
    )},
    /* creating publisher */
    publisher{std::make_shared<Publisher>(
              "publisher", outputBuffer, dataPublished, shouldExit, terminationNotifier, outputStream, outputStreamLock, errorStream)},
    /* creating command processor */
    inputProcessor{
      std::make_shared<InputProcessor>(
        bulkSize,
        bulkOpenDelimiter, bulkCloseDelimiter,
        inputBuffer, outputBuffer,
        errorStream
        )
      },

    dataPublished{false}, dataLogged{false}, shouldExit{false},
    metricsOut{metricsStream}, errorOut{errorStream}, globalMetrics{}
  {
    /* connect broadcasters and listeners */
    this->addMessageListener(inputReader);

    inputReader->addMessageListener(inputBuffer);

    inputBuffer->addMessageListener(inputProcessor);
    inputBuffer->addNotificationListener(inputProcessor);

    inputProcessor->addMessageListener(outputBuffer);

    outputBuffer->addNotificationListener(publisher);
    outputBuffer->addMessageListener(publisher);                
    outputBuffer->addNotificationListener(logger);
    outputBuffer->addMessageListener(logger);

    logger->addMessageListener(publisher);
    publisher->addMessageListener(logger);

    /* creating metrics*/
    globalMetrics["input processor"] = inputProcessor->getMetrics();
    globalMetrics["publisher"] = publisher->getMetrics();

    SharedMultyMetrics loggerMetrics{logger->getMetrics()};
    for (size_t idx{0}; idx < loggingThreadsCount; ++idx)
    {
      auto threadName = std::string{"logger thread#"} + std::to_string(idx);
      globalMetrics[threadName] = loggerMetrics[idx];
    }
  }

  /// Runs input reading and processing
  void run()
  {
    inputBuffer->start();
    outputBuffer->start();
    publisher->start();
    logger->start();

    inputReader->read();

    #ifdef _DEBUG
      std::cout << "\n                     CP is going to wait. dataLogged = "
                << dataLogged << " dataPublished = " << dataPublished << "\n";
    #endif

    while (shouldExit != true
           && ((dataLogged && dataPublished) != true))
    {
      #ifdef _DEBUG
        std::cout << "\n                     CP waiting\n";
      #endif

      std::unique_lock<std::mutex> lockNotifier{notifierLock};
      terminationNotifier.wait_for(lockNotifier, std::chrono::seconds{1}, [this]()
      {
        return (shouldExit) || (dataLogged && dataPublished);
      });
      lockNotifier.unlock();
    }

    #ifdef _DEBUG
      std::cout << "\n                     CP waiting ended\n";
    #endif

    if (shouldExit == true)
    {
      sendMessage(Message::Abort);
      errorOut << "Abnormal termination\n";
    }

    /* waiting for all workers to finish */
    while(inputReader->getWorkerState() != WorkerState::Finished
          && inputProcessor->getWorkerState() != WorkerState::Finished
          && inputBuffer->getWorkerState() != WorkerState::Finished
          && outputBuffer->getWorkerState() != WorkerState::Finished
          && logger->getWorkerState() != WorkerState::Finished
          && publisher->getWorkerState() != WorkerState::Finished)
    {}

    #ifdef _DEBUG
      std::cout << "\n                     CP metrics output\n";
    #endif

    /* Output metrics */
    metricsOut << "main thread - "
               << globalMetrics["input processor"]->totalStringCount << " string(s), "
               << globalMetrics["input processor"]->totalCommandCount << " command(s), "
               << globalMetrics["input processor"]->totalBulkCount << " bulk(s)" << std::endl
               << "log thread - "
               << globalMetrics["publisher"]->totalBulkCount << " bulk(s), "
               << globalMetrics["publisher"]->totalCommandCount << " command(s)" << std::endl;

    for (size_t threadIndex{}; threadIndex < loggingThreadsCount; ++threadIndex)
    {
      auto threadName = std::string{"logger thread#"} + std::to_string(threadIndex);
      metricsOut << "file thread #" << threadIndex << " - "
                 << globalMetrics[threadName]->totalBulkCount << " bulk(s), "
                 << globalMetrics[threadName]->totalCommandCount << " command(s)" << std::endl;
    }
  }

  const std::shared_ptr<SmartBuffer<std::string>>&
  getCommandBuffer()
  { return inputBuffer; }

  const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>>&
  getBulkBuffer()
  { return outputBuffer; }

private:
  std::shared_ptr<SmartBuffer<std::string>> inputBuffer;
  std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>> outputBuffer;
  std::shared_ptr<InputReader> inputReader;
  std::shared_ptr<Logger<loggingThreadsCount>> logger;
  std::shared_ptr<Publisher> publisher;
  std::shared_ptr<InputProcessor> inputProcessor;

  std::mutex inputStreamLock{};
  std::mutex outputStreamLock{};
  std::mutex errorStreamLock{};

  std::atomic<bool> dataPublished;
  std::atomic<bool> dataLogged;
  std::atomic<bool> shouldExit;

  std::condition_variable terminationNotifier{};
  std::mutex notifierLock;

  std::mutex outputStreamLock;
  std::mutex errorStreamLock;

  std::ostream& errorOut;
  std::ostream& metricsOut;
  SharedGlobalMetrics globalMetrics;
};

