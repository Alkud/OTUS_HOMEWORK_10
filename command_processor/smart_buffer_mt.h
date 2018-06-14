// smart_buffer.h in Otus homework#11 project

#pragma once

#include <deque>
#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include <thread>
#include <atomic>

#include "async_worker.h"
#include "broadcasters.h"
#include "weak_ptr_less.h"

template<class T>
class SmartBuffer : public NotificationBroadcaster,
                    public MessageListener,
                    public MessageBroadcaster,
                    public AsyncWorker<1>
{
public:

  std::mutex dataLock;

  using ListenerSet = std::set<std::weak_ptr<NotificationListener>, WeakPtrLess<NotificationListener>>;

  SmartBuffer() = delete;

  SmartBuffer(const std::string& newWorkerName, std::ostream& newErrorOut = std::cerr) :
    AsyncWorker<1>{newWorkerName},
    errorOut{newErrorOut}
  {
    data.clear();
  }

  ~SmartBuffer()
  {
    stop();
  }

  /// Each element in the buffer has the list of recipients.
  /// When a recepient gets an element, it is added to this list.
  /// When all recipients have received this element, we can remove
  /// it from the buffer.
  struct Record
  {    
    Record(T newValue) :
      value{newValue} {}

    Record(T newValue, const ListenerSet& newRecipients) :
      value{newValue}, recipients{newRecipients} {}

    T value{};
    ListenerSet recipients{};
  };

  /// Copy new element to the buffer
  void putItem(const T& newItem)
  {
    data.emplace_back(newItem, notificationListeners);
    ++notificationCount;
    threadNotifier.notify_one();
  }

  /// Move new element to the buffer
  void putItem(T&& newItem)
  {
    data.emplace_back(std::move(newItem), notificationListeners);
    ++notificationCount;
    threadNotifier.notify_one();
  }  

  /// Each recipient starts looking from the first element in the queue.
  /// When an element is found that wasn't received yet by this recipient,
  /// the recipient gets the value of this element and updates pecipient list
  /// for this element.
  std::pair<bool, T> getItem(const std::shared_ptr<NotificationListener> recipient = nullptr)
  {
    std::weak_ptr<NotificationListener> weakRecipient{recipient};

    if (data.empty() == true)
    {
      throw std::out_of_range{"Buffer is empty!"};
    }

    if (nullptr == recipient)
    {
      auto result {std::make_pair(true, data.front().value)};

      if (data.front().recipients.empty() == true)
      {
        data.pop_front();

        if (true == data.empty() && true == noMoreData)
        {
          shouldExit = true;
          threadNotifier.notify_all();
        }
      }

      return result;
    }

    auto iter {data.begin()};
    while(iter != data.end()
          && (iter->recipients.find(weakRecipient) == iter->recipients.end()))
    {
      ++iter;
    }

    if (iter == data.end()
        && (notificationListeners.find(weakRecipient) != notificationListeners.end()))
    {
        return std::make_pair(false, data.front().value);
    }

    auto result {std::make_pair(true, iter->value)};

    iter->recipients.erase(weakRecipient);

    if (iter->recipients.empty() == true)
    {
      data.erase(iter);
      if (true == data.empty() && true == noMoreData)
      {
        //std::cout << "\n                    " << workerName<< " should exit\n";
        shouldExit = true;
        threadNotifier.notify_all();
      }
    }

    return result;
  }

  /// Get elements count in the queue
  size_t dataSize()
  {
    return data.size();
  }

  /// Clear data
  void clear()
  {
    data.clear();
  }

  void reactMessage(MessageBroadcaster* sender, Message message) override
  {
    switch(message)
    {
    case Message::NoMoreData :
      //std::cout << "\n                     " << workerName<< " NoMoreData received\n";
      noMoreData = true;
      threadNotifier.notify_one();
      break;

    case Message::Abort :
      shouldExit = true;
      noMoreData = true;
      threadNotifier.notify_all();
      sendMessage(Message::Abort);
      break;
    }
  }

  void sanitize()
  {
    std::lock_guard<std::mutex> lockData{dataLock};

    auto record {data.begin()};
    for (; record != data.end(); ++record)
    {
      if (record->recipients.empty() == true)
      {
        data.erase(record);
      }
    }
  }

private:

  bool run(const size_t) override
  {
    try
    {
      while(noMoreData != true)
      {
        if (true == shouldExit)
        {
          break;
        }

        if (notificationCount.load() > 0)
        {
          notify();
          --notificationCount;
        }
//        else if (noMoreData == true && data.size() == 0)
//        {
//          break;
//        }
        else if (noMoreData != true)
        {
          /* wait for new notifications or exit request */
          std::unique_lock<std::mutex> lockNotifier{notifierLock};

          threadNotifier.wait(lockNotifier,
                              [this](){return this->notificationCount.load() > 0
                              || shouldExit
                              || noMoreData;});

          lockNotifier.unlock();
        }
      }

      if (shouldExit != true)
      {
        while (notificationCount.load() > 0)
        {
          notify();
          --notificationCount;
        }
      }

      if (data.empty() != true)
      {
        while(shouldExit != true)
        {
          std::unique_lock<std::mutex> lockNotifier{notifierLock};
          threadNotifier.wait(lockNotifier, [this](){return shouldExit;});
          lockNotifier.unlock();
        }
      }

      sendMessage(Message::NoMoreData);      

      //std::cout << "\n                    buffer finished\n";

    }
    catch (const std::exception& ex)
    {
      sendMessage(Message::Abort);
      errorOut << workerName << " stopped. Reason: " << ex.what() << std::endl;
      return false;
    }
  }


  std::ostream& errorOut;

  std::deque<Record> data;
};

