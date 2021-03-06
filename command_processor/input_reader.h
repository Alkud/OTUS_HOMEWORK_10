// input_reader.h in Otus homework#7 project

#pragma once

#include "broadcasters.h"
#include "smart_buffer_mt.h"
#include <iostream>
#include <memory>

enum class InputReaderSettings
{
  MaxInputStringSize = 80
};

class InputReader : public MessageBroadcaster,
                    public MessageListener
{
public:

  InputReader(std::istream& newInput, std::mutex& newInputLock,
              const SharedStringBuffer& newBuffer,
              std::ostream& newErrorOut, std::mutex& newErrorOutLock);

  ~InputReader();

  /// Read from input stream until eof
  void read();

  void reactMessage(class MessageBroadcaster* sender, Message message) override;

  WorkerState getWorkerState();

private:

  std::istream& input;
  std::mutex& inputLock;
  SharedStringBuffer buffer;

  bool shouldExit;

  std::ostream& errorOut;
  std::mutex& errorOutLock;

  std::atomic<WorkerState> state;
};
