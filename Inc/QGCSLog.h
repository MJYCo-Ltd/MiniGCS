#ifndef QGCSLOG_H
#define QGCSLOG_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h> // support for user defined types

#ifndef MAV_FMT_STR
#define MAV_FMT_STR "[mavsdk] {}:{}"
#endif

#ifndef PLAT_FMT_STR
#define PLAT_FMT_STR "[mavsdk] system_id={} {}:{}"
#endif

#ifndef SYS_FMT_STR
#define SYS_FMT_STR "[MiniGCS] {}:{}"
#endif

#endif // QGCSLOG_H
