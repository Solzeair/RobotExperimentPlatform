// Formation.cpp
// 队形管理实现 - 定义32个区域的战术队形

#include "Formation.h"

using namespace GeometryUtils;

/**
 * 根据区域号获取队形
 * 这是队形管理的主入口函数
 */
std::vector<Role> Formation::getFormation(int areaNo, const BallInfo& ball,
                                          const FieldGeometry& field) {
    switch (areaNo) {
    case 1:  return getFormation1(ball, field);   // 左上角 - 护球+防守
    case 2:  return getFormation2(ball, field);   // 上边界 - 边线推球+防守
    case 3:  return getFormation3(ball, field);   // 中上区域 - 防守为主
    case 4:  return getFormation4(ball, field);   // 右边界 - 后卫+防守
    case 5:  return getFormation5(ball, field);   // 小禁区左侧 - 密集防守
    case 6:  return getFormation6(ball, field);   // 大禁区左侧 - 防守反击
    case 7:  return getFormation7(ball, field);   // 中场左 - 平衡
    case 8:  return getFormation8(ball, field);   // 中场中 - 有机会就射门
    case 9:  return getFormation9(ball, field);   // 小禁区上 - 防守
    case 10: return getFormation10(ball, field);  // 大禁区上 - 防守
    case 11: return getFormation11(ball, field);  // 中场左上 - 进攻准备
    case 12: return getFormation12(ball, field);  // 中场中上 - 进攻
    case 13: return getFormation13(ball, field);  // 下边界 - 防守
    case 14: return getFormation14(ball, field);  // 中场中下 - 平衡
    case 15: return getFormation15(ball, field);  // 中场右下 - 反击
    case 16: return getFormation16(ball, field);  // 左下角 - 护球
    case 17: return getFormation17(ball, field);  // 右上角 - 进攻
    case 18: return getFormation18(ball, field);  // 上边界右 - 边线推球
    case 19: return getFormation19(ball, field);  // 中上右 - 进攻
    case 20: return getFormation20(ball, field);  // 中场右 - 进攻
    case 21: return getFormation21(ball, field);  // 小禁区右侧 - 防守
    case 22: return getFormation22(ball, field);  // 大禁区右侧 - 进攻
    case 23: return getFormation23(ball, field);  // 中场右上 - 进攻
    case 24: return getFormation24(ball, field);  // 中场中右 - 进攻
    case 25: return getFormation25(ball, field);  // 下边界右 - 防守
    case 26: return getFormation26(ball, field);  // 中下右 - 反击
    case 27: return getFormation27(ball, field);  // 右下角 - 护球
    case 28: return getFormation28(ball, field);  // 中场区域 - 组织进攻
    case 29: return getFormation29(ball, field);  // 中场区域 - 组织进攻2
    case 30: return getFormation30(ball, field);  // 中场区域 - 防守反击
    case 31: return getFormation31(ball, field);  // 中场区域 - 控球
    case 32: return getFormation32(ball, field);  // 中场区域 - 全面进攻
    default: return getFormation1(ball, field);
    }
}

/* ==================== 区域1-16：我方半场防守型队形 ==================== */

/**
 * 区域1：左上角
 * 策略：护球为主，边线推球，两个专职后卫保护，守门员镇守球门
 */
std::vector<Role> Formation::getFormation1(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    // 优先级0.9：左上护球（保护球权）
    roles[0] = Role(ROLE_WAIT_135_UP, Point(25, 140), 0.9, "左上护球", 60);
    // 优先级0.8：边线推球（将球从边界推回）
    roles[1] = Role(ROLE_BOUND_PUSH, ball.pos, 0.8, "边线推球", 70);
    // 优先级0.7：横向等球2（接应传球）
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_2, Point(25, ball.pos.y), 0.7, "横向等球2", 65);
    // 优先级0.6：下专职后卫（防守下方区域）
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    // 优先级0.5：守门员
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

/**
 * 区域2：上边界区域
 * 策略：边线推球为主，大禁区后卫保护
 */
std::vector<Role> Formation::getFormation2(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_WAIT_135_UP, Point(25, 140), 0.9, "左上护球", 60);
    roles[1] = Role(ROLE_BOUND_PUSH, ball.pos, 0.8, "边线推球", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_2, Point(25, ball.pos.y), 0.7, "横向等球2", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

/**
 * 区域3：中上区域
 * 策略：防守为主，上下专职后卫保护
 */
std::vector<Role> Formation::getFormation3(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_2, Point(25, ball.pos.y), 0.7, "横向等球2", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}
// ==================== 区域4：右边界区域 - 后卫+防守 ====================
std::vector<Role> Formation::getFormation4(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.7, "大禁区专职后卫(下)", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域5：小禁区左侧 - 密集防守 ====================
std::vector<Role> Formation::getFormation5(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_3, Point(25, ball.pos.y), 0.7, "横向等球3", 65);
    roles[3] = Role(ROLE_WAIT_LEFT, Point(80, ball.pos.y), 0.6, "左边等球", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域6：大禁区左侧 - 防守反击 ====================
std::vector<Role> Formation::getFormation6(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_2, Point(25, ball.pos.y), 0.7, "横向等球2", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域7：中场左 - 平衡 ====================
std::vector<Role> Formation::getFormation7(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL, Point(25, ball.pos.y), 0.7, "横向等球", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}
/**
 * 区域8：中场中
 * 策略：有机会就射门
 */
std::vector<Role> Formation::getFormation8(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.7, "射门", 80);  // 射门角色
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}
// ==================== 区域9：小禁区上 - 防守 ====================
std::vector<Role> Formation::getFormation9(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL, Point(25, ball.pos.y), 0.7, "横向等球", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域10：大禁区上 - 防守 ====================
std::vector<Role> Formation::getFormation10(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_2, Point(25, ball.pos.y), 0.7, "横向等球2", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域11：中场左上 - 进攻准备 ====================
std::vector<Role> Formation::getFormation11(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.7, "射门", 80);
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域12：中场中上 - 进攻 ====================
std::vector<Role> Formation::getFormation12(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.7, "射门", 80);
    roles[3] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.6, "直冲", 85);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域13：下边界区域 - 防守 ====================
std::vector<Role> Formation::getFormation13(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.9, "下专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.8, "上专职后卫", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_3, Point(25, ball.pos.y), 0.7, "横向等球3", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域14：中场中下 - 平衡 ====================
std::vector<Role> Formation::getFormation14(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.9, "下专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.8, "上专职后卫", 70);
    roles[2] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.7, "射门", 80);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域15：中场右下 - 反击 ====================
std::vector<Role> Formation::getFormation15(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.9, "下专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.8, "上专职后卫", 70);
    roles[2] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.7, "射门", 80);
    roles[3] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.6, "直冲", 85);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域16：左下角 - 护球 ====================
std::vector<Role> Formation::getFormation16(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.9, "下专职后卫", 70);
    roles[1] = Role(ROLE_BOUND_PUSH, ball.pos, 0.8, "边线推球", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL_3, Point(25, ball.pos.y), 0.7, "横向等球3", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

/* ==================== 区域17-32：对方半场进攻型队形 ==================== */

/**
 * 区域17：右上角
 * 策略：进攻为主，边线推球+射门
 */
std::vector<Role> Formation::getFormation17(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_BOUND_PUSH, ball.pos, 0.9, "边线推球", 70);
    roles[1] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.8, "射门", 80);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}
// ==================== 区域18：上边界右 - 边线推球 ====================
std::vector<Role> Formation::getFormation18(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_BOUND_PUSH, ball.pos, 0.9, "边线推球", 70);
    roles[1] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.8, "射门", 80);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_WAIT_RIGHT, Point(140, 90), 0.6, "右边等球", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}
/**
 * 区域19：中上右
 * 策略：进攻，直冲+射门
 */
std::vector<Role> Formation::getFormation19(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}
// ==================== 区域20：中场右 - 进攻 ====================
std::vector<Role> Formation::getFormation20(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域21：小禁区右侧 - 防守 ====================
std::vector<Role> Formation::getFormation21(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_BOUND_PUSH, ball.pos, 0.8, "边线推球", 70);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域22：大禁区右侧 - 进攻 ====================
std::vector<Role> Formation::getFormation22(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.6, "大禁区专职后卫(上)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域23：中场右上 - 进攻 ====================
std::vector<Role> Formation::getFormation23(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域24：中场中右 - 进攻 ====================
std::vector<Role> Formation::getFormation24(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域25：下边界右 - 防守 ====================
std::vector<Role> Formation::getFormation25(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_BOUND_PUSH, ball.pos, 0.9, "边线推球", 70);
    roles[1] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.8, "射门", 80);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.7, "下专职后卫", 70);
    roles[3] = Role(ROLE_WAIT_RIGHT, Point(140, 90), 0.6, "右边等球", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域26：中下右 - 反击 ====================
std::vector<Role> Formation::getFormation26(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.7, "下专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.6, "上专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域27：右下角 - 护球 ====================
std::vector<Role> Formation::getFormation27(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_BOUND_PUSH, ball.pos, 0.9, "边线推球", 70);
    roles[1] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.8, "射门", 80);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.7, "下专职后卫", 70);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域28：中场区域 - 组织进攻 ====================
std::vector<Role> Formation::getFormation28(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_WAIT_RIGHT, Point(140, 90), 0.8, "右边等球", 65);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域29：中场区域 - 组织进攻2 ====================
std::vector<Role> Formation::getFormation29(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_WAIT_LEFT, Point(80, ball.pos.y), 0.8, "左边等球", 65);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域30：中场区域 - 防守反击 ====================
std::vector<Role> Formation::getFormation30(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.9, "直冲", 85);
    roles[1] = Role(ROLE_WAIT_HORIZONTAL_2, Point(25, ball.pos.y), 0.8, "横向等球2", 65);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域31：中场区域 - 控球 ====================
std::vector<Role> Formation::getFormation31(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_WAIT_CENTER, Point(110, 90), 0.9, "中间等球", 65);
    roles[1] = Role(ROLE_WAIT_LEFT, Point(80, ball.pos.y), 0.8, "左边等球", 65);
    roles[2] = Role(ROLE_WAIT_RIGHT, Point(140, ball.pos.y), 0.7, "右边等球", 65);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.6, "上专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 区域32：中场区域 - 全面进攻 ====================
std::vector<Role> Formation::getFormation32(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL, Point(25, ball.pos.y), 0.7, "横向等球", 65);
    roles[3] = Role(ROLE_WAIT_CENTER, Point(110, 90), 0.6, "中间等球", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// ==================== 特殊队形实现 ====================

// 边界队形
std::vector<Role> Formation::getBoundaryFormation(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_BOUND_PUSH, ball.pos, 0.9, "边线推球", 70);
    roles[1] = Role(ROLE_BOUND_PUSH, ball.pos, 0.8, "边线推球", 70);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// 角球队形
std::vector<Role> Formation::getCornerFormation(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_BOUND_PUSH, ball.pos, 0.8, "边线推球", 70);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL, Point(25, ball.pos.y), 0.7, "横向等球", 65);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.6, "上专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// 进攻队形
std::vector<Role> Formation::getAttackFormation(int areaNo, const BallInfo& /*ball*/, const FieldGeometry& field) {
    (void)areaNo;
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.8, "直冲", 85);
    roles[2] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.7, "射门", 80);
    roles[3] = Role(ROLE_WAIT_RIGHT, Point(140, 90), 0.6, "右边等球", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// 防守队形
std::vector<Role> Formation::getDefenseFormation(int areaNo, const BallInfo& ball, const FieldGeometry& field) {
    (void)areaNo;
    std::vector<Role> roles(5);
    Point stdBall = field.transformToStandard(ball.pos);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, stdBall.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, stdBall.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.7, "大禁区专职后卫(上)", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

// 快速反击队形
std::vector<Role> Formation::getCounterAttackFormation(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_DIRECT_CHARGE, field.getOppGoalPos(), 0.9, "直冲", 85);
    roles[1] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.8, "射门", 80);
    roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.7, "上专职后卫", 70);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.6, "下专职后卫", 70);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}
/**
 * 开球队形
 * 我方开球：高速直线+高速弧线进攻
 * 对方开球：防守为主
 */
std::vector<Role> Formation::getKickoffFormation(bool isOurKickoff, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    if (isOurKickoff) {
        // 我方开球：双前锋进攻
        roles[0] = Role(ROLE_HIGH_SPEED_LINE, Point(115, 85), 0.9, "高速直线", 80);
        roles[1] = Role(ROLE_HIGH_SPEED_ARC, Point(220, 73), 0.8, "高速弧线", 80);
        roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, 95), 0.7, "上专职后卫", 70);
        roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, 85), 0.6, "下专职后卫", 70);
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    } else {
        // 对方开球：密集防守
        roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, 95), 0.9, "上专职后卫", 70);
        roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, 85), 0.8, "下专职后卫", 70);
        roles[2] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.7, "射门", 80);
        roles[3] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.6, "射门", 80);
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    }
    return roles;
}

/**
 * 门球队形
 * 我方门球：扫球开出，队友接应
 * 对方门球：前场逼抢
 */
std::vector<Role> Formation::getGoalKickFormation(bool isOurGoalKick, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    if (isOurGoalKick) {
        roles[0] = Role(ROLE_CLEAR, field.getOppGoalPos(), 0.9, "扫球", 74);
        roles[1] = Role(ROLE_WAIT_LEFT, Point(80, 75), 0.8, "左边等球", 65);
        roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, 95), 0.7, "上专职后卫", 70);
        roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, 85), 0.6, "下专职后卫", 70);
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    } else {
        roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
        roles[1] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.8, "射门", 80);
        roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, 95), 0.7, "上专职后卫", 70);
        roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, 85), 0.6, "下专职后卫", 70);
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    }
    return roles;
}

/**
 * 全力防守队形（领先时使用）
 * 5名球员全部参与防守，密集防守球门
 */
std::vector<Role> Formation::getFullDefenseFormation(const BallInfo& ball, const FieldGeometry& field) {
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, ball.pos.y + 10), 0.9, "上专职后卫", 70);
    roles[1] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, ball.pos.y - 10), 0.8, "下专职后卫", 70);
    roles[2] = Role(ROLE_PENALTY_AREA_UP, Point(45, 110), 0.7, "大禁区专职后卫(上)", 65);
    roles[3] = Role(ROLE_PENALTY_AREA_DOWN, Point(45, 70), 0.6, "大禁区专职后卫(下)", 65);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

/**
 * 全力进攻队形（落后时使用）
 * 4名球员参与进攻，只有守门员防守
 */
std::vector<Role> Formation::getFullAttackFormation(const BallInfo& ball, const FieldGeometry& field) {
    (void)ball;
    std::vector<Role> roles(5);
    roles[0] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.9, "射门", 80);
    roles[1] = Role(ROLE_SHOOT, field.getOppGoalPos(), 0.8, "射门", 80);
    roles[2] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.7, "直冲", 85);
    roles[3] = Role(ROLE_DIRECT_CHARGE, Point(165, 90), 0.6, "直冲", 85);
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    return roles;
}

std::vector<Role> Formation::getPenaltyFormation(bool isOurPenalty, const FieldGeometry& field) {
    std::vector<Role> roles(5);

    if (isOurPenalty) {
        // ========== 我方点球：进攻阵型 ==========
        // 角色0：点球手 - 主罚点球
        roles[0] = Role(ROLE_PENALTY_KICKER, field.getOppGoalPos(), 0.95, "点球手", 90, 0.9, 0.1, 0.9, 0.1);

        // 角色1：近点接应 - 准备补射（球门左侧）
        Point nearSupport = field.getSupportPosition(field.getOppGoalPos(), 0);
        nearSupport.x -= 20;  // 靠近球门
        roles[1] = Role(ROLE_WAIT_LEFT, nearSupport, 0.85, "近点接应", 75, 0.6, 0.2, 0.8, 0.5);

        // 角色2：远点接应 - 准备补射（球门右侧）
        Point farSupport = field.getSupportPosition(field.getOppGoalPos(), 1);
        farSupport.x -= 20;
        roles[2] = Role(ROLE_WAIT_RIGHT, farSupport, 0.85, "远点接应", 75, 0.6, 0.2, 0.8, 0.5);

        // 角色3：中场保护 - 防止对方反击
        Point midProtect(field.getFieldWidth() / 2, field.getFieldHeight() / 2);
        roles[3] = Role(ROLE_ZONAL_DEFENSE, midProtect, 0.7, "中场保护", 65, 0.3, 0.7, 0.3, 0.2);

        // 角色4：守门员 - 留守本方球门
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55, 0.1, 0.95, 0.05, 0.0);
    }
    else {
        // ========== 对方点球：防守阵型 ==========
        double goalCenterY = field.getFieldHeight() / 2;
        double goalLineX = field.getOurGoalLineX();

        // 角色0-3：人墙（4名球员排成人墙）
        // 人墙位置：球门线前9.15米（标准点球距离），但实际场地要按比例调整
        double wallX = goalLineX + (field.isOurGoalOnRight() ? -15 : 15);

        // 人墙位置1：左2
        roles[0] = Role(ROLE_WALL_FORMATION, Point(wallX, goalCenterY - 12), 0.9, "人墙左2", 0, 0.0, 0.95, 0.05, 0.0);

        // 人墙位置2：左1
        roles[1] = Role(ROLE_WALL_FORMATION, Point(wallX, goalCenterY - 4), 0.9, "人墙左1", 0, 0.0, 0.95, 0.05, 0.0);

        // 人墙位置3：右1
        roles[2] = Role(ROLE_WALL_FORMATION, Point(wallX, goalCenterY + 4), 0.9, "人墙右1", 0, 0.0, 0.95, 0.05, 0.0);

        // 人墙位置4：右2
        roles[3] = Role(ROLE_WALL_FORMATION, Point(wallX, goalCenterY + 12), 0.9, "人墙右2", 0, 0.0, 0.95, 0.05, 0.0);

        // 角色4：守门员 - 防守球门
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55, 0.1, 0.95, 0.05, 0.0);
    }

    return roles;
}
/**
 * 门球队形
 * 争球
 */
std::vector<Role> Formation::getFreeBallFormation(const Point& ballPos, const FieldGeometry& field) {
    std::vector<Role> roles(5);

    // 争球时的策略：快速抢占球权
    Point stdBall = field.transformToStandard(ballPos);
    double fieldW = field.getFieldWidth();
    double fieldH = field.getFieldHeight();

    // 角色0：主抢球员 - 直接冲向球
    roles[0] = Role(ROLE_TACKLE, ballPos, 0.95, "主抢球员", 85, 0.9, 0.5, 0.7, 0.2);

    // 角色1：协防球员 - 站在球的后方，防止对方突破
    Point supportPos = ballPos;
    if (field.isInOurHalf(ballPos)) {
        supportPos.x -= 20;  // 我方半场，站在球后方
    }
    else {
        supportPos.x -= 30;  // 对方半场，稍微靠后
    }
    roles[1] = Role(ROLE_WAIT_SUPPORT, supportPos, 0.85, "协防球员", 70, 0.4, 0.8, 0.3, 0.3);

    // 角色2：接应球员 - 准备接球组织进攻
    Point receivePos = field.getSupportPosition(ballPos, 1);
    roles[2] = Role(ROLE_WAIT_HORIZONTAL, receivePos, 0.8, "接应球员", 75, 0.5, 0.3, 0.7, 0.5);

    // 角色3：保护球员 - 保护球门方向
    Point protectPos = field.getDefenseLinePosition(ballPos);
    roles[3] = Role(ROLE_SPECIAL_DEFENDER_UP, protectPos, 0.75, "保护球员", 65, 0.3, 0.85, 0.2, 0.2);

    // 角色4：守门员 - 留守球门
    roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55, 0.1, 0.95, 0.05, 0.0);

    return roles;
}
//任意球队形
std::vector<Role> Formation::getFreeKickFormation(bool isOurFreeKick, const FieldGeometry& field) {
    std::vector<Role> roles(5);

    if (isOurFreeKick) {
        // 我方任意球：主罚+人墙+接应
        roles[0] = Role(ROLE_FREE_KICKER, field.getOppGoalPos(), 0.95, "任意球手", 80);
        roles[1] = Role(ROLE_WALL_FORMATION, Point(110, 90), 0.8, "人墙", 0);
        roles[2] = Role(ROLE_WAIT_SUPPORT, field.getSupportPosition(field.getOppGoalPos(), 0), 0.7, "接应", 70);
        roles[3] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, 95), 0.6, "后卫", 70);
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    }
    else {
        // 对方任意球：防守
        roles[0] = Role(ROLE_WALL_FORMATION, Point(110, 90), 0.9, "人墙", 0);
        roles[1] = Role(ROLE_WALL_FORMATION, Point(110, 70), 0.8, "人墙2", 0);
        roles[2] = Role(ROLE_SPECIAL_DEFENDER_UP, Point(18, 95), 0.7, "后卫", 70);
        roles[3] = Role(ROLE_SPECIAL_DEFENDER_DOWN, Point(18, 85), 0.6, "后卫", 70);
        roles[4] = Role(ROLE_GOALIE, field.getOurGoalPos(), 0.5, "守门员", 55);
    }

    return roles;
}