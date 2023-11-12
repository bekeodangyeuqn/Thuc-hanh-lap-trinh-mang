#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <regex.h>

bool isIpAddress(const char *str)
{
    // Split the string into octets using dots as delimiters
    int octets[4];
    int numParsed = sscanf(str, "%d.%d.%d.%d", &octets[0], &octets[1], &octets[2], &octets[3]);

    if (numParsed != 4)
    {
        return false; // Failed to parse all four octets
    }

    return true; // All checks passed, it's an IP address
}

bool isDomainName(const char *input)
{
    int length = strlen(input);

    // Check if the length of the input is within valid domain name limits
    if (length < 1 || length > 255)
    {
        return false;
    }

    // Check for valid characters and format
    int dotCount = 0;
    for (int i = 0; i < length; i++)
    {
        char c = input[i];
        if (isalnum(c) || c == '-' || c == '.')
        {
            if (c == '.')
            {
                // Check for consecutive dots
                if (i > 0 && input[i - 1] == '.')
                {
                    return false;
                }
                dotCount++;
            }
        }
        else
        {
            return false; // Invalid character
        }
    }

    // There should be exactly one dot, and it should not be at the start or end
    if (dotCount != 1 || input[0] == '.' || input[length - 1] == '.')
    {
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <1 or 2> <IP address or domain name>\n", argv[0]);
        return 1;
    }

    int option = atoi(argv[1]);
    char *host = argv[2];

    if (option == 1)
    {
        // Convert IP address to domain name
        if (isDomainName(host))
        {
            printf("Wrong parameter\n");
            return 1;
        }

        struct hostent *he;
        struct in_addr addr;

        if (inet_pton(AF_INET, host, &addr) != 1)
        {
            printf("Not found information.\n");
            return 1;
        }

        he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
        if (he != NULL)
        {
            printf("Official Name: %s\n", he->h_name);
            printf("Alias Name:\n");
            for (int i = 0; he->h_aliases[i] != NULL; i++)
            {
                printf("%s\n", he->h_aliases[i]);
            }
        }
        else
        {
            printf("Not found information.\n");
            return 1;
        }
    }
    else if (option == 2)
    {
        // Convert domain name to IP address
        if (isIpAddress(host))
        {
            printf("Wrong parameter\n");
            return 1;
        }

        struct addrinfo hints, *res;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int status = getaddrinfo(host, NULL, &hints, &res);
        if (status != 0)
        {
            printf("Not found information.\n");
            return 1;
        }
        struct addrinfo *p = res;
        struct sockaddr_in *ip_addr = (struct sockaddr_in *)(p->ai_addr);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ip_addr->sin_addr), ip, sizeof(ip));
        printf("Official IP: %s\n", ip);
        printf("Alias IP:\n");
        for (p = res->ai_next; p != NULL; p = p->ai_next)
        {
            struct sockaddr_in *ip_addr = (struct sockaddr_in *)(p->ai_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(ip_addr->sin_addr), ip, sizeof(ip));
            printf("%s\n", ip);
        }
        freeaddrinfo(res);
    }
    else
    {
        fprintf(stderr, "Invalid option. Use '1' for IP to domain or '2' for domain to IP.\n");
        return 1;
    }

    return 0;
}
