// FieldGeometry.cpp
// 场地几何类的实现文件

#include "FieldGeometry.h"
#include <algorithm>

using namespace GeometryUtils;

/**
 * 构造函数：初始化默认场地参数
 * 标准场地尺寸：宽220cm，高180cm
 * 默认我方球门在右侧
 */
FieldGeometry::FieldGeometry()
    : m_isOurGoalOnRight(true)           // 默认我方球门在右侧
    , m_fieldWidth(220.0)                // 场地宽度220cm
    , m_fieldHeight(180.0)               // 场地高度180cm
    , m_goalWidth(40.0)                  // 球门宽度40cm
    , m_goalAreaWidth(50.0)              // 门区宽度50cm
    , m_goalAreaHeight(15.0)             // 门区高度15cm
    , m_penaltyAreaWidth(80.0)           // 罚球区宽度80cm
    , m_penaltyAreaHeight(35.0)          // 罚球区高度35cm
    , m_penaltyArcLength(25.0)           // 罚球弧长度25cm
    , m_penaltyArcHeight(5.0)            // 罚球弧高度5cm
    , m_centerCircleRadius(25.0)         // 中圈半径25cm
    , m_wallHeight(5.0)                  // 围墙高度5cm
    , m_wallThickness(2.5) {             // 围墙厚度2.5cm

    // 计算球门的上下边界位置
    m_goalUpLine = m_fieldHeight / 2 + m_goalWidth / 2;    // 球门上沿
    m_goalDownLine = m_fieldHeight / 2 - m_goalWidth / 2;  // 球门下沿
}

/**
 * 获取防守线位置
 * 防守线是后卫线的平均位置，根据球的x坐标动态调整
 * 当球靠近球门时，防守线后撤；球远离时，防守线前压
 */
Point FieldGeometry::getDefenseLinePosition(const Point& ballPos) const {
    Point goalPos = getOurGoalPos();      // 我方球门位置
    Point stdBall = transformToStandard(ballPos);  // 转换到标准坐标系

    // 防守线x坐标：球越靠近球门，防守线越靠后（x越小）
    // 范围：25cm（球门线附近）到 50cm（中场附近）
    double defenseX = 30.0 + (stdBall.x / m_fieldWidth) * 20.0;
    defenseX = std::max(25.0, std::min(50.0, defenseX));

    // 计算防守线y坐标：站在球和球门的连线上
    if (stdBall.x > 10) {
        double slope = (stdBall.y - goalPos.y) / (stdBall.x - goalPos.x);  // 连线斜率
        double defenseY = slope * (defenseX - goalPos.x) + goalPos.y;      // 连线上的y坐标
        // 限制在场地范围内，留出边距
        defenseY = std::max(20.0, std::min(m_fieldHeight - 20.0, defenseY));
        return transformFromStandard(Point(defenseX, defenseY));
    }

    // 球太靠近球门时，站在球门线前防守
    return transformFromStandard(Point(defenseX, goalPos.y));
}

/**
 * 获取进攻支援位置
 * 用于接应球员的站位，通常位于球的侧后方
 */
Point FieldGeometry::getSupportPosition(const Point& ballPos, int robotIndex) const {
    Point stdBall = transformToStandard(ballPos);

    // 支援位置偏移：球的后方20cm，侧方根据机器人索引交替选择左右
    double offsetX = -20.0;  // 球的后方
    double offsetY = (robotIndex % 2 == 0 ? 15.0 : -15.0);  // 交替左右

    // 如果球在对方半场，支援位置可以稍微靠前
    if (stdBall.x > m_fieldWidth / 2) {
        offsetX = -15.0;
    }

    Point supportPos(stdBall.x + offsetX, stdBall.y + offsetY);
    // 限制在场地范围内
    supportPos.x = std::max(10.0, std::min(m_fieldWidth - 10.0, supportPos.x));
    supportPos.y = std::max(10.0, std::min(m_fieldHeight - 10.0, supportPos.y));

    return transformFromStandard(supportPos);
}

/**
 * 获取边界推球目标点
 * 当球靠近边界时，建议踢向此点
 */
Point FieldGeometry::getBoundaryPushTarget(const Point& ballPos) const {
    Point stdBall = transformToStandard(ballPos);

    // 目标点：对方半场的中线位置
    Point target;
    if (m_isOurGoalOnRight) {
        target.x = m_fieldWidth;   // 球门在右侧，推向右方
    } else {
        target.x = 0;              // 球门在左侧，推向左方
    }
    target.y = m_fieldHeight / 2;  // 中线位置

    // 如果球靠近上下边界，踢向对应的角球区
    if (stdBall.y < 30) {
        target.y = m_fieldHeight - 30;  // 踢向上角
    } else if (stdBall.y > m_fieldHeight - 30) {
        target.y = 30;                  // 踢向下角
    }

    return target;
}

/**
 * 判断球是否在门区内（兼容接口）
 */
bool FieldGeometry::isBallInGoalArea(const Point& pos) const {
    return isInGoalArea(pos);
}

/**
 * 判断球是否在罚球区内（兼容接口）
 */
bool FieldGeometry::isBallInPenaltyArea(const Point& pos) const {
    return isInPenaltyArea(pos);
}

/**
 * 设置我方球门在哪一侧
 */
void FieldGeometry::setOurGoalSide(bool onRight) {
    m_isOurGoalOnRight = onRight;
}

/**
 * 交换半场
 * 攻防转换时调用
 */
void FieldGeometry::swapHalves() {
    m_isOurGoalOnRight = !m_isOurGoalOnRight;
}

/**
 * 获取我方球门位置
 */
Point FieldGeometry::getOurGoalPos() const {
    return m_isOurGoalOnRight ? Point(m_fieldWidth, m_fieldHeight / 2)
                              : Point(0, m_fieldHeight / 2);
}

/**
 * 获取对方球门位置
 */
Point FieldGeometry::getOppGoalPos() const {
    return m_isOurGoalOnRight ? Point(0, m_fieldHeight / 2)
                              : Point(m_fieldWidth, m_fieldHeight / 2);
}

/**
 * 获取我方球门线的x坐标
 */
double FieldGeometry::getOurGoalLineX() const {
    return m_isOurGoalOnRight ? m_fieldWidth : 0;
}

/**
 * 获取对方球门线的x坐标
 */
double FieldGeometry::getOppGoalLineX() const {
    return m_isOurGoalOnRight ? 0 : m_fieldWidth;
}

/**
 * 获取球门上沿y坐标
 */
double FieldGeometry::getGoalUpLine() const {
    double goalY = m_fieldHeight / 2;
    return m_isOurGoalOnRight ? goalY + m_goalWidth / 2 : goalY + m_goalWidth / 2;
}

/**
 * 获取球门下沿y坐标
 */
double FieldGeometry::getGoalDownLine() const {
    double goalY = m_fieldHeight / 2;
    return m_isOurGoalOnRight ? goalY - m_goalWidth / 2 : goalY - m_goalWidth / 2;
}

/**
 * 判断点是否在我方半场
 */
bool FieldGeometry::isInOurHalf(const Point& pos) const {
    double midLine = m_fieldWidth / 2;  // 中场线x坐标
    if (m_isOurGoalOnRight) {
        return pos.x > midLine;  // 球门在右侧，x > 中线为我方半场
    } else {
        return pos.x < midLine;  // 球门在左侧，x < 中线为我方半场
    }
}

/**
 * 判断点是否在对方半场
 */
bool FieldGeometry::isInOppHalf(const Point& pos) const {
    return !isInOurHalf(pos);
}

/**
 * 判断点是否在我方门区内
 * 门区定义：球门前的一个矩形区域
 */
bool FieldGeometry::isInGoalArea(const Point& pos) const {
    double goalX = getOurGoalLineX();
    double goalCenterY = m_fieldHeight / 2;

    // 门区矩形边界
    double left = m_isOurGoalOnRight ? goalX - m_goalAreaWidth : goalX;
    double right = m_isOurGoalOnRight ? goalX : goalX + m_goalAreaWidth;
    double bottom = goalCenterY - m_goalAreaHeight / 2;
    double top = goalCenterY + m_goalAreaHeight / 2;

    return pos.x >= left && pos.x <= right && pos.y >= bottom && pos.y <= top;
}

/**
 * 判断点是否在我方罚球区内
 * 罚球区包含矩形区域和罚球弧区域
 */
bool FieldGeometry::isInPenaltyArea(const Point& pos) const {
    double goalX = getOurGoalLineX();
    double goalCenterY = m_fieldHeight / 2;

    // 罚球区矩形边界
    double left = m_isOurGoalOnRight ? goalX - m_penaltyAreaWidth : goalX;
    double right = m_isOurGoalOnRight ? goalX : goalX + m_penaltyAreaWidth;
    double bottom = goalCenterY - m_penaltyAreaHeight / 2;
    double top = goalCenterY + m_penaltyAreaHeight / 2;

    // 检查是否在矩形区域内
    if (pos.x >= left && pos.x <= right && pos.y >= bottom && pos.y <= top) {
        return true;
    }

    // 检查是否在罚球弧内（半圆形区域）
    double arcLeft = m_isOurGoalOnRight ? goalX - m_penaltyArcLength : goalX;
    double arcRight = m_isOurGoalOnRight ? goalX : goalX + m_penaltyArcLength;
    if (pos.x >= arcLeft && pos.x <= arcRight) {
        double dx = m_isOurGoalOnRight ? goalX - pos.x : pos.x - goalX;
        double dy = fabs(pos.y - goalCenterY);
        return dx <= m_penaltyArcHeight && dy <= m_penaltyArcHeight;
    }

    return false;
}

/**
 * 判断点是否在对方门区内
 */
bool FieldGeometry::isInOppGoalArea(const Point& pos) const {
    double goalX = getOppGoalLineX();
    double goalCenterY = m_fieldHeight / 2;

    double left = m_isOurGoalOnRight ? goalX : goalX - m_goalAreaWidth;
    double right = m_isOurGoalOnRight ? goalX + m_goalAreaWidth : goalX;
    double bottom = goalCenterY - m_goalAreaHeight / 2;
    double top = goalCenterY + m_goalAreaHeight / 2;

    return pos.x >= left && pos.x <= right && pos.y >= bottom && pos.y <= top;
}

/**
 * 转换到标准坐标系（我方球门在左侧）
 * 标准坐标系便于算法统一处理
 */
Point FieldGeometry::transformToStandard(const Point& pos) const {
    if (m_isOurGoalOnRight) {
        // 球门在右侧，镜像翻转x坐标
        return Point(m_fieldWidth - pos.x, pos.y);
    }
    return pos;
}

/**
 * 从标准坐标系转换回原始坐标系
 */
Point FieldGeometry::transformFromStandard(const Point& pos) const {
    if (m_isOurGoalOnRight) {
        // 镜像翻转回来
        return Point(m_fieldWidth - pos.x, pos.y);
    }
    return pos;
}
