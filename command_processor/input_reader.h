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
              const std::shared_ptr<SmartBuffer<std::string>>& newBuffer);

  ~InputReader();

  /// Read from input stream until eof
  void read();

  void reactMessage(class MessageBroadcaster* sender, Message message) override;

private:

  std::istream& input;
  std::mutex& inputLock;
  std::shared_ptr<SmartBuffer<std::string>> buffer;

  bool allDataReceived;
};
