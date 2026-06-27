#pragma once

#include <QString>
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>

using namespace Pylon;
using namespace Basler_UsbCameraParams;

/*
* Camera.h - Basler USB 相机封装（单例）
*
* 设计要点：
* - 私有构造 + 静态实例，保证全局唯一相机句柄，避免重复打开设备
* - 既是参数管理器（读写配置），也是抓取控制器，统一对外接口
* - 暴露 RGB/增益/快门等参数，底层映射到 Basler GenApi 节点
*/

class Camera : private Pylon::CConfigurationEventHandler
{
private:
    Camera();

public:
    ~Camera();
    static Camera* GetInstance();
    unsigned int GetBlackLevel()const;
    unsigned int GetGain()const;
    unsigned int GetGamma()const;
    unsigned int GetShutter()const;
    unsigned int GetRed()const;
    unsigned int GetGreen()const;
    unsigned int GetBlue()const;
    bool SetBlackLevel(unsigned int value);
    bool SetGain(unsigned int value);
    bool SetGamma(unsigned int value);
    bool SetShutter(unsigned int value);
    bool SetRed(unsigned int value);
    bool SetGreen(unsigned int value);
    bool SetBlue(unsigned int value);
    bool IsOpen()const;
    bool IsGrabbing()const;
    void ReadConfig();
    void WriteConfig();
    bool Open();
    bool StartGrabbing();
    void StopGrabbing();
    void Close();
    // 兼容旧 MFC 调用约定的接口签名（void* 透传结果）
    bool RetrieveResult(void* ptrResult);
    bool GrabOne(void* ptrResult);
    bool IsCameraDeviceRemoved();
    void ConvertBitmap(unsigned char* pDest, unsigned char* pSource, int width, int height);

private:
    void OnCameraDeviceRemoved(CInstantCamera & camera)override;
    
    // 嵌入式 GC：进程退出时释放单例，避免相机句柄泄漏
    class GarbageCollector {
    public:
        ~GarbageCollector();
    };
    static GarbageCollector gc;

private:
    static Camera* _pCamera;
    CBaslerUsbInstantCamera m_camera;
    PylonAutoInitTerm autoInitTerm;

    unsigned int m_nRed;
    unsigned int m_nGreen;
    unsigned int m_nBlue;
    unsigned int m_nGain;
    unsigned int m_nShutter;
    unsigned int m_nBlackLevel;
    unsigned int m_nGamma;
    
    BYTE m_pLutG[256];
    BYTE m_pLutR[256];
    BYTE m_pLutB[256];
    
    void SetBGain(double gain);
    void SetRGain(double gain);
    
    // Bayer 解码：按传感器行序（GB/RG/BG/GR）分别处理，输出 RGB
    void ProcessGBLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
    void ProcessRGLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
    void ProcessBGLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
    void ProcessGRLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
};
