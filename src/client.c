#include "../common/common.h"




int client_socket;
struct sockaddr_in server_addr;
socklen_t server_addr_len = sizeof(server_addr);
char current_username[USERNAME_LEN] = "";
bool is_logged_in = false;



void send_to_server(Packet* packet) {
    if (sendto(client_socket, packet, sizeof(Packet), 0, 
    (const struct sockaddr*)&server_addr, server_addr_len) < 0) {
        perror("sendto failed");
    }
}

void* receive_messages(void* arg) {
    Packet packet;
    struct sockaddr_in from_addr;
    socklen_t from_addr_len = sizeof(from_addr);

    while (1) {
        ssize_t bytes_received = recvfrom(client_socket, &packet, 
            sizeof(Packet), 0, (struct sockaddr*)&from_addr, &from_addr_len);
        if (bytes_received < 0) {
            perror("recvfrom failed");
            continue;
        }

        
        if (from_addr.sin_port != server_addr.sin_port 
            || strcmp(inet_ntoa(from_addr.sin_addr), SERVER_IP) != 0) {
            continue;
        }

        if (strcmp(packet.type, "BROADCAST") == 0) {
            if (strlen(current_username) > 0 && strcmp(packet.sender, current_username) != 0) {
                printf("%s > %s\n", packet.sender, packet.message);
                printf("%s > ", current_username);
                fflush(stdout);
            }

        } else if (strcmp(packet.type, "REGISTER") == 0 || strcmp(packet.type, "LOGIN") == 0 || 
                   strcmp(packet.type, "LOGOUT") == 0 || strcmp(packet.type, "USERS") == 0 || 
                   strcmp(packet.type, "ACK") == 0) { 
            if (strlen(packet.response) > 0) {
                printf("%s\n", packet.response);
            }
            if (strcmp(packet.status, "OK") == 0 && strcmp(packet.type, "LOGIN") == 0) {
               
                strcpy(current_username, packet.sender); 
                
                is_logged_in = true;
            }
           
            
            if (strlen(current_username) > 0) {
                printf("%s > ", current_username);
                fflush(stdout);
            }
        } else {
          
            if (strlen(packet.response) > 0) {
                printf("[%s] %s\n", packet.status, packet.response);
            }
        }
    }
    return NULL;
}




int main() {
    char command[BUFF_SIZE];
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    char message_text[BUFF_SIZE];

/*-------------------------------------------------------------------------*/


    /* Make UDP socket */
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }



    /*settings*/
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet  failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    server_addr_len = sizeof(server_addr);

    
    pthread_t receiver_thread;
    if (pthread_create(&receiver_thread, NULL, receive_messages, NULL) != 0) {
        perror("pthread_create failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Welcome to UDP chat!\n");
    printf("commands /register <username> <password>",
        " /login <username> <password>",
         "/send <message>",
        " /users, /quit\n");

    while (1) {
        
        if (strlen(current_username) > 0) {
            printf("%s > ", current_username);
        } else {
            printf("Enter command: ");
        }
        fflush(stdout); 

        if (fgets(command, sizeof(command), stdin) == NULL) {
            if (strlen(current_username) > 0) {
                strcpy(command, "/quit"); 
            } else {
                break; 
            }
        }
        command[strcspn(command, "\n")] = 0; 
        Packet packet;
        memset(&packet, 0, sizeof(Packet));

        if (strncmp(command, "/register", 9) == 0) {
            sscanf(command, "/register %s %s", username, password);
            strcpy(packet.type, "REGISTER");
            strcpy(packet.sender, username);
            strcpy(packet.message, password);
            send_to_server(&packet);

        } else if (strncmp(command, "/login", 6) == 0) {
            sscanf(command, "/login %s %s", username, password);
            strcpy(packet.type, "LOGIN");
            strcpy(packet.sender, username);
            strcpy(packet.message, password);
            send_to_server(&packet);

        } else if (strlen(current_username) > 0 && strncmp(command, "/send", 5) == 0) {
            char* message_start = strstr(command, " ");
            if (message_start != NULL) {
                message_start++; 
                strcpy(packet.message, message_start);
                strcpy(packet.type, "SEND");
                strcpy(packet.sender, current_username);
                send_to_server(&packet);
            } else {
                printf("Usage: /send <message>\n");
            }

        } else if (strlen(current_username) > 0 && strcmp(command, "/users") == 0) {
            strcpy(packet.type, "USERS");
            strcpy(packet.sender, current_username);
            send_to_server(&packet);

        } else if (strcmp(command, "/quit") == 0) {
            if (strlen(current_username) > 0) { 
                strcpy(packet.type, "LOGOUT");
                strcpy(packet.sender, current_username);
                send_to_server(&packet);
            }
            printf("Disconnecting...\n");
           
            break; 
        } else {
            
            if (strlen(current_username) > 0) {
                strcpy(packet.message, command);
                strcpy(packet.type, "SEND");
                strcpy(packet.sender, current_username);
                send_to_server(&packet);
            } else {
                printf("Unknown command. Please use /register or /login first.\n");
            }
        }
    }

    close(client_socket);
    
   
    return 0;
}
