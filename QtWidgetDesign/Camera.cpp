#include "Camera.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include "Debug.h"
#include <algorithm>

// 显示尺寸常量
const int DISPLAY_W = 640;
const int DISPLAY_H = 480;

/*
* Camera.cpp - 相机操作和参数管理类实现
* 
* 功能：
* 1. 实现相机的打开、关闭、开始/停止抓取
* 2. 管理相机参数（亮度、增益、对比度、快门、RGB通道）
* 3. 从配置文件读取和保存相机参数
* 4. 使用Basler Pylon SDK实现实际相机图像捕获
* 5. 提供图像数据转换功能
* 6. 打开的是USB连接的Basler相机
*/

Camera* Camera::_pCamera = nullptr;
Camera::GarbageCollector Camera::gc;

Camera::Camera()
    : m_nRed(1219)
    , m_nGreen(1000)
    , m_nBlue(1984)
    , m_nGain(5744)
    , m_nShutter(8000)
    , m_nBlackLevel(0)
    , m_nGamma(1000)
{
    SetRGain(1.0);
    SetBGain(1.0);
    for (int i = 0; i < 256; i++)
    {
        m_pLutG[i] = i;
    }

    ReadConfig();
    Open();
}

Camera::~Camera()
{
    WriteConfig();
    Close();
}

Camera::GarbageCollector::~GarbageCollector()
{
    if (nullptr != Camera::_pCamera)
    {
        delete Camera::_pCamera;
        Camera::_pCamera = nullptr;
    }
}

/**
* @brief 获取Camera类的单例实例
* @return Camera* - 相机实例指针
*/
Camera* Camera::GetInstance()
{
    if (nullptr == _pCamera)
    {
        _pCamera = new Camera();
    }
    return _pCamera;
}

/**
* @brief 相机设备移除回调
*/
void Camera::OnCameraDeviceRemoved(CInstantCamera & camera)
{
    Debug::get()->print(L"摄像头已拔出");
}

/**
* @brief 获取相机黑电平值
* @return unsigned int - 黑电平值
*/
unsigned int Camera::GetBlackLevel()const
{
    return m_nBlackLevel;
}

/**
* @brief 获取相机增益值
* @return unsigned int - 增益值
*/
unsigned int Camera::GetGain()const
{
    return m_nGain;
}

/**
* @brief 获取相机伽马值
* @return unsigned int - 伽马值
*/
unsigned int Camera::GetGamma()const
{
    return m_nGamma;
}

/**
* @brief 获取相机快门值
* @return unsigned int - 快门值
*/
unsigned int Camera::GetShutter()const
{
    return m_nShutter;
}

/**
* @brief 获取相机红色通道值
* @return unsigned int - 红色通道值
*/
unsigned int Camera::GetRed()const
{
    return m_nRed;
}

/**
* @brief 获取相机绿色通道值
* @return unsigned int - 绿色通道值
*/
unsigned int Camera::GetGreen()const
{
    return m_nGreen;
}

/**
* @brief 获取相机蓝色通道值
* @return unsigned int - 蓝色通道值
*/
unsigned int Camera::GetBlue()const
{
    return m_nBlue;
}

/**
* @brief 设置相机黑电平值
* @param value - 黑电平值
* @return bool - 设置是否成功
*/
bool Camera::SetBlackLevel(unsigned int value)
{
    m_nBlackLevel = value;
    if (IsOpen())
    {
        try {
            m_camera.BlackLevel = m_nBlackLevel / 1000.0;
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

/**
* @brief 设置相机增益值
* @param value - 增益值
* @return bool - 设置是否成功
*/
bool Camera::SetGain(unsigned int value)
{
    m_nGain = value;
    if (IsOpen())
    {
        try {
            m_camera.Gain = m_nGain / 1000.0;
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

/**
* @brief 设置相机伽马值
* @param value - 伽马值
* @return bool - 设置是否成功
*/
bool Camera::SetGamma(unsigned int value)
{
    m_nGamma = value;
    if (IsOpen())
    {
        try {
            m_camera.Gamma = m_nGamma / 1000.0;
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

/**
* @brief 设置相机快门值
* @param value - 快门值
* @return bool - 设置是否成功
*/
bool Camera::SetShutter(unsigned int value)
{
    m_nShutter = value;
    if (IsOpen())
    {
        try {
            m_camera.ExposureTime = m_nShutter;
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

/**
* @brief 设置相机红色通道值
* @param value - 红色通道值
* @return bool - 设置是否成功
*/
bool Camera::SetRed(unsigned int value)
{
    m_nRed = value;
    if (IsOpen())
    {
        try {
            m_camera.BalanceRatioSelector = BalanceRatioSelector_Red;
            m_camera.BalanceRatio = m_nRed / 1000.0;
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

/**
* @brief 设置相机绿色通道值
* @param value - 绿色通道值
* @return bool - 设置是否成功
*/
bool Camera::SetGreen(unsigned int value)
{
    m_nGreen = value;
    if (IsOpen())
    {
        try {
            m_camera.BalanceRatioSelector = BalanceRatioSelector_Green;
            m_camera.BalanceRatio = m_nGreen / 1000.0;
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

/**
* @brief 设置相机蓝色通道值
* @param value - 蓝色通道值
* @return bool - 设置是否成功
*/
bool Camera::SetBlue(unsigned int value)
{
    m_nBlue = value;
    if (IsOpen())
    {
        try {
            m_camera.BalanceRatioSelector = BalanceRatioSelector_Blue;
            m_camera.BalanceRatio = m_nBlue / 1000.0;
        }
        catch (...) {
            return false;
        }
    }
    return true;
}

/**
* @brief 检查相机是否打开
* @return bool - 相机是否打开
*/
bool Camera::IsOpen()const
{
    return m_camera.IsOpen();
}

/**
* @brief 检查相机是否正在抓取图像
* @return bool - 相机是否正在抓取
*/
bool Camera::IsGrabbing()const
{
    return m_camera.IsGrabbing();
}

/**
* @brief 从配置文件读取相机参数
*/
void Camera::ReadConfig()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/CameraConfig.ini";
    QFile file(configPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList parts = line.split("=");
            if (parts.size() == 2)
            {
                QString key = parts[0].trimmed();
                unsigned int value = parts[1].trimmed().toUInt();
                if (key == "BlackLevel")
                    m_nBlackLevel = value;
                else if (key == "Gain")
                    m_nGain = value;
                else if (key == "Gamma")
                    m_nGamma = value;
                else if (key == "Shutter")
                    m_nShutter = value;
                else if (key == "Red")
                    m_nRed = value;
                else if (key == "Green")
                    m_nGreen = value;
                else if (key == "Blue")
                    m_nBlue = value;
            }
        }
        file.close();
    }
}

/**
* @brief 将相机参数保存到配置文件
*/
void Camera::WriteConfig()
{
    QString configPath = QCoreApplication::applicationDirPath() + "/CameraConfig.ini";
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << "BlackLevel=" << m_nBlackLevel << "\n";
        out << "Gain=" << m_nGain << "\n";
        out << "Gamma=" << m_nGamma << "\n";
        out << "Shutter=" << m_nShutter << "\n";
        out << "Red=" << m_nRed << "\n";
        out << "Green=" << m_nGreen << "\n";
        out << "Blue=" << m_nBlue << "\n";
        file.close();
    }
}

/**
* @brief 打开相机
* @return bool - 相机是否成功打开
*/
bool Camera::Open()
{
    // 测试摄像头是否能正常运行
    // Debug::get()->print(L"[Debug模式] 假装摄像头已打开，跳过真实硬件连接...");
    // return true;
    // 以下为正常注释
    if (IsOpen())
        Close();
    try
    {
        // 枚举所有已连接的 Basler 设备
        Pylon::DeviceInfoList_t devices;
        CTlFactory::GetInstance().EnumerateDevices(devices);
        Debug::get()->print(QString("Pylon发现设备数量: %1").arg(devices.size()));

        if (devices.empty()) {
            Debug::get()->print(L"未找到任何 Basler 相机设备，请检查连接和驱动");
            return false;
        }

        // 用第一个找到的设备创建相机
        m_camera.Attach(CTlFactory::GetInstance().CreateDevice(devices[0]));
        m_camera.RegisterConfiguration(this, RegistrationMode_ReplaceAll, Cleanup_None);
        m_camera.Open();
        m_camera.MaxNumQueuedBuffer = 10;
        m_camera.MaxNumBuffer = 20;
        m_camera.OutputQueueSize = 10;

        m_camera.Width = DISPLAY_W;
        m_camera.Height = DISPLAY_H;
        m_camera.OffsetX = 0;
        m_camera.OffsetY = 0;

        m_camera.TriggerSelector = TriggerSelector_FrameStart;
        m_camera.TriggerMode = TriggerMode_Off;
        m_camera.TriggerSelector = TriggerSelector_FrameBurstStart;
        m_camera.TriggerMode = TriggerMode_Off;
        m_camera.AcquisitionMode = AcquisitionMode_Continuous;
        m_camera.ExposureMode = ExposureMode_Timed;
        m_camera.ExposureAuto = ExposureAuto_Off;

        SetBlackLevel(m_nBlackLevel);
        SetGain(m_nGain);
        SetShutter(m_nShutter);
        SetGamma(m_nGamma);
        SetRed(m_nRed);
        SetGreen(m_nGreen);
        SetBlue(m_nBlue);

        Debug::get()->print(L"摄像头已打开...");
        return true;
    }
    catch (...)
    {
        Debug::get()->print(L"摄像头打开失败...");
        Close();
        return false;
    }
}

//bool Camera::Open()
//{
//    if (IsOpen())
//        Close();
//    try
//    {
//        // 枚举所有已连接的 Basler 设备
//        Pylon::DeviceInfoList_t devices;
//        CTlFactory::GetInstance().EnumerateDevices(devices);
//        Debug::get()->print(QString("Pylon发现设备数量: %1").arg(devices.size()));
//
//        if (devices.empty()) {
//            Debug::get()->print(L"未找到任何 Basler 相机设备，请检查连接和驱动");
//            return false;
//        }
//
//        // 用第一个找到的设备创建相机
//        m_camera.Attach(CTlFactory::GetInstance().CreateFirstDevice(devices[0]));
//
//        m_camera.RegisterConfiguration(this, RegistrationMode_ReplaceAll, Cleanup_None);
//        m_camera.Open();
//        m_camera.MaxNumQueuedBuffer = 10;
//        m_camera.MaxNumBuffer = 20;
//        m_camera.OutputQueueSize = 10;
//
//        m_camera.Width = DISPLAY_W;
//        m_camera.Height = DISPLAY_H;
//        m_camera.OffsetX = 0;
//        m_camera.OffsetY = 0;
//
//        m_camera.TriggerSelector = TriggerSelector_FrameStart;
//        m_camera.TriggerMode = TriggerMode_Off;
//        m_camera.TriggerSelector = TriggerSelector_FrameBurstStart;
//        m_camera.TriggerMode = TriggerMode_Off;
//        m_camera.AcquisitionMode = AcquisitionMode_Continuous;
//        m_camera.ExposureMode = ExposureMode_Timed;
//        m_camera.ExposureAuto = ExposureAuto_Off;
//
//        SetBlackLevel(m_nBlackLevel);
//        SetGain(m_nGain);
//        SetShutter(m_nShutter);
//        SetGamma(m_nGamma);
//        SetRed(m_nRed);
//        SetGreen(m_nGreen);
//        SetBlue(m_nBlue);
//
//        Debug::get()->print(L"摄像头已打开...");
//        return true;
//    }
//    catch (const GenericException& e)
//    {
//        QString err = QString("摄像头打开失败: %1").arg(e.GetDescription());
//        Debug::get()->print(err.toStdWString().c_str());
//        Close();
//        return false;
//    }
//    catch (...)
//    {
//        Debug::get()->print(L"摄像头打开失败（未知异常）...");
//        Close();
//        return false;
//    }
//}

/**
* @brief 开始相机图像抓取
* @return bool - 是否成功开始抓取
*/
bool Camera::StartGrabbing()
{
    if (!IsOpen())
    {
        if (!Open())
            return false;
    }

    try
    {
        m_camera.StartGrabbing(GrabStrategy_LatestImages);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

/**
* @brief 停止相机图像抓取
*/
void Camera::StopGrabbing()
{
    m_camera.StopGrabbing();
}

/**
* @brief 关闭相机
*/
void Camera::Close()
{
    if (IsGrabbing())
        m_camera.StopGrabbing();
    m_camera.DestroyDevice();
    m_camera.Close();
    Debug::get()->print(L"摄像头已关闭...");
}

/**
* @brief 检索相机抓取的图像结果
* @param ptrResult - 存储图像数据的指针
* @return bool - 是否成功检索到图像
*/
//bool Camera::RetrieveResult(void* ptrResult)
//{
//    if (!IsOpen() || !IsGrabbing())
//        return false;
//    try
//    {
//        CGrabResultPtr ptrGrabResult;
//        // 将 300 改为 1000 毫秒，多给点宽容度
//        if (m_camera.RetrieveResult(1000, ptrGrabResult, TimeoutHandling_Return)) {
//            if (ptrGrabResult->GrabSucceeded()) {
//                ConvertBitmap((unsigned char*)ptrResult, (unsigned char*)ptrGrabResult->GetBuffer(), DISPLAY_W, DISPLAY_H);
//                return true;
//            }
//            else {
//                // 如果抓图失败，打印出具体的底层错误（非常重要！）
//                Debug::get()->print(QString("抓图丢包或失败: %1").arg(ptrGrabResult->GetErrorDescription().c_str()).toStdWString().c_str());
//            }
//        }
//        return false;
//    }
//    catch (...)
//    {
//        StopGrabbing();
//        return false;
//    }
//}
// 在文件顶部确保包含了格式转换的头文件（通常已被 PylonIncludes.h 包含，保险起见可以确认一下）
#include <pylon/ImageFormatConverter.h>

bool Camera::RetrieveResult(void* ptrResult)
{
    if (!IsOpen() || !IsGrabbing())
        return false;
    try
    {
        CGrabResultPtr ptrGrabResult;
        // 建议把超时时间改宽容一点，比如 1000ms
        if (m_camera.RetrieveResult(1000, ptrGrabResult, TimeoutHandling_Return))
        {
            if (ptrGrabResult->GrabSucceeded())
            {
                // 🔥 扔掉旧的 ConvertBitmap，使用 Pylon 官方的高性能转换器
                Pylon::CImageFormatConverter converter;
                converter.OutputPixelFormat = Pylon::PixelType_RGB8packed;
                converter.OutputBitAlignment = Pylon::OutputBitAlignment_MsbAligned;

                // 这一步会自动处理任何 Bayer/Mono/YUV 格式，直接输出完美的 RGB 存入 pBuffer
                converter.Convert(ptrResult, DISPLAY_W * DISPLAY_H * 3, ptrGrabResult);

                return true;
            }
        }
        return false;
    }
    catch (...)
    {
        StopGrabbing();
        return false;
    }
}
/**
* @brief 抓取单帧图像
* @param ptrResult - 存储图像数据的指针
* @return bool - 是否成功抓取图像
*/
bool Camera::GrabOne(void* ptrResult)
{
    if (!IsOpen())
    {
        if (!Open())
            return false;
    }

    try
    {
        CGrabResultPtr ptrGrabResult;
        if (m_camera.GrabOne(300, ptrGrabResult, TimeoutHandling_Return))
        {
            if (ptrGrabResult->GrabSucceeded())
            {
                // 使用 Pylon 官方转换器，自动检测 Bayer 模式，避免红蓝对调
                Pylon::CImageFormatConverter converter;
                converter.OutputPixelFormat = Pylon::PixelType_RGB8packed;
                converter.OutputBitAlignment = Pylon::OutputBitAlignment_MsbAligned;
                converter.Convert(ptrResult, DISPLAY_W * DISPLAY_H * 3, ptrGrabResult);
                return true;
            }
        }
        return false;
    }
    catch (...)
    {
        return false;
    }
}

/**
* @brief 检查相机设备是否已移除
* @return bool - 相机设备是否已移除
*/
bool Camera::IsCameraDeviceRemoved()
{
    return m_camera.IsCameraDeviceRemoved();
}

/**
* @brief 设置蓝色通道增益
* @param gain - 增益值
*/
void Camera::SetBGain(double gain)
{
    for (int i = 0; i < 256; i++)
    {
        m_pLutB[i] = (BYTE)std::min(255, (int)(i * gain));
    }
}

/**
* @brief 设置红色通道增益
* @param gain - 增益值
*/
void Camera::SetRGain(double gain)
{
    for (int i = 0; i < 256; i++)
    {
        m_pLutR[i] = (BYTE)std::min(255, (int)(i * gain));
    }
}

/**
* @brief 处理GB行
* @param pDest - 目标缓冲区
* @param pSource - 源缓冲区
* @param width - 图像宽度
* @param height - 图像高度
* @param lineoffset - 行偏移
*/
void Camera::ProcessGBLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset)
{
    const unsigned char* pLastLine = pSource + width * (height - 1);
    const unsigned char* pRaw = pSource + lineoffset * width;
    unsigned char* pRGB = pDest + width * (height - lineoffset - 1) * 3;
    const unsigned char* pEnd;
    while (pRaw < pLastLine)
    {
        pEnd = pRaw + width - 2;  // 跳过最后一列
        while (pRaw < pEnd)
        {
            // GREENPIXEL_B
            pRGB[2] = m_pLutB[*(pRaw + 1)];
            pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
            pRGB[0] = m_pLutR[*(pRaw + width)];
            pRGB += 3;
            pRaw++;
            
            // BLUEPIXEL
            pRGB[2] = m_pLutB[*pRaw];
            pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
            pRGB[0] = m_pLutR[*(pRaw + width + 1)];
            pRGB += 3;
            pRaw++;
        }
        // 处理最后一个像素
        pRGB[2] = m_pLutB[*(pRaw + 1)];
        pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
        pRGB[0] = m_pLutR[*(pRaw + width)];
        pRGB += 3;
        pRaw++;
        
        pRaw += width + 1;
        pRGB -= (3 * width - 1);
    }
}

/**
* @brief 处理RG行
* @param pDest - 目标缓冲区
* @param pSource - 源缓冲区
* @param width - 图像宽度
* @param height - 图像高度
* @param lineoffset - 行偏移
*/
void Camera::ProcessRGLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset)
{
    const unsigned char* pLastLine = pSource + width * (height - 1);
    const unsigned char* pRaw = pSource + lineoffset * width;
    unsigned char* pRGB = pDest + width * (height - lineoffset - 1) * 3;
    const unsigned char* pEnd;
    while (pRaw < pLastLine)
    {
        pEnd = pRaw + width - 2;  // 跳过最后一列
        while (pRaw < pEnd)
        {
            // REDPIXEL
            pRGB[2] = m_pLutB[*(pRaw + width + 1)];
            pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
            pRGB[0] = m_pLutR[*pRaw];
            pRGB += 3;
            pRaw++;
            
            // GREENPIXEL_R
            pRGB[2] = m_pLutB[*(pRaw + width)];
            pRGB[1] = m_pLutG[*pRaw];
            pRGB[0] = m_pLutR[*(pRaw + 1)];
            pRGB += 3;
            pRaw++;
        }
        // 处理最后一个像素
        pRGB[2] = m_pLutB[*(pRaw + width + 1)];
        pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
        pRGB[0] = m_pLutR[*pRaw];
        pRGB += 3;
        pRaw++;
        
        pRaw += width + 1;
        pRGB -= (3 * width - 1);
    }
}

/**
* @brief 处理BG行
* @param pDest - 目标缓冲区
* @param pSource - 源缓冲区
* @param width - 图像宽度
* @param height - 图像高度
* @param lineoffset - 行偏移
*/
void Camera::ProcessBGLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset)
{
    const unsigned char* pLastLine = pSource + width * (height - 1);
    const unsigned char* pRaw = pSource + lineoffset * width;
    unsigned char* pRGB = pDest + width * (height - lineoffset - 1) * 3;
    const unsigned char* pEnd;
    while (pRaw < pLastLine)
    {
        pEnd = pRaw + width - 2;  // 跳过最后一列
        while (pRaw < pEnd)
        {
            // BLUEPIXEL
            pRGB[2] = m_pLutB[*pRaw];
            pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
            pRGB[0] = m_pLutR[*(pRaw + width + 1)];
            pRGB += 3;
            pRaw++;
            
            // GREENPIXEL_B
            pRGB[2] = m_pLutB[*(pRaw + 1)];
            pRGB[1] = m_pLutG[*pRaw];
            pRGB[0] = m_pLutR[*(pRaw + width)];
            pRGB += 3;
            pRaw++;
        }
        // 处理最后一个像素
        pRGB[2] = m_pLutB[*pRaw];
        pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
        pRGB[0] = m_pLutR[*(pRaw + width + 1)];
        pRGB += 3;
        pRaw++;
        
        pRaw += width + 1;
        pRGB -= (3 * width - 1);
    }
}

/**
* @brief 处理GR行
* @param pDest - 目标缓冲区
* @param pSource - 源缓冲区
* @param width - 图像宽度
* @param height - 图像高度
* @param lineoffset - 行偏移
*/
void Camera::ProcessGRLines(unsigned char* pDest, const unsigned char* pSource, int width, int height, unsigned int lineoffset)
{
    const unsigned char* pLastLine = pSource + width * (height - 1);
    const unsigned char* pRaw = pSource + lineoffset * width;
    unsigned char* pRGB = pDest + width * (height - lineoffset - 1) * 3;
    const unsigned char* pEnd;
    while (pRaw < pLastLine)
    {
        pEnd = pRaw + width - 2;  // 跳过最后一列
        while (pRaw < pEnd)
        {
            // GREENPIXEL_R
            pRGB[2] = m_pLutB[*(pRaw + width)];
            pRGB[1] = m_pLutG[*pRaw];
            pRGB[0] = m_pLutR[*(pRaw + 1)];
            pRGB += 3;
            pRaw++;
            
            // REDPIXEL
            pRGB[2] = m_pLutB[*(pRaw + width + 1)];
            pRGB[1] = m_pLutG[(BYTE)((*(pRaw + 1) + *(pRaw + width)) >> 1)];
            pRGB[0] = m_pLutR[*pRaw];
            pRGB += 3;
            pRaw++;
        }
        // 处理最后一个像素
        pRGB[2] = m_pLutB[*(pRaw + width)];
        pRGB[1] = m_pLutG[*pRaw];
        pRGB[0] = m_pLutR[*(pRaw + 1)];
        pRGB += 3;
        pRaw++;
        
        pRaw += width + 1;
        pRGB -= (3 * width - 1);
    }
}

/**
* @brief 转换位图数据
* @param pDest - 目标缓冲区指针
* @param pSource - 源缓冲区指针
* @param width - 图像宽度
* @param height - 图像高度
*/
void Camera::ConvertBitmap(unsigned char* pDest, unsigned char* pSource, int width, int height)
{
    enum PatternOrigin_t
    {
        poGB = 1,
        poGR,
        poB,
        poR
    };
    
    PatternOrigin_t PatternOrigin = poB; // BGGR格式

    switch (PatternOrigin)
    {
    case poGB: // GBGR
        ProcessGBLines(pDest, pSource, width, height, 0);
        ProcessRGLines(pDest, pSource, width, height, 1);
        break;
    case poGR: // GRBG
        ProcessGRLines(pDest, pSource, width, height, 0);
        ProcessBGLines(pDest, pSource, width, height, 1);
        break;
    case poB: // BGGR
        ProcessBGLines(pDest, pSource, width, height, 0);
        ProcessGRLines(pDest, pSource, width, height, 1);
        break;
    case poR: // RGGB
        ProcessRGLines(pDest, pSource, width, height, 0);
        ProcessGBLines(pDest, pSource, width, height, 1);
        break;
    }

    // 处理边界：擦除目标图像的最右列和最后一行
    unsigned char* pRGB = pDest;
    // 设置最右列为零
    for (int i = 0; i < height; i++)
    {
        pRGB[(i + 1) * width * 3 - 3] = 0;
        pRGB[(i + 1) * width * 3 - 2] = 0;
        pRGB[(i + 1) * width * 3 - 1] = 0;
    }
    // 设置最后一行为零
    for (int i = 0; i < width; i++)
    {
        pRGB[i * 3] = 0;
        pRGB[i * 3 + 1] = 0;
        pRGB[i * 3 + 2] = 0;
    }
}
