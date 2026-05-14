// BallPredictor.h
// 球轨迹预测模块 - 预测球的未来位置和运动状态
#ifndef BALLPREDICTOR_H
#define BALLPREDICTOR_H

#include "StrategyCore.h"
#include "FieldGeometry.h"
#include <vector>

class BallPredictor {
public:
    // ========== 位置预测 ==========

    // 基于历史轨迹预测球的位置
    // history: 球的历史位置序列（时间顺序）
    // steps: 预测步数（每步约33ms）
    // 返回: 预测位置
    static Point predictPosition(const std::vector<Point>& history, int steps);

    // 预测球与指定竖直直线的交点
    // ball: 球的当前信息
    // x: 直线x坐标
    // field: 场地几何信息
    // 返回: 交点坐标
    static Point predictIntersection(const BallInfo& ball, double x, const FieldGeometry& field);

    // ========== 事件预测 ==========

    // 预测球是否将进入门区
    // ball: 球的当前信息
    // field: 场地几何信息
    // 返回: true-将会进入门区
    static bool willEnterGoalArea(const BallInfo& ball, const FieldGeometry& field);

    // 预测球是否将出界
    // ball: 球的当前信息
    // field: 场地几何信息
    // outPoint: 输出参数，出界点坐标
    // 返回: true-将会出界
    static bool willGoOutOfBounds(const BallInfo& ball, const FieldGeometry& field, Point& outPoint);

    // ========== 历史管理 ==========

    // 更新历史记录
    // pos: 当前位置
    // history: 历史记录容器（输入输出）
    // maxSize: 最大历史记录数
    static void updateHistory(const Point& pos, std::vector<Point>& history, int maxSize);

    // ========== 速度计算 ==========

    // 计算球的速度和运动方向
    // history: 历史位置序列
    // velocity: 输出参数，速度大小
    // angle: 输出参数，运动方向角
    static void calculateVelocity(const std::vector<Point>& history, double& velocity, double& angle);
};

#endif
