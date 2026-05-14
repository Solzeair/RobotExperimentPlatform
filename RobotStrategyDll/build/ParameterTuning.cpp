// ParameterTuning.cpp
// 参数调优模块实现

#include "ParameterTuning.h"
#include <fstream>
#include <iostream>

// ========== 初始化参数 ==========
void ParameterTuning::initialize() {
    // 运动控制参数
    m_parameters["kp_linear"] = TuningParameter("kp_linear", 0.8, 0.1, 2.0, 0.05, "线性比例系数");
    m_parameters["kp_angular"] = TuningParameter("kp_angular", 2.5, 0.5, 5.0, 0.1, "角向比例系数");
    m_parameters["max_speed"] = TuningParameter("max_speed", 75, 50, 100, 5, "最大速度");
    m_parameters["min_speed"] = TuningParameter("min_speed", 15, 5, 40, 2, "最小速度");

    // 避障参数
    m_parameters["avoid_distance"] = TuningParameter("avoid_distance", 25, 15, 40, 1, "避障距离");
    m_parameters["avoid_weight"] = TuningParameter("avoid_weight", 1.5, 0.5, 3.0, 0.1, "避障权重");
    m_parameters["smooth_factor"] = TuningParameter("smooth_factor", 0.3, 0.1, 0.9, 0.05, "平滑因子");

    // 射门参数
    m_parameters["shoot_power"] = TuningParameter("shoot_power", 80, 60, 100, 5, "射门力量");
    m_parameters["shoot_angle_tolerance"] = TuningParameter("shoot_angle_tolerance", 0.2, 0.05, 0.5, 0.02, "射门角度容差");

    // 战术参数
    m_parameters["attack_aggression"] = TuningParameter("attack_aggression", 0.6, 0.2, 1.0, 0.05, "进攻侵略性");
    m_parameters["defense_depth"] = TuningParameter("defense_depth", 0.5, 0.2, 1.0, 0.05, "防守深度");
    m_parameters["pressing_intensity"] = TuningParameter("pressing_intensity", 0.5, 0.1, 1.0, 0.05, "压迫强度");
    m_parameters["pass_success_threshold"] = TuningParameter("pass_success_threshold", 0.6, 0.3, 0.9, 0.05, "传球成功率阈值");

    // 门将参数
    m_parameters["goalie_aggression"] = TuningParameter("goalie_aggression", 0.5, 0.1, 1.0, 0.05, "门将侵略性");
    m_parameters["goalie_speed"] = TuningParameter("goalie_speed", 60, 40, 80, 5, "门将速度");
}

// ========== 获取参数值 ==========
double ParameterTuning::getParameter(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end()) {
        return it->second.value;
    }
    return 0;
}

// ========== 设置参数值 ==========
void ParameterTuning::setParameter(const std::string& name, double value) {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end()) {
        // 限制值在范围内
        value = std::max(it->second.minValue, std::min(it->second.maxValue, value));
        it->second.value = value;

        // 触发回调函数
        auto cbIt = m_callbacks.find(name);
        if (cbIt != m_callbacks.end()) {
            for (auto& callback : cbIt->second) {
                callback(value);
            }
        }
    }
}

// ========== 获取所有参数 ==========
const std::map<std::string, TuningParameter>& ParameterTuning::getAllParameters() const {
    return m_parameters;
}

// ========== 保存到文件 ==========
bool ParameterTuning::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    for (const auto& pair : m_parameters) {
        file << pair.first << " = " << pair.second.value << std::endl;
    }

    file.close();
    return true;
}

// ========== 从文件加载 ==========
bool ParameterTuning::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string name = line.substr(0, eqPos);
            name.erase(name.find_last_not_of(" \t") + 1);
            double value = std::stod(line.substr(eqPos + 1));
            setParameter(name, value);
        }
    }

    file.close();
    return true;
}

// ========== 重置为默认值 ==========
void ParameterTuning::resetToDefaults() {
    for (auto& pair : m_parameters) {
        pair.second.value = (pair.second.minValue + pair.second.maxValue) / 2;
    }
}

// ========== 注册参数变化回调 ==========
void ParameterTuning::onParameterChanged(const std::string& name, std::function<void(double)> callback) {
    m_callbacks[name].push_back(callback);
}
