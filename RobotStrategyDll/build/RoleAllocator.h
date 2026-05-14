// RoleAllocator.h
// 角色分配器 - 将战术角色分配给具体机器人

#ifndef ROLEALLOCATOR_H
#define ROLEALLOCATOR_H

#include "FieldGeometry.h"
#include "RoleTable.h"
#include <vector>

class RoleAllocator {
public:
    // ========== 守门员选择 ==========

    // 选择守门员（离球门最近的机器人）
    // robots: 机器人位姿数组
    // robotCount: 机器人数量
    // field: 场地几何信息
    // 返回: 守门员机器人索引
    static int selectGoalie(const RobotPose robots[], int robotCount,
                            const FieldGeometry& field);

    // ========== 角色分配 ==========

    // 分配角色（基于匈牙利算法）
    // robots: 机器人位姿数组
    // robotCount: 机器人数量
    // roles: 需要分配的角色列表
    // assignedRoles: 输出数组，每个机器人分配的角色ID
    static void assignRoles(const RobotPose robots[], int robotCount,
                            const std::vector<Role>& roles,
                            int assignedRoles[]);

    // ========== 性能评估 ==========

    // 计算机器人执行角色的性能分数
    // robot: 机器人位姿
    // role: 角色
    // 返回: 性能分数（越高越适合）
    static double calculatePerformance(const RobotPose& robot, const Role& role);
};

#endif
