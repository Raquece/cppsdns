#include <cppsdns/bindserv.h>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>

namespace bindserv
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    std::vector<sockaddr_in> clients;

    /// @brief Binds the server to a socket
    /// @param port The port to bind to
    /// @return Exit code
    int bind_socket(int port)
    {
        // Attempt to create the socket object
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            std::cerr << "SOCKET FAIL\n";
            exit(1);
        }

        memset(&servaddr, 0, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port);

        // Attempt to bind to the port
        if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            std::cerr << "BIND FAIL\n";
            exit(1);
        }

        clients = std::vector<sockaddr_in>();

        return 0;
    }

    /// @brief Receives a packet from the bound port
    /// @param clientaddr_id Pointer to the client address (will be set)
    /// @return The payload from the packet
    std::vector<char> recv(int *clientaddr_id)
    {
        // Initialise receiving buffer
        char buffer[250];
        memset(&cliaddr, 0, sizeof(cliaddr));

        clients.push_back(cliaddr);
        *clientaddr_id = clients.size() - 1;

        // Receive payload from socket
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);

        // Return payload as a vector
        return std::vector<char>(std::begin(buffer), std::end(buffer));
    }

    /// @brief Sends a payload back to the previous sender
    /// @param clientaddr_id The ID of the client.
    /// @param buffer The payload to send back
    /// @param __n The length of the payload
    void sendback(int clientaddr_id, char *buffer, size_t __n)
    {
        socklen_t len = sizeof(cliaddr);
        sendto(sockfd, buffer, __n, MSG_CONFIRM, (const struct sockaddr *) &clients[clientaddr_id], len);
        clients.erase(clients.begin()+clientaddr_id);
    }
}