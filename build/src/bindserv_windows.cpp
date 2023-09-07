#include <cppsdns/bindserv.h>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

namespace bindserv
{
    SOCKET sock;
    struct sockaddr_in server, client;
    int slen, recv_len;
    WSADATA wsa;

    int bind_socket(int port)
    {
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            std::cerr << "ERROR)\n";
        }

        slen = sizeof(client);

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(port);

        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        {
            wchar_t *s = NULL;
            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                        NULL, WSAGetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPWSTR)&s, 0, NULL);
            fprintf(stderr, "%S\n", s);
            LocalFree(s);

            std::cerr << "\nSOCKET CREATION FAILED\n";
            exit(1);
        }

        auto binderr = bind(sock, (struct sockaddr *)&server, sizeof(server));
        if(binderr == SOCKET_ERROR)
        {
            wchar_t *s = NULL;
            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                        NULL, WSAGetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPWSTR)&s, 0, NULL);
            fprintf(stderr, "%S\n", s);
            LocalFree(s);

            std::cerr << "\nBIND FAILED\n";
            exit(1);
        }

        return 0;
    }

    std::vector<char> recv()
    {
        char buffer[150];

        if ((recv_len = recvfrom(sock, buffer, 150, 0, (struct sockaddr *) &client, &slen)) == SOCKET_ERROR)
        {
            wchar_t *s = NULL;
            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                        NULL, WSAGetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPWSTR)&s, 0, NULL);
            fprintf(stderr, "%S\n", s);
            LocalFree(s);

            std::cerr << "\nRECV FAILED\n";
        }

        return std::vector<char>(std::begin(buffer), std::end(buffer));
    }
}