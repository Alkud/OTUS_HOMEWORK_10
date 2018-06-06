// broadcasters.h in Otus homework#10 project

#pragma once

#include <set>
#include <string>
#include <memory>
#include "listeners.h"
#include "weak_ptr_less.h"

enum class Message
{
  NoMoreData,
  AllDataReceived
};
/// Base class for a brodcaster, sending messages,
/// containing instructions for listeners
class MessageBroadcaster
{
public:  
  virtual ~MessageBroadcaster()
  {
    clearMessageListenersList();
  }

  /// Add a new listeners
  virtual void
  addMessageListener(const std::shared_ptr<MessageListener>& newListener)
  {
    if (newListener != nullptr)
    {
      messageListeners.insert(std::weak_ptr<MessageListener>{newListener});
    }
  }

  /// Remove a listener
  virtual void
  removeMessageListener(const std::shared_ptr<MessageListener>& listener)
  {
    if (listener != nullptr)
    {
      messageListeners.erase(std::weak_ptr<MessageListener>{listener});
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
  virtual void clearMessageListenersList()
  {
    messageListeners.clear();
  }

protected:
  std::set<std::weak_ptr<MessageListener>, WeakPtrLess<MessageListener>> messageListeners;
};



/// Base class for a brodcaster, acting as a trigger,
/// forcing listeners to take an action
class NotificationBroadcaster
{
public:
  virtual ~NotificationBroadcaster()
  {
    clearNotificationListenersList();
  }

  /// Add a listener to the list
  virtual void
  addNotificationListener(const std::shared_ptr<NotificationListener>& newListener)
  {
    if (newListener != nullptr)
    {
      notificationListeners.insert(std::weak_ptr<NotificationListener>{newListener});
    }
  }

  /// Remove a listener from the list
  virtual void
  removeNotificationListener(const std::shared_ptr<NotificationListener>& listener)
  {
    if (listener != nullptr)
    {
      notificationListeners.erase(std::weak_ptr<NotificationListener>{listener});
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
  virtual void clearNotificationListenersList()
  {
    notificationListeners.clear();
  }

protected:
  std::set<std::weak_ptr<NotificationListener>, WeakPtrLess<NotificationListener>> notificationListeners;
};
