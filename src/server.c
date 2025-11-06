#include "../common/common.h"


User registered_users[USERS];
int num_registered_users = 0;
int server_fd;



User* find_registered_user(const char* username) {
    for (int i = 0; i < num_registered_users; ++i) {
        if (strcmp(registered_users[i].username, username) == 0) {
            return &registered_users[i];
        }
    }
    return NULL;
}


bool is_user_online(const char* username) {
    User* user = find_registered_user(username);
    if (user && user->online) {
        return true;
    }
    return false;
}


void broadcast_message(const char* sender_username,
     const char* message_text,
      const struct sockaddr_in* sender_addr, 
      socklen_t sender_addr_len) {


    Packet packet;
    strcpy(packet.type, "BROADCAST");
    strcpy(packet.sender, sender_username);
    strcpy(packet.message, message_text);
    strcpy(packet.status, "INFO");

    struct sockaddr_in broadcast_addr;
    socklen_t broadcast_addr_len = sizeof(broadcast_addr);

    for (int i = 0; i < num_registered_users; ++i) {
        if (registered_users[i].online) {
          
            
            if (sender_addr != NULL && sender_addr_len > 0) {
                if (memcmp(&registered_users[i].client_addr, sender_addr, sender_addr_len) == 0) {
                    continue;
                }
            }
            
            memcpy(&broadcast_addr, &registered_users[i].client_addr, sizeof(struct sockaddr_in));
            broadcast_addr_len = registered_users[i].client_addr_len;

            if (sendto(server_fd, &packet, sizeof(Packet), 0, (struct sockaddr*)&broadcast_addr, broadcast_addr_len) < 0) {
                perror("sendto broadcast failed");
            }
        }
    }
}




int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    Packet packet;

    
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

   
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(PORT);      

    if (bind(server_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("server running on the port %d...\n", PORT);

 
    for (int i = 0; i < USERS; ++i) {
        registered_users[i].online = false;
    }

    while (1) {
      

        ssize_t bytes_received = recvfrom(server_fd, &packet, sizeof(Packet), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytes_received < 0) {
            perror("recvfrom failed");
            continue;
        }

       
        if (strcmp(packet.type, "REGISTER") == 0) {
            if (num_registered_users >= USERS) {
                strcpy(packet.response, "[SERVER ERROR] Max users reached.");
                strcpy(packet.status, "ERROR");
            } else {
                User* existing_user = find_registered_user(packet.sender);
                if (existing_user) {
                    strcpy(packet.response, "[SERVER ERROR] Username already exists.");
                    strcpy(packet.status, "ERROR");
                } else {
                    strcpy(registered_users[num_registered_users].username, packet.sender);
                    strcpy(registered_users[num_registered_users].password, packet.message); 
                    registered_users[num_registered_users].online = false;
                    num_registered_users++;
                    strcpy(packet.response, "[SERVER OK] Registration successful. Please login.");
                    strcpy(packet.status, "OK");
                }
            }
            sendto(server_fd, &packet, sizeof(Packet), 0, (const struct sockaddr*)&client_addr, client_addr_len);

        } else if (strcmp(packet.type, "LOGIN") == 0) {
            User* user = find_registered_user(packet.sender);
            if (!user) {
                strcpy(packet.response, "[SERVER ERROR] User not registered.");
                strcpy(packet.status, "ERROR");
            } else if (user->online) {
                strcpy(packet.response, "[SERVER ERROR] User already logged in.");
                strcpy(packet.status, "ERROR");
            } else if (strcmp(user->password, packet.message) != 0) {
                strcpy(packet.response, "[SERVER ERROR] Invalid password.");
                strcpy(packet.status, "ERROR");
            } else {
                user->online = true;
                memcpy(&user->client_addr, &client_addr, sizeof(struct sockaddr_in));
                user->client_addr_len = client_addr_len;

                sprintf(packet.response, "[SERVER OK] Login successful. Welcome to the chat!");
                strcpy(packet.status, "OK");

                sendto(server_fd, &packet, sizeof(Packet), 0, (const struct sockaddr*)&client_addr, client_addr_len);

                char notification[BUFF_SIZE];
                sprintf(notification, "--- %s has joined the chat. ---", packet.sender);
              
                broadcast_message(packet.sender, notification, &client_addr, client_addr_len); 
            }
            
            if (strcmp(packet.status, "ERROR") == 0) {
                sendto(server_fd, &packet, sizeof(Packet), 0, (const struct sockaddr*)&client_addr, client_addr_len);
            }

        } else if (strcmp(packet.type, "LOGOUT") == 0) {
            User* user = find_registered_user(packet.sender);
            if (user && user->online) {
                user->online = false;
                strcpy(packet.response, "[SERVER OK] Logout successful.");
                strcpy(packet.status, "OK");
                sendto(server_fd, &packet, sizeof(Packet), 0, (const struct sockaddr*)&client_addr, client_addr_len);

                char notification[BUFF_SIZE];
                sprintf(notification, "--- %s has left the chat. ---", packet.sender);
               
                broadcast_message(packet.sender, notification, &client_addr, client_addr_len); 
            } else {
                strcpy(packet.response, "[SERVER ERROR] You are not logged in.");
                strcpy(packet.status, "ERROR");
                sendto(server_fd, &packet, sizeof(Packet), 0, (const struct sockaddr*)&client_addr, client_addr_len);
            }

        } else if (strcmp(packet.type, "SEND") == 0) {
            User* sender_user = find_registered_user(packet.sender);
            if (!sender_user || !sender_user->online) {
                strcpy(packet.response, "[SERVER ERROR] You are not logged in.");
                strcpy(packet.status, "ERROR");
                sendto(server_fd, &packet, sizeof(Packet), 0, (const struct sockaddr*)&client_addr, client_addr_len);
                continue;
            }

            strcpy(packet.type, "BROADCAST");
            strcpy(packet.status, "INFO");
            strcpy(packet.response, ""); 




            broadcast_message(packet.sender, packet.message, &client_addr, client_addr_len);

        } else if (strcmp(packet.type, "USERS") == 0) {
            User* sender_user = find_registered_user(packet.sender);
            if (!sender_user || !sender_user->online) {
                strcpy(packet.response, "[SERVER ERROR] You are not logged in.");
                strcpy(packet.status, "ERROR");
            } else {
                char user_list[BUFF_SIZE] = "--- Active Users ---\n";
                for (int i = 0; i < num_registered_users; ++i) {
                    if (registered_users[i].online) {
                        strcat(user_list, registered_users[i].username);
                        strcat(user_list, "\n");
                    }
                }
                strcat(user_list, "--------------------");
                strcpy(packet.response, user_list);
                strcpy(packet.status, "OK");
            }
            sendto(server_fd, &packet, sizeof(Packet), 0, (const struct sockaddr*)&client_addr, client_addr_len);
        }
    }

    close(server_fd);
    return 0;
}
