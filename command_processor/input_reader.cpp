// input_reader.cpp in Otus homework#7 project

#include "input_reader.h"
#include <string>
#include <stdexcept>
#include <mutex>

InputReader::InputReader(std::istream& newInput, std::mutex& newInputLock,
                         const std::shared_ptr<SmartBuffer<std::string> >& newBuffer) :
  input{newInput},
  inputLock{newInputLock},
  buffer{newBuffer},
  allDataReceived{false}
{
  if (nullptr == buffer)
  {
    throw(std::invalid_argument{"Input reader destination buffer not defined!"});
  }
}

InputReader::~InputReader()
{

}



void InputReader::read()
{
  std::string nextString{};
  try
  {
    std::lock_guard<std::mutex> lockInput{inputLock};
    while(std::getline(input, nextString))
    {
      if (nextString.size() > (size_t)InputReaderSettings::MaxInputStringSize)
      {
        std::cerr << "Maximum command length exceeded! String truncated";
        nextString = nextString.substr((size_t)InputReaderSettings::MaxInputStringSize);
      }
      std::unique_lock<std::mutex> lockBuffer{buffer->dataLock};
      buffer->putItem(nextString);
      lockBuffer.unlock();
    }
  }
  catch(std::exception& ex)
  {
    sendMessage(Message::NoMoreData);
    throw;
  }
  sendMessage(Message::NoMoreData);

  while(allDataReceived != true){
    std::this_thread::sleep_for(std::chrono::milliseconds{20});
  }
}

void InputReader::reactMessage(MessageBroadcaster* sender, Message message)
{
  if (Message::AllDataReceived == message)
  {
    allDataReceived = true;
  }
}
