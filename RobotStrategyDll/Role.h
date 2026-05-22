// Role.h
// 角色定义 - 描述机器人在战术中扮演的角色

#ifndef ROLE_H
#define ROLE_H

#include "GeometryUtils.h"
#include <string>

/**
 * 角色结构体
 * 描述一个机器人需要执行的任务
 */
struct Role {
    int roleId;           // 角色ID，参见RoleTable.h中的RoleID枚举
    Point targetPos;      // 目标位置（世界坐标系）
    double priority;      // 优先级（0-1），越高越重要
    std::string name;     // 角色名称，用于调试

    // 默认构造函数
    Role() : roleId(0), targetPos(0,0), priority(0), name("") {}

    // 带参数的构造函数
    Role(int id, Point pos, double pri, const std::string& n)
        : roleId(id), targetPos(pos), priority(pri), name(n) {}
};

#endif
