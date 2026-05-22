#include "UnifiedStrategy.h"
#include "AreaDivider.h"
#include "MotionControl.h"
#include "Goalie.h"
#include "Formation.h"
#include "ParameterTuning.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <deque>

using namespace GeometryUtils;

UnifiedStrategy::UnifiedStrategy()
    : m_frameCount(0)
    , m_matchState(MatchState::STATE_NORMAL)
    , m_tacticalPhase(TacticalPhase::PHASE_DEFENSE)
    , m_stateTimer(0)
    , m_isOurKickoff(true)
    , m_currentFormationNo(0)
    , m_areaNo(0)
    , m_formationNo(0)
    , m_attackAggression(0.6)
    , m_defenseDepth(0.5)
    , m_pressingIntensity(0.5)
    , m_useOffsideTrap(false)
    , m_isSingleDefender(false)
    , m_useZonalDefense(true) {

    // 初始化控制参数（与原MFC对应）
    m_params.kp_pos = 12.0;      // 位置比例系数
    m_params.kp_angle = 22.0;    // 角度比例系数
    m_params.kd_pos = 0.5;       // 位置微分系数
    m_params.kd_angle = 7.0;     // 角度微分系数
    m_params.max_speed = 75.0;   // 最大速度
    m_params.angle_error = 0.05; // 角度误差阈值

    // 初始化分配的角色
    for (int i = 0; i < 5; i++) {
        m_assignedRoles[i] = ROLE_STOP;
    }
}

UnifiedStrategy::~UnifiedStrategy() {}

// ==================== 原MFC核心函数实现 ====================

void UnifiedStrategy::preProcess(const BallInfo& ball, const RobotPose robots[]) {
    // 1. 坐标转换到标准坐标系（我方球门在左侧）
    Point stdBall = m_field.transformToStandard(ball.pos);

    // 2. 记录球的历史位置（7个周期，与原MFC一致）
    m_ballHistory.push_back(stdBall);
    while ((int)m_ballHistory.size() > 7) {
        m_ballHistory.pop_front();
    }

    // 3. 记录机器人的历史位置
    for (int i = 0; i < 5; i++) {
        RobotPose stdRobot = robots[i];
        stdRobot.x = m_field.transformToStandard(Point(robots[i].x, robots[i].y)).x;
        stdRobot.y = m_field.transformToStandard(Point(robots[i].x, robots[i].y)).y;
        m_robotHistory[i].add(stdRobot);
    }

    // 4. 预测球的位置（对应原MFC forcastball()）
    forecastBall(ball);

    // 5. 预测机器人位置（可选）
    // for (int i = 0; i < 5; i++) {
    //     m_predictedRobots[i] = BallPredictor::predictPosition(m_robotHistory[i].positions, 3);
    // }
}

void UnifiedStrategy::forecastBall(const BallInfo& ball) {
    // 原MFC的预测算法：使用最近7个点的线性预测
    if (m_ballHistory.size() < 2) {
        m_predictedBall = ball.pos;
        return;
    }

    // 计算平均速度（使用最近7个点，与原MFC一致）
    double avgVx = 0, avgVy = 0;
    int count = std::min((int)m_ballHistory.size() - 1, 7);

    for (int i = 1; i <= count; i++) {
        const Point& curr = m_ballHistory[m_ballHistory.size() - i];
        const Point& prev = m_ballHistory[m_ballHistory.size() - i - 1];
        avgVx += (curr.x - prev.x);
        avgVy += (curr.y - prev.y);
    }

    if (count > 0) {
        avgVx /= count;
        avgVy /= count;
    }

    // 预测3个时间步后的位置（与原MFC一致）
    Point last = m_ballHistory.back();
    m_predictedBall.x = last.x + avgVx * 3;
    m_predictedBall.y = last.y + avgVy * 3;

    // 限制在场地内
    m_predictedBall.x = std::max(0.0, std::min(m_field.getFieldWidth(), m_predictedBall.x));
    m_predictedBall.y = std::max(0.0, std::min(m_field.getFieldHeight(), m_predictedBall.y));

    // 转换回原始坐标系
    m_predictedBall = m_field.transformFromStandard(m_predictedBall);
}

void UnifiedStrategy::taskDecompose(int areaNo, const BallInfo& ball) {
    // 原MFC的 taskDecompose() 函数
    // 根据区域号确定队形号
    m_areaNo = areaNo;
    // 先处理边界特殊情况（优先级最高）
    if (isNearBoundary(ball.pos)) {
        m_formationNo = 100;
        return;
    }
    if (isCornerSituation(ball.pos)) {
        m_formationNo = 101;
        return;
    }

    // 正常情况：根据策略模式调整区域号
    int effectiveAreaNo = areaNo;
    if (m_attackAggression > 0.75 && effectiveAreaNo <= 16) {
        // 进攻策略：球在后场时也向前压
        effectiveAreaNo = std::min(32, effectiveAreaNo + 8);
    }
    else if (m_attackAggression < 0.4 && effectiveAreaNo >= 17) {
        // 防守策略：球在前场时也向后撤
        effectiveAreaNo = std::max(1, effectiveAreaNo - 8);
    }

    m_formationNo = effectiveAreaNo;
}

// 在 UnifiedStrategy.cpp 的 formInterpret 函数中添加边界检查
void UnifiedStrategy::formInterpret(int formationNo, const BallInfo& ball) {
    std::vector<Role> roles;

    if (formationNo == 100) {
        roles = Formation::getBoundaryFormation(ball, m_field);
    }
    else if (formationNo == 101) {
        roles = Formation::getCornerFormation(ball, m_field);
    }
    else if (formationNo >= 1 && formationNo <= 32) {
        roles = Formation::getFormation(formationNo, ball, m_field);
    }
    else {
        roles = Formation::getFormation(1, ball, m_field);
    }

    // ✅ 单双后卫调整（在赋值给 m_roles 之前处理 roles）
    if (m_isSingleDefender) {
        for (auto& role : roles) {
            if (role.roleId == ROLE_SPECIAL_DEFENDER_DOWN) {
                role.roleId = ROLE_WAIT_SUPPORT;
                role.name = "单后卫-支援";
                break;
            }
        }
    }

    // 赋值给 m_roles
    m_roles = roles;

    // 确保有5个角色
    while ((int)m_roles.size() < 5) {
        m_roles.push_back(Role(ROLE_WAIT_CENTER, Point(110, 90), 0.5, "默认"));
    }
}

// 原MFC的 charAllot() 函数 - 使用匈牙利算法分配角色

void UnifiedStrategy::charAllot(const RobotPose robots[], int robotCount) {
    int n = std::min(robotCount, 5);
    int m = std::min((int)m_roles.size(), 5);

    if (n == 0 || m == 0) return;
    // 构建成本矩阵（距离越远成本越高）
    double costMatrix[5][5];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            Point target = m_roles[j].targetPos;
            double dist = pointToPointDistance(robots[i], target);
            // 守门员角色：距离权重加倍
            if (m_roles[j].roleId == ROLE_GOALIE) {
                dist *= 0.5;
            } // 射门角色：考虑与球门的距离
            else if (m_roles[j].roleId == ROLE_SHOOT) {
                double goalDist = pointToPointDistance(robots[i], m_field.getOppGoalPos());
                dist += goalDist * 0.3;
            }

            costMatrix[i][j] = dist;
        }
    }
    // 匈牙利算法求解最优分配
    int assignment[5];
    hungarianAlgorithm(costMatrix, assignment);
    // 应用分配结果
    for (int i = 0; i < n; i++) {
        if (assignment[i] >= 0 && assignment[i] < m) {
            m_assignedRoles[i] = m_roles[assignment[i]].roleId;
        }
        else {
            m_assignedRoles[i] = ROLE_WAIT_CENTER;
        }
    }
    // 确保有守门员
    bool hasGoalie = false;
    for (int i = 0; i < n; i++) {
        if (m_assignedRoles[i] == ROLE_GOALIE) {
            hasGoalie = true;
            break;
        }
    }
    // 如果没有守门员，将离球门最近的机器人设为守门员
    if (!hasGoalie && n > 0) {
        int goalieIdx = 0;
        double minDist = 1e9;
        Point goalPos = m_field.getOurGoalPos();
        for (int i = 0; i < n; i++) {
            double dist = pointToPointDistance(robots[i], goalPos);
            if (dist < minDist) {
                minDist = dist;
                goalieIdx = i;
            }
        }
        m_assignedRoles[goalieIdx] = ROLE_GOALIE;
    }
}
/**
* 匈牙利算法
* 用于求解二分图最小权匹配问题
* @param costMatrix 成本矩阵（5x5）
* @param assignment 输出分配数组
*/
void UnifiedStrategy::hungarianAlgorithm(double costMatrix[5][5], int assignment[5]) {
    // 匈牙利算法实现（修复数组越界问题）
    int n = 5;

    // 初始化
    for (int i = 0; i < n; i++) {
        assignment[i] = -1;
    }

    double u[5] = { 0 }, v[5] = { 0 };
    int p[6] = { 0 };  // 改为6，避免越界（索引1-5）
    int way[6] = { 0 };

    for (int i = 1; i <= n; i++) {
        p[0] = i;
        int j0 = 0;
        double minv[6];  // 改为6
        for (int j = 1; j <= n; j++) {
            minv[j] = 1e9;
        }
        bool used[6] = { false };  // 改为6

        do {
            used[j0] = true;
            int i0 = p[j0];
            int j1 = 0;
            double delta = 1e9;

            for (int j = 1; j <= n; j++) {
                if (!used[j]) {
                    double cur = costMatrix[i0 - 1][j - 1] - u[i0 - 1] - v[j - 1];
                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }
            }

            for (int j = 0; j <= n; j++) {
                if (used[j]) {
                    u[p[j] - 1] += delta;
                    if (j > 0) v[j - 1] -= delta;
                }
                else {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);

        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);
    }

    for (int j = 1; j <= n; j++) {
        if (p[j] != 0) {
            assignment[p[j] - 1] = j - 1;
        }
    }
}

void UnifiedStrategy::robotManager(const RobotPose robots[], const Point oppRobots[],
    const BallInfo& ball, WheelVelocity velocities[]) {
    // 原MFC的 robotManager() 函数
    // 管理机器人状态，处理异常情况

    for (int i = 0; i < 5; i++) {
        // 检查机器人是否卡在边界
        if (robots[i].x < 5 || robots[i].x > m_field.getFieldWidth() - 5 ||
            robots[i].y < 5 || robots[i].y > m_field.getFieldHeight() - 5) {
            // 边界卡住，反向移动
            velocities[i].left = 30;
            velocities[i].right = -30;
            continue;
        }

        // 检查机器人是否与其他机器人碰撞
        bool collision = false;
        for (int j = 0; j < 5; j++) {
            if (i != j && pointToPointDistance(robots[i], robots[j]) < 10) {
                collision = true;
                break;
            }
        }

        if (collision) {
            // 碰撞处理：分离移动
            velocities[i].left = -20;
            velocities[i].right = 20;
        }
    }
}

void UnifiedStrategy::actProcess(const RobotPose robots[], const Point oppRobots[],
    const BallInfo& ball, WheelVelocity velocities[]) {
    // 原MFC的 actProcess() 函数
    // 执行每个机器人的动作

    // 收集障碍物
    Point allObstacles[10];
    int obsCount = 0;
    for (int i = 0; i < 5 && obsCount < 10; i++) {
        allObstacles[obsCount++] = Point(robots[i].x, robots[i].y);
    }
    for (int i = 0; i < 5 && obsCount < 10; i++) {
        allObstacles[obsCount++] = oppRobots[i];
    }

    RobotPose oppPoses[5];
    for (int i = 0; i < 5 && i < obsCount; i++) {
        oppPoses[i].x = oppRobots[i].x;
        oppPoses[i].y = oppRobots[i].y;
        oppPoses[i].theta = 0;
        oppPoses[i].vx = 0;
        oppPoses[i].vy = 0;
        oppPoses[i].vtheta = 0;
    } for (int i = 0; i < 5; i++) {
        int roleId = m_assignedRoles[i];

        // 守门员特殊处理
        if (roleId == ROLE_GOALIE) {
            Goalie::goalieAction(robots[i], ball, velocities[i], m_field, m_params);
            continue;
        }

        // 获取角色目标
        Point target = RoleTable::getRoleTarget(roleId, robots[i], ball, m_field);

        // 应用避障
        Point avoidTarget = MotionControl::smoothAvoidObstacles(robots[i], target,
            allObstacles, obsCount, nullptr);

        // 传球决策
        if (shouldPass(robots[i], ball) && roleId != ROLE_SHOOT) {
            PassOption pass = m_passCoordinator.evaluatePass(robots[i], ball, robots, 5, oppPoses, 5);
            if (pass.successRate > 0.6) {
                m_passCoordinator.executePass(robots[i], pass, velocities[i], m_params);
                continue;
            }
        }

        // 执行角色动作
        RoleTable::executeRole(roleId, robots[i], ball, oppRobots, 5,
            m_field, m_params, velocities[i]);
    }
    // 在函数末尾添加调试输出
    std::cout << "actProcess: velocities calculated" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "  Robot " << i << ": L=" << velocities[i].left
            << ", R=" << velocities[i].right << std::endl;
    }
}

// ==================== 主决策函数 ====================
void UnifiedStrategy::decide(const RobotPose robots[], const Point oppRobots[],
    const BallInfo& ball, WheelVelocity velocities[]) {
    std::cout << "decide called, frame: " << m_frameCount << std::endl;
    m_frameCount++;

    // 1. 信息预处理（原MFC preProcess）
    preProcess(ball, robots);
    std::cout << "preProcess done" << std::endl;


    // 2. 更新比赛状态
    updateMatchState(ball);
    std::cout << "updateMatchState done" << std::endl;

    // 3. 获取区域号（原MFC GetAreaNo）
    m_areaNo = AreaDivider::getAreaNo(ball.pos, m_field);
    std::cout << "areaNo: " << m_areaNo << std::endl;

    // 4. 任务分解（原MFC taskDecompose）
    taskDecompose(m_areaNo, ball);
    std::cout << "taskDecompose done, formationNo: " << m_formationNo << std::endl;
    // ========== 5. 队形解释 - 根据比赛状态选择特殊队形 ==========
    switch (m_matchState) {
    case MatchState::STATE_PENALTY:
        // 点球状态
        if (m_isOurKickoff) {
            m_roles = Formation::getPenaltyFormation(true, m_field);
        }
        else {
            m_roles = Formation::getPenaltyFormation(false, m_field);
        }
        break;

    case MatchState::STATE_GOALKICK:
        // 门球状态
        m_roles = Formation::getGoalKickFormation(m_isOurKickoff, m_field);
        break;

    case MatchState::STATE_FREEBALL:
        // 争球状态
        m_roles = Formation::getFreeBallFormation(ball.pos, m_field);
        break;

    case MatchState::STATE_KICKOFF:
        // 开球状态
        m_roles = Formation::getKickoffFormation(m_isOurKickoff, m_field);
        break;

    case MatchState::STATE_CORNER:
        // 角球状态
        m_roles = Formation::getCornerFormation(ball, m_field);
        break;

    case MatchState::STATE_BOUNDARY:
        // 边界球状态
        m_roles = Formation::getBoundaryFormation(ball, m_field);
        break;

    case MatchState::STATE_FREEKICK:
        // 任意球队形
        m_roles = Formation::getFreeKickFormation(m_isOurKickoff, m_field);
        break;

    default:
        // 正常比赛状态，使用区域对应的队形
        formInterpret(m_formationNo, ball);
        break;
    }

    // 应用单双后卫调整
    if (m_isSingleDefender && m_matchState == MatchState::STATE_NORMAL) {
        for (auto& role : m_roles) {
            if (role.roleId == ROLE_SPECIAL_DEFENDER_DOWN) {
                role.roleId = ROLE_WAIT_SUPPORT;
                role.name = "单后卫-支援";
                break;
            }
        }
    }

    // 6. 角色分配（原MFC charAllot）
    charAllot(robots, 5);
    std::cout << "charAllot done" << std::endl;


    // 7. 机器人管理（原MFC robotManager）
    robotManager(robots, oppRobots, ball, velocities);
    std::cout << "robotManager done" << std::endl;

    // 8. 动作执行（原MFC actProcess）
    actProcess(robots, oppRobots, ball, velocities);
    std::cout << "actProcess done" << std::endl;
}

// ==================== 辅助函数 ====================

void UnifiedStrategy::updateMatchState(const BallInfo& ball) {
    // 简化版：根据比赛规则判断状态
    // 实际应该从视觉系统获取

    if (m_stateTimer > 0) {
        m_stateTimer--;
        if (m_stateTimer == 0) {
            m_matchState = MatchState::STATE_NORMAL;
        }
    }
}

void UnifiedStrategy::updateTacticalPhase(const BallInfo& ball) {
    if (m_field.isInOppHalf(ball.pos)) {
        if (ball.velocity > 30 && ball.vel_x > 0) {
            m_tacticalPhase = TacticalPhase::PHASE_ATTACK;
        }
        else {
            m_tacticalPhase = TacticalPhase::PHASE_PRESSURE;
        }
    }
    else if (m_field.isInOurHalf(ball.pos)) {
        if (ball.velocity > 25 && ball.vel_x > 0) {
            m_tacticalPhase = TacticalPhase::PHASE_COUNTER;
        }
        else if (ball.pos.x < 40) {
            m_tacticalPhase = TacticalPhase::PHASE_DEFENSE;
        }
        else {
            m_tacticalPhase = TacticalPhase::PHASE_POSSESSION;
        }
    }
}

bool UnifiedStrategy::isInAttackState(const BallInfo& ball) {

    if (m_field.isInOppHalf(ball.pos)) {
        return true;
    }

    if (ball.velocity > 20 && ball.vel_x > 0) {
        return true;
    }

    return m_tacticalPhase == TacticalPhase::PHASE_ATTACK ||
        m_tacticalPhase == TacticalPhase::PHASE_COUNTER;
}

bool UnifiedStrategy::isInDefenseState(const BallInfo& ball) {
    Point stdBall = m_field.transformToStandard(ball.pos);

    if (stdBall.x < 50) {
        return true;
    }

    if (ball.velocity > 20 && ball.vel_x < 0) {
        return true;
    }

    return m_tacticalPhase == TacticalPhase::PHASE_DEFENSE;
}

bool UnifiedStrategy::isNearBoundary(const Point& pos) {
    double margin = 12.0;
    return (pos.x < margin || pos.x > m_field.getFieldWidth() - margin ||
        pos.y < margin || pos.y > m_field.getFieldHeight() - margin);
}

bool UnifiedStrategy::isCornerSituation(const Point& pos) {
    double margin = 15.0;
    return (pos.x < margin && pos.y < margin) ||
        (pos.x < margin && pos.y > m_field.getFieldHeight() - margin) ||
        (pos.x > m_field.getFieldWidth() - margin && pos.y < margin) ||
        (pos.x > m_field.getFieldWidth() - margin && pos.y > m_field.getFieldHeight() - margin);
}

bool UnifiedStrategy::shouldPass(const RobotPose& robot, const BallInfo& ball) {
    double distToBall = pointToPointDistance(robot, ball.pos);
    return distToBall < 12 && distToBall > 3 && ball.velocity < 20;
}

PassOption UnifiedStrategy::evaluatePassOptions(const RobotPose& passer, const BallInfo& ball,
    const RobotPose receivers[], int receiverCount,
    const Point opponents[], int opponentCount) {
    // 转换 opponents 为 RobotPose 数组
    RobotPose oppPoses[5];
    for (int i = 0; i < opponentCount && i < 5; i++) {
        oppPoses[i].x = opponents[i].x;
        oppPoses[i].y = opponents[i].y;
        oppPoses[i].theta = 0;
        oppPoses[i].vx = 0;
        oppPoses[i].vy = 0;
        oppPoses[i].vtheta = 0;
    }
    return m_passCoordinator.evaluatePass(passer, ball, receivers, receiverCount, oppPoses, opponentCount);
}

double UnifiedStrategy::calculatePerformanceScore(const RobotPose& robot, const Role& role) {
    double score = 0;
    double dist = pointToPointDistance(robot, role.targetPos);
    score -= dist * 0.1;

    if (role.roleId == ROLE_GOALIE) {
        score -= dist * 0.2;
    }
    else if (role.roleId == ROLE_SHOOT) {
        double goalDist = pointToPointDistance(robot, m_field.getOppGoalPos());
        score -= goalDist * 0.05;
    }

    return score;
}

Point UnifiedStrategy::getDynamicTarget(const Role& role, const BallInfo& ball) {
    Point target = role.targetPos;

    if (role.roleId == ROLE_WAIT_HORIZONTAL || role.roleId == ROLE_WAIT_HORIZONTAL_2) {
        target.y = ball.pos.y + ball.vel_y * 0.2;
        target.y = std::max(20.0, std::min(m_field.getFieldHeight() - 20.0, target.y));
    }

    return target;
}

void UnifiedStrategy::reset() {
    m_frameCount = 0;
    m_stateTimer = 0;
    m_matchState = MatchState::STATE_NORMAL;
    m_tacticalPhase = TacticalPhase::PHASE_DEFENSE;
    m_ballHistory.clear();

    for (int i = 0; i < 5; i++) {
        m_robotHistory[i].positions.clear();
        m_assignedRoles[i] = ROLE_STOP;
    }
}

void UnifiedStrategy::setParameter(const std::string& key, double value) {
    std::cout << "UnifiedStrategy::setParameter: " << key << " = " << value << std::endl;

    // 运动控制参数
    if (key == "max_speed") {
        m_params.max_speed = value;
        std::cout << "max_speed updated to: " << m_params.max_speed << std::endl;
    }
    else if (key == "min_speed") {
        m_params.min_speed = value;
        std::cout << "min_speed updated to: " << m_params.min_speed << std::endl;
    }
    else if (key == "kp_pos") {
        m_params.kp_pos = value;
        std::cout << "kp_pos updated to: " << m_params.kp_pos << std::endl;
    }
    else if (key == "kp_angle") {
        m_params.kp_angle = value;
        std::cout << "kp_angle updated to: " << m_params.kp_angle << std::endl;
    }
    else if (key == "kd_pos") {
        m_params.kd_pos = value;
        std::cout << "kd_pos updated to: " << m_params.kd_pos << std::endl;
    }
    else if (key == "kd_angle") {
        m_params.kd_angle = value;
        std::cout << "kd_angle updated to: " << m_params.kd_angle << std::endl;
    }
    // 战术参数
    else if (key == "attack_aggression") {
        m_attackAggression = value;
        std::cout << "attack_aggression updated to: " << m_attackAggression << std::endl;
    }
    else if (key == "defense_depth") {
        m_defenseDepth = value;
        std::cout << "defense_depth updated to: " << m_defenseDepth << std::endl;  
    }
    else if (key == "pressing_intensity") {
        m_pressingIntensity = value;
        std::cout << "pressing_intensity updated to: " << m_pressingIntensity << std::endl;  
    }
    // 射门参数
    else if (key == "shoot_power") {
        m_params.shoot_power = value;
        std::cout << "shoot_power updated to: " << m_params.shoot_power << std::endl;
    }
    else if (key == "shoot_angle_tolerance") {
        m_params.shoot_angle_tolerance = value;
        std::cout << "shoot_angle_tolerance updated to: " << m_params.shoot_angle_tolerance << std::endl;
    }
    // 传球参数
    else if (key == "pass_success_threshold") {
        m_params.pass_success_threshold = value;
        std::cout << "pass_success_threshold updated to: " << m_params.pass_success_threshold << std::endl;
    }
    // 门将参数
    else if (key == "goalie_aggression") {
        m_params.goalie_aggression = value;
        std::cout << "goalie_aggression updated to: " << m_params.goalie_aggression << std::endl;
    }
    else if (key == "goalie_speed") {
        m_params.goalie_speed = value;
        std::cout << "goalie_speed updated to: " << m_params.goalie_speed << std::endl;
    }
    // 避障参数
    else if (key == "avoid_distance") {
        m_params.avoid_distance = value;
        std::cout << "avoid_distance updated to: " << m_params.avoid_distance << std::endl;
    }
    else if (key == "avoid_weight") {
        m_params.avoid_weight = value;
        std::cout << "avoid_weight updated to: " << m_params.avoid_weight << std::endl;
    }
    // 场地配置
    else if (key == "our_goal_right") {
        m_field.setOurGoalSide(value > 0);
    }
    else if (key == "use_offside_trap") {
        m_useOffsideTrap = value > 0;
    }
    else if (key == "use_zonal_defense") {
        m_useZonalDefense = value > 0;
    }
    else {
        std::cout << "Warning: Unknown parameter key: " << key << std::endl;
    }

    ParameterTuning::getInstance().setParameter(key, value);
}
void UnifiedStrategy::setControlParam(const std::string& key, double value) {

    setParameter(key, value);
}
double UnifiedStrategy::getParameter(const std::string& key) const {
    if (key == "max_speed") return m_params.max_speed;
    if (key == "min_speed") return m_params.min_speed;
    if (key == "kp_pos") return m_params.kp_pos;
    if (key == "kp_angle") return m_params.kp_angle;
    if (key == "kd_pos") return m_params.kd_pos;
    if (key == "kd_angle") return m_params.kd_angle;
    if (key == "shoot_power") return m_params.shoot_power;
    if (key == "shoot_angle_tolerance") return m_params.shoot_angle_tolerance;
    if (key == "pass_success_threshold") return m_params.pass_success_threshold;
    if (key == "goalie_aggression") return m_params.goalie_aggression;
    if (key == "goalie_speed") return m_params.goalie_speed;
    if (key == "avoid_distance") return m_params.avoid_distance;
    if (key == "avoid_weight") return m_params.avoid_weight;

    // 战术参数（单独存储的）
    if (key == "attack_aggression") return m_attackAggression;
    if (key == "defense_depth") return m_defenseDepth;
    if (key == "pressing_intensity") return m_pressingIntensity;
    if (key == "use_offside_trap") return m_useOffsideTrap ? 1.0 : 0.0;
    if (key == "use_zonal_defense") return m_useZonalDefense ? 1.0 : 0.0;
    return 0.0;
}
double UnifiedStrategy::getControlParam(const std::string& key) const {
    if (key == "max_speed") return m_params.max_speed;
    if (key == "kp_pos") return m_params.kp_pos;
    if (key == "kp_angle") return m_params.kp_angle;
    if (key == "attack_aggression") return m_attackAggression;
    if (key == "defense_depth") return m_defenseDepth;
    if (key == "pressing_intensity") return m_pressingIntensity;
    return 0;
}

bool UnifiedStrategy::saveConfig(const std::string& filename) {
    return ParameterTuning::getInstance().saveToFile(filename);
}

bool UnifiedStrategy::loadConfig(const std::string& filename) {
    return ParameterTuning::getInstance().loadFromFile(filename);
}
void UnifiedStrategy::applyStrategyConfig(const StrategyConfig& config) {
    // 应用进攻侵略性
    m_attackAggression = config.attackAggression;

    // 应用防守深度
    m_defenseDepth = config.defenseDepth;

    // 应用压迫强度
    m_pressingIntensity = config.pressingIntensity;

    // 应用越位陷阱设置
    m_useOffsideTrap = config.useOffsideTrap;

    // 应用区域防守设置
    m_useZonalDefense = config.useZonalDefense;

    // 根据侵略性调整控制参数
    m_params.kp_pos = 12.0 + (m_attackAggression - 0.6) * 10.0;
    m_params.kp_angle = 22.0 + (m_attackAggression - 0.6) * 15.0;
    m_params.max_speed = 75.0 + (m_attackAggression - 0.6) * 25.0;

    // 限制参数范围
    if (m_params.kp_pos < 8.0) m_params.kp_pos = 8.0;
    if (m_params.kp_pos > 25.0) m_params.kp_pos = 25.0;
    if (m_params.kp_angle < 15.0) m_params.kp_angle = 15.0;
    if (m_params.kp_angle > 40.0) m_params.kp_angle = 40.0;
    if (m_params.max_speed < 60.0) m_params.max_speed = 60.0;
    if (m_params.max_speed > 100.0) m_params.max_speed = 100.0;
}
void UnifiedStrategy::setKickoffType(int type) {
    // 根据开球类型设置比赛状态
    switch (type) {
    case 0: // 普通
        m_matchState = MatchState::STATE_NORMAL;
        break;
    case 1: // 点球
        m_matchState = MatchState::STATE_PENALTY;
        break;
    case 2: // 门球
        m_matchState = MatchState::STATE_GOALKICK;
        break;
    case 3: // 任意球
        m_matchState = MatchState::STATE_FREEKICK;
        break;
    case 4: // 争球
        m_matchState = MatchState::STATE_FREEBALL;
        break;
    case 5: // 收车
        parkRobots(); break;
    default: break; // 收车
    }
}

void UnifiedStrategy::setFormationType(bool isSingleDefender) {
    m_isSingleDefender = isSingleDefender;
    Formation::setSingleDefender(isSingleDefender);
}

// 点球模式设置
void UnifiedStrategy::setPenaltyKickMode(int direction, int mode) {
    m_penaltyDirection = direction;  // 0左晃,1直冲,2右晃
    m_goaliePosition = mode;         // 守门位置
    Goalie::setGoaliePosition(mode); // 设置守门员位置
}

// 策略选择
void UnifiedStrategy::selectStrategy(int strategyIndex) {
    m_currentStrategy = strategyIndex;
    // 根据策略索引调整战术参数
    switch (strategyIndex) {
    case 0: // 1号策略：进攻型 - 积极进攻，防线靠前
        m_attackAggression = 0.85;
        m_defenseDepth = 0.3;
        m_pressingIntensity = 0.8;
        break;

    case 1: // 2号策略：防守型 - 稳固防守，防线靠后
        m_attackAggression = 0.3;
        m_defenseDepth = 0.8;
        m_pressingIntensity = 0.3;
        break;

    case 2: // 3号策略：平衡型 - 攻守平衡
        m_attackAggression = 0.6;
        m_defenseDepth = 0.5;
        m_pressingIntensity = 0.5;
        break;

    case 3: // 4号策略：保守型 - 稳妥为主，控制节奏
        m_attackAggression = 0.4;
        m_defenseDepth = 0.6;
        m_pressingIntensity = 0.4;
        break;

    default:
        break;
    }
}
void UnifiedStrategy::setMatchState(int state) {
    m_matchState = static_cast<MatchState>(state);
}
void UnifiedStrategy::parkRobots() {
    // 收车：所有机器人停止并移动到指定位置
    m_matchState = MatchState::STATE_NORMAL;
    // 实际收车逻辑由上层控制，这里只设置标志
}
void UnifiedStrategy::startMatch() {
    m_isMatchRunning = true;
}

void UnifiedStrategy::stopMatch() {
    m_isMatchRunning = false;
}

bool UnifiedStrategy::saveConfig(const char* filename) {
    return saveConfig(std::string(filename));
}

bool UnifiedStrategy::loadConfig(const char* filename) {
    return loadConfig(std::string(filename));
}