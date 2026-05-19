// ParameterTuning.h
// 参数调优模块 - 管理策略参数，支持运行时调整和持久化

#ifndef PARAMETERTUNING_H
#define PARAMETERTUNING_H

#include <string>
#include <map>
#include <vector>
#include <functional>

// 调优参数结构体
struct TuningParameter {
    std::string name;        // 参数名称
    double value;            // 当前值
    double minValue;         // 最小值
    double maxValue;         // 最大值
    double step;             // 步长
    std::string description; // 参数描述

    TuningParameter(const std::string& n = "", double v = 0, double min = 0,
                    double max = 100, double s = 1, const std::string& desc = "")
        : name(n), value(v), minValue(min), maxValue(max), step(s), description(desc) {}
};

class ParameterTuning {
public:
    // 获取单例实例
    static ParameterTuning& getInstance() {
        static ParameterTuning instance;
        return instance;
    }

    // ========== 初始化 ==========
    void initialize();

    // ========== 参数读写 ==========
    double getParameter(const std::string& name) const;
    void setParameter(const std::string& name, double value);
    const std::map<std::string, TuningParameter>& getAllParameters() const;

    // ========== 持久化 ==========
    bool saveToFile(const std::string& filename);   // 保存到文件
    bool loadFromFile(const std::string& filename); // 从文件加载
    void resetToDefaults();                          // 重置为默认值

    // ========== 回调机制 ==========
    // 注册参数变化回调
    void onParameterChanged(const std::string& name, std::function<void(double)> callback);

private:
    ParameterTuning() {}   // 私有构造函数（单例模式）
    ~ParameterTuning() {}  // 私有析构函数
    ParameterTuning(const ParameterTuning&) = delete;  // 禁止拷贝
    ParameterTuning& operator=(const ParameterTuning&) = delete;  // 禁止赋值

    std::map<std::string, TuningParameter> m_parameters;  // 参数映射表
    std::map<std::string, std::vector<std::function<void(double)>>> m_callbacks;  // 回调函数列表
};

#endif
