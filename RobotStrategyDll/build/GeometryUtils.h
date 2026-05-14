// GeometryUtils.h
// 几何工具库 - 提供点、线、圆等几何结构的定义和常用几何计算函数
#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
/**
 * 二维点结构体
 * 表示平面上的一个点，包含x和y坐标
 */
struct Point {// 点的x坐标和y坐标
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
};

// 直线方程 ax + by + c = 0
struct Line {
    double a, b, c;
    Line(double a = 0, double b = 0, double c = 0) : a(a), b(b), c(c) {}
};

// 圆方程 (x - cx)^2 + (y - cy)^2 = r^2
struct Circle {// 圆心x坐标、圆心y坐标、半径

    double cx, cy, r;
    Circle(double cx = 0, double cy = 0, double r = 0) : cx(cx), cy(cy), r(r) {}
};
/**
 * 机器人位姿结构体
 * 包含机器人的位置、朝向和速度信息
 */
struct RobotPose {
    double x, y, theta;   // 机器人位置(x,y)和朝向角theta(弧度)
    double vx, vy;        // 机器人在x和y方向上的速度
    double vtheta;        // 机器人的角速度

    RobotPose(double x = 0, double y = 0, double theta = 0,
              double vx = 0, double vy = 0, double vtheta = 0)
        : x(x), y(y), theta(theta), vx(vx), vy(vy), vtheta(vtheta) {}
     // 转换为Point类型（丢弃角度信息）
    operator Point() const {
        return Point(x, y);}
};

namespace GeometryUtils {
/**
 * 将角度规范化到[-π, π]区间
 * @param angle 输入角度（弧度）
 * @return 规范化后的角度
 */
// 角度规范化
inline double normalizeAngle(double angle) {
    angle = fmod(angle, 2 * M_PI); // 取模，映射到[0, 2π)
    if (angle > M_PI) angle -= 2 * M_PI;// 超过π映射到负半轴
    if (angle < -M_PI) angle += 2 * M_PI;
    return angle;
}
/**
 * 将角度规范化到[0, 2π]区间
 * @param angle 输入角度（弧度）
 * @return 规范化后的角度
 */
inline double trimAngle2PI(double angle) {
    angle = fmod(angle, 2 * M_PI);
    if (angle < 0) angle += 2 * M_PI;
    return angle;
}
/**
 * 计算两点之间的距离
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 欧氏距离
 */
// 两点距离
inline double pointToPointDistance(const Point& p1, const Point& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return sqrt(dx*dx + dy*dy);
}
// 重载：支持 RobotPose 参数
inline double pointToPointDistance(const RobotPose& p1, const Point& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return sqrt(dx*dx + dy*dy);
}
// 重载：两个RobotPose参数
inline double pointToPointDistance(const RobotPose& p1, const RobotPose& p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return sqrt(dx*dx + dy*dy);
}

/**
 * 计算点到直线的距离
 * @param p 点
 * @param line 直线
 * @return 点到直线的垂直距离
 */
// 点到直线距离
double pointToLineDistance(const Point& p, const Line& line);
/**
 * 计算点到直线的垂直投影点
 * @param p 点
 * @param line 直线
 * @return 投影点坐标
 */
// 点到直线的垂直投影点
Point pointPerpendicularToLine(const Point& p, const Line& line);
/**
 * 根据两点确定直线方程
 * @param p1 第一个点
 * @param p2 第二个点
 * @return 通过两点的直线
 */
// 两点确定直线
Line lineFromPoints(const Point& p1, const Point& p2);
/**
 * 过一点作已知直线的垂线
 * @param p 点
 * @param line 已知直线
 * @return 垂线方程
 */
// 过一点作已知直线的垂线
Line perpendicularLineThroughPoint(const Point& p, const Line& line);

/**
 * 计算两条直线的交点
 * @param l1 第一条直线
 * @param l2 第二条直线
 * @param cross 输出参数，交点坐标
 * @return -1: 平行无交点, 0: 有唯一交点
 */
// 两直线交点
int twoLinesCrossPoint(const Line& l1, const Line& l2, Point& cross);
/**
 * 计算直线与圆的交点
 * @param line 直线
 * @param circle 圆
 * @param p1 输出参数，第一个交点
 * @param p2 输出参数，第二个交点
 * @return -1: 错误, 0: 无交点, 1: 相切, 2: 两个交点
 */
// 直线与圆的交点
int lineCircleCross(const Line& line, const Circle& circle, Point& p1, Point& p2);

// 两圆交点
int twoCirclesCross(const Circle& c1, const Circle& c2, Point& p1, Point& p2);

// 点P绕点O旋转angle弧度
Point rotatePoint(const Point& p, const Point& o, double angle);

// 计算两点间角度（从p1指向p2）
inline double angleToPoint(const Point& from, const Point& to) {
    return trimAngle2PI(atan2(to.y - from.y, to.x - from.x));
}
// 重载：支持 RobotPose 作为起点
inline double angleToPoint(const RobotPose& from, const Point& to) {
    return trimAngle2PI(atan2(to.y - from.y, to.x - from.x));
}

// 限制数值范围
inline double limit(double value, double maxVal) {
    if (value > maxVal) return maxVal;
    if (value < -maxVal) return -maxVal;
    return value;
}

// 线性插值
inline double lerp(double a, double b, double t) {
    return a + (b - a) * t;
}
}

#endif
