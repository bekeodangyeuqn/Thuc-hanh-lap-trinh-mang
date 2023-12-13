#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define LISTENQ 1

int num_clients = 0;

int listenfd, connfd, max_fd, activity, new_socket, sd, valread;
int connfds[MAX_CLIENTS];
int inputFlag[MAX_CLIENTS];
int countLogin = 0;
fd_set read_fds, ready_fds;
struct sockaddr_in server_address, client_address;
socklen_t client_len;

struct timeval tv;

typedef struct KHACHHANG
{
    char username[100];
    char password[100];
    int status;
    struct KHACHHANG *left;
    struct KHACHHANG *right;
} KHACHHANG;

typedef KHACHHANG *TREE;

TREE client[MAX_CLIENTS];

TREE temp = NULL;

void initTree(TREE *t)
{
    *t = NULL;
}

void addNodeToTree(char username[100], char password[100], int status, TREE *t)
{
    if (*t == NULL)
    {
        TREE p = (KHACHHANG *)malloc(sizeof(KHACHHANG));
        strcpy(p->username, username);
        strcpy(p->password, password);
        p->status = status;
        p->left = NULL;
        p->right = NULL;
        *t = p;
    }
    else
    {
        if (strcmp((*t)->username, username) > 0)
            addNodeToTree(username, password, status, &(*t)->left);
        else if (strcmp((*t)->username, username) < 0)
            addNodeToTree(username, password, status, &(*t)->right);
    }
}

int havespace(char s[100])
{
    int n = strlen(s);
    int boo = 0;
    int i;
    for (i = 0; i < n; i++)
    {
        if (s[i] == ' ')
        {
            boo = 1;
            break;
        }
    }
    return boo;
}

TREE searchDataInTree(char username[100], TREE t)
{
    if (t == NULL || (strcmp(t->username, username) == 0))
        return t;
    if (strcmp(t->username, username) > 0)
        return searchDataInTree(username, t->left);
    return searchDataInTree(username, t->right);
}

TREE mostLeftChild(TREE t)
{
    TREE temp1 = t;
    while (temp1 && temp1->left != NULL)
        temp1 = temp1->left;
    return temp1;
}

TREE deleteNode(TREE t, char username[100])
{
    if (t == NULL)
        return t;
    else if (strcmp(t->username, username) < 0)
        t->right = deleteNode(t->right, username);
    else if (strcmp(t->username, username) > 0)
        t->left = deleteNode(t->left, username);
    else
    {
        if (t->left == NULL)
        {
            TREE temp = t->right;
            free(t);
            return temp;
        }
        else if (t->right == NULL)
        {
            TREE temp = t->left;
            free(t);
            return temp;
        }
        TREE temp = mostLeftChild(t->right);
        strcpy(t->username, temp->username);
        strcpy(t->password, temp->password);
        t->status = temp->status;
        t->right = deleteNode(t->right, temp->username);
    }
    return t;
}

void readFile(FILE *fp, TREE *t)
{
    fp = fopen("nguoidung.txt", "r");
    if (fp == NULL)
    {
        printf("Nhap file khong hanh cong\n");
        exit(-1);
    }
    while (!feof(fp))
    {
        char username[100];
        char password[100];
        int status;
        fscanf(fp, "%s\t%s\t%d\n", username, password, &status);
        TREE p = searchDataInTree(username, *t);
        if (p == NULL)
        {
            addNodeToTree(username, password, status, t);
        }
    }
    fclose(fp);
}

void writeFile(TREE t, FILE *fp)
{
    if (t != NULL)
    {
        writeFile(t->left, fp);
        fprintf(fp, "%s\t%s\t%d\n", t->username, t->password, t->status);
        writeFile(t->right, fp);
    }
}

void saveFile(TREE t, FILE *fp)
{
    writeFile(t, fp);
    fclose(fp);
}

int checkValidPassword(char *password)
{
    int len = strlen(password);
    int isValid = 1;
    for (int i = 0; i < len; i++)
    {
        char ch = password[i];
        if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z'))
        {
            isValid = 0;
            break;
        }
    }
    return isValid;
}

void handleClient(int connfd, int i)
{
    TREE t;
    initTree(&t);
    FILE *fin;
    fin = fopen("nguoidung.txt", "r+");
    readFile(fin, &t);
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    TREE logged = NULL;
    char buffer[MAX_BUFFER_SIZE];

    int n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
    if (n <= 0)
    {
        getpeername(sd, (struct sockaddr *)&client_address, &client_len);
        printf("Host disconnected , ip %s , port %d \n",
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        FD_CLR(connfd, &read_fds);
        close(connfd);
        connfds[i] = -1;
        inputFlag[i] = 0;
        return;
    }
    buffer[n] = '\0';
    if (inputFlag[i] == 0)
    {
        char username[MAX_BUFFER_SIZE];
        strcpy(username, buffer);
        printf("username: %s\n", username);

        client[i] = searchDataInTree(username, t);
        if (client[i] == NULL)
        {
            char *ack = "Account not found";
            send(connfd, ack, strlen(ack), 0);
            FD_CLR(connfd, &read_fds);
            close(connfd);
            connfds[i] = -1;

            return;
        }
        char *ack = "Insert password";
        send(connfd, ack, strlen(ack), 0);
        inputFlag[i] = 1;
        strcpy(client[i]->username, username);
    }
    else if (inputFlag[i] == 1)
    {
        if (countLogin < 3)
        {
            // Receive the password
            temp = client[i];
            // n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
            // buffer[n] = '\0';
            char password[MAX_BUFFER_SIZE];
            strcpy(password, buffer);
            printf("Password: %s\n", password);
            if (strcmp(temp->password, password) != 0)
            {
                countLogin++;
                char *ack = "Not OK";
                send(connfd, (const char *)ack, strlen(ack), 0);

                if (countLogin >= 3)
                {
                    t = deleteNode(t, temp->username);
                    addNodeToTree(temp->username, temp->password, 0, &t);
                    FILE *fp;
                    fp = fopen("nguoidung.txt", "r+");
                    saveFile(t, fp);
                    char *ack = "Account is blocked";
                    send(connfd, (const char *)ack, strlen(ack), 0);
                    FD_CLR(connfd, &read_fds);
                    close(connfd);
                    connfds[i] = -1;
                    inputFlag[i] = 0;
                }
                return;
            }
        }

        if (temp->status == 1)
        {
            logged = temp;
            printf("Login as username: %s\n", logged->username);

            char *ack = "OK";
            send(connfd, (const char *)ack, strlen(ack), 0);
            inputFlag[i] = 2;
            // n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
            // buffer[n] = '\0';
        }
        else if (temp->status == 0 || temp->status == 2)
        {
            char *ack4 = "Account not ready";
            send(connfd, (const char *)ack4, strlen(ack4), 0);
        }
    }
    else if (inputFlag[i] == 2)
    {
        logged = client[i];
        if (strcmp(buffer, "bye") == 0)
        {
            char bye[150] = "Goodbye ";
            strcat(bye, logged->username);
            send(connfd, (const char *)bye, strlen(bye), 0);

            logged = NULL;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Port number>\n", argv[0]);
        return 1;
    }

    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        connfds[i] = -1;
        inputFlag[i] = 0;
        client[i] = NULL;
    }
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
    printf("Server running... waiting for connections\n");

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(listenfd, &read_fds);
        max_fd = listenfd;

        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            connfd = connfds[i];
            if (connfd > 0)
            {
                FD_SET(connfd, &read_fds);
                if (connfd > max_fd)
                {
                    max_fd = connfd;
                }
            }
        }

        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0)
        {
            perror("Select error");
            exit(1);
        }

        if (FD_ISSET(listenfd, &read_fds))
        {
            new_socket = accept(listenfd, (struct sockaddr *)&client_address, &client_len);
            // max_fd = connfd;
            if (new_socket <= 0)
            {
                perror("Accepting client connection failed");
                continue;
            }
            printf("New client connected: %d\n", new_socket);

            // Add the new client socket to the array
            for (int i = 0; i < MAX_CLIENTS; ++i)
            {
                if (connfds[i] == -1)
                {
                    connfds[i] = new_socket;
                    printf("Client: connfds[%d] = %d\n", i, new_socket);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            sd = connfds[i];
            if (FD_ISSET(sd, &read_fds))
            {
                printf("Handling client with fd: %d\n", sd);
                handleClient(sd, i);
            }
        }
    }
    close(listenfd);
    return 0;
}
