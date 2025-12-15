#ifndef ASYNCSENDMAVLINK_H
#define ASYNCSENDMAVLINK_H
#include <mavsdk/plugins/mavlink_passthrough/mavlink_passthrough.h>

namespace CAsyncSendMavLink {

void send_command_long_async(
    mavsdk::MavlinkPassthrough* passthrough,
    const mavsdk::MavlinkPassthrough::CommandLong &command,
    std::function<void(mavsdk::MavlinkPassthrough::Result)> callback);
};

#endif // ASYNCSENDMAVLINK_H
