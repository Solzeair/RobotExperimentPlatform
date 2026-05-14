// AreaDivider.cpp
// 区域划分器实现 - 将足球场划分为32个区域

#include "AreaDivider.h"
#include <cmath>
#include <string>

/**
 * 获取球所在区域的编号（对外接口）
 * 先将球坐标转换到标准坐标系，再计算区域
 */
int AreaDivider::getAreaNo(const Point& ball, const FieldGeometry& field) {
    Point stdBall = field.transformToStandard(ball);  // 转换到标准坐标系
    return calculateAreaInStandard(stdBall.x, stdBall.y, field);
}

/**
 * 在标准坐标系下计算区域
 *
 * 分区逻辑：
 *   Y轴边界（从顶部到底部）：160, 135, 90, 45, 20
 *   X轴边界（从左到右）：30, 60, 90
 *
 * 左半场（x <= 中线）返回区域1-16
 * 右半场（x > 中线）返回区域17-32（镜像对称）
 */
int AreaDivider::calculateAreaInStandard(double x, double y, const FieldGeometry& field) {
    // 动态获取场地尺寸（支持不同大小的场地）
    double fieldW = field.getFieldWidth();      // 场地宽度，标准220cm
    double fieldH = field.getFieldHeight();     // 场地高度，标准180cm
    double midX = fieldW / 2.0;                 // 中线位置，标准110cm

    // 动态计算分区边界（基于场地比例）
    // 原MFC分区边界：Y轴: 160,135,90,45,20 (相对于180高度)
    //               X轴: 30,60,90           (相对于220宽度)
    double y_boundary_1 = fieldH * 160.0 / 180.0;  // 上边界，约160
    double y_boundary_2 = fieldH * 135.0 / 180.0;  // 上中场边界，约135
    double y_boundary_3 = fieldH * 90.0 / 180.0;   // 中场边界，约90
    double y_boundary_4 = fieldH * 45.0 / 180.0;   // 下中场边界，约45
    double y_boundary_5 = fieldH * 20.0 / 180.0;   // 下边界，约20

    double x_boundary_1 = fieldW * 30.0 / 220.0;   // 第一列边界，约30
    double x_boundary_2 = fieldW * 60.0 / 220.0;   // 第二列边界，约60
    double x_boundary_3 = fieldW * 90.0 / 220.0;   // 第三列边界，约90

    // ========== 左半场（我方半场，x <= 中线）==========
    if (x <= midX + 0.01) {
        // 区域1: 左上角 (y >= 160)
        if (y >= y_boundary_1) return 1;

        // 区域2-4: y >= 135 的区域（上边界区域）
        if (y >= y_boundary_2) {
            if (x <= x_boundary_1) return 2;   // 左列
            if (x <= x_boundary_2) return 3;   // 中列
            return 4;                           // 右列
        }

        // 区域5-8: y >= 90 的区域（上中场区域）
        if (y >= y_boundary_3) {
            if (x <= x_boundary_1) return 5;
            if (x <= x_boundary_2) return 6;
            if (x <= x_boundary_3) return 7;
            return 8;
        }

        // 区域9-12: y >= 45 的区域（中场区域）
        if (y >= y_boundary_4) {
            if (x <= x_boundary_1) return 9;
            if (x <= x_boundary_2) return 10;
            if (x <= x_boundary_3) return 11;
            return 12;
        }

        // 区域13-15: y >= 20 的区域（下中场区域）
        if (y >= y_boundary_5) {
            if (x <= x_boundary_1) return 13;
            if (x <= x_boundary_2) return 14;
            return 15;
        }

        // 区域16: 左下角
        return 16;
    }
    // ========== 右半场（对方半场，x > 中线）==========
    else {
        // 使用对称映射：计算相对于右边界的距离
        double rx = fieldW - x;  // 镜像x坐标

        // 区域17: 右上角
        if (y >= y_boundary_1) return 17;

        // 区域18-20: 对方上边界区域
        if (y >= y_boundary_2) {
            if (rx <= x_boundary_1) return 18;
            if (rx <= x_boundary_2) return 19;
            return 20;
        }

        // 区域21-24: 对方上中场区域
        if (y >= y_boundary_3) {
            if (rx <= x_boundary_1) return 21;
            if (rx <= x_boundary_2) return 22;
            if (rx <= x_boundary_3) return 23;
            return 24;
        }

        // 区域25-28: 对方中场区域
        if (y >= y_boundary_4) {
            if (rx <= x_boundary_1) return 25;
            if (rx <= x_boundary_2) return 26;
            if (rx <= x_boundary_3) return 27;
            return 28;
        }

        // 区域29-31: 对方下中场区域
        if (y >= y_boundary_5) {
            if (rx <= x_boundary_1) return 29;
            if (rx <= x_boundary_2) return 30;
            return 31;
        }

        // 区域32: 右下角
        return 32;
    }
}

/**
 * 获取区域名称
 * 用于调试输出和日志记录
 */
std::string AreaDivider::getAreaName(int areaNo) {
    const char* names[] = {
        "左上角", "上边界左", "上边界中", "上边界右",
        "左中上", "中上左", "中上中", "中上右",
        "左中", "中左", "中中", "中右",
        "左中下", "中下左", "中下右", "左下角",
        "右上角", "上边界右2", "上边界中2", "上边界左2",
        "右中上", "中上右2", "中上中2", "中上左2",
        "右中", "中右2", "中中2", "中左2",
        "右中下", "中下右2", "中下左2", "右下角"
    };
    if (areaNo >= 1 && areaNo <= 32) {
        return names[areaNo - 1];
    }
    return "未知区域";
}

/**
 * 判断两个点是否在同一区域
 */
bool AreaDivider::isSameArea(const Point& p1, const Point& p2, const FieldGeometry& field) {
    return getAreaNo(p1, field) == getAreaNo(p2, field);
}

/**
 * 获取相邻区域
 * @param areaNo 当前区域编号
 * @param direction 方向：0=上, 1=右, 2=下, 3=左
 *
 * 相邻关系：
 *   - 上下相邻：区域编号相差4
 *   - 左右相邻：区域编号相差1（边界处除外）
 */
int AreaDivider::getAdjacentArea(int areaNo, int direction) {
    int adjacent[33][4] = {{0}};  // 1-indexed，4个方向

    // 初始化相邻关系
    for (int i = 1; i <= 32; i++) {
        // 上下相邻
        if (i >= 1 && i <= 4) {
            adjacent[i][0] = i;      // 上：边界区域，上方向是自己
            adjacent[i][2] = i + 4;  // 下：+4
        }
        else if (i >= 5 && i <= 8) {
            adjacent[i][0] = i - 4;  // 上：-4
            adjacent[i][2] = i + 4;  // 下：+4
        }
        else if (i >= 9 && i <= 12) {
            adjacent[i][0] = i - 4;
            adjacent[i][2] = i + 4;
        }
        else if (i >= 13 && i <= 16) {
            adjacent[i][0] = i - 4;
            adjacent[i][2] = i;      // 下：边界区域，下方向是自己
        }
        else if (i >= 17 && i <= 20) {
            adjacent[i][0] = i;
            adjacent[i][2] = i + 4;
        }
        else if (i >= 21 && i <= 24) {
            adjacent[i][0] = i - 4;
            adjacent[i][2] = i + 4;
        }
        else if (i >= 25 && i <= 28) {
            adjacent[i][0] = i - 4;
            adjacent[i][2] = i + 4;
        }
        else if (i >= 29 && i <= 32) {
            adjacent[i][0] = i - 4;
            adjacent[i][2] = i;
        }

        // 左右相邻
        if (i == 1 || i == 5 || i == 9 || i == 13) {
            adjacent[i][1] = i + 1;   // 右：+1
            adjacent[i][3] = i;       // 左：边界，左方向是自己
        }
        else if (i == 4 || i == 8 || i == 12 || i == 16) {
            adjacent[i][1] = i;       // 右：边界，右方向是自己
            adjacent[i][3] = i - 1;   // 左：-1
        }
        else {
            adjacent[i][1] = i + 1;
            adjacent[i][3] = i - 1;
        }
    }

    if (areaNo >= 1 && areaNo <= 32 && direction >= 0 && direction <= 3) {
        return adjacent[areaNo][direction];
    }
    return areaNo;
}
