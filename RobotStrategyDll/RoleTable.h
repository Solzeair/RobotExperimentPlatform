// RoleTable.h
#ifndef ROLETABLE_H
#define ROLETABLE_H

#include "StrategyCore.h"
#include "FieldGeometry.h"
#include "ControlParams.h"
#include <string>

// 角色ID定义 - 完整版 (0-127)
enum RoleID {
    // ========== 基础控制角色 (0-9) ==========
    ROLE_STOP = 0,          // 停止运动
    ROLE_IDLE = 1,          // 空闲，不执行任何动作
    ROLE_GO_TO_POINT = 2,   // 移动到指定点
    ROLE_FOLLOW_BALL = 3,   // 跟随球移动
    ROLE_AVOID_BALL = 4,    // 避开球

    // ========== 边界角色 (10-19) ==========
    ROLE_LEFT_BOUND = 10,       // 左边界防守
    ROLE_TOP_BOUND = 11,        // 上边界防守
    ROLE_RIGHT_BOUND = 12,      // 右边界防守
    ROLE_BOTTOM_BOUND = 13,     // 下边界防守
    ROLE_BOUND_PUSH = 14,       // 边线推球（将球从边界推回场内）
    ROLE_PUSH_OUT = 15,         // 推出边界（将球踢出边界）
    ROLE_CORNER_DEFENSE = 16,   // 角球防守
    ROLE_SIDELINE_PUSH = 17,    // 边线推球

    // ========== 防守角色 (20-39) ==========
    ROLE_CLEAR = 20,                // 解围（大脚开出）
    ROLE_SWEEPER = 21,              // 清道夫（拖后中卫）
    ROLE_ZONAL_DEFENSE = 22,        // 区域防守
    ROLE_MAN_MARK = 23,             // 人盯人防守
    ROLE_OFFSIDE_TRAP = 24,         // 越位陷阱
    ROLE_TACKLE = 25,               // 抢断
    ROLE_INTERCEPT = 26,            // 拦截传球
    ROLE_MARK = 27,                 // 盯人
    ROLE_BLOCK_SHOT = 28,           // 封堵射门
    ROLE_COVER_GAP = 29,            // 补位
    ROLE_SPECIAL_DEFENDER_UP = 30,  // 上专职后卫
    ROLE_SPECIAL_DEFENDER_DOWN = 31,// 下专职后卫
    ROLE_PENALTY_AREA_UP = 32,      // 大禁区上后卫
    ROLE_PENALTY_AREA_DOWN = 33,    // 大禁区下后卫
    ROLE_GOAL_LINE_DEFENSE = 34,    // 球门线防守
    ROLE_NEAR_POST = 35,            // 近门柱防守
    ROLE_FAR_POST = 36,             // 远门柱防守

    // ========== 进攻角色 (40-59) ==========
    ROLE_SHOOT = 40,            // 射门
    ROLE_DIRECT_CHARGE = 41,    // 直冲（带球突破）
    ROLE_HIGH_SPEED_LINE = 42,  // 高速直线跑动
    ROLE_HIGH_SPEED_ARC = 43,   // 高速弧线跑动
    ROLE_WING_LEFT = 44,        // 左边锋
    ROLE_WING_RIGHT = 45,       // 右边锋
    ROLE_HOLD_BALL = 46,        // 护球
    ROLE_CROSS = 47,            // 传中
    ROLE_HEADER = 48,           // 头球
    ROLE_VOLLEY = 49,           // 凌空抽射
    ROLE_QUICK_COUNTER = 50,    // 快速反击
    ROLE_PRESSURE = 51,         // 压迫
    ROLE_FALLBACK = 52,         // 回撤
    ROLE_DECOY = 53,            // 佯攻/跑位吸引防守
    ROLE_PENETRATION = 54,      // 前插/渗透
    ROLE_CHANNEL_RUN = 55,      // 通道跑位

    // ========== 等待接应角色 (60-79) ==========
    ROLE_WAIT_135_UP = 60,      // 135度等球（上）
    ROLE_WAIT_45_UP = 61,       // 45度等球（上）
    ROLE_WAIT_HORIZONTAL = 62,  // 横向等球（x=25）
    ROLE_WAIT_HORIZONTAL_2 = 63,// 横向等球2（x=35）
    ROLE_WAIT_HORIZONTAL_3 = 64,// 横向等球3（x=45）
    ROLE_WAIT_CENTER = 65,      // 中间等球
    ROLE_WAIT_LEFT = 66,        // 左边等球
    ROLE_WAIT_RIGHT = 67,       // 右边等球
    ROLE_WAIT_LONG = 68,        // 远点等球
    ROLE_WAIT_SHORT = 69,       // 近点等球
    ROLE_WAIT_DIAGONAL = 70,    // 对角线等球
    ROLE_WAIT_SUPPORT = 71,     // 支援等球

    // ========== 传球角色 (80-99) ==========
    ROLE_LONG_PASS = 80,        // 长传
    ROLE_SHORT_PASS = 81,       // 短传
    ROLE_THROUGH_BALL = 82,     // 直塞球
    ROLE_WALL_PASS = 83,        // 撞墙配合
    ROLE_OVERLAP = 84,          // 套边插上
    ROLE_UNDERLAP = 85,         // 内切插上
    ROLE_DUMMY_RUN = 86,        // 虚跑
    ROLE_ONE_TOUCH = 87,        // 一脚传球
    ROLE_BACK_PASS = 88,        // 回传
    ROLE_SWITCH_SIDE = 89,      // 转移
    ROLE_CROSS_PASS = 90,       // 传中

    // ========== 门将角色 (100-109) ==========
    ROLE_GOALIE = 100,              // 守门员（默认）
    ROLE_GOALIE_NORMAL = 101,       // 守门员（普通模式）
    ROLE_GOALIE_AGGRESSIVE = 102,   // 守门员（出击模式）
    ROLE_GOALIE_CONSERVATIVE = 103, // 守门员（保守模式）
    ROLE_GOALIE_SWEEPER = 104,      // 清道夫门将
    ROLE_GOALIE_ONE_ON_ONE = 105,   // 一对一守门员

    // ========== 特殊战术角色 (110-119) ==========
    ROLE_CORNER_KICKER = 110,   // 角球手
    ROLE_FREE_KICKER = 111,     // 任意球手
    ROLE_PENALTY_KICKER = 112,  // 点球手
    ROLE_LONG_THROWER = 113,    // 大力界外球
    ROLE_KICKOFF_TAKER = 114,   // 开球手
    ROLE_WALL_FORMATION = 115,  // 人墙

    // ========== 辅助角色 (120-127) ==========
    ROLE_ASSIST_NEAR_UP = 120,  // 近点支援（上）
    ROLE_ASSIST_NEAR_DOWN = 121,// 近点支援（下）
    ROLE_ASSIST_FAR_UP = 122,   // 远点支援（上）
    ROLE_ASSIST_FAR_DOWN = 123, // 远点支援（下）
    ROLE_TRIANGLE_CUT = 124,    // 三角配合-切入
    ROLE_TRIANGLE_ASSIST = 125, // 三角配合-助攻
    ROLE_TRIANGLE_FRONT = 126,  // 三角配合-前点
    ROLE_TRIANGLE_BACK = 127    // 三角配合-后点
};

/**
 * 角色结构体（增强版）
 * 包含角色的完整属性
 */
struct Role {
    int roleId;                 // 角色ID
    Point targetPos;            // 目标位置
    double priority;            // 优先级（0-1）
    std::string name;           // 角色名称
    double maxSpeed;            // 最大移动速度（cm/s）
    double aggression;          // 侵略性（0-1）
    double defense;             // 防守能力（0-1）
    double offense;             // 进攻能力（0-1）
    double passProbability;     // 传球概率（0-1）
    // 基础构造函数
    Role(int id = 0, const Point& pos = Point(), double pri = 0.5,
         const std::string& n = "", double speed = 70.0)
        : roleId(id), targetPos(pos), priority(pri), name(n), maxSpeed(speed),
        aggression(0.5), defense(0.5), offense(0.5), passProbability(0.3) {}

    // 完整构造函数
    Role(int id, const Point& pos, double pri, const std::string& n,
         double speed, double agg, double def, double off, double passProb = 0.3)
        : roleId(id), targetPos(pos), priority(pri), name(n), maxSpeed(speed),
        aggression(agg), defense(def), offense(off), passProbability(passProb) {}
};

class RoleTable {
public:
      // ========== 查询接口 ==========
    /**
     * 获取角色的目标位置
     * @param roleId 角色ID
     * @param robot 机器人当前位姿
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 目标位置
     */
    static Point getRoleTarget(int roleId, const RobotPose& robot,
                               const BallInfo& ball, const FieldGeometry& field);
    /**
     * 获取角色的最大移动速度
     * @param roleId 角色ID
     * @return 最大速度（cm/s）
     */
    static double getRoleMaxSpeed(int roleId);
    /**
     * 执行角色动作
     * @param roleId 角色ID
     * @param robot 机器人当前位姿
     * @param ball 球的信息
     * @param oppRobots 对方机器人位置数组
     * @param oppCount 对方机器人数量
     * @param field 场地几何信息
     * @param params 控制参数
     * @param vel 输出：轮速指令
     */
    static void executeRole(int roleId, const RobotPose& robot, const BallInfo& ball,
                            const Point oppRobots[], int oppCount,
                            const FieldGeometry& field, const ControlParams& params,
                            WheelVelocity& vel);
    /**
     * 获取角色名称
     * @param roleId 角色ID
     * @return 角色名称字符串
     */
    static std::string getRoleName(int roleId);
    /**
     * 获取角色的侵略性
     * @param roleId 角色ID
     * @return 侵略性值（0-1）
     */
    static double getRoleAggression(int roleId);
    /**
     * 获取角色的防守能力
     * @param roleId 角色ID
     * @return 防守能力值（0-1）
     */
    static double getRoleDefense(int roleId);
    /**
     * 获取角色的进攻能力
     * @param roleId 角色ID
     * @return 进攻能力值（0-1）
     */
    static double getRoleOffense(int roleId);

    // 传球相关
    /**
     * 判断是否应该传球
     * @param roleId 角色ID
     * @param robot 机器人位姿
     * @param ball 球的信息
     * @return true: 应该传球
     */
    static bool shouldPass(int roleId, const RobotPose& robot, const BallInfo& ball);
    /**
     * 获取传球目标点
     * @param roleId 角色ID
     * @param robot 机器人位姿
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 传球目标点
     */
    static Point getPassTarget(int roleId, const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field);

private:
    // 基础角色实现
    static void roleStop(const RobotPose& robot, WheelVelocity& vel);
    static void roleGoToPoint(const RobotPose& robot, const Point& target,
                              double speed, WheelVelocity& vel, const ControlParams& params);

    // 边界角色
    static void roleLeftBound(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel);
    static void roleTopBound(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleRightBound(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel);
    static void roleBottomBound(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel);
    static void roleBoundPush(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel);
    static void rolePushOut(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel);
    static void roleCornerDefense(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, WheelVelocity& vel);
    static void roleSidelinePush(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel);

    // 防守角色
    static void roleClear(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, const ControlParams& params,
                          WheelVelocity& vel);
    static void roleSweeper(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel);
    static void roleZonalDefense(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel);
    static void roleManMark(const RobotPose& robot, const Point& opponent,
                            WheelVelocity& vel);
    static void roleOffsideTrap(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel);
    static void roleTackle(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, WheelVelocity& vel);
    static void roleIntercept(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel);
    static void roleMark(const RobotPose& robot, const Point& opponent,
                         WheelVelocity& vel);
    static void roleBlockShot(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel);
    static void roleCoverGap(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleSpecialDefender(const RobotPose& robot, const BallInfo& ball,
                                    const FieldGeometry& field, bool isUp,
                                    WheelVelocity& vel);
    static void rolePenaltyArea(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, bool isUp,
                                WheelVelocity& vel);
    static void roleGoalLineDefense(const RobotPose& robot, const BallInfo& ball,
                                    const FieldGeometry& field, WheelVelocity& vel);
    static void roleNearPost(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleFarPost(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel);

    // 进攻角色
    static void roleShoot(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, const ControlParams& params,
                          WheelVelocity& vel);
    static void roleDirectCharge(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, const ControlParams& params,
                                 WheelVelocity& vel);
    static void roleHighSpeedLine(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, WheelVelocity& vel);
    static void roleHighSpeedArc(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel);
    static void roleWing(const RobotPose& robot, const BallInfo& ball,
                         const FieldGeometry& field, bool isLeft,
                         WheelVelocity& vel);
    static void roleHoldBall(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleCross(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, WheelVelocity& vel);
    static void roleHeader(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, WheelVelocity& vel);
    static void roleVolley(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, WheelVelocity& vel);
    static void roleQuickCounter(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel);
    static void rolePressure(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleFallback(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleDecoy(const RobotPose& robot, const BallInfo& ball,
                          const FieldGeometry& field, WheelVelocity& vel);
    static void rolePenetration(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel);
    static void roleChannelRun(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel);

    // 等待接应角色
    static void roleWait(const RobotPose& robot, const Point& target,
                         const BallInfo& ball, WheelVelocity& vel);
    static void roleWait135Up(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel);
    static void roleWait45Up(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleWaitHorizontal(const RobotPose& robot, const BallInfo& ball,
                                   const FieldGeometry& field, double offset,
                                   WheelVelocity& vel);
    static void roleWaitCenter(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel);
    static void roleWaitLeft(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleWaitRight(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel);
    static void roleWaitSupport(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel);

    // 传球角色
    static void roleLongPass(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleShortPass(const RobotPose& robot, const BallInfo& ball,
                              const FieldGeometry& field, WheelVelocity& vel);
    static void roleThroughBall(const RobotPose& robot, const BallInfo& ball,
                                const FieldGeometry& field, WheelVelocity& vel);
    static void roleWallPass(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleOverlap(const RobotPose& robot, const BallInfo& ball,
                            const FieldGeometry& field, WheelVelocity& vel);
    static void roleUnderlap(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleOneTouch(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleBackPass(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, WheelVelocity& vel);
    static void roleSwitchSide(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel);

    // 门将角色
    static void roleGoalie(const RobotPose& robot, const BallInfo& ball,
                           const FieldGeometry& field, const ControlParams& params,
                           WheelVelocity& vel);
    static void roleGoalieNormal(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, const ControlParams& params,
                                 WheelVelocity& vel);
    static void roleGoalieAggressive(const RobotPose& robot, const BallInfo& ball,
                                     const FieldGeometry& field, const ControlParams& params,
                                     WheelVelocity& vel);
    static void roleGoalieConservative(const RobotPose& robot, const BallInfo& ball,
                                       const FieldGeometry& field, const ControlParams& params,
                                       WheelVelocity& vel);
    static void roleGoalieSweeper(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, const ControlParams& params,
                                  WheelVelocity& vel);
    static void roleGoalieOneOnOne(const RobotPose& robot, const BallInfo& ball,
                                   const FieldGeometry& field, const ControlParams& params,
                                   WheelVelocity& vel);

    // 特殊战术角色
    static void roleCornerKicker(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel);
    static void roleFreeKicker(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, WheelVelocity& vel);
    static void rolePenaltyKicker(const RobotPose& robot, const BallInfo& ball,
                                  const FieldGeometry& field, WheelVelocity& vel);
    static void roleKickoffTaker(const RobotPose& robot, const BallInfo& ball,
                                 const FieldGeometry& field, WheelVelocity& vel);

    // 辅助角色
    static void roleAssistNear(const RobotPose& robot, const BallInfo& ball,
                               const FieldGeometry& field, bool isUp,
                               WheelVelocity& vel);
    static void roleTriangle(const RobotPose& robot, const BallInfo& ball,
                             const FieldGeometry& field, int type,
                             WheelVelocity& vel);

    // 辅助函数
    static double calculateAvoidanceForce(const RobotPose& robot, const Point& target,
                                          const Point obstacles[], int count,
                                          Point& forceOut);
    static bool isCollisionRisk(const RobotPose& robot, const Point& target,
                                const Point obstacles[], int count);
};

#endif // ROLETABLE_H
