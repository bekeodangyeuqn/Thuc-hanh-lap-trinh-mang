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

    char username[MAX_BUFFER_SIZE];
    char password[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];

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

    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);

    username[strcspn(username, "\n")] = 0;
    send(sockfd, (const char *)username, strlen(username), 0);
    n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);

    buffer[n] = '\0';
    printf("%s\n", buffer);
    while (strcmp(buffer, "Account not found") == 0)
    {
        char buffer2[MAX_BUFFER_SIZE];
        printf("Enter your username: ");
        fgets(username, sizeof(username), stdin);

        username[strcspn(username, "\n")] = 0;
        send(sockfd, username, strlen(username), 0);

        n = recv(sockfd, buffer2, MAX_BUFFER_SIZE, 0);
        buffer[n] = '\0';
        printf("%s\n", buffer2);
        strcpy(buffer, buffer2);
    }
    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);

    // Remove the newline character at the end of the password
    password[strcspn(password, "\n")] = 0;
    send(sockfd, password, strlen(password), 0);

    n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
    buffer[n] = '\0';
    printf("%s\n", buffer);

    while (strcmp(buffer, "Not OK") == 0)
    {
        printf("Enter your password: ");
        fgets(password, sizeof(password), stdin);

        password[strcspn(password, "\n")] = 0;
        send(sockfd, password, strlen(password), 0);

        n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
        buffer[n] = '\0';
        printf("%s\n", buffer);
    }
    if (strcmp(buffer, "OK") == 0)
    {
        while (1)
        {
            // n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
            // if (n != 0)
            // {
            //     buffer[n] = '\0';
            //     printf("%s\n", buffer);
            // }
            char newPassword[MAX_BUFFER_SIZE];
            fgets(newPassword, sizeof(newPassword), stdin);
            newPassword[strcspn(newPassword, "\n")] = 0;
            if (strlen(newPassword) == 0)
            {
                printf("Closing client...\n");
                close(sockfd);
                return 0;
            }
            send(sockfd, newPassword, strlen(newPassword), 0);
            n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
            buffer[n] = '\0';
            printf("%s\n", buffer);
            if (strcmp(newPassword, "bye") == 0)
            {
                break;
            }
        }
    }
    close(sockfd);

    return 0;
}
