// smart_buffer.h in Otus homework#10 project

#pragma once
#include "broadcasters.h"
#include <deque>
#include <algorithm>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include "weak_ptr_less.h"
#include <iostream>
#include <thread>
#include <atomic>


template<class T>
class SmartBuffer : public NotificationBroadcaster,
                    public MessageListener,
                    public MessageBroadcaster
{
public:

  std::mutex dataLock;

  using ListenerSet = std::set<std::weak_ptr<NotificationListener>, WeakPtrLess<NotificationListener>>;

  SmartBuffer() noexcept(false) :
    shouldExit{false}, noMoreData{false}, notificationsCount{0}
  {
    notifyingThread = std::thread{&SmartBuffer::run, this};
  }

  ~SmartBuffer()
  {
    //std::cout << "Buffer destructor\n";
    if (notifyingThread.joinable())
    {
      notifyingThread.join();
    }
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

    T value;
    ListenerSet recipients;
  };

  /// Add new element to the buffer
  void putItem(const T& newItem)
  {
    {
      data.emplace_back(newItem, notificationListeners);
    }
    ++notificationsCount;
    threadNotifier.notify_one();
  }

  /// Add new element to the buffer
  void putItem(T&& newItem)
  {
    {
      data.emplace_back(std::move(newItem), notificationListeners);
    }
    ++notificationsCount;
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
      if (notificationListeners.empty() == true)
      {
        data.pop_front();
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
        std::unique_lock<std::mutex> lockNotifier{notifierLock};
        shouldExit = true;
        lockNotifier.unlock();

        threadNotifier.notify_one();
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
    if (Message::NoMoreData == message)
    {
      noMoreData = true;
      threadNotifier.notify_one();
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

  void run()
  {
    while(shouldExit != true)
    {
      std::unique_lock<std::mutex> lockNotifier{notifierLock};

      threadNotifier.wait(lockNotifier,
                          [this](){return this->notificationsCount.load() > 0
                          || shouldExit
                          || noMoreData;});


      lockNotifier.unlock();

      if (notificationsCount.load() > 0)
      {
        notify();
        --notificationsCount;
      }

      if (noMoreData == true && data.size() == 0)
      {
        shouldExit = true;
      }
    }
    sendMessage(Message::NoMoreData);
    sendMessage(Message::AllDataReceived);
  }


private:  
  std::deque<Record> data;

  std::thread notifyingThread;
  bool shouldExit;
  bool noMoreData;
  std::atomic<size_t> notificationsCount;
  std::condition_variable threadNotifier{};
  std::mutex notifierLock;
};

