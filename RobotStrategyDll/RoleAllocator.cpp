// RoleAllocator.cpp
// 角色分配器实现

#include "RoleAllocator.h"
#include "GeometryUtils.h"

using namespace GeometryUtils;

// ========== 选择守门员 ==========
// 策略：选择离球门最近的机器人作为守门员
int RoleAllocator::selectGoalie(const RobotPose robots[], int robotCount,
                                const FieldGeometry& field) {
    Point goalPos = field.getOurGoalPos();
    double minDist = 1e9;
    int goalieIdx = 0;

    for (int i = 0; i < robotCount && i < 5; i++) {
        double dist = pointToPointDistance(Point(robots[i].x, robots[i].y), goalPos);
        if (dist < minDist) {
            minDist = dist;
            goalieIdx = i;
        }
    }
    return goalieIdx;
}

// ========== 分配角色 ==========
// 使用贪心算法：为每个角色选择最合适的机器人
void RoleAllocator::assignRoles(const RobotPose robots[], int robotCount,
                                const std::vector<Role>& roles,
                                int assignedRoles[]) {
    bool roleUsed[5] = {false};   // 角色是否已分配
    bool robotUsed[5] = {false};  // 机器人是否已分配
    int roleCount = (int)roles.size();
    if (roleCount > 5) roleCount = 5;

    // 贪心分配：为每个角色选择最合适的机器人
    for (int r = 0; r < roleCount; r++) {
        int bestRobot = -1;
        double bestScore = -1e9;

        for (int i = 0; i < robotCount && i < 5; i++) {
            if (robotUsed[i]) continue;
            double score = calculatePerformance(robots[i], roles[r]);
            if (score > bestScore) {
                bestScore = score;
                bestRobot = i;
            }
        }

        if (bestRobot >= 0) {
            assignedRoles[bestRobot] = roles[r].roleId;
            robotUsed[bestRobot] = true;
            roleUsed[r] = true;
        }
    }

    // 为剩余机器人分配未使用的角色
    for (int i = 0; i < robotCount && i < 5; i++) {
        if (robotUsed[i]) continue;
        for (int r = 0; r < roleCount; r++) {
            if (!roleUsed[r]) {
                assignedRoles[i] = roles[r].roleId;
                roleUsed[r] = true;
                break;
            }
        }
    }
}

// ========== 计算机器人执行角色的性能分数 ==========
// 分数 = -距离 * 权重
double RoleAllocator::calculatePerformance(const RobotPose& robot, const Role& role) {
    double score = 0;
    double dist = pointToPointDistance(Point(robot.x, robot.y), role.targetPos);
    score -= dist * 0.1;  // 距离惩罚

    // 守门员角色：距离权重更高
    if (role.roleId == ROLE_GOALIE) {
        score -= dist * 0.2;
    }

    return score;
}
