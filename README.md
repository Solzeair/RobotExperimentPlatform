# 机器人策略DLL模块 - 技术说明文档

**最后更新**: 2026年4月13日  
**负责人**: 吴佳杰
**状态**: ✅ 编译通过，功能完整

---

## 一、模块概述

本模块实现了一个完整的机器人足球策略系统，以 Windows DLL 形式提供，可被上层应用程序动态加载调用。

### 核心功能
- ✅ 5v5 机器人足球策略决策
- ✅ 32区域战术队形管理
- ✅ 动态角色分配（匈牙利算法）
- ✅ PID运动控制 + 避障算法
- ✅ 守门员专用逻辑
- ✅ 传球协调器
- ✅ 射门判断与执行
- ✅ **策略模式选择器（新增）**：根据比赛场景自动选择进攻/防守模式

---

## 二、已完成功能清单

### 2.1 策略核心模块

| 文件 | 功能 | 状态 |
|------|------|------|
| UnifiedStrategy.cpp/h | 统一策略主控类 | ✅ 完成 |
| FieldGeometry.cpp/h | 场地几何管理 | ✅ 完成 |
| AreaDivider.cpp/h | 32区域划分 | ✅ 完成 |
| Formation.cpp/h | 队形管理 | ✅ 完成 |
| RoleAllocator.cpp/h | 角色分配（匈牙利算法） | ✅ 完成 |
| RoleTable.cpp/h | 角色行为库（80+角色） | ✅ 完成 |

### 2.2 运动控制模块

| 文件 | 功能 | 状态 |
|------|------|------|
| MotionControl.cpp/h | PID控制、避障、轨迹跟踪 | ✅ 完成 |
| ControlParams.h | 控制参数结构 | ✅ 完成 |

### 2.3 专项功能模块

| 文件 | 功能 | 状态 |
|------|------|------|
| Goalie.cpp/h | 守门员逻辑 | ✅ 完成 |
| Shoot.cpp/h | 射门判断与执行 | ✅ 完成 |
| PassCoordinator.cpp/h | 传球协调器 | ✅ 完成 |
| BallPredictor.cpp/h | 球轨迹预测 | ✅ 完成 |
| BoundaryHandler.cpp/h | 边界处理 | ✅ 完成 |

### 2.4 策略选择模块（本次新增）

| 文件 | 功能 | 状态 |
|------|------|------|
| StrategySelector.cpp/h | 策略模式选择器 | ✅ 完成 |
| StrategyInitializer.cpp/h | 策略初始化器 | ✅ 完成 |

### 2.5 DLL导出接口

| 文件 | 功能 | 状态 |
|------|------|------|
| robotstrategydll.cpp/h | DLL导出接口 | ✅ 完成 |
| robotstrategydll_global.h | 导出宏定义 | ✅ 完成 |

### 2.6 测试程序

| 文件 | 功能 | 状态 |
|------|------|------|
| main.cpp | Qt测试程序 | ✅ 完成 |
| ParameterDialog.cpp/h | 参数调优UI | ✅ 完成 |

---

## 三、DLL导出接口说明

### 3.1 基础接口

| 函数名 | 参数 | 返回值 | 说明 |
|--------|------|--------|------|
| `CreateStrategy` | int type | void* | 创建策略实例 |
| `DestroyStrategy` | void* strategy | void | 销毁策略实例 |
| `decide` | 机器人、球、对手数据 | void | 核心决策函数 |
| `reset` | void* strategy | void | 重置策略状态 |
| `setParameter` | key, value | void | 设置策略参数 |
| `getParameter` | key | double | 获取策略参数 |

### 3.2 配置接口

| 函数名 | 参数 | 说明 |
|--------|------|------|
| `setOurGoalOnRight` | bool onRight | 设置我方球门方向 |
| `setOurKickoff` | bool isOurKickoff | 设置开球权 |

### 3.3 策略选择接口（新增）

| 函数名 | 参数 | 说明 |
|--------|------|------|
| `InitializeStrategy` | ourScore, oppScore, time, half, kickoff | 根据场景初始化策略模式 |
| `GetCurrentStrategyMode` | void* strategy | 获取当前策略模式 |
| `SetStrategyMode` | int mode | 设置策略模式（锁定前可用） |

---

## 四、策略模式选择逻辑

### 4.1 四种策略模式

| 模式 | 适用场景 | 进攻侵略性 | 防守深度 | 压迫强度 |
|------|----------|------------|----------|----------|
| 激进进攻 | 落后、上半场开球、最后时刻 | 0.85 | 0.3 | 0.8 |
| 平衡 | 默认、上半场对方开球 | 0.6 | 0.5 | 0.5 |
| 保守防守 | 领先2球、最后1分钟领先 | 0.3 | 0.8 | 0.3 |
| 防守反击 | 领先1球且时间充裕 | 0.5 | 0.7 | 0.4 |

### 4.2 决策流程
上半场：如果是我方开球，直接采用进攻模式；如果是对方开球，先采用平衡模式试探。

下半场：

如果我方领先 2 球及以上，转为防守模式；

如果我方领先 1 球且比赛进入最后 1 分钟，转为防守模式，否则保持平衡模式；

如果我方落后，无论时间多少，都采用进攻模式；

如果平局，最后 1 分钟采用进攻模式，否则保持平衡模式。
### 4.3 调用示例
// 比赛开始前调用
InitializeStrategy(
    strategy,    // 策略实例
    0,           // 我方比分
    0,           // 对方比分
    300,         // 剩余时间（秒）
    1,           // 上半场（1=上半场，0=下半场）
    1            // 我方开球（1=我方，0=对方）
);
五、编译与部署
5.1 编译环境
项目	配置
IDE	Visual Studio 2026
编译器	MSVC v145 (VS2022工具集)
Qt版本	6.5.3 MSVC 2019 64-bit
平台	x64
5.2 编译步骤
打开 RobotStrategyDll.sln 解决方案

选择 x64 | Debug 配置

生成 → 生成解决方案

DLL 输出路径：debug\RobotStrategyDll.dll

5.3 部署说明
调用方需要：

将 RobotStrategyDll.dll 放在 exe 同目录

确保 Qt 运行环境可用（windeployqt.exe 部署）

六、文件结构
text
RobotStrategyDll/
├── 核心策略模块
│   ├── UnifiedStrategy.cpp/h      # 主策略类
│   ├── FieldGeometry.cpp/h        # 场地几何
│   ├── AreaDivider.cpp/h          # 区域划分
│   ├── Formation.cpp/h            # 队形管理
│   ├── RoleAllocator.cpp/h        # 角色分配
│   └── RoleTable.cpp/h            # 角色行为库
│
├── 运动控制模块
│   ├── MotionControl.cpp/h        # PID控制+避障
│   └── ControlParams.h            # 控制参数
│
├── 专项功能模块
│   ├── Goalie.cpp/h               # 守门员
│   ├── Shoot.cpp/h                # 射门
│   ├── PassCoordinator.cpp/h      # 传球
│   ├── BallPredictor.cpp/h        # 球预测
│   └── BoundaryHandler.cpp/h      # 边界处理
│
├── 策略选择模块（新增）
│   ├── StrategySelector.cpp/h     # 策略选择器
│   └── StrategyInitializer.cpp/h  # 策略初始化
│
├── DLL导出模块
│   ├── robotstrategydll.cpp/h     # 导出接口
│   ├── robotstrategydll_global.h  # 导出宏
│   └── StrategyFactory.cpp/h      # 工厂类
│
├── 测试程序
│   ├── main.cpp                   # Qt测试入口
│   ├── ParameterDialog.cpp/h      # 参数调优UI
│   └── ParameterDialog.ui         # UI文件
│
└── 项目文件
    ├── RobotStrategyDll.pro       # Qt项目文件
    └── RobotStrategyDll.vcxproj   # VS项目文件

> 我的策略模块主要实现了5v5足球机器人的策略决策，包括32区域队形、角色分配、运动控制、守门员、射门、传球等功能。另外我还根据比赛场景（比分、时间、半场、开球权）做了策略模式选择器，可以在比赛开始前自动选择进攻、平衡或防守模式。DLL接口已经导出，你们可以直接调用。详细的接口说明和调用示例都在文档里。
