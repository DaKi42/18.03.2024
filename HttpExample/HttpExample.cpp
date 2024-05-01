#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <ctime>
using namespace std;
string unixTimeToString(int unixTime) {
    time_t t = unixTime;
    struct tm tmStruct;
    localtime_s(&tmStruct, &t);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &tmStruct);
    return string(buffer);
}
int Random(int min, int max) {return min + rand() % (max - min);}
void setConsoleColor(WORD color) { HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); SetConsoleTextAttribute(hConsole, color);}

int main() {
    setlocale(0, "ru");
    string cityName;
    cout << "Введите название города: ";
    getline(cin, cityName); // Получаем название города от пользователя
    cout << endl;
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {cout << "WSAStartup failed with error: " << err << endl; return 1;}
    char hostname[255] = "api.openweathermap.org";
    addrinfo* result = NULL;
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int iResult = getaddrinfo(hostname, "80", &hints, &result); // Используем порт 80 для HTTP
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << endl;
        WSACleanup();
        return 3;
    }
    SOCKET connectSocket = INVALID_SOCKET;
    addrinfo* ptr = NULL;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result); // Освобождаем память, выделенную под результаты getaddrinfo
    string uri = "/data/2.5/weather?q=" + cityName + "&appid=75f6e64d49db78658d09cb5ab201e483&units=metric";
    string request = "GET " + uri + " HTTP/1.1\r\n";
    request += "Host: " + string(hostname) + "\r\n";
    request += "Accept: */*\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";
    if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }
    const int bufferLength = 512;
    char recvbuf[bufferLength];
    int bytesReceived;
    string response;

    do {
        bytesReceived = recv(connectSocket, recvbuf, bufferLength, 0);
        if (bytesReceived > 0) {
            response.append(recvbuf, bytesReceived);
        }
    } while (bytesReceived > 0);

    // cout << "Response: " << response << endl << endl;

    size_t t1 = response.find("name");
    if (t1 != string::npos) {
        t1 = response.find(":", t1);
        size_t endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string cityName = response.substr(t1 + 2, endPos - t1 - 3);
            setConsoleColor(1);
            printf("Город: %s\n", cityName.c_str());
        }
    }

    t1 = response.find("coord");
    if (t1 != string::npos) {
        t1 = response.find("lon", t1);
        t1 = response.find(":", t1);
        size_t endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string lon = response.substr(t1 + 1, endPos - t1 - 1);
            setConsoleColor(2);
            printf("Координаты: %s", lon.c_str());
        }

        t1 = response.find("lat", t1);
        t1 = response.find(":", t1);
        endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string lat = response.substr(t1 + 1, endPos - t1 - 2);
            printf(", %s\n", lat.c_str());
        }
    }

    t1 = response.find("main");
    if (t1 != string::npos) {
        t1 = response.find("temp", t1);
        t1 = response.find(":", t1);
        size_t endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string temp = response.substr(t1 + 1, endPos - t1 - 1);
            setConsoleColor(3);
            printf("Температура: %s\n", temp.c_str());
        }
    }

    t1 = response.find("dt");
    if (t1 != string::npos) {
        t1 = response.find(":", t1);
        size_t endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string dt = response.substr(t1 + 1, endPos - t1 - 1);
            setConsoleColor(4);
            printf("Дата: %s\n", unixTimeToString(stoi(dt)).c_str());
        }
    }

    t1 = response.find("sys");
    if (t1 != string::npos) {
        t1 = response.find("country", t1);
        t1 = response.find(":", t1);
        size_t endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string country = response.substr(t1 + 2, endPos - t1 - 3);
            setConsoleColor(5);
            printf("Страна: %s\n", country.c_str());
        }

        t1 = response.find("sunrise", t1);
        t1 = response.find(":", t1);
        endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string sunrise = response.substr(t1 + 1, endPos - t1 - 1);
            setConsoleColor(6);
            printf("Рассвет: %s\n", unixTimeToString(stoi(sunrise)).c_str());
        }

        t1 = response.find("sunset", t1);
        t1 = response.find(":", t1);
        endPos = response.find(",", t1);
        if (endPos != string::npos) {
            string sunset = response.substr(t1 + 1, endPos - t1 - 1);
            setConsoleColor(8);
            printf("Закат: %s\n", unixTimeToString(stoi(sunset)).c_str());
        }
    }
    setConsoleColor(7);
    iResult = shutdown(connectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 7;
    }
    closesocket(connectSocket);
    WSACleanup();
    return 0;
}