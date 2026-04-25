#include "Camera.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include "Debug.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

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
* 4. 使用OpenCV实现实际相机图像捕获
* 5. 提供图像数据转换功能
* 6.打开的是电脑的摄像头
*/

Camera* Camera::_pCamera = nullptr;

Camera::Camera()
    : m_nRed(8000)
    , m_nGreen(8000)
    , m_nBlue(8000)
    , m_nGain(10000)
    , m_nShutter(10000)
    , m_nBlackLevel(5000)
    , m_nGamma(2000)
    , m_isOpen(false)
    , m_isGrabbing(false)
    , m_isDeviceRemoved(false)
    , m_capture(nullptr)
{
    // 初始化时读取配置文件
    ReadConfig();
}

Camera::~Camera()
{
    if (m_capture)
    {
        delete m_capture;
        m_capture = nullptr;
    }
}

/**
* @brief 获取Camera类的单例实例
* @return Camera* - 相机实例指针
*/
Camera* Camera::GetInstance()
{
    if (_pCamera == nullptr)
    {
        _pCamera = new Camera();
    }
    return _pCamera;
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
    if (m_isOpen && m_capture)
    {
        // OpenCV不支持黑电平设置，使用亮度作为替代
        double brightness = value / 1000.0; // 将0-10000范围映射到0-10
        m_capture->set(cv::CAP_PROP_BRIGHTNESS, brightness);
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
    if (m_isOpen && m_capture)
    {
        // OpenCV增益范围通常是0-1或0-255，将0-10000映射到0-1
        double gain = value / 10000.0;
        m_capture->set(cv::CAP_PROP_GAIN, gain);
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
    // OpenCV的VideoCapture不支持伽马设置，暂不实现
    // 如果需要，可以通过后处理实现
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
    if (m_isOpen && m_capture)
    {
        // OpenCV曝光时间转换为毫秒（Basler的value通常是微秒）
        double exposure = value / 1000.0;
        m_capture->set(cv::CAP_PROP_EXPOSURE, exposure);
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
    // OpenCV的VideoCapture不支持白平衡个别通道设置
    // 只能在后处理中进行颜色校正
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
    // OpenCV的VideoCapture不支持白平衡个别通道设置
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
    // OpenCV的VideoCapture不支持白平衡个别通道设置
    return true;
}

/**
* @brief 检查相机是否打开
* @return bool - 相机是否打开
*/
bool Camera::IsOpen()const
{
    return m_isOpen;
}

/**
* @brief 检查相机是否正在抓取图像
* @return bool - 相机是否正在抓取
*/
bool Camera::IsGrabbing()const
{
    return m_isGrabbing;
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
    // 尝试打开默认相机
    if (!m_capture)
    {
        m_capture = new cv::VideoCapture(0);
        if (m_capture->isOpened())
        {
            m_isOpen = true;

            // 应用保存的相机参数
            SetBlackLevel(m_nBlackLevel);
            SetGain(m_nGain);
            SetShutter(m_nShutter);

            Debug::get()->print(L"摄像头已打开...");
            return true;
        }
        else
        {
            delete m_capture;
            m_capture = nullptr;
            Debug::get()->print(L"摄像头打开失败...");
            return false;
        }
    }
    return m_isOpen;
}

/**
* @brief 开始相机图像抓取
* @return bool - 是否成功开始抓取
*/
bool Camera::StartGrabbing()
{
    if (m_isOpen && m_capture)
    {
        m_isGrabbing = true;
        return true;
    }
    return false;
}

/**
* @brief 停止相机图像抓取
*/
void Camera::StopGrabbing()
{
    m_isGrabbing = false;
}

/**
* @brief 关闭相机
*/
void Camera::Close()
{
    m_isGrabbing = false;
    m_isOpen = false;
    if (m_capture)
    {
        delete m_capture;
        m_capture = nullptr;
    }
    Debug::get()->print(L"摄像头已关闭...");
}

/**
* @brief 检索相机抓取的图像结果
* @param ptrResult - 存储图像数据的指针
* @return bool - 是否成功检索到图像
*/
bool Camera::RetrieveResult(void* ptrResult)
{
    if (!m_isOpen || !m_capture || !m_isGrabbing)
        return false;

    cv::Mat frame;
    if (m_capture->read(frame))
    {
        // 调整图像大小以匹配目标缓冲区（DISPLAY_W x DISPLAY_H）
        cv::Mat resizedFrame;
        if (frame.cols != DISPLAY_W || frame.rows != DISPLAY_H)
        {
            cv::resize(frame, resizedFrame, cv::Size(DISPLAY_W, DISPLAY_H));
        }
        else
        {
            resizedFrame = frame;
        }

        // 将OpenCV的BGR转换为RGB格式（交换B和R通道）
        cv::Mat rgbFrame = resizedFrame.clone();
        cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_BGR2RGB);

        // 确保目标缓冲区有足够的空间
        int width = rgbFrame.cols;
        int height = rgbFrame.rows;
        int size = width * height * 3;

        // 复制数据到目标缓冲区
        memcpy(ptrResult, rgbFrame.data, size);
        return true;
    }
    return false;
}

/**
* @brief 抓取单帧图像
* @param ptrResult - 存储图像数据的指针
* @return bool - 是否成功抓取图像
*/
bool Camera::GrabOne(void* ptrResult)
{
    if (!m_isOpen || !m_capture)
        return false;

    cv::Mat frame;
    if (m_capture->read(frame))
    {
        // 调整图像大小以匹配目标缓冲区（DISPLAY_W x DISPLAY_H）
        cv::Mat resizedFrame;
        if (frame.cols != DISPLAY_W || frame.rows != DISPLAY_H)
        {
            cv::resize(frame, resizedFrame, cv::Size(DISPLAY_W, DISPLAY_H));
        }
        else
        {
            resizedFrame = frame;
        }

        // 将OpenCV的BGR转换为RGB格式
        cv::Mat rgbFrame;
        cv::cvtColor(resizedFrame, rgbFrame, cv::COLOR_BGR2RGB);

        // 确保目标缓冲区有足够的空间
        int width = rgbFrame.cols;
        int height = rgbFrame.rows;
        int size = width * height * 3;

        // 复制数据到目标缓冲区
        memcpy(ptrResult, rgbFrame.data, size);
        return true;
    }
    return false;
}

/**
* @brief 检查相机设备是否已移除
* @return bool - 相机设备是否已移除
*/
bool Camera::IsCameraDeviceRemoved()
{
    if (!m_isOpen || !m_capture)
    {
        m_isDeviceRemoved = true;
        Debug::get()->print(L"摄像头已拔出");
        return true;
    }
    return false;
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
    // OpenCV的VideoCapture已经返回解码后的BGR图像
    // 这里直接复制数据即可
    // 如果需要Bayer转换，应该在GrabOne/RetrieveResult中进行
    int size = width * height * 3;
    memcpy(pDest, pSource, size);
}
