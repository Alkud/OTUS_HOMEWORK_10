// broadcasters.h in Otus homework#10 project

#pragma once

#include <set>
#include <string>
#include <memory>
#include "listeners.h"
#include "weak_ptr_less.h"

/// inteprocess exchange messages
enum class Message : unsigned int
{
  /// Data flow messages:

  /// no more data will be committed
  NoMoreData = 1u,
  /// all characters have been received
  AllDataReceived = 2u,
  /// all bulks  have been published
  AllDataPublsihed = 3u,
  /// all bulks have been written to files
  AllDataLogged = 4u,


  /// Error code messages:

  /// any unknown exception
  SystemError = 1001u,
  /// log file exception
  FileCreationError = 1002u,
  /// input reader getline() exception
  CharacterReadingError = 1003u,
  /// source buffer pointer not defined or expired
  SourceNullptr = 1004u,
  /// destination buffer pointer not defined or expired
  DestinationNullptr = 1005u,
  /// source buffer empty while trying to read on notification
  BufferEmpty = 1006u
};

template<typename T = unsigned int>
typename std::enable_if_t<std::is_integral<T>::value, T>
messageCode(const Message& message)
{
  return static_cast<T> (message);
}

/// Base class for a brodcaster, sending messages,
/// containing instructions for listeners
class MessageBroadcaster
{
public:  

  using SharedMessageListener = std::shared_ptr<MessageListener>;
  using WeakMessageListener = std::weak_ptr<MessageListener>;

  virtual ~MessageBroadcaster()
  {
    clearMessageListenerList();
  }

  /// Add a new listeners
  virtual void
  addMessageListener(const SharedMessageListener& newListener)
  {
    if (newListener != nullptr)
    {
      messageListeners.insert(WeakMessageListener{newListener});
    }
  }

  /// Remove a listener
  virtual void
  removeMessageListener(const SharedMessageListener& listener)
  {
    if (listener != nullptr)
    {
      messageListeners.erase(WeakMessageListener{listener});
    }
  }

  /// Send instruction to all registered listeners
  virtual void sendMessage(Message message)
  {
    for (const auto& listener : messageListeners)
    {
      if (listener.expired() != true)
      {
        listener.lock()->reactMessage(this, message);
      }
      else
      {
        messageListeners.erase(listener);
      }
    }
  }

  /// Remove all listenenrs from the list
  virtual void clearMessageListenerList()
  {
    messageListeners.clear();
  }

protected:
  std::set<WeakMessageListener, WeakPtrLess<MessageListener>> messageListeners;
};



/// Base class for a brodcaster, acting as a trigger,
/// forcing listeners to take an action
class NotificationBroadcaster
{
public:

  using SharedNotificationListener = std::shared_ptr<NotificationListener>;
  using WeakNotificationListener = std::weak_ptr<NotificationListener>;

  virtual ~NotificationBroadcaster()
  {
    clearNotificationListenerList();
  }

  /// Add a listener to the list
  virtual void
  addNotificationListener(const SharedNotificationListener& newListener)
  {
    if (newListener != nullptr)
    {
      notificationListeners.insert(WeakNotificationListener{newListener});
    }
  }

  /// Remove a listener from the list
  virtual void
  removeNotificationListener(const SharedNotificationListener& listener)
  {
    if (listener != nullptr)
    {
      notificationListeners.erase(WeakNotificationListener{listener});
    }
  }

  /// Send notification to all listeners in the list
  virtual void notify()
  {
    for (const auto& listener : notificationListeners)
    {
      if (listener.expired() != true)
      {
        listener.lock()->reactNotification(this);
      }
      else
      {
        notificationListeners.erase(listener);
      }
    }
  }

  /// Remove alll listenenrs from the list
  virtual void clearNotificationListenerList()
  {
    notificationListeners.clear();
  }

protected:
  std::set<WeakNotificationListener, WeakPtrLess<NotificationListener>> notificationListeners;
};
