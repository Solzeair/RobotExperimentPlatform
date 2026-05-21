#ifndef USB340PROXY_H
#define USB340PROXY_H

#include <windows.h>
#include <string>

/**
 * @brief USB340 硬件通信代理类
 * * 作为一个独立的控制台进程运行，充当 Qt 主程序与底层 32位 DLL 之间的通信桥梁。
 * 它通过标准输入(stdin)接收纯文本命令，调用底层 DLL 函数，
 * 并通过标准输出(stdout)将执行结果返回给调用者。
 */
class USB340Proxy {
public:

    USB340Proxy();
    ~USB340Proxy();

    /**
     * @brief 初始化硬件设备
     * 调用底层的 InitUSB340() 等函数，建立与 USB 设备的物理连接。
     * @return true 初始化成功，硬件就绪
     * @return false 初始化失败，找不到设备或设备被占用
     */
    bool Initialize();

    /**
     * @brief 启动代理的主循环 (阻塞函数)
     * 持续监听标准输入(std::cin)，一旦接收到外部程序发送的命令字符串，
     * 就会将其交给 ProcessCommand() 进行解析和执行，直到接收到退出指令。
     */
    void Run();

private:
    /**
     * @brief 解析并执行单条指令
     * 将接收到的文本字符串（如 "BuildCarSpeed 1 100 50"）拆解，
     * 提取出指令名和参数，然后映射到对应的底层 DLL 函数进行调用。
     * * @param cmd 从外部接收到的完整指令字符串
     * @return true 指令执行成功
     * @return false 指令未识别或执行失败
     */
    bool ProcessCommand(const std::string& cmd);

    /**
     * @brief 发送执行结果的反馈
     * 将底层函数的执行结果（如 "OK" 或 "FAIL"）打印到标准输出(std::cout)，
     * 供 Qt 主程序的 QProcess 读取和判断。
     * * @param response 需要返回给主程序的文本消息
     */
    void SendResponse(const std::string& response);

    /**
     * @brief 硬件初始化状态标志
     * 记录 Initialize() 是否成功调用，防止在未初始化的情况下执行硬件操作。
     */
    bool m_initialized;
};

#endif // USB340PROXY_H