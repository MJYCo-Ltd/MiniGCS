Firmware adapter layer (MiniGCS)
================================

Purpose
-------
This folder provides a minimal FirmwareAdapter interface and an ArduPilotAdapter
implementation. The adapter is intended to be used when you need firmware-specific
behavior and/or must bypass MAVSDK high-level APIs in favor of sending/receiving
raw MAVLink messages through MAVSDK's MavlinkPassthrough plugin.

Design notes
------------
- Use MAVSDK's recommended MavlinkPassthrough plugin to send/subscribe raw MAVLink.
- Avoid using deprecated MAVSDK high-level APIs for firmware-specific sequences.
- ArduPilotAdapter implements small mapping helpers and mission adaptation hook.

Integration guide
-----------------
1. Ensure your build links MAVSDK and includes MAVLink headers. Example dependencies:
   - libmavsdk (C++)
   - mavlink headers (e.g., from pymavlink or your mavlink submodule)

2. Typical usage (sketch):
   - Create/obtain a `mavsdk::System` (after discovery).
   - Create `mavsdk::MavlinkPassthrough passthrough(system);`
   - Instantiate an `ArduPilotAdapter` object and call its helpers to prepare mission
     items or to perform mode mapping.
   - Use passthrough.subscribe_message(...) to receive COMMAND_ACK / MISSION_REQUEST etc.
   - Use passthrough.send_message(...) to send encoded mavlink_message_t structures.

3. Threading: do MAVSDK I/O in a dedicated thread (or follow MAVSDK threading model).
   Use Qt signals/slots (queued) to cross thread boundaries safely.

Compatibility notes
-------------------
MAVSDK versions might vary in the exact type used for messages in MavlinkPassthrough.
If your MAVSDK exposes `MavlinkMessage` (with a `payload` vector) instead of raw
`mavlink_message_t`, wrap encode bytes accordingly:

   // If MavlinkPassthrough::Message is { std::vector<uint8_t> payload; }
   mavlink_message_t msg = ...; // built with mavlink_msg_*
   std::vector<uint8_t> buf(MAVLINK_MAX_PACKET_LEN);
   int len = mavlink_msg_to_send_buffer(buf.data(), &msg);
   MavlinkPassthrough::Message m; m.payload = std::move(buf);
   passthrough.send_message(m);

License
-------
MIT
