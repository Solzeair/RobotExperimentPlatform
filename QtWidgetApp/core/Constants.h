#pragma once

namespace Constants {
    // Display dimensions
    inline constexpr int kDisplayW = 640;
    inline constexpr int kDisplayH = 480;

    // Main window layout
    inline constexpr int kClientW = 1200;
    inline constexpr int kClientH = 700;
    inline constexpr int kDisplayDlgH = 512;
    inline constexpr int kDisplayDlgW = kDisplayW;
    inline constexpr int kDebugH = (kClientH - kDisplayDlgH - 20);
    inline constexpr int kDebugW = kDisplayDlgW;
    inline constexpr int kControlW = (kClientW - kDisplayDlgW);
    inline constexpr int kControlH = kClientH;

    // Robot configuration
    inline constexpr int kMaxRobotNum = 5;

    // Field dimensions (cm)
    inline constexpr double kFieldWidth = 220.0;
    inline constexpr double kFieldHeight = 180.0;
    inline constexpr double kFieldCenterX = kFieldWidth / 2.0;
    inline constexpr double kFieldCenterY = kFieldHeight / 2.0;

    // Goal dimensions
    inline constexpr double kGoalWidth = 40.0;
    inline constexpr double kGoalDepth = 10.0;

    // Math constants
    inline constexpr double kPi = 3.14159265358979323846;
    inline constexpr double kInf = 100.0;
    inline constexpr double kInfLarge = 1e10;
    inline constexpr double kAbsSmall = 1e-10;
    inline constexpr double kSqrt2 = 1.41421356237;

    // Direction constants
    inline constexpr int kClockwise = 1;
    inline constexpr int kAntiClock = -1;
    inline constexpr int kNoClock = 0;
    inline constexpr int kForward = 1;
    inline constexpr int kBackward = -1;

    // Timing (ms)
    inline constexpr int kVisionFrameInterval = 50;    // ~20 FPS grab
    inline constexpr int kDisplayUpdateInterval = 33;  // ~30 FPS display
    inline constexpr int kDecisionInterval = 33;       // ~30 Hz
    inline constexpr int kCommInterval = 33;           // ~30 Hz
    inline constexpr int kWatchdogCheckInterval = 10;  // 10ms check

    // Watchdog timeouts (ms)
    inline constexpr int kVisionTimeoutMs = 50;
    inline constexpr int kDecisionTimeoutMs = 100;
    inline constexpr int kCommTimeoutMs = 200;

    // SPSC queue capacity
    inline constexpr int kVisionQueueCapacity = 16;
    inline constexpr int kCommandQueueCapacity = 16;

    // Config file names
    inline constexpr const char* kDefaultConfigFile = "config/default_config.json";
    inline constexpr const char* kGroundDataFile = "ground.dat";
    inline constexpr const char* kPointsTemplateFile = "points_template.dat";
    inline constexpr const char* kCameraConfigFile = "CameraConfig.ini";

    // UDP protocol
    inline constexpr uint32_t kUdpMagic = 0x524F424F; // "ROBO"
    inline constexpr uint8_t kProtocolVersion = 1;
    inline constexpr int kCommandPacketSize = 64;
}
