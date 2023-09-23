#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct KHACHHANG{
	char username[100];
	char password[100];
	int status;
	struct KHACHHANG *left;
	struct KHACHHANG *right;
} KHACHHANG;

typedef KHACHHANG* TREE;

void initTree(TREE *t){
	*t = NULL;
}

void addNodeToTree(char username[100], char password[100], int status, TREE *t){
	if (*t == NULL){
		TREE p = (KHACHHANG*)malloc(sizeof(KHACHHANG));
		strcpy(p->username, username);
		strcpy(p->password, password);
		p->status = status;
		p->left = NULL;
		p->right = NULL;
		*t = p;
	} else {
		if (strcmp((*t)->username, username) > 0)
			addNodeToTree(username, password, status, &(*t)->left);
		else if (strcmp((*t)->username, username) < 0)
			addNodeToTree(username, password, status, &(*t)->right);
	}
}

int havespace(char s[100]){
	int n = strlen(s);
	int boo = 0;
	int i;
	for (i=0; i<n; i++){
		if (s[i] == ' '){
			boo = 1;
			break;
		}
	}
	return boo;
}

TREE searchDataInTree(char username[100], TREE t){
	if (t == NULL || (strcmp(t->username, username) == 0)) return t;
	if (strcmp(t->username, username) > 0)
		return searchDataInTree(username, t->left);
	return searchDataInTree(username,t->right);
}

TREE mostLeftChild(TREE t){
	TREE temp = t;
	while (temp && temp->left != NULL)
		temp = temp->left;
	return temp;
}

TREE deleteNode(TREE t, char username[100]){
	if (t == NULL) return t;
	else if (strcmp(t->username,username) < 0)
		t->right = deleteNode(t->right,username);
	else if (strcmp(t->username,username) > 0)
		t->left = deleteNode(t->left,username);
	else {
		if (t->left == NULL){
			TREE temp = t->right;
			free(t);
			return temp;
		}
		else if (t->right == NULL){
			TREE temp = t->left;
			free(t);
			return temp;
		}
		TREE temp = mostLeftChild(t->right);
		strcpy(t->username,temp->username);
		strcpy(t->password,temp->password) ;
		t->status = temp->status;
		t->right =  deleteNode(t->right,temp->username);
	}
	return t;
}

void readFile(FILE *fp, TREE *t){
	fp = fopen("nguoidung.txt", "r");
	if (fp == NULL) {
		printf("Nhap file khong hanh cong");
		exit(-1);
	}
	while (!feof(fp)){
		char username[100];
		char password[100];
		int status;
		fscanf(fp, "%s\t%s\t%d\n",username, password, &status);
		TREE p = searchDataInTree(username, *t);
		if (p == NULL){
			addNodeToTree(username, password, status, t);
		}
	}
	fclose(fp);
}

void writeFile(TREE t, FILE *fp){
	if (t != NULL){
		writeFile(t->left, fp);
		fprintf(fp, "%s\t%s\t%d\n", t->username, t->password, t->status);
		writeFile(t->right, fp);
	}
}

void saveFile(TREE t, FILE *fp){
	writeFile(t,fp);
	fclose(fp);
}

void printData(TREE t){
	if (t!=NULL){
		printData(t->left);
		printf("%s\t%s\t%d\n",t->username,t->password, t->status);
		printData(t->right);
	}
}

void clearNewline(char string[100]){
	int len = strlen(string);
	if (len > 0 && string[len - 1] == '\n') {
            string[len - 1] = '\0';  // Replace '\n' with '\0'
    }
}

void clean_stdin(void)
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

void displayMenu(FILE *fin, TREE t){
	int choice;
	TREE logged = NULL;
	while(1){
		fin = fopen("nguoidung.txt", "r+");
		readFile(fin,&t);
		TREE readT = t;
//		printData(readT);
		fclose(fin);
		printf("\nUSER MANAGEMENT PROGRAM\n");
		int i;
		for (i = 1; i <= 35; i++)
			printf("-");
		if (logged != NULL)
			printf("\nLogin at: %s\n", logged->username);
		printf("\n1. Register\n2. Activate\n3. Sign in\n4. Search\n5. Change password\n6. Sign out\n");
		printf("Your choice (1-6, other to quit): "); scanf("%d", &choice);
		if (choice < 1 || choice > 6)
			exit(-1);
		else if (choice == 1) {
			char username[100];
			char password[100];
			printf("REGISTER\n");
			printf("Username:");clean_stdin();fgets(username, sizeof(username), stdin);clearNewline(username);
			TREE temp = searchDataInTree(username, readT);
			while (temp != NULL) {
				printf("Account existed.\n");
				printf("Username:");clean_stdin();fgets(username, sizeof(username), stdin);
				temp = searchDataInTree(username, readT);
			}
			printf("Password:");fgets(password, sizeof(password), stdin); clearNewline(password);
			if (havespace(password) != 0){
				printf("Password must not have space\n");
			} else {
				addNodeToTree(username, password, 2, &readT);
				FILE *fp;
				fp = fopen("nguoidung.txt", "w+");
				saveFile(readT, fp);
				printf("Successful registration. Activation required.\n");
			}
		} else if (choice == 2) {
			printf("ACTIVATE\n");
//			printData(readT);
			char username[100];
			char password[100];
			printf("Username:");clean_stdin();fgets(username, sizeof(username), stdin); clearNewline(username);
			printf("Password:");fgets(password, sizeof(password), stdin); clearNewline(password); 
			TREE temp = searchDataInTree(username, readT);
			if (temp == NULL){
				printf("Cannot find account\n");
			} else if (strcmp(temp->password, password) != 0) {
				printf("Password is incorrect\n");
			} else {
				int countActivate = 0;
				while (countActivate < 4){
					char activateCode[20];
					printf("Activate code:");fgets(activateCode, sizeof(activateCode), stdin); clearNewline(activateCode); 
					if (strcmp("20205020", activateCode) != 0){
						countActivate++;
						printf("Account is not activated\n");
					} else {
						FILE *fp;
						fp = fopen("nguoidung.txt", "w+");
						readT = deleteNode(readT,username);
						addNodeToTree(username, password, 1, &readT);
						saveFile(readT, fp);
						printf("Account is activated\n");
						break;
					}
				}
				if (countActivate >= 4){
					FILE *fp;
					fp = fopen("nguoidung.txt", "w+");
					readT = deleteNode(readT,username);
					addNodeToTree(username, temp->password, 0, &readT);
					saveFile(readT, fp);
					printf("Activation code is incorrect. Account is blocked\n");
				}
			}
		} else if (choice == 3) {
//			printData(t);
			printf("SIGN IN\n");
			if (logged != NULL)
				printf("You are loging as %s. You need to sign out before sign in again.\n", logged->username);
			else {
				char username[100];
				printf("Username:");clean_stdin();fgets(username, sizeof(username), stdin); clearNewline(username);
				TREE temp = searchDataInTree(username, readT);
				if (temp == NULL){
					printf("Cannot find account\n");
				} else {
					int countLogin = 0;
					while (countLogin < 3){
						char password[100];
						printf("Password:");fgets(password, sizeof(password), stdin); clearNewline(password);
						if (strcmp(temp->password, password) != 0) {
							countLogin++;
							printf("Password is incorrect. You have %d times left\n", 3-countLogin);
						} else {
							printf("Hello %s\n", temp->username);
							logged = temp;
							break;
						}
					}
					if (countLogin >= 3){
						FILE *fp;
						fp = fopen("nguoidung.txt", "w+");
						readT = deleteNode(readT,username);
						addNodeToTree(username, temp->password, 0, &readT);
						saveFile(readT, fp);
						printf("Password is incorrect. Account is blocked\n");
					} 
				}	
			}
		} else if (choice == 4) {
			printf("SEARCH\n");
			char username[100];
			printf("Username:");clean_stdin();fgets(username, sizeof(username), stdin); clearNewline(username); 
			TREE temp = searchDataInTree(username, t);
			if (logged == NULL) {
				printf("Account is not sign in\n");
			} else if (logged->status == 0){
				printf("Account is blocked\n");
			} else if (temp == NULL) {
				printf("Cannot find account\n");
			} else if (temp->status == 0) {
				printf("Account is blocked\n");
			} else if (temp->status == 1) {
				printf("Account is active\n");
			} else if (temp->status == 2) {
				printf("Account is not active\n");
			}
		} else if (choice == 5) {
			printf("CHANGE PASSWORD\n");
			char password[100];
			char newPassword[100];
			if (logged == NULL) 
				printf("Account is not sign in\n");
			else if (logged->status == 0)
				printf("Account is blocked\n");
			else {
				while (1) {
					printf("Username: %s\n", logged->username);
					char username[100];
					strcpy(username, logged->username);
					printf("Password:");clean_stdin();fgets(password, sizeof(password), stdin); clearNewline(password); 
					printf("NewPassword:");fgets(newPassword, sizeof(newPassword), stdin); clearNewline(newPassword); 
					if (strcmp(logged->password, password) == 0) {
						FILE *fp;
						fp = fopen("nguoidung.txt", "w+");
						readT = deleteNode(readT, logged->username);
						addNodeToTree(username, newPassword, logged->status, &readT);
						saveFile(readT, fp);
						printf("Password is changed\n");
						break;
					} else {
						printf("Current password is incorrect. Please try again");
					}
				}
			}
		} else if (choice == 6) {
			printf("SIGN OUT\n");
			char username[100];
			printf("Username:");clean_stdin();fgets(username, sizeof(username), stdin); clearNewline(username); 
			if (logged == NULL) 
				printf("Account is not sign in\n");
			else if (strcmp(logged->username, username) == 0) {
				printf("Goodbye %s\n", logged->username);
				logged = NULL;
			} else {
				printf("Cannot find account\n");
			}
		}
		int k;
		for (k=1; k<= 35; k++)
			printf("#");
		printf("\n");
	}
}

int main(){
	TREE t;
	initTree(&t);
	FILE *fin;
	fin = fopen("nguoidung.txt", "r+");
//	readFile(fin,&t);
//	printData(t);
	displayMenu(fin, t);
	fclose(fin);
}

