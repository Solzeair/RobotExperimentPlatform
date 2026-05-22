// UnifiedStrategy.h
// 统一策略 - 整合所有模块的完整策略实现

#ifndef UNIFIEDSTRATEGY_H
#define UNIFIEDSTRATEGY_H

#include "StrategyBase.h"
#include "FieldGeometry.h"
#include "ControlParams.h"
#include "StrategyContext.h"     
#include "RoleTable.h"
#include "PassCoordinator.h"
#include <vector>
#include <deque>
#include <string>


/**
 * 比赛状态枚举
 * 描述比赛的特殊状态
 */
enum class MatchState {
    STATE_NORMAL,       // 正常比赛状态
    STATE_KICKOFF,      // 开球状态
    STATE_GOALKICK,     // 门球状态
    STATE_PENALTY,      // 点球状态
    STATE_FREEBALL,     // 争球状态
    STATE_BOUNDARY,     // 边界球状态
    STATE_CORNER,       // 角球状态
    STATE_FREEKICK      // 任意球状态
};

/**
 * 战术阶段枚举
 * 根据场上形势动态切换战术阶段
 */
enum class TacticalPhase {
    PHASE_ATTACK,       // 进攻阶段 - 积极向前，创造机会
    PHASE_DEFENSE,      // 防守阶段 - 稳固防守，保护球门
    PHASE_COUNTER,      // 反击阶段 - 快速由守转攻
    PHASE_POSSESSION,   // 控球阶段 - 控制节奏，消耗时间
    PHASE_PRESSURE      // 压迫阶段 - 高位逼抢，迫使对方失误
};
enum class KickoffType {
    NORMAL = 0,    // 普通
    PENALTY = 1,   // 点球
    GOAL_KICK = 2, // 门球
    FREE_KICK = 3, // 任意球
    FIGHT_BALL = 4,// 争球
    PARK = 5       // 收车
};

/**
 * 开局场景枚举
 * 用于初始策略选择
 */
enum class StartScenario {
    SCENARIO_ATTACK,    // 进攻型开局（我方开球、对方弱队）
    SCENARIO_DEFENSE,   // 防守型开局（对方开球、我方领先、对方强队）
    SCENARIO_BALANCE    // 平衡型开局
};

/**
 * 机器人历史信息结构体
 * 用于记录机器人的历史轨迹，便于预测位置和速度
 */
struct RobotHistory {
    std::deque<RobotPose> positions;  // 历史位置队列（最多7个）
    Point velocity;                    // 当前速度向量

    RobotHistory() : velocity(0, 0) {}

    /**
     * 添加新的位姿到历史记录
     * @param pose 新的机器人位姿
     */
    void add(const RobotPose& pose) {
        positions.push_back(pose);
        // 保持历史记录不超过7个（与原MFC一致）
        while ((int)positions.size() > 7) {
            positions.pop_front();
        }

        // 计算速度（基于最近两个位置）
        if (positions.size() >= 2) {
            const RobotPose& prev = positions[positions.size() - 2];
            const RobotPose& curr = positions.back();
            velocity.x = (curr.x - prev.x) / 0.033;  // 假设33ms一帧
            velocity.y = (curr.y - prev.y) / 0.033;
        }
    }
};

/**
 * 统一策略类
 * 实现了原MFC的所有核心功能：
 * - preProcess: 信息预处理
 * - forecastBall: 球轨迹预测
 * - taskDecompose: 任务分解
 * - formInterpret: 队形解释
 * - charAllot: 角色分配
 * - robotManager: 机器人管理
 * - actProcess: 动作执行
 */
class UnifiedStrategy : public StrategyBase {
public:
    UnifiedStrategy();
    ~UnifiedStrategy() override;

    // ========== StrategyBase接口实现 ==========
    void decide(const RobotPose robots[], const Point oppRobots[],
        const BallInfo& ball, WheelVelocity velocities[]) override;
    void reset() override;
    void setParameter(const std::string& key, double value) override;
    std::string getStrategyName() const override { return "Unified MFC Strategy"; }

    // ========== 配置接口 ==========
    void setOurGoalOnRight(bool onRight) { m_field.setOurGoalSide(onRight); }
    void setOurKickoff(bool isOurKickoff) { m_isOurKickoff = isOurKickoff; }

    // 应用策略配置
    void applyStrategyConfig(const StrategyConfig& config);

    // 获取当前战术阶段
    TacticalPhase getTacticalPhase() const { return m_tacticalPhase; }

    // 设置战术阶段（只有一个定义，没有重复）
    void setTacticalPhase(TacticalPhase phase) { m_tacticalPhase = phase; }

    // ========== 参数接口 ==========
    double getParameter(const std::string& key) const;
    void setControlParam(const std::string& key, double value);
    double getControlParam(const std::string& key) const;
    bool saveConfig(const std::string& filename);
    bool loadConfig(const std::string& filename);

    void setKickoffType(int type);
    void setFormationType(bool isSingleDefender);

    void setPenaltyKickMode(int direction, int mode);  // direction: 0左晃,1直冲,2右晃; mode: 守门位置 0左,1中,2右
    void selectStrategy(int strategyIndex);            // 策略选择
    void setMatchState(int state);    
    void parkRobots();                                 // 收车
    void startMatch();                                 // 开始比赛
    void stopMatch();                                  // 停止比赛
    bool saveConfig(const char* filename);             // 保存配置
    bool loadConfig(const char* filename);             // 加载配置
private:
    // ========== 核心函数（原MFC）==========
    bool m_isSingleDefender;
    /**
     * 信息预处理
     * 包括坐标转换、历史记录更新、球位置预测
     */
    void preProcess(const BallInfo& ball, const RobotPose robots[]);

    /**
     * 球轨迹预测（原MFC forcastball）
     * 使用最近7个点的线性预测
     */
    void forecastBall(const BallInfo& ball);

    /**
     * 任务分解（原MFC taskDecompose）
     * 根据区域号确定队形号
     */
    void taskDecompose(int areaNo, const BallInfo& ball);

    /**
     * 队形解释（原MFC formInterpret）
     * 将队形号转换为具体的角色列表
     */
    void formInterpret(int formationNo, const BallInfo& ball);

    /**
     * 角色分配（原MFC charAllot）
     * 使用匈牙利算法分配角色
     */
    void charAllot(const RobotPose robots[], int robotCount);

    /**
     * 机器人管理（原MFC robotManager）
     * 处理机器人异常情况（卡边界、碰撞等）
     */
    void robotManager(const RobotPose robots[], const Point oppRobots[],
        const BallInfo& ball, WheelVelocity velocities[]);

    /**
     * 动作执行（原MFC actProcess）
     * 执行每个机器人的角色动作
     */
    void actProcess(const RobotPose robots[], const Point oppRobots[],
        const BallInfo& ball, WheelVelocity velocities[]);

    // ========== 辅助函数 ==========
    void updateMatchState(const BallInfo& ball);      // 更新比赛状态
    void updateTacticalPhase(const BallInfo& ball);   // 更新战术阶段
    bool isInAttackState(const BallInfo& ball);       // 是否进攻状态
    bool isInDefenseState(const BallInfo& ball);      // 是否防守状态
    bool isNearBoundary(const Point& pos);            // 是否靠近边界
    bool isCornerSituation(const Point& pos);         // 是否角球情况
    double calculatePerformanceScore(const RobotPose& robot, const Role& role);  // 计算性能分数

    bool shouldPass(const RobotPose& robot, const BallInfo& ball);  // 是否应该传球
    PassOption evaluatePassOptions(const RobotPose& passer, const BallInfo& ball,
        const RobotPose receivers[], int receiverCount,
        const Point opponents[], int opponentCount);  // 评估传球
    Point getDynamicTarget(const Role& role, const BallInfo& ball);  // 获取动态目标点

    // 匈牙利算法（用于角色分配）
    void hungarianAlgorithm(double costMatrix[5][5], int assignment[5]);

private:
    // ========== 成员变量 ==========
    int m_penaltyDirection;    // 点球方向 (0=左晃,1=直冲,2=右晃)
    int m_goaliePosition;      // 守门员位置(0=左,1=中,2=右)
    int m_currentStrategy;     // 当前策略索引(0-3)
    bool m_isMatchRunning;     // 比赛是否运行中

    FieldGeometry m_field;           // 场地几何信息
    ControlParams m_params;          // 控制参数
    int m_frameCount;                // 帧计数器

    MatchState m_matchState;         // 比赛状态
    TacticalPhase m_tacticalPhase;   // 战术阶段
    int m_stateTimer;                // 状态计时器
    bool m_isOurKickoff;             // 是否我方开球
    int m_currentFormationNo;        // 当前队形号

    // 状态变量
    int m_areaNo;                    // 当前区域号
    int m_formationNo;               // 当前队形号
    int m_assignedRoles[5];          // 分配给每个机器人的角色ID
    Point m_predictedBall;           // 预测的球位置
    std::vector<Role> m_roles;       // 角色列表

    // 历史数据
    std::deque<Point> m_ballHistory; // 球的历史位置
    RobotHistory m_robotHistory[5];  // 机器人的历史位姿

    // 战术参数
    double m_attackAggression;       // 进攻侵略性 (0-1)
    double m_defenseDepth;           // 防守深度 (0-1)
    double m_pressingIntensity;      // 压迫强度 (0-1)
    bool m_useOffsideTrap;           // 是否使用越位陷阱
    bool m_useZonalDefense;          // 是否使用区域防守
    // 传球协调器
    PassCoordinator m_passCoordinator;
};

#endif // UNIFIEDSTRATEGY_H