#pragma once
#include <cstdint>
#include <chrono>

namespace RobotPlatform {

// Robot identification
struct RobotInfo {
    float x_cm = 0.0f;
    float y_cm = 0.0f;
    float theta_rad = 0.0f;
    float vx_cm_s = 0.0f;
    float vy_cm_s = 0.0f;
    float omega_rad_s = 0.0f;
    uint8_t id = 0;
    uint8_t visible = 0;
    uint8_t confidence = 0;
    uint8_t reserved = 0;
};

// Ball information
struct BallInfoData {
    float x_cm = 0.0f;
    float y_cm = 0.0f;
    float z_cm = 0.0f;
    float vx_cm_s = 0.0f;
    float vy_cm_s = 0.0f;
    float vz_cm_s = 0.0f;
    uint8_t visible = 0;
    uint8_t confidence = 0;
    uint8_t reserved[2] = {};
};

// Field information
struct FieldInfo {
    float width_cm = 220.0f;
    float height_cm = 180.0f;
    float goal_width_cm = 40.0f;
    float reserved = 0.0f;
};

// Vision data - pushed from VisionThread to DecisionThread (~512 bytes)
struct VisionData {
    int64_t timestamp_us = 0;
    uint32_t frame_number = 0;
    uint32_t reserved1 = 0;
    RobotInfo our_robots[5];
    RobotInfo opp_robots[5];
    BallInfoData ball;
    FieldInfo field;
    uint8_t padding[60] = {}; // pad to ~512 bytes
};

// Robot command - computed by strategy
struct RobotCommand {
    float vx_cm_s = 0.0f;
    float vy_cm_s = 0.0f;
    float omega_rad_s = 0.0f;
    uint8_t robot_id = 0;
    uint8_t kick = 0;
    uint8_t chip = 0;
    uint8_t kick_power = 0;
    float dribble = 0.0f;
};

// Decision commands - pushed from DecisionThread to CommThread (~128 bytes)
struct DecisionCommands {
    int64_t timestamp_us = 0;
    uint32_t frame_number = 0;
    uint32_t reserved = 0;
    RobotCommand commands[5];
};

// UDP protocol structures
#pragma pack(push, 1)

struct UDPHeader {
    uint32_t magic = 0x524F424F;   // "ROBO"
    uint8_t protocol_version = 1;
    uint8_t packet_type = 0;
    uint16_t sequence_number = 0;
    int64_t timestamp_us = 0;
};

struct RobotCommandUDP {
    uint8_t robot_id = 0;
    int16_t vx_mm_s = 0;
    int16_t vy_mm_s = 0;
    int16_t omega_mrad_s = 0;
    uint8_t flags = 0; // bit0=kick, bit1=chip, bit2-4=dribble, bit5-7=kickPower
};

struct CommandPacket {
    UDPHeader header;
    uint8_t robot_count = 0;
    RobotCommandUDP commands[5];
    uint16_t checksum = 0;
};
static_assert(sizeof(CommandPacket) == 43, "CommandPacket must be 43 bytes before padding");

// Padded to 64 bytes for alignment
struct CommandPacketAligned {
    CommandPacket packet;
    uint8_t padding[64 - sizeof(CommandPacket)] = {};
};
static_assert(sizeof(CommandPacketAligned) == 64, "CommandPacketAligned must be 64 bytes");

#pragma pack(pop)

// Strategy DLL data types (shared between DLL and app)
struct StrategyPoint {
    double x = 0.0;
    double y = 0.0;
};

struct StrategyRobotPose {
    double x = 0.0;
    double y = 0.0;
    double theta = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double vtheta = 0.0;
};

struct StrategyBallInfo {
    StrategyPoint pos;
    double vel_x = 0.0;
    double vel_y = 0.0;
    double velocity = 0.0;
    double angle = 0.0;
    StrategyPoint predictPos;
};

struct StrategyWheelVelocity {
    double left = 0.0;
    double right = 0.0;
};

} // namespace RobotPlatform
