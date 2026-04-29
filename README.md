# RobotStrategyDll 代码分析与函数调用流程说明文档
**适用项目**：5v5 机器人足球策略决策库  
**用途**：开发调试、维护、二次开发、功能交接

---

## 一、项目整体概览
### 1.1 项目定位
5v5 机器人足球比赛**策略决策动态库（DLL）**，接收场地、机器人、球状态数据，输出左右轮速指令，实现全自主攻防战术。

### 1.2 核心架构分层
```
外部调用层（main / 比赛GUI）
        ↓
DLL 接口层（StrategyFactory + 导出函数）
        ↓
核心调度层（UnifiedStrategy / StrategyCore）
        ↓
战术业务层（区域划分 → 队形 → 角色 → 动作）
        ↓
基础工具层（几何、运动控制、参数、预测）
```

### 1.3 模块分类清单
1. **接口层**：StrategyFactory、export.h、robotstrategydll.h  
2. **核心策略**：UnifiedStrategy、StrategyCore、StrategyInitializer、StrategySelector  
3. **区域与队形**：AreaDivider、Formation  
4. **角色系统**：Role、RoleAllocator、RoleTable  
5. **运动控制**：MotionControl、Goalie  
6. **战术能力**：BallPredictor、BoundaryHandler、PassCoordinator、Shoot  
7. **基础工具**：FieldGeometry、GeometryUtils、ControlParams、ParameterTuning  
8. **上层GUI**：ParameterDialog、MatchDlg_5vs5、main

---

## 二、完整函数调用流程
### 2.1 初始化流程（执行一次）
```
main() / MatchDlg_5vs5
  ├─ 加载 RobotStrategyDll.dll
  ├─ 解析导出函数指针
  ├─ CreateStrategy(0)
  │     └─ StrategyFactory::CreateStrategy
  │           └─ new UnifiedStrategy()
  │
  ├─ InitializeStrategy(比分、时间、半场)
  │     └─ StrategyInitializer::initializeStrategy
  │           ├─ StrategySelector::selectMode()
  │           └─ UnifiedStrategy::applyStrategyConfig()
  │
  ├─ setOurGoalOnRight / setOurKickoff
  └─ setParameter（速度、PID、战术参数）
```

### 2.2 每帧决策流程（核心闭环）
```
外部循环 → decide(...) 【DLL导出函数】
        ↓
UnifiedStrategy::decide(...)
        ├─ 1. preProcess()
        │     ├─ FieldGeometry::transformToStandard()
        │     └─ BallPredictor::updateHistory + predictPosition
        │
        ├─ 2. AreaDivider::getAreaNo(ball)
        │     └─ calculateAreaInStandard → 1~32 / 100 / 101
        │
        ├─ 3. taskDecompose(areaNo)
        │     ├─ 边界 → 100，角球 → 101
        │     └─ 常规 → 原区域号
        │
        ├─ 4. formInterpret(formationNo)
        │     └─ Formation::getFormation(areaNo)
        │           └─ getFormation1~32 → 生成5个Role
        │
        ├─ 5. charAllot / RoleAllocator::assignRoles
        │     ├─ 构建成本矩阵
        │     └─ 匈牙利算法 → 机器人-角色最优匹配
        │
        ├─ 6. robotManager()
        │     ├─ 边界卡死检测
        │     └─ MotionControl::avoidAllRobots()
        │
        └─ 7. actProcess()
              └─ 遍历5台机器人
                    └─ RoleTable::executeRole(roleId)
                          ├─ 门将：Goalie::goalieAction()
                          ├─ 射门：Shoot::shouldShoot() + endProcess()
                          ├─ 传球：PassCoordinator::evaluatePass()
                          └─ 移动：MotionControl::moveToPoint()
                                └─ 输出轮速 velocities[5]
```

### 2.3 特殊比赛状态流程
```
UnifiedStrategy::decide
  └─ updateMatchState()
        └─ switch (STATE_XXX)
              ├─ 点球 / 开球 / 门球 / 任意球 / 争球
              └─ 调用 Formation 对应特殊队形
```

---

## 三、关键模块内部调用关系
### 3.1 区域划分（AreaDivider）
```
getAreaNo(ball, field)
  ├─ transformToStandard(ball)
  └─ calculateAreaInStandard(x, y, field)
        ├─ Y轴分层：160、135、90、45、20
        ├─ X轴分列：30、60、90
        ├─ 左半场 → 1~16
        └─ 右半场 → 坐标镜像 → 17~32
```

### 3.2 队形生成（Formation）
```
getFormation(areaNo)
  └─ switch → getFormationX()
        └─ 返回 vector<Role>(5)
              ├─ 角色ID
              ├─ 目标点 targetPos
              ├─ 优先级 priority
              └─ 角色名称
```

### 3.3 角色分配（RoleAllocator）
```
assignRoles
  ├─ 选择门将：离球门最近机器人
  ├─ 构建距离成本矩阵
  └─ 贪心/匈牙利最优分配 → assignedRoles[5]
```

### 3.4 角色执行（RoleTable）
```
executeRole(roleId)
  └─ switch (roleId)
        ├─ ROLE_SHOOT → 射门
        ├─ ROLE_BOUND_PUSH → 边线推球
        ├─ ROLE_GOALIE → 门将逻辑
        ├─ ROLE_WAIT_* → 接应等待
        └─ 统一调用 MotionControl 移动
```

### 3.5 运动控制（MotionControl）
```
moveToPoint
  ├─ 计算距离与目标角度
  ├─ PD 闭环控制（kp_pos + kd_pos）
  ├─ 近距离减速
  └─ 输出左右轮速
```

### 3.6 门将逻辑（Goalie）
```
goalieAction
  ├─ 预测球与球门线交点
  ├─ 移动到最佳防守站位
  ├─ 满足条件 → 出击截球
  └─ 门柱防撞处理
```

### 3.7 边界处理（BoundaryHandler）
```
handleBoundary
  ├─ 球在角点 → handleCornerKick
  ├─ 球靠近边线 → pushBallFromBoundary
  └─ 调用 MotionControl 推球回场
```

### 3.8 球轨迹预测（BallPredictor）
```
predictPosition
  ├─ 使用最近7帧计算平均速度
  └─ 线性外推未来位置
```

### 3.9 传球协调（PassCoordinator）
```
evaluatePass
  ├─ 遍历所有接应队员
  ├─ 计算传球成功率（距离+防守干扰）
  └─ 返回最优传球目标
```

---

## 四、数据结构传递链路
### 4.1 输入（外部 → DLL）
```cpp
RobotPose robots[5];    // 我方位姿
Point oppRobots[5];     // 敌方位置
BallInfo ball;          // 球信息
```

### 4.2 内部流转
```
AreaNo → FormationNo → vector<Role> → 角色分配表 → roleId
```

### 4.3 输出（DLL → 外部）
```cpp
WheelVelocity velocities[5]; // 左右轮速
```

---

## 五、函数调用极简流程图
```
CreateStrategy → InitializeStrategy → decide()【每帧】
                                           ├─ preProcess → BallPredictor
                                           ├─ AreaDivider → 区域号
                                           ├─ Formation → 角色列表
                                           ├─ RoleAllocator → 分配角色
                                           └─ actProcess
                                                 ├─ RoleTable
                                                 │    ├─ Goalie
                                                 │    ├─ Shoot
                                                 │    ├─ PassCoordinator
                                                 │    └─ BoundaryHandler
                                                 └─ MotionControl → 输出轮速
```

---

## 六、模块依赖关系
- 所有模块依赖：**GeometryUtils、FieldGeometry**  
- 策略核心依赖：**AreaDivider、Formation、RoleAllocator、RoleTable**  
- 角色执行依赖：**MotionControl、Goalie、Shoot、PassCoordinator**  
- GUI 上层依赖：**DLL 导出函数、ParameterTuning**

---

## 七、典型场景调用示例
### 7.1 常规进攻
```
decide
  ├─ 球在对方半场区域24
  ├─ 队形输出：射门 + 直冲 + 双后卫
  ├─ 分配最近机器人执行射门
  └─ 运动控制移动 → 射门
```

### 7.2 边线球
```
decide
  ├─ 判定边界场景 → formationNo=100
  ├─ 队形：边线推球
  └─ BoundaryHandler 将球推回场内
```

### 7.3 点球
```
decide
  ├─ 比赛状态=点球
  ├─ 队形：点球手 + 接应 + 人墙
  └─ 高速射门动作
