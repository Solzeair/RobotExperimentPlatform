// FieldGeometry.h
// 场地几何信息 - 定义足球场地的尺寸、门区位置等几何参数

#ifndef FIELDGEOMETRY_H
#define FIELDGEOMETRY_H

#include "GeometryUtils.h"

/**
 * 场地几何类
 * 管理场地的尺寸信息、坐标系转换和各种区域的判断
 */
class FieldGeometry {
public:
    FieldGeometry();  // 构造函数，初始化默认场地参数

    // ========== 场地配置接口 ==========

    /**
     * 设置我方球门在哪一侧
     * @param onRight true表示球门在右侧，false表示在左侧
     */
    void setOurGoalSide(bool onRight);

    /**
     * 交换半场（攻防转换时调用）
     * 交换我方和对方球门的位置
     */
    void swapHalves();

    /**
     * 判断我方球门是否在右侧
     * @return true: 球门在右侧，false: 球门在左侧
     */
    bool isOurGoalOnRight() const { return m_isOurGoalOnRight; }

    // ========== 关键点获取 ==========

    /**
     * 获取我方球门位置（球门中心点）
     * @return 球门中心点坐标
     */
    Point getOurGoalPos() const;

    /**
     * 获取对方球门位置（球门中心点）
     * @return 球门中心点坐标
     */
    Point getOppGoalPos() const;

    /**
     * 获取防守线位置（根据球的位置动态计算）
     * 防守线是后卫线的平均位置，距离球门约25-50cm
     * @param ballPos 球的当前位置
     * @return 防守线上建议的站位点
     */
    Point getDefenseLinePosition(const Point& ballPos) const;

    /**
     * 获取进攻支援位置
     * 用于接应球员的站位，通常位于球的侧后方
     * @param ballPos 球的当前位置
     * @param robotIndex 机器人索引，用于决定站在球的左侧还是右侧
     * @return 支援站位点
     */
    Point getSupportPosition(const Point& ballPos, int robotIndex) const;

    /**
     * 获取边界推球目标点
     * 当球靠近边界时，建议踢向此点
     * @param ballPos 球的当前位置
     * @return 推球目标点
     */
    Point getBoundaryPushTarget(const Point& ballPos) const;

    // ========== 区域判断 ==========

    /**
     * 判断球是否在门区内
     * @param pos 球的位置
     * @return true: 在门区内
     */
    bool isBallInGoalArea(const Point& pos) const;

    /**
     * 判断球是否在罚球区内
     * @param pos 球的位置
     * @return true: 在罚球区内
     */
    bool isBallInPenaltyArea(const Point& pos) const;

    /**
     * 获取我方球门线的x坐标
     * @return 球门线的x坐标值
     */
    double getOurGoalLineX() const;

    /**
     * 获取对方球门线的x坐标
     * @return 球门线的x坐标值
     */
    double getOppGoalLineX() const;

    /**
     * 获取球门的上沿y坐标
     * @return 上沿y坐标
     */
    double getGoalUpLine() const;

    /**
     * 获取球门的下沿y坐标
     * @return 下沿y坐标
     */
    double getGoalDownLine() const;

    /**
     * 获取场地宽度
     * @return 宽度（cm），标准值为220
     */
    double getFieldWidth() const { return m_fieldWidth; }

    /**
     * 获取场地高度
     * @return 高度（cm），标准值为180
     */
    double getFieldHeight() const { return m_fieldHeight; }

    /**
     * 判断点是否在我方半场
     * @param pos 待判断的点
     * @return true: 在我方半场
     */
    bool isInOurHalf(const Point& pos) const;

    /**
     * 判断点是否在对方半场
     * @param pos 待判断的点
     * @return true: 在对方半场
     */
    bool isInOppHalf(const Point& pos) const;

    /**
     * 判断点是否在我方门区内
     * @param pos 待判断的点
     * @return true: 在我方门区内
     */
    bool isInGoalArea(const Point& pos) const;

    /**
     * 判断点是否在我方罚球区内
     * @param pos 待判断的点
     * @return true: 在我方罚球区内
     */
    bool isInPenaltyArea(const Point& pos) const;

    /**
     * 判断点是否在对方门区内
     * @param pos 待判断的点
     * @return true: 在对方门区内
     */
    bool isInOppGoalArea(const Point& pos) const;

    // ========== 坐标系转换 ==========

    /**
     * 转换到标准坐标系（我方球门在左侧）
     * 标准坐标系便于算法统一处理，无论实际攻防方向如何
     * @param pos 原始坐标系下的点
     * @return 标准坐标系下的点
     */
    Point transformToStandard(const Point& pos) const;

    /**
     * 从标准坐标系转换回原始坐标系
     * @param pos 标准坐标系下的点
     * @return 原始坐标系下的点
     */
    Point transformFromStandard(const Point& pos) const;

private:
    bool m_isOurGoalOnRight;      // 我方球门是否在右侧
    double m_fieldWidth;          // 场地宽度（cm），标准220
    double m_fieldHeight;         // 场地高度（cm），标准180
    double m_goalWidth;           // 球门宽度（cm），标准40
    double m_goalAreaWidth;       // 门区宽度（cm），标准50
    double m_goalAreaHeight;      // 门区高度（cm），标准15
    double m_penaltyAreaWidth;    // 罚球区宽度（cm），标准80
    double m_penaltyAreaHeight;   // 罚球区高度（cm），标准35
    double m_penaltyArcLength;    // 罚球弧长度（cm），标准25
    double m_penaltyArcHeight;    // 罚球弧高度（cm），标准5
    double m_centerCircleRadius;  // 中圈半径（cm），标准25
    double m_wallHeight;          // 围墙高度（cm），标准5
    double m_wallThickness;       // 围墙厚度（cm），标准2.5
    double m_goalUpLine;          // 球门上沿y坐标
    double m_goalDownLine;        // 球门下沿y坐标
};

#endif
