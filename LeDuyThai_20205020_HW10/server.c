#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define LISTENQ 1

enum MessageType
{
    LOGIN,
    TEXT_MESSAGE,
};

struct message
{
    enum MessageType type;
    char payload[MAX_BUFFER_SIZE];
};

char clientUsername[1050] = "";
int isLogged = 0;

void handleClient(struct message *msg, int connfd)
{
    FILE *fp;
    char filename[1050] = "";
    switch (msg->type)
    {
    case LOGIN:
        printf("Username: %s\n", msg->payload);
        strcpy(filename, msg->payload);
        strcat(filename, ".txt");
        printf("Filename: %s\n", filename);
        fp = fopen(filename, "ab+");

        if (fp == NULL)
        {
            perror("Failed to open or create file");
            fclose(fp);
            char *ack = "Failed to open or create file";
            send(connfd, ack, strlen(ack), 0);
            return;
        }
        isLogged = 1;
        strcpy(clientUsername, msg->payload);
        char ack[1050] = "Hello ";
        strcat(ack, msg->payload);
        send(connfd, ack, strlen(ack), 0);
        break;
    case TEXT_MESSAGE:
        strcat(filename, clientUsername);
        strcat(filename, ".txt");
        fp = fopen(filename, "a");
        if (fp == NULL)
        {
            perror("Failed to open or create file");
            fclose(fp);
            char *ack1 = "Failed to open or create file";
            send(connfd, ack1, strlen(ack1), 0);
            return;
        }
        fprintf(fp, "%s\n", msg->payload);
        char *ack2 = "Server saved message to log file\n";
        send(connfd, ack2, strlen(ack2), 0);
        break;
    }
    fclose(fp);
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Port number>\n", argv[0]);
        return 1;
    }

    int listenfd, connfd, n;
    struct sockaddr_in server_address, client_address;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t client_len;
    printf("Connecting...\n");
    int portNum = atoi(argv[1]);
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(portNum);

    client_len = sizeof(client_address);

    if (bind(listenfd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Binding successful\n");
        printf("[%s:%d]\n", inet_ntoa(server_address.sin_addr),
               ntohs(server_address.sin_port));
    }
    if (listen(listenfd, LISTENQ) == -1)
    {
        perror("Error in listening");
        exit(1);
    }
    printf("Server running... waiting for connections: %d\n", listenfd);
    connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_len);
    if (connfd == -1)
    {
        printf("Error in connecting: %d\n", connfd);
        perror("Error in accepting connection");
        exit(1);
    }

    while (1)
    {
        struct message msg;
        // Log in
        client_len = sizeof(client_address);
        printf("Is client connected: %d\n", isLogged);
        if (isLogged == 0)
        {
            n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
            if (n <= 0)
            {
                perror("Error recieving connection");
                char *ack = "Error recieving connection";
                send(connfd, ack, strlen(ack), 0);
                exit(1);
            }
            buffer[n] = '\0';
            printf("Buffer recieved: %s\n", buffer);
            msg.type = LOGIN;
            strcpy(msg.payload, buffer);
            struct message *msgptr1 = &msg;
            handleClient(msgptr1, connfd);
        }
        else
        {
            client_len = sizeof(client_address);
            n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
            if (n <= 0)
            {
                perror("Error recieving connection");
                char *ack = "Error recieving connection";
                send(connfd, ack, strlen(ack), 0);
                exit(1);
            }
            buffer[n] = '\0';
            printf("Buffer received: %s\n", buffer);
            if (strcmp(buffer, "bye") == 0)
            {
                isLogged = 0;
            }
            else
            {
                msg.type = TEXT_MESSAGE;
                strcpy(msg.payload, buffer);
                struct message *msgptr2 = &msg;
                handleClient(msgptr2, connfd);
            }
        }
    }
    close(listenfd);
    return 0;
}
