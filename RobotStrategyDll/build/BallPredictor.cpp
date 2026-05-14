// BallPredictor.cpp
// 球轨迹预测模块实现

#include "BallPredictor.h"
#include <cmath>

using namespace GeometryUtils;

// ========== 基于历史轨迹预测球的位置 ==========
// 算法：计算最近7个点的平均速度，然后线性外推
Point BallPredictor::predictPosition(const std::vector<Point>& history, int steps) {
    // 历史数据不足，返回最后一个位置
    if (history.size() < 2) {
        return history.empty() ? Point() : history.back();
    }

    // 简单线性预测：计算平均速度
    int size = static_cast<int>(history.size());
    double avgVx = 0, avgVy = 0;

    // 使用最近7个点计算平均速度
    for (int i = 1; i < size && i <= 7; i++) {
        avgVx += (history[size - i].x - history[size - i - 1].x);
        avgVy += (history[size - i].y - history[size - i - 1].y);
    }

    int count = std::min(size - 1, 7);
    if (count > 0) {
        avgVx /= count;
        avgVy /= count;
    }

    // 外推预测
    Point last = history.back();
    return Point(last.x + avgVx * steps, last.y + avgVy * steps);
}

// ========== 预测球与指定竖直直线的交点 ==========
// 用于预测球与球门线的交点（判断是否进门）
Point BallPredictor::predictIntersection(const BallInfo& ball, double x, const FieldGeometry& field) {
    // 水平速度为零，无法计算交点
    if (ball.vel_x == 0) {
        return ball.pos;
    }

    // 计算到达x坐标的时间
    double t = (x - ball.pos.x) / ball.vel_x;
    double y = ball.pos.y + ball.vel_y * t;

    // 限制在场地范围内
    y = std::max(0.0, std::min(field.getFieldHeight(), y));

    return Point(x, y);
}

// ========== 预测球是否将进入门区 ==========
bool BallPredictor::willEnterGoalArea(const BallInfo& ball, const FieldGeometry& field) {
    // 预测10个时间步后的位置
    Point predicted = predictPosition({ball.pos}, 10);
    return field.isInGoalArea(predicted);
}

// ========== 预测球是否将出界 ==========
// 算法：计算球与四条边界的交点，取最小正时间
bool BallPredictor::willGoOutOfBounds(const BallInfo& ball, const FieldGeometry& field, Point& outPoint) {
    double fieldW = field.getFieldWidth();
    double fieldH = field.getFieldHeight();

    // 球静止，不会出界
    if (ball.vel_x == 0 && ball.vel_y == 0) {
        return false;
    }

    // 计算与四条边界的交点时间
    double tx_left = (0 - ball.pos.x) / ball.vel_x;           // 左边界
    double tx_right = (fieldW - ball.pos.x) / ball.vel_x;     // 右边界
    double ty_bottom = (0 - ball.pos.y) / ball.vel_y;         // 下边界
    double ty_top = (fieldH - ball.pos.y) / ball.vel_y;       // 上边界

    double t_min = 1e6;
    int hit_edge = -1;

    // 找出最小正时间
    if (tx_left > 0 && tx_left < t_min) { t_min = tx_left; hit_edge = 0; }
    if (tx_right > 0 && tx_right < t_min) { t_min = tx_right; hit_edge = 1; }
    if (ty_bottom > 0 && ty_bottom < t_min) { t_min = ty_bottom; hit_edge = 2; }
    if (ty_top > 0 && ty_top < t_min) { t_min = ty_top; hit_edge = 3; }

    // 20个时间步内会出界（约0.66秒）
    if (hit_edge >= 0 && t_min < 20) {
        outPoint.x = ball.pos.x + ball.vel_x * t_min;
        outPoint.y = ball.pos.y + ball.vel_y * t_min;
        return true;
    }

    return false;
}

// ========== 更新历史记录 ==========
void BallPredictor::updateHistory(const Point& pos, std::vector<Point>& history, int maxSize) {
    history.push_back(pos);
    // 保持历史记录不超过最大长度
    while ((int)history.size() > maxSize) {
        history.erase(history.begin());
    }
}

// ========== 计算球的速度和运动方向 ==========
void BallPredictor::calculateVelocity(const std::vector<Point>& history, double& velocity, double& angle) {
    if (history.size() < 2) {
        velocity = 0;
        angle = 0;
        return;
    }

    int size = static_cast<int>(history.size());
    double dx = history[size - 1].x - history[0].x;
    double dy = history[size - 1].y - history[0].y;
    double dt = (size - 1) * 0.033;  // 假设每帧33ms

    // 计算速度大小和方向
    velocity = sqrt(dx*dx + dy*dy) / dt;
    angle = atan2(dy, dx);
}
