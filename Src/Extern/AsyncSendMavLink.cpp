#include <thread>
#include "AsyncSendMavLink.h"

void CAsyncSendMavLink::send_command_long_async(
    std::shared_ptr<mavsdk::MavlinkPassthrough> passthrough,
    const mavsdk::MavlinkPassthrough::CommandLong &command,
    std::function<void(mavsdk::MavlinkPassthrough::Result)> callback) {
    std::thread([passthrough, command, callback]() {
        auto res = passthrough->send_command_long(command);
        callback(res);
    }).detach(); // 分离线程，不阻塞主线程
}
