#pragma once

#include "net/notification_queue.hpp"
#include "net/types.hpp"

namespace botf::net {

using InboundQueue = NotificationQueue<InboundNotification>;
using OutboundQueue = NotificationQueue<OutboundNotification>;
using ClientInboundQueue = NotificationQueue<ClientInboundNotification>;

}  // namespace botf::net
