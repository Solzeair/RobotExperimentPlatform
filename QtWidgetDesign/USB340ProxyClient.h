#ifndef USB340PROXYCLIENT_H
#define USB340PROXYCLIENT_H

#include <QProcess>
#include <QMutex>
#include <QStringList>

class USB340ProxyClient {
public:
    static USB340ProxyClient* getInstance();
    
    bool init();
    bool checkIfExist();
    bool setFre(int fre, bool op);
    bool changeCarFre(unsigned char carNum, bool freq);
    bool changeCarNum(unsigned char oldNum, unsigned char newNum);
    bool buildCarSpeed(unsigned char carNum, int speed, int angle, int time);
    bool sendAll(int cmd);
    bool sendOneCar(int carNum);
    void release();
    
private:
    USB340ProxyClient();
    ~USB340ProxyClient();
    
    QStringList getPossibleProxyPaths();
    bool sendCommand(const QString& cmd, QString& response);
    
    QProcess* m_process;
    QMutex m_mutex;
    bool m_ready;
    
    static USB340ProxyClient* m_instance;
    static QMutex m_instanceMutex;
};

#endif // USB340PROXYCLIENT_H