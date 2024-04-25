#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//definere længden på en IPv4 addresse
#define INET_ADDRSTRLEN 16

//returnere IPv4 adressen fra eth0
//kan returnere NULL
char* eth0_ip_get() {

    //test
    struct ifaddrs *ifaddr, *ifa;
    int family;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        //skipper de interfaces hvor der ikke er nogen addresser
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET && strcmp(ifa->ifa_name, "eth0") == 0) {
            char* ip_address = (char*)malloc(INET_ADDRSTRLEN * sizeof(char));
            if (ip_address == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            if (inet_ntop(AF_INET, &addr->sin_addr, ip_address, INET_ADDRSTRLEN) == NULL) {
                perror("inet_ntop");
                exit(EXIT_FAILURE);
            }
            freeifaddrs(ifaddr);
            return ip_address;
        }
    }

    freeifaddrs(ifaddr);

    return NULL;
}