#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define LISTENQ 1

int num_clients = 0;

int listenfd, connfd, max_fd, activity, new_socket;
int connfds[MAX_CLIENTS];
int inputFlag[MAX_CLIENTS];
int countLogin[MAX_CLIENTS];
struct sockaddr_in server_address, client_address;
socklen_t client_len;

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
    fclose(fin);

    TREE logged = NULL;
    char buffer[MAX_BUFFER_SIZE];

    int n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
    if (n <= 0)
    {
        getpeername(connfd, (struct sockaddr *)&client_address, &client_len);
        printf("Host disconnected , ip %s , port %d \n",
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
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
            return;
        }
        char *ack = "Insert password";
        send(connfd, ack, strlen(ack), 0);
        inputFlag[i] = 1;
        strcpy(client[i]->username, username);
    }
    else if (inputFlag[i] == 1)
    {
        if (countLogin[i] < 3)
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
                countLogin[i]++;
                char *ack = "Not OK";
                send(connfd, (const char *)ack, strlen(ack), 0);

                if (countLogin[i] >= 3)
                {
                    t = deleteNode(t, temp->username);
                    addNodeToTree(temp->username, temp->password, 0, &t);
                    FILE *fp;
                    fp = fopen("nguoidung.txt", "r+");
                    saveFile(t, fp);
                    char *ack = "Account is blocked";
                    send(connfd, (const char *)ack, strlen(ack), 0);
                    inputFlag[i] = 0;
                    fclose(fp);
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
            // for (int j = 0; j < MAX_CLIENTS; j++)
            // {
            //     if (j == i)
            //     {
            //         printf("Logging connfd: %d\n", connfds[j]);
            //         continue;
            //     }
            //     else if (inputFlag[j] == 2)
            //     {
            //         printf("Connecting connfd: %d\n", connfds[j]);
            //         char res[MAX_BUFFER_SIZE];
            //         char snum[3];
            //         sprintf(snum, "%d", connfds[i]);
            //         strcpy(res, logged->username);
            //         strcat(res, " has logged in with client id is ");
            //         strcat(res, snum);
            //         send(connfds[j], (const char *)res, strlen(res), 0);
            //     }
            // }
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
        // Recieve new password
        if (strcmp(buffer, "bye") == 0)
        {
            char bye[150] = "Goodbye ";
            strcat(bye, logged->username);
            send(connfd, (const char *)bye, strlen(bye), 0);
            inputFlag[i] = 0;
            logged = NULL;
            return;
        }
        char newPassword[MAX_BUFFER_SIZE];
        strcpy(newPassword, buffer);
        if (checkValidPassword(newPassword) == 0)
        {
            char *ack1 = "Error";
            send(connfd, (const char *)ack1, strlen(ack1), 0);
        }
        else
        {
            printf("Change Password as: %s\n", logged->username);
            TREE t2 = deleteNode(t, logged->username);
            addNodeToTree(logged->username, newPassword, logged->status, &t2);
            FILE *fp;
            fp = fopen("nguoidung.txt", "r+");
            saveFile(t2, fp);
            t = t2;
            char chars[256] = "";   // String to store characters
            char numbers[256] = ""; // String to store numbers

            int char_index = 0;
            int number_index = 0;
            fclose(fp);

            for (int i = 0; i < strlen(newPassword); i++)
            {
                if (isalpha(newPassword[i]))
                {
                    chars[char_index++] = newPassword[i];
                }
                else if (isdigit(newPassword[i]))
                {
                    numbers[number_index++] = newPassword[i];
                }
            }

            chars[char_index] = '\0';     // Null-terminate the character string
            numbers[number_index] = '\0'; // Null-terminate the number string
            char ack2[MAX_BUFFER_SIZE];
            strcpy(ack2, chars);
            strcat(ack2, "\n");
            strcat(ack2, numbers);
            send(connfd, (const char *)ack2, strlen(ack2), 0);
            // for (int j = 0; j < MAX_CLIENTS; j++)
            // {
            //     if (j == i)
            //         continue;
            //     else if (inputFlag[j] == 2)
            //     {
            //         char res[MAX_BUFFER_SIZE];
            //         char snum[3];
            //         sprintf(snum, "%d", connfds[i]);
            //         strcpy(res, logged->username);
            //         strcat(res, " has changed password in with client id is ");
            //         strcat(res, snum);
            //         send(connfds[j], (const char *)res, strlen(res), 0);
            //     }
            // }
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
        countLogin[i] = 0;
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

    struct pollfd fds[MAX_CLIENTS + 1]; // +1 for the server socket
    memset(fds, 0, sizeof(fds));

    fds[0].fd = listenfd;
    fds[0].events = POLLIN;

    while (1)
    {
        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            fds[i + 1].fd = connfds[i];
            fds[i + 1].events = POLLIN;
        }

        activity = poll(fds, num_clients + 1, 3500);

        if (activity < 0)
        {
            perror("Poll error");
            exit(1);
        }
        // Check if there is a new connection
        if (fds[0].revents & POLLIN)
        {
            connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_len);

            if (connfd == -1)
            {
                perror("Accepting client connection failed");
                continue;
            }

            printf("New client connected: %d\n", connfd);

            // Find an empty slot in the clients array
            int empty_index = -1;
            for (int i = 0; i < MAX_CLIENTS; ++i)
            {
                if (connfds[i] == -1)
                {
                    empty_index = i;
                    break;
                }
            }

            if (empty_index != -1)
            {
                connfds[empty_index] = connfd;
                if (empty_index + 1 > num_clients)
                {
                    num_clients = empty_index + 1;
                }
            }
            else
            {
                // Reject the connection if no empty slots are available
                send(connfd, "Server is full. Try again later.\n", 32, 0);
                close(connfd);
            }
        }

        // Check each client socket for data
        for (int i = 0; i < num_clients; ++i)
        {
            if (fds[i + 1].revents & POLLIN)
            {
                handleClient(connfds[i], i);
            }
        }
    }
    close(listenfd);
    return 0;
}
