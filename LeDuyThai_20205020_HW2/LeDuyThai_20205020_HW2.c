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

int isDomainName(const char *domain_name)
{
    regex_t regex;
    int match;

    // Compile the regular expression.
    regcomp(&regex, "^(?!-)[A-Za-z0-9-]+([\\-\\.]{1}[a-z0-9]+)*\\.[A-Za-z]{2,6}$", REG_ICASE);

    // Try to match the domain name against the regular expression.
    match = regexec(&regex, domain_name, 0, NULL, 0);

    // Free the compiled regular expression.
    regfree(&regex);

    // Return true if the domain name matches the regular expression, false otherwise.
    return match == 0;
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
        printf("Domain: %d\n", isDomainName("google.com"));
        if (isDomainName(host))
        {
            printf("Wrong parameter");
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
    }
    else if (option == 2)
    {
        // Convert domain name to IP address
        if (isIpAddress(host))
        {
            printf("Wrong parameter");
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
