// GeometryUtils.cpp
// 几何工具库的实现文件

#include "GeometryUtils.h"
#include <cmath>

namespace GeometryUtils {

/**
 * 计算点到直线的距离
 * 使用公式: distance = |a*x0 + b*y0 + c| / sqrt(a^2 + b^2)
 */
double pointToLineDistance(const Point& p, const Line& line) {
    return fabs(line.a * p.x + line.b * p.y + line.c) / sqrt(line.a * line.a + line.b * line.b);
}

/**
 * 计算点到直线的垂直投影点
 * 通过解方程组实现：投影点同时在直线上，且与原点连线垂直于直线
 */
Point pointPerpendicularToLine(const Point& p, const Line& line) {
    double denominator = line.a * line.a + line.b * line.b;
    if (denominator == 0) return p;  // 直线无效，返回原点
    // 计算投影点坐标
    double x = (line.b * (line.b * p.x - line.a * p.y) - line.a * line.c) / denominator;
    double y = (line.a * (-line.b * p.x + line.a * p.y) - line.b * line.c) / denominator;
    return Point(x, y);
}

/**
 * 根据两点确定直线方程
 * 使用向量叉积计算系数：a = y2-y1, b = x1-x2, c = x2*y1 - x1*y2
 */
Line lineFromPoints(const Point& p1, const Point& p2) {
    double a = p2.y - p1.y;      // y方向差值
    double b = p1.x - p2.x;      // x方向差值的相反数
    double c = p2.x * p1.y - p1.x * p2.y;  // 常数项
    return Line(a, b, c);
}

/**
 * 过一点作已知直线的垂线
 * 垂线的系数为：a' = b, b' = -a, c' = -b*x0 + a*y0
 */
Line perpendicularLineThroughPoint(const Point& p, const Line& line) {
    return Line(line.b, -line.a, -line.b * p.x + line.a * p.y);
}

/**
 * 计算两条直线的交点
 * 使用克莱姆法则求解联立方程
 */
int twoLinesCrossPoint(const Line& l1, const Line& l2, Point& cross) {
    double det = l1.a * l2.b - l2.a * l1.b;  // 行列式
    if (fabs(det) < 1e-10) return -1;        // 平行或重合

    // 克莱姆法则求解
    cross.x = (l1.b * l2.c - l2.b * l1.c) / det;
    cross.y = (l2.a * l1.c - l1.a * l2.c) / det;
    return 0;
}

/**
 * 计算直线与圆的交点
 * 算法：将直线方程代入圆方程，求解二次方程
 */
int lineCircleCross(const Line& line, const Circle& circle, Point& p1, Point& p2) {
    // 将圆平移到原点，简化计算
    double a = line.a;
    double b = line.b;
    double c = line.c + line.a * circle.cx + line.b * circle.cy;

    double denominator = a * a + b * b;
    if (denominator == 0) return -1;  // 直线无效

    // 计算投影点
    double x0 = -a * c / denominator;
    double y0 = -b * c / denominator;

    // 计算判别式
    double d = circle.r * circle.r - c * c / denominator;
    if (d < 0) return 0;      // 无交点
    if (d == 0) {             // 相切
        p1.x = x0 - circle.cx;
        p1.y = y0 - circle.cy;
        p1.x += circle.cx;
        p1.y += circle.cy;
        return 1;
    }

    // 两个交点
    double mult = sqrt(d / denominator);
    double ax = x0 + b * mult;
    double ay = y0 - a * mult;
    double bx = x0 - b * mult;
    double by = y0 + a * mult;

    p1.x = ax - circle.cx;
    p1.y = ay - circle.cy;
    p1.x += circle.cx;
    p1.y += circle.cy;
    p2.x = bx - circle.cx;
    p2.y = by - circle.cy;
    p2.x += circle.cx;
    p2.y += circle.cy;

    return 2;
}

/**
 * 计算两个圆的交点
 * 算法：连接圆心，利用余弦定理计算交点位置
 */
int twoCirclesCross(const Circle& c1, const Circle& c2, Point& p1, Point& p2) {
    double dx = c2.cx - c1.cx;      // 圆心x距离
    double dy = c2.cy - c1.cy;      // 圆心y距离
    double d = sqrt(dx*dx + dy*dy); // 圆心距

    // 无交点条件：圆心距大于半径和 或 圆心距小于半径差
    if (d > c1.r + c2.r + 1e-10) return 0;
    if (d < fabs(c1.r - c2.r) - 1e-10) return 0;

    // 计算交点
    double a = (c1.r * c1.r - c2.r * c2.r + d * d) / (2 * d);  // 投影距离
    double h = sqrt(c1.r * c1.r - a * a);                      // 垂直距离

    double xm = c1.cx + (dx * a) / d;   // 交点中点x坐标
    double ym = c1.cy + (dy * a) / d;   // 交点中点y坐标

    // 计算两个交点
    p1.x = xm + (dy * h) / d;
    p1.y = ym - (dx * h) / d;
    p2.x = xm - (dy * h) / d;
    p2.y = ym + (dx * h) / d;

    return 2;
}

/**
 * 点绕指定点旋转
 * 使用旋转矩阵：x' = x*cosθ - y*sinθ, y' = x*sinθ + y*cosθ
 */
Point rotatePoint(const Point& p, const Point& o, double angle) {
    double cosA = cos(angle);
    double sinA = sin(angle);
    double dx = p.x - o.x;   // 相对x坐标
    double dy = p.y - o.y;   // 相对y坐标
    // 旋转后坐标 = 旋转中心 + 旋转后的相对坐标
    return Point(o.x + dx * cosA - dy * sinA,
                 o.y + dx * sinA + dy * cosA);
}

} // namespace GeometryUtils
