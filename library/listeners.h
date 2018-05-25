// listeners.h in Otus homework#7 project

#pragma once

#include <string>

/// Base class for message broadcast subscribers
class MessageListener
{
public:

  virtual ~MessageListener(){}

  /// Describes object's reaction to a message
  virtual void reactMessage(class MessageBroadcaster* sender, std::string message) = 0;
};

class NotificationListener
{
public:

  virtual ~NotificationListener(){}

  /// Describes object's reaction to a notification
  virtual void reactNotification(class NotificationBroadcaster* sender) = 0;
};
