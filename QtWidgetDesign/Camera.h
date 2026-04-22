#pragma once

#include <QString>

class Camera
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
    // 添加MFC版本中存在的方法
    bool RetrieveResult(void* ptrResult);
    bool GrabOne(void* ptrResult);
    bool IsCameraDeviceRemoved();
    void ConvertBitmap(unsigned char* pDest, unsigned char* pSource, int width, int height);

private:
    static Camera* _pCamera;

    unsigned int m_nRed;
    unsigned int m_nGreen;
    unsigned int m_nBlue;
    unsigned int m_nGain;
    unsigned int m_nShutter;
    unsigned int m_nBlackLevel;
    unsigned int m_nGamma;
    
    bool m_isOpen;
    bool m_isGrabbing;
    bool m_isDeviceRemoved;
};
