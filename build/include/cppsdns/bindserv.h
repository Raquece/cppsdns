#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define DEFAULT_BIND_PORT 53

namespace bindserv
{
    int bind_socket(int port = DEFAULT_BIND_PORT);
    std::vector<char> recv(int *clientaddr_id);
    void sendback(int clientaddr_id, char *buffer, size_t __n);
}