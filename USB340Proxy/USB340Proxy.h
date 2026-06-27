#ifndef USB340PROXY_H
#define USB340PROXY_H

#include <windows.h>
#include <string>

/**
 * @brief USB340 硬件通信代理
 *
 * 以独立控制台进程运行，桥接 Qt 主程序与底层 32 位 DLL：
 * 经 stdin 接收文本命令并调用 DLL 执行，经 stdout 回传结果。
 * 独立进程隔离 32 位 DLL，规避主程序位数不匹配的加载问题。
 */
class USB340Proxy {
public:

    USB340Proxy();
    ~USB340Proxy();

    /**
     * @brief 初始化硬件设备
     *
     * 调用底层 InitUSB340() 建立 USB 连接并完成通信握手。
     * @return true 硬件就绪
     * @return false 未发现设备或设备被占用
     */
    bool Initialize();

    /**
     * @brief 命令处理主循环（阻塞）
     *
     * 逐行从 stdin 读取外部命令，交由 ProcessCommand() 解析执行，直至收到退出指令。
     * 阻塞设计使代理可作为常驻进程被 QProcess 调用。
     */
    void Run();

private:
    /**
     * @brief 解析并执行单条命令
     *
     * 将文本命令（如 "BuildCarSpeed 1 100 50"）拆分为指令名与参数，
     * 再映射到对应底层 DLL 函数调用。
     * @param cmd 外部传入的命令字符串
     * @return true 执行成功
     * @return false 指令未识别或执行失败
     */
    bool ProcessCommand(const std::string& cmd);

    /**
     * @brief 回传执行结果
     *
     * 将底层执行结果（如 "OK"/"FAIL"）写入 stdout，供 Qt 主程序经 QProcess 读取判断。
     * 统一出口便于协议解析。
     */
    void SendResponse(const std::string& response);

    /**
     * @brief 硬件初始化状态标志
     *
     * 记录 Initialize() 是否成功，防止未初始化即下发硬件指令。
     */
    bool m_initialized;
};

#endif // USB340PROXY_H