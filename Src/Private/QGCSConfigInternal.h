#ifndef QGCSCONFIGINTERNAL_H
#define QGCSCONFIGINTERNAL_H

#include <QString>
#include <cstdint>

/**
 * @brief 核心适配层专用配置入口（不进公开 Inc API）
 */
namespace QGCSConfigInternal {

void handleFirmwareLog(uint32_t vehicleId, int severity, const QString &text);
QString messageExtensionFile();
int commandAckTimeoutMs();

double telemetryPositionHz();
double telemetryPositionVelocityNedHz();
double telemetryGpsInfoHz();
double telemetryBatteryHz();
double telemetryRawGpsHz();
double telemetryAttitudeHz();
double telemetryLandedStateHz();
double telemetryHealthHz();
double telemetryHomeHz();
double telemetryFixedwingMetricsHz();

} // namespace QGCSConfigInternal

#endif // QGCSCONFIGINTERNAL_H
