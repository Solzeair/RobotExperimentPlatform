#include "USB340Proxy.h"
#include "USB340HID61_DEF.h"
#include <iostream>
#include <sstream>

#pragma comment(lib, "USB340HID61.lib")

USB340Proxy::USB340Proxy() : m_initialized(false) {}

USB340Proxy::~USB340Proxy() {
    if (m_initialized) {
        // 设备已初始化时需释放其底层资源（当前为占位，预留清理入口）
    }
}

bool USB340Proxy::Initialize() {
    bool result = InitUSB340();
    m_initialized = result;
    return result;
}

void USB340Proxy::Run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") {
            break;
        }
        ProcessCommand(line);
    }
}

bool USB340Proxy::ProcessCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string command;
    iss >> command;
    
    if (command == "InitUSB340") {
        bool result = InitUSB340();
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else if (command == "CheckIfExist") {
        bool result = CheckIfExist();
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else if (command == "SetFre") {
        int fre;
        int opInt;
        iss >> fre >> opInt;
        bool op = (opInt != 0);
        bool result = SetFre(fre, op);
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else if (command == "ChangeCarFre") {
        int carNumInt;
        int freqInt;
        iss >> carNumInt >> freqInt;
        unsigned char carNum = (unsigned char)carNumInt;
        bool result = ChangeCarFre(carNum, freqInt, true);
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else if (command == "ChangeCarNum") {
        int oldNumInt, newNumInt;
        iss >> oldNumInt >> newNumInt;
        unsigned char oldNum = (unsigned char)oldNumInt;
        unsigned char newNum = (unsigned char)newNumInt;
        bool result = ChangeCarNum(oldNum, newNum);
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else if (command == "BuildCarSpeed") {
        int carNumInt, speed, angle;
        iss >> carNumInt >> speed >> angle;
        unsigned char carNum = (unsigned char)carNumInt;
        bool result = BuildCarSpeed(carNum, speed, angle);
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else if (command == "SendAll") {
        int cmdInt;
        iss >> cmdInt;
        bool result = SendAll(cmdInt);
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else if (command == "SendOneCar") {
        int carNum;
        iss >> carNum;
        bool result = SendOneCar(carNum);
        SendResponse(result ? "OK" : "FAIL");
        return result;
    }
    else {
        SendResponse("UNKNOWN");
        return false;
    }
}

void USB340Proxy::SendResponse(const std::string& response) {
    std::cout << response << std::endl;
}

int main() {
    USB340Proxy proxy;
    if (proxy.Initialize()) {
        std::cout << "READY" << std::endl;
        proxy.Run();
    }
    else {
        std::cout << "INIT_FAILED" << std::endl;
        return 1;
    }
    return 0;
}