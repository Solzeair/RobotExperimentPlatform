// ControlParams.h
// 控制参数结构体 - 定义PID控制器参数和运动限制

#ifndef CONTROLPARAMS_H
#define CONTROLPARAMS_H

/**
 * 控制参数结构体
 * 包含PID控制器的系数和运动限制参数
 */
struct ControlParams {
    double kp_pos = 0.8;      // 位置比例系数 - 控制机器人移动到目标点的转向力度
    double kd_pos = 0.2;      // 位置微分系数 - 减少位置控制的震荡
    double kp_angle = 2.5;    // 角度比例系数 - 控制机器人转向到目标角度的力度
    double kd_angle = 0.3;    // 角度微分系数 - 减少角度控制的震荡
    double max_speed = 100.0; // 最大速度限制（cm/s），防止速度过大导致失控
    double angle_error = 0.1;// 角度误差阈值（弧度），小于此值认为已到达目标角度
    double min_speed = 15.0;
    // 射门参数
    double shoot_power = 80.0;             
    double shoot_angle_tolerance = 0.2;     

    // 传球参数
    double pass_success_threshold = 0.6;    

    // 门将参数
    double goalie_aggression = 0.5;       
    double goalie_speed = 60.0;        

    // 避障参数
    double avoid_distance = 25.0;
    double avoid_weight = 1.5;
    ControlParams() = default;  // 默认构造函数，使用上述初始值
};

#endif
