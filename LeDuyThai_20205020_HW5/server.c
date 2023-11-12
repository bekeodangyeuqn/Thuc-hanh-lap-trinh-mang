#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 1024
#define LISTENQ 1

typedef struct KHACHHANG
{
    char username[100];
    char password[100];
    int status;
    struct KHACHHANG *left;
    struct KHACHHANG *right;
} KHACHHANG;

typedef KHACHHANG *TREE;

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
    TREE temp = t;
    while (temp && temp->left != NULL)
        temp = temp->left;
    return temp;
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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Port number>\n", argv[0]);
        return 1;
    }

    TREE t;
    initTree(&t);
    FILE *fin;
    fin = fopen("nguoidung.txt", "r+");
    readFile(fin, &t);

    TREE logged = NULL;

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
    connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_len);
    if (connfd == -1)
    {
        perror("Error in accepting connection");
        exit(1);
    }

    while (1)
    {
        client_len = sizeof(client_address);
        // printf("%d\n", connfd);
        // printf("Recieved request...%d\n", connfd);
        // Receive the username
        n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
        buffer[n] = '\0';
        char username[MAX_BUFFER_SIZE];
        strcpy(username, buffer);
        printf("username: %s\n", username);

        TREE temp = searchDataInTree(username, t);
        if (temp == NULL)
        {
            char *ack = "Account not found";
            send(connfd, ack, strlen(ack), 0);
        }
        else
        {
            printf("username: %s\n", temp->username);
            char *ack = "Insert password";
            send(connfd, ack, strlen(ack), 0);
            int countLogin = 0;
            while (countLogin < 3)
            {
                // Receive the password
                n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
                buffer[n] = '\0';
                char password[MAX_BUFFER_SIZE];
                strcpy(password, buffer);
                printf("Password: %s\n", password);
                if (strcmp(temp->password, password) != 0)
                {
                    countLogin++;
                    char *ack = "Not OK";
                    send(connfd, (const char *)ack, strlen(ack), 0);
                }
                else
                {
                    break;
                }
                if (countLogin >= 3)
                {
                    t = deleteNode(t, username);
                    addNodeToTree(username, temp->password, 0, &t);
                    FILE *fp;
                    fp = fopen("nguoidung.txt", "r+");
                    saveFile(t, fp);
                    char *ack = "Account is blocked";
                    send(connfd, (const char *)ack, strlen(ack), 0);
                    break;
                }
            }

            if (temp->status == 1)
            {
                logged = temp;
                printf("Login as username: %s\n", logged->username);

                char *ack = "OK";
                send(connfd, (const char *)ack, strlen(ack), 0);
                while (1)
                {
                    // Recieve new password
                    n = recv(connfd, (char *)buffer, MAX_BUFFER_SIZE, 0);
                    buffer[n] = '\0';
                    if (strcmp(buffer, "bye") == 0)
                    {
                        char bye[150] = "Goodbye ";
                        strcat(bye, logged->username);
                        send(connfd, (const char *)bye, strlen(bye), 0);

                        logged = NULL;
                        break;
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
                        printf("Change Password as: %s - %s\n", logged->username, username);
                        TREE t2 = deleteNode(t, logged->username);
                        addNodeToTree(username, newPassword, logged->status, &t2);
                        FILE *fp;
                        fp = fopen("nguoidung.txt", "r+");
                        saveFile(t2, fp);
                        t = t2;
                        char chars[256] = "";   // String to store characters
                        char numbers[256] = ""; // String to store numbers

                        int char_index = 0;
                        int number_index = 0;

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
                    }
                }
            }
            else if (temp->status == 0 || temp->status == 2)
            {
                char *ack4 = "Account not ready";
                send(connfd, (const char *)ack4, strlen(ack4), 0);
                break;
            }
        }
    }
    close(connfd);
    return 0;
}
