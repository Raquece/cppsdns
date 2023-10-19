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
    struct sockaddr_in servaddr;
    std::vector<std::shared_ptr<struct sockaddr_in>> clients;
    std::stack<int> removed_clients;

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

        clients = std::vector<std::shared_ptr<sockaddr_in>>();

        return 0;
    }

    /// @brief Receives a packet from the bound port
    /// @param clientaddr_id Pointer to the client address (will be set)
    /// @return The payload from the packet
    std::vector<char> recv(int *clientaddr_id)
    {
        // Initialise receiving buffer
        char buffer[250];

        struct sockaddr_in *cliaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

        memset(cliaddr, 0, sizeof(cliaddr));

        // Receive payload from socket
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) cliaddr, &len);

        if (removed_clients.empty())
        {
            clients.push_back(std::shared_ptr<sockaddr_in>(cliaddr));
            *clientaddr_id = clients.size() - 1;
        }
        else
        {
            int id = removed_clients.top();
            removed_clients.pop();
            clients[id] = std::shared_ptr<sockaddr_in>(cliaddr);
        }

        // Return payload as a vector
        return std::vector<char>(std::begin(buffer), std::end(buffer));
    }

    /// @brief Sends a payload back to the previous sender
    /// @param clientaddr_id The ID of the client.
    /// @param buffer The payload to send back
    /// @param __n The length of the payload
    void sendback(int clientaddr_id, char *buffer, size_t __n)
    {
        socklen_t len = sizeof(struct sockaddr_in);
        sendto(sockfd, buffer, __n, MSG_CONFIRM, (const struct sockaddr *) clients[clientaddr_id].get(), len);
        removed_clients.push(clientaddr_id);
    }
}