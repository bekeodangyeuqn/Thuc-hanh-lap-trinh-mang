#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IP Address> <Port number>\n", argv[0]);
        return 1;
    }
    char *ipAdd = argv[1];
    int portNum = atoi(argv[2]);
    int sockfd;
    socklen_t n;
    struct sockaddr_in server_address;

    char buffer[MAX_BUFFER_SIZE];
    char username[MAX_BUFFER_SIZE];
    char message[MAX_BUFFER_SIZE];

    // Create a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNum);
    server_address.sin_addr.s_addr = inet_addr(ipAdd);

    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(EXIT_FAILURE);
    }

    printf("Connect to server sucessfully!!\n");

    while (1)
    {
        printf("Enter your username: ");
        fgets(username, sizeof(username), stdin);

        username[strcspn(username, "\n")] = 0;
        send(sockfd, (const char *)username, strlen(username), 0);
        n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
        if (n <= 0)
        {
            perror("Error recieve from server");
            exit(1);
        }

        buffer[n] = '\0';
        printf("%s\n", buffer);
        while (1)
        {
            printf("Enter your message: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = 0;
            if (strlen(message) == 0)
            {
                printf("Closing client...\n");
                close(sockfd);
                return 0;
            }
            send(sockfd, (const char *)message, strlen(message), 0);
            if (strcmp(message, "bye") == 0)
            {
                printf("You have logged out.\n");
                break;
            }
            n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
            if (n < 0)
            {
                perror("Error recieve from server");
                exit(1);
            }

            buffer[n] = '\0';
            printf("%s\n", buffer);
        }
    }
    close(sockfd);

    return 0;
}
