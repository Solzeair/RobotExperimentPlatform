#include "Camera.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

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
{
    // 初始化时读取配置文件
    ReadConfig();
}

Camera::~Camera()
{
}

Camera* Camera::GetInstance()
{
    if (_pCamera == nullptr)
    {
        _pCamera = new Camera();
    }
    return _pCamera;
}

unsigned int Camera::GetBlackLevel()const
{
    return m_nBlackLevel;
}

unsigned int Camera::GetGain()const
{
    return m_nGain;
}

unsigned int Camera::GetGamma()const
{
    return m_nGamma;
}

unsigned int Camera::GetShutter()const
{
    return m_nShutter;
}

unsigned int Camera::GetRed()const
{
    return m_nRed;
}

unsigned int Camera::GetGreen()const
{
    return m_nGreen;
}

unsigned int Camera::GetBlue()const
{
    return m_nBlue;
}

bool Camera::SetBlackLevel(unsigned int value)
{
    m_nBlackLevel = value;
    return true;
}

bool Camera::SetGain(unsigned int value)
{
    m_nGain = value;
    return true;
}

bool Camera::SetGamma(unsigned int value)
{
    m_nGamma = value;
    return true;
}

bool Camera::SetShutter(unsigned int value)
{
    m_nShutter = value;
    return true;
}

bool Camera::SetRed(unsigned int value)
{
    m_nRed = value;
    return true;
}

bool Camera::SetGreen(unsigned int value)
{
    m_nGreen = value;
    return true;
}

bool Camera::SetBlue(unsigned int value)
{
    m_nBlue = value;
    return true;
}

bool Camera::IsOpen()const
{
    return m_isOpen;
}

bool Camera::IsGrabbing()const
{
    return m_isGrabbing;
}

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

bool Camera::Open()
{
    m_isOpen = true;
    return true;
}

bool Camera::StartGrabbing()
{
    if (m_isOpen)
    {
        m_isGrabbing = true;
        return true;
    }
    return false;
}

void Camera::StopGrabbing()
{
    m_isGrabbing = false;
}

void Camera::Close()
{
    m_isGrabbing = false;
    m_isOpen = false;
}

bool Camera::RetrieveResult(void* ptrResult)
{
    // 由于Qt版本可能没有使用Pylon库，暂时返回false
    return false;
}

bool Camera::GrabOne(void* ptrResult)
{
    // 由于Qt版本可能没有使用Pylon库，暂时返回false
    return false;
}

bool Camera::IsCameraDeviceRemoved()
{
    return m_isDeviceRemoved;
}

void Camera::ConvertBitmap(unsigned char* pDest, unsigned char* pSource, int width, int height)
{
    // 实现图像转换逻辑，将pSource转换为pDest
    // 这里提供一个基本的实现，与MFC版本保持一致
    int size = width * height * 3;
    for (int i = 0; i < size; i++) {
        pDest[i] = pSource[i];
    }
}
