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
class CommandProcessor
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
    inputBuffer{std::make_shared<SmartBuffer<std::string>>()},
    outputBuffer{std::make_shared<SmartBuffer<std::pair<size_t, std::string>>>()},
    /* creating command reader */
    inputReader{std::make_shared<InputReader>(inputStream, inputStreamLock, inputBuffer)},
    /* creating command processor */
    inputProcessor{
      std::make_shared<InputProcessor>
      (
        bulkSize,
        bulkOpenDelimiter, bulkCloseDelimiter,
        inputBuffer, outputBuffer
      )
    },
    /* creating logger */
    logger{std::make_shared<Logger<loggingThreadsCount>>(outputBuffer, "", errorStream, metricsStream)},
    /* creating publisher */
    publisher{std::make_shared<Publisher>(outputBuffer, outputStream, outputStreamLock, errorStream, metricsStream)}

  {
    /* connect broadcasters and listeners */
    inputReader->addMessageListener(inputProcessor);
    inputBuffer->addNotificationListener(inputProcessor);
    outputBuffer->addNotificationListener(logger);
    outputBuffer->addNotificationListener(publisher);
  }

  /// Runs input reader
  void run() const
  {
    if (inputReader != nullptr)
    {
      inputReader->read();
    }
  }

  const std::shared_ptr<SmartBuffer<std::string>>&
  getCommandBuffer() const
  { return inputBuffer; }

  const std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>>&
  getBulkBuffer() const
  { return outputBuffer; }

private:
  std::shared_ptr<SmartBuffer<std::string>> inputBuffer;
  std::shared_ptr<SmartBuffer<std::pair<size_t, std::string>>> outputBuffer;
  std::shared_ptr<InputReader> inputReader;
  std::shared_ptr<InputProcessor> inputProcessor;
  std::shared_ptr<Logger<loggingThreadsCount>> logger;
  std::shared_ptr<Publisher> publisher;

  std::mutex inputStreamLock{};
  std::mutex outputStreamLock{};
};

