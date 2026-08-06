// SPDX-License-Identifier: MIT
#pragma once

#include <QObject>
#include "../FirmwareAdapter.h"
#include "../MavlinkHelper.h"
#include <memory>

class ArduPilotAdapterQt : public QObject, public ArduPilotAdapter {
    Q_OBJECT
public:
    explicit ArduPilotAdapterQt(mavsdk::System& system, QObject* parent = nullptr);
    ~ArduPilotAdapterQt() override;

    Q_INVOKABLE bool arm(uint8_t target_sys, uint8_t target_comp);
    Q_INVOKABLE bool disarm(uint8_t target_sys, uint8_t target_comp);
    Q_INVOKABLE bool set_flight_mode(const QString& mode, uint8_t target_sys, uint8_t target_comp);
    Q_INVOKABLE bool upload_mission(const QVariantList& mission_items, uint8_t target_sys, uint8_t target_comp);

signals:
    void armedChanged(bool armed);
    void missionUploadCompleted(bool success);

private:
    std::unique_ptr<MavlinkHelper> _helper;
};
