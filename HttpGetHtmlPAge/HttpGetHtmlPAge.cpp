#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>
#include <ctime> // Для роботи з часом
using namespace std;

// Функція для перетворення Unix часу в рядковий формат
string unixTimeToString(int unixTime) {
    time_t t = unixTime;
    struct tm tmStruct;
    localtime_s(&tmStruct, &t);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &tmStruct);
    return string(buffer);
}

int main()
{
    setlocale(0, "ru");

    //1. ініціалізація "Ws2_32.dll" для поточного процесу
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    // ініціалізація структури, щоб вказати IP-адресу та порт сервера, з яким ми хочемо з'єднатися
    char hostname[255] = "api.openweathermap.org";

    addrinfo* result = NULL;

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(hostname, "http", &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << endl;
        WSACleanup();
        return 3;
    }

    SOCKET connectSocket = INVALID_SOCKET;
    addrinfo* ptr = NULL;

    //Пробуємо приєднатися до отриманої адреси
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        //2. створення клієнтського сокету
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        //3. З'єднуємося з сервером
        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    //4. HTTP Request

    string uri = "/data/2.5/weather?q=Odessa&appid=75f6e64d49db78658d09cb5ab201e483&units=metric";

    string request = "GET " + uri + " HTTP/1.1\n";
    request += "Host: " + string(hostname) + "\n";
    request += "Accept: */*\n";
    request += "Accept-Encoding: gzip, deflate, br\n";
    request += "Connection: close\n";
    request += "\n";

    //відправка повідомлення
    if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }
    cout << "send data" << endl;

    //5. HTTP Response

    string response;

    const size_t BUFFERSIZE = 1024;
    char resBuf[BUFFERSIZE];

    int respLength;

    do {
        respLength = recv(connectSocket, resBuf, BUFFERSIZE, 0);
        if (respLength > 0) {
            response += string(resBuf).substr(0, respLength);
        }
        else {
            cout << "recv failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 6;
        }

    } while (respLength == BUFFERSIZE);

    cout << "Response: " << response << endl << endl;

    // Парсинг рядка відповіді для отримання потрібних даних
    size_t pos = response.find("name");
    if (pos != string::npos) {
        pos = response.find(":", pos);
        size_t endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string cityName = response.substr(pos + 2, endPos - pos - 3);
            cout << "Город: " << cityName << endl;
        }
    }

    pos = response.find("coord");
    if (pos != string::npos) {
        pos = response.find("lon", pos);
        pos = response.find(":", pos);
        size_t endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string lon = response.substr(pos + 1, endPos - pos - 1);
            cout << "Координаты: " << lon;
        }

        pos = response.find("lat", pos);
        pos = response.find(":", pos);
        endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string lat = response.substr(pos + 1, endPos - pos - 2);
            cout << ", " << lat << endl;
        }
    }

    pos = response.find("main");
    if (pos != string::npos) {
        pos = response.find("temp", pos);
        pos = response.find(":", pos);
        size_t endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string temp = response.substr(pos + 1, endPos - pos - 1);
            cout << "Температура: " << temp << endl;
        }
    }

    pos = response.find("dt");
    if (pos != string::npos) {
        pos = response.find(":", pos);
        size_t endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string dt = response.substr(pos + 1, endPos - pos - 1);
            cout << "Дата: " << unixTimeToString(stoi(dt)) << endl;
        }
    }

    pos = response.find("sys");
    if (pos != string::npos) {
        pos = response.find("country", pos);
        pos = response.find(":", pos);
        size_t endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string country = response.substr(pos + 2, endPos - pos - 3);
            cout << "Страна: " << country << endl;
        }

        pos = response.find("sunrise", pos);
        pos = response.find(":", pos);
        endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string sunrise = response.substr(pos + 1, endPos - pos - 1);
            cout << "Рассвет: " << unixTimeToString(stoi(sunrise)) << endl;
        }

        pos = response.find("sunset", pos);
        pos = response.find(":", pos);
        endPos = response.find(",", pos);
        if (endPos != string::npos) {
            string sunset = response.substr(pos + 1, endPos - pos - 1);
            cout << "Закат: " << unixTimeToString(stoi(sunset)) << endl;
        }
    }

    iResult = shutdown(connectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 7;
    }

    closesocket(connectSocket);
    WSACleanup();
}