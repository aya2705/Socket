#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

int main() {
    char ipAddr[INET6_ADDRSTRLEN];
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    char hostname[NI_MAXHOST];
    if (gethostname(hostname, NI_MAXHOST) != 0) {
        fprintf(stderr, "gethostname failed\n");
        WSACleanup();
        return 1;
    }

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
        fprintf(stderr, "getaddrinfo failed\n");
        WSACleanup();
        return 1;
    }

    struct addrinfo *ptr = result;
    while (ptr != NULL) {
        void *addr;
        if (ptr->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)ptr->ai_addr;
            addr = &ipv4->sin_addr;
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ptr->ai_addr;
            addr = &ipv6->sin6_addr;
        }
        // inet_ntop(ptr->ai_family, addr, ipAddr, INET6_ADDRSTRLEN);
        strcpy(ipAddr , inet_ntoa(*(struct in_addr* )addr));
        printf("IP Address: %s\n", ipAddr);
        ptr = ptr->ai_next;
    }

    freeaddrinfo(result);
    WSACleanup();

#else
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *sa;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return 1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }

        sa = (struct sockaddr_in *)ifa->ifa_addr;
        inet_ntop(AF_INET, &sa->sin_addr, ipAddr, INET_ADDRSTRLEN);
        printf("Interface: %s\t Address: %s\n", ifa->ifa_name, ipAddr);
    }

    freeifaddrs(ifaddr);
#endif

    return 0;
}
