#include "USB340ProxyClient.h"
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

USB340ProxyClient* USB340ProxyClient::m_instance = nullptr;
QMutex USB340ProxyClient::m_instanceMutex;

USB340ProxyClient::USB340ProxyClient()
    : m_process(nullptr), m_ready(false) {}

USB340ProxyClient::~USB340ProxyClient() {
    release();
}

USB340ProxyClient* USB340ProxyClient::getInstance() {
    QMutexLocker locker(&m_instanceMutex);
    if (!m_instance) {
        m_instance = new USB340ProxyClient();
    }
    return m_instance;
}

QStringList USB340ProxyClient::getPossibleProxyPaths() {
    QStringList paths;

    // 1. 主程序所在目录
    QString appPath = QCoreApplication::applicationDirPath();
    paths << appPath + "/USB340Proxy.exe";

    // 2. 项目源文件目录
    QString projectDir = QDir(appPath).absoluteFilePath("../../USB340Proxy");
    paths << projectDir + "/USB340Proxy.exe";

    // 3. x64/Debug 目录
    QString debugPath = QDir(appPath).absoluteFilePath("../x64/Debug/USB340Proxy.exe");
    paths << debugPath;

    // 4. x64/Release 目录
    QString releasePath = QDir(appPath).absoluteFilePath("../x64/Release/USB340Proxy.exe");
    paths << releasePath;

    // 5. Win32/Debug 目录（代理是32位）
    QString win32DebugPath = QDir(appPath).absoluteFilePath("../Win32/Debug/USB340Proxy.exe");
    paths << win32DebugPath;

    // 6. Win32/Release 目录（代理是32位）
    QString win32ReleasePath = QDir(appPath).absoluteFilePath("../Win32/Release/USB340Proxy.exe");
    paths << win32ReleasePath;

    return paths;
}

bool USB340ProxyClient::init() {
    if (m_process) {
        return m_ready;
    }

    QStringList paths = getPossibleProxyPaths();
    QString foundPath;

    // 查找存在的代理程序
    for (const QString& path : paths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists() && fileInfo.isFile()) {
            foundPath = fileInfo.absoluteFilePath();
            qDebug() << "找到USB340Proxy.exe在:" << foundPath;
            break;
        }
    }

    if (foundPath.isEmpty()) {
        qDebug() << "未找到USB340Proxy.exe，已查找位置:";
        for (const QString& path : paths) {
            qDebug() << "  " << path;
        }
        return false;
    }

    m_process = new QProcess();
    m_process->setReadChannel(QProcess::StandardOutput);
    m_process->start(foundPath);

    if (!m_process->waitForStarted(3000)) {
        qDebug() << "启动USB340Proxy.exe失败:" << m_process->errorString();
        delete m_process;
        m_process = nullptr;
        return false;
    }

    if (!m_process->waitForReadyRead(3000)) {
        release();
        return false;
    }

    QString response = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
    m_ready = (response == "READY");
    return m_ready;
}

bool USB340ProxyClient::sendCommand(const QString& cmd, QString& response) {
    if (!m_ready || !m_process) {
        return false;
    }

    QMutexLocker locker(&m_mutex);

    m_process->write(cmd.toUtf8() + "\n");

    if (!m_process->waitForReadyRead(1000)) {
        return false;
    }

    response = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
    return true;
}

bool USB340ProxyClient::checkIfExist() {
    QString response;
    return sendCommand("CheckIfExist", response) && response == "OK";
}

bool USB340ProxyClient::setFre(int fre, bool op) {
    QString response;
    return sendCommand(QString("SetFre %1 %2").arg(fre).arg(op ? "1" : "0"), response) && response == "OK";
}

bool USB340ProxyClient::changeCarFre(unsigned char carNum, bool freq) {
    QString response;
    int actualFreq = freq ? 450 : 460;
    return sendCommand(QString("ChangeCarFre %1 %2").arg((int)carNum).arg(actualFreq), response) && response == "OK";
}

bool USB340ProxyClient::changeCarNum(unsigned char oldNum, unsigned char newNum) {
    QString response;
    return sendCommand(QString("ChangeCarNum %1 %2").arg((int)oldNum).arg((int)newNum), response) && response == "OK";
}

bool USB340ProxyClient::buildCarSpeed(unsigned char carNum, int speed, int angle, int time) {
    QString response;
    return sendCommand(QString("BuildCarSpeed %1 %2 %3 %4").arg((int)carNum).arg(speed).arg(angle).arg(time), response) && response == "OK";
}

bool USB340ProxyClient::sendAll(int cmd) {
    QString response;
    return sendCommand(QString("SendAll %1").arg(cmd), response) && response == "OK";
}

bool USB340ProxyClient::sendOneCar(int carNum) {
    QString response;
    return sendCommand(QString("SendOneCar %1").arg(carNum), response) && response == "OK";
}

void USB340ProxyClient::release() {
    if (m_process) {
        m_process->write("exit\n");
        m_process->waitForFinished(1000);
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_ready = false;
}