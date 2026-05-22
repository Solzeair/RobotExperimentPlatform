// Formation.h
// 队形管理 - 根据球的位置区域选择相应的战术队形

#ifndef FORMATION_H
#define FORMATION_H

#include "StrategyCore.h"
#include "FieldGeometry.h"
#include "RoleTable.h"
#include <vector>
#include <string>

/**
 * 队形配置结构体
 * 用于存储一个队形的基本信息
 */
struct FormationConfig {
    std::vector<int> roleIds;      // 角色ID列表（5个）
    std::vector<Point> positions;  // 对应位置列表
    std::string name;              // 队形名称

    FormationConfig() : roleIds(5, 0), positions(5), name("") {}
};

/**
 * 队形管理类
 * 管理32个区域对应的队形，以及特殊比赛情况下的队形
 *
 * 队形设计原则：
 * - 区域1-16：我方半场防守型队形
 * - 区域17-32：对方半场进攻型队形
 * - 每个队形包含5个角色，优先级从高到低
 */
class Formation {
public:
    // 新增：设置单双后卫模式
    static void setSingleDefender(bool isSingle) { m_isSingleDefender = isSingle; }
    static bool isSingleDefender() { return m_isSingleDefender; }
    /**
     * 根据区域号获取队形（5个角色）
     * @param areaNo 区域编号（1-32）
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表（按优先级排序）
     */
    static std::vector<Role> getFormation(int areaNo, const BallInfo& ball,
                                          const FieldGeometry& field);

    // ========== 特殊队形 ==========

    /**
     * 开球队形
     * @param isOurKickoff 是否我方开球
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getKickoffFormation(bool isOurKickoff,
                                                 const FieldGeometry& field);

    /**
     * 门球队形
     * @param isOurGoalKick 是否我方门球
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getGoalKickFormation(bool isOurGoalKick,
                                                  const FieldGeometry& field);

    /**
     * 点球队形
     * @param isOurPenalty 是否我方点球
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getPenaltyFormation(bool isOurPenalty,
                                                 const FieldGeometry& field);

    /**
     * 争球队形
     * @param ballPos 球的位置
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getFreeBallFormation(const Point& ballPos,
                                                  const FieldGeometry& field);

    // ========== 战术队形 ==========

    /**
     * 进攻队形
     * @param areaNo 区域编号
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getAttackFormation(int areaNo, const BallInfo& ball,
                                                const FieldGeometry& field);

    /**
     * 防守队形
     * @param areaNo 区域编号
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getDefenseFormation(int areaNo, const BallInfo& ball,
                                                 const FieldGeometry& field);

    /**
     * 边界队形
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getBoundaryFormation(const BallInfo& ball,
                                                  const FieldGeometry& field);

    /**
     * 角球队形
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getCornerFormation(const BallInfo& ball,
                                                const FieldGeometry& field);

    /**
     * 快速反击队形
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getCounterAttackFormation(const BallInfo& ball,
                                                       const FieldGeometry& field);

    /**
     * 全力防守队形（领先时使用）
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getFullDefenseFormation(const BallInfo& ball,
                                                     const FieldGeometry& field);

    /**
     * 全力进攻队形（落后时使用）
     * @param ball 球的信息
     * @param field 场地几何信息
     * @return 角色列表
     */
    static std::vector<Role> getFullAttackFormation(const BallInfo& ball,
                                                    const FieldGeometry& field);
 //任意球队形声明
    static std::vector<Role> getFreeKickFormation(bool isOurFreeKick, const FieldGeometry& field);
private:
    // 新增：静态成员变量，存储单双后卫状态
    static bool m_isSingleDefender;
    // ========== 32个区域的队形定义 ==========
    static std::vector<Role> getFormation1(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation2(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation3(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation4(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation5(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation6(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation7(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation8(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation9(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation10(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation11(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation12(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation13(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation14(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation15(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation16(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation17(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation18(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation19(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation20(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation21(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation22(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation23(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation24(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation25(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation26(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation27(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation28(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation29(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation30(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation31(const BallInfo& ball, const FieldGeometry& field);
    static std::vector<Role> getFormation32(const BallInfo& ball, const FieldGeometry& field);
};

#endif
