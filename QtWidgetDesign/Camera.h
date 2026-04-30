#pragma once

#include <QString>
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>

using namespace Pylon;
using namespace Basler_UsbCameraParams;

/*
* Camera.h - 相机操作和参数管理类头文件
* 
* 功能：
* 1. 定义相机操作的接口和参数管理
* 2. 提供相机打开、关闭、开始/停止抓取的方法
* 3. 管理相机参数（亮度、增益、对比度、快门、RGB通道）
* 4. 提供从配置文件读取和保存相机参数的方法
* 5. 支持实际相机图像捕获和数据转换
* 6. 使用Basler Pylon SDK打开USB摄像头
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
    // MFC版本中存在的方法
    bool RetrieveResult(void* ptrResult);
    bool GrabOne(void* ptrResult);
    bool IsCameraDeviceRemoved();
    void ConvertBitmap(unsigned char* pDest, unsigned char* pSource, int width, int height);

private:
    void OnCameraDeviceRemoved(CInstantCamera & camera)override;
    
    // 垃圾回收器，确保单例模式的正确释放
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
    
    // 图像处理方法
    void ProcessGBLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
    void ProcessRGLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
    void ProcessBGLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
    void ProcessGRLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset);
};
