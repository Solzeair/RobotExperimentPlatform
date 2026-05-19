// AreaDivider.h
// 区域划分器 - 将足球场划分为32个区域，用于战术决策
#ifndef AREADIVIDER_H
#define AREADIVIDER_H

#include "GeometryUtils.h"
#include "FieldGeometry.h"
#include <string>
/**
 * 区域划分器类
 * 将足球场划分为32个区域（4x8网格），每个区域对应不同的战术行为
 *
 * 区域布局（以标准坐标系为准，我方球门在左侧）：
 *   区域1-4:   上边界区域（y > 160）
 *   区域5-8:   上中场区域（135 < y <= 160）
 *   区域9-12:  中场区域（90 < y <= 135）
 *   区域13-16: 下中场区域（45 < y <= 90）
 *   区域17-20: 对方上边界区域
 *   区域21-24: 对方上中场区域
 *   区域25-28: 对方中场区域
 *   区域29-32: 对方下中场区域
 */

class AreaDivider {
public:
    // 获取区域编号（1-32）
    static int getAreaNo(const Point& ball, const FieldGeometry& field);

    // 获取区域名称
    static std::string getAreaName(int areaNo);

    // 判断是否在同一区域
    static bool isSameArea(const Point& p1, const Point& p2, const FieldGeometry& field);

    // 获取相邻区域
    static int getAdjacentArea(int areaNo, int direction); // direction: 0上,1右,2下,3左

private:
    // 根据标准坐标系计算区域（左下角为原点）
    static int calculateAreaInStandard(double x, double y, const FieldGeometry& field);
};

#endif
