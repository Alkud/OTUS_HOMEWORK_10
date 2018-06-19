// command_processor.h in Otus homework#10 project

#pragma once

#include <memory>
#include <mutex>
#include "input_reader.h"
#include "input_processor.h"
#include "smart_buffer_mt.h"
#include "publisher_mt.h"
#include "logger_mt.h"

template <size_t loggingThreadCount = 2u>
class CommandProcessor : public MessageBroadcaster,
                         public MessageListener
{
public:

  CommandProcessor
  (
    std::istream& newInputStream,
    std::ostream& newOutputStream,
    std::ostream& newErrorStream,
    std::ostream& newMetricsStream,
    const size_t& bulkSize,
    const char& bulkOpenDelimiter,
    const char& bulkCloseDelimiter
   ) :
    errorStream{newErrorStream}, metricsStream{newMetricsStream},

    /* creating buffers */
    inputBuffer{std::make_shared<StringBuffer>(
      "command buffer",errorStream, errorStreamLock
    )},

    outputBuffer{std::make_shared<SizeStringBuffer>(
      "bulk buffer", errorStream, errorStreamLock
    )},

    /* creating command reader */
    inputReader{std::make_shared<InputReader>(
      newInputStream, inputStreamLock, inputBuffer,
      errorStream, errorStreamLock
    )},

    /* creating logger */
    logger{std::make_shared<Logger<loggingThreadCount>>(
      "logger", outputBuffer, errorStream, errorStreamLock, ""
    )},

    /* creating publisher */
    publisher{std::make_shared<Publisher>(
      "publisher", outputBuffer,
      newOutputStream, outputStreamLock,
      errorStream, errorStreamLock
    )},

    /* creating command processor */
    inputProcessor{
      std::make_shared<InputProcessor>(
        bulkSize,
        bulkOpenDelimiter, bulkCloseDelimiter,
        inputBuffer, outputBuffer,
        errorStream, errorStreamLock
    )},

    dataReceived{false}, dataPublished{false}, dataLogged{false}, shouldExit{false},
    globalMetrics{}
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

    /* creating metrics*/
    globalMetrics["input processor"] = inputProcessor->getMetrics();
    globalMetrics["publisher"] = publisher->getMetrics();

    SharedMultyMetrics loggerMetrics{logger->getMetrics()};
    for (size_t idx{0}; idx < loggingThreadCount; ++idx)
    {
      auto threadName = std::string{"logger thread#"} + std::to_string(idx);
      globalMetrics[threadName] = loggerMetrics[idx];
    }
  }

  void reactMessage(class MessageBroadcaster* sender, Message message) override
  {
    if (messageCode(message) < 1000) // non error message
    {
      switch(message)
      {
      case Message::AllDataReceived :
        #ifdef _DEBUG
          std::cout << "\n                     CP all data received\n";
        #endif

        dataReceived.store(true);
        terminationNotifier.notify_all();
        break;

      case Message::AllDataLogged :
        #ifdef _DEBUG
          std::cout << "\n                     CP all data logged\n";
        #endif

        dataLogged.store(true);
        terminationNotifier.notify_all();
        break;

      case Message::AllDataPublsihed :
        #ifdef _DEBUG
          std::cout << "\n                     CP all data published\n";
        #endif

        dataPublished.store(true);
        terminationNotifier.notify_all();
        break;

      default:
        break;
      }
    }
    else                             // error message
    {
      if (shouldExit.load() != true)
      {
        shouldExit.store(true);
        sendMessage(message);
      }
    }
  }

  /// Runs input reading and processing
  void run()
  {
    sharedThis = std::shared_ptr<CommandProcessor<loggingThreadCount>>{this, DummyDeleter()};

    inputReader->addMessageListener(sharedThis);
    logger->addMessageListener(sharedThis);
    publisher->addMessageListener(sharedThis);

    inputBuffer->start();
    outputBuffer->start();
    publisher->start();
    logger->start();

    inputReader->read();

    #ifdef NDEBUG
    #else
      std::cout << "\n                     CP is going to wait. dataLogged = "
                << dataLogged << " dataPublished = " << dataPublished << "\n";
    #endif

    while (shouldExit.load() != true
           && ((dataLogged.load() && dataPublished.load()) != true))
    {
      #ifdef NDEBUG
      #else
        std::cout << "\n                     CP waiting\n";
      #endif

      std::unique_lock<std::mutex> lockNotifier{notifierLock};
      terminationNotifier.wait_for(lockNotifier, std::chrono::seconds{1}, [this]()
      {
        return (shouldExit.load()) || (dataLogged.load() && dataPublished.load());
      });
      lockNotifier.unlock();
    }

    #ifdef _DEBUG
      std::cout << "\n                     CP waiting ended\n";
    #endif

    /* waiting for all workers to finish */
    while(inputReader->getWorkerState() != WorkerState::Finished
          && inputProcessor->getWorkerState() != WorkerState::Finished
          && inputBuffer->getWorkerState() != WorkerState::Finished
          && outputBuffer->getWorkerState() != WorkerState::Finished
          && logger->getWorkerState() != WorkerState::Finished
          && publisher->getWorkerState() != WorkerState::Finished)
    {}

    if (shouldExit.load() == true)
    {
      std::lock_guard<std::mutex> lockErrorStream{errorStreamLock};
      errorStream << "Abnormal termination\n";
    }

    #ifdef _DEBUG
      std::cout << "\n                     CP metrics output\n";
    #endif

    /* Output metrics */    
    std::lock_guard<std::mutex> lockOutputStream{outputStreamLock};
    metricsStream << "main thread - "
                  << globalMetrics["input processor"]->totalStringCount << " string(s), "
                  << globalMetrics["input processor"]->totalCommandCount << " command(s), "
                  << globalMetrics["input processor"]->totalBulkCount << " bulk(s)" << std::endl
                  << "log thread - "
                  << globalMetrics["publisher"]->totalBulkCount << " bulk(s), "
                  << globalMetrics["publisher"]->totalCommandCount << " command(s)" << std::endl;

    for (size_t threadIndex{}; threadIndex < loggingThreadCount; ++threadIndex)
    {
      auto threadName = std::string{"logger thread#"} + std::to_string(threadIndex);
      metricsStream << "file thread #" << threadIndex << " - "
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
  std::ostream& errorStream;
  std::ostream& metricsStream;

  std::mutex inputStreamLock{};
  std::mutex outputStreamLock{};
  std::mutex errorStreamLock{};

  std::shared_ptr<SmartBuffer<std::string>> inputBuffer;
  std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>> outputBuffer;
  std::shared_ptr<InputReader> inputReader;
  std::shared_ptr<Logger<loggingThreadCount>> logger;
  std::shared_ptr<Publisher> publisher;
  std::shared_ptr<InputProcessor> inputProcessor;

  std::atomic_bool dataReceived;
  std::atomic_bool dataPublished;
  std::atomic_bool dataLogged;
  std::atomic_bool shouldExit;

  std::condition_variable terminationNotifier{};
  std::mutex notifierLock;

  SharedGlobalMetrics globalMetrics;

  struct DummyDeleter{
    template<typename T>
    void operator()(T*){}
  };

  std::shared_ptr<CommandProcessor<loggingThreadCount>> sharedThis{};
};

