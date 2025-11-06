#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <pthread.h>

#define USERS 10
#define BUFF_SIZE 1024
#define USERNAME_LEN 50
#define PASSWORD_LEN 50


#define PORT 12345
#define SERVER_IP "127.0.0.1"


typedef struct {
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    bool online;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
} User; 


typedef struct {
    char type[20];
    char sender[USERNAME_LEN];
    char recipient[PASSWORD_LEN];
    char message[BUFF_SIZE - 100];  /* command */
    char status[50];
    char response[BUFF_SIZE];
}Packet;



#endif