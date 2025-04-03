#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#define BUFFER_SIZE 512
#define TIMEOUT_SECONDS 5

int tftp_send_file(const char *server, int port, const char *filename) {
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return -1;
        }
    #endif

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        #ifdef _WIN32
            WSACleanup();
        #endif
        return -1;
    }

    // Set socket timeout for receiving ACKs
    #ifdef _WIN32
        DWORD timeout = TIMEOUT_SECONDS * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    #else
        struct timeval tv;
        tv.tv_sec = TIMEOUT_SECONDS;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    #endif

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server);

    // Extract just the base filename without path
    const char *base_filename = filename;
    const char *last_slash = strrchr(filename, '\\');
    if (last_slash) {
        base_filename = last_slash + 1;
    } else {
        last_slash = strrchr(filename, '/');
        if (last_slash) {
            base_filename = last_slash + 1;
        }
    }

    // Open file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        closesocket(sock);
        #ifdef _WIN32
            WSACleanup();
        #endif
        return -1;
    }

    // Create WRQ packet
    char buffer[BUFFER_SIZE + 4];
    memset(buffer, 0, sizeof(buffer));
    *(uint16_t*)buffer = htons(OPCODE_WRQ);
    strcpy(buffer + 2, base_filename);
    strcpy(buffer + 2 + strlen(base_filename) + 1, "octet");
    size_t wrq_size = 2 + strlen(base_filename) + 1 + strlen("octet") + 1;

    // Send WRQ and wait for ACK
    int max_retries = 5;
    int retries = 0;
    int received_ack = 0;
    struct sockaddr_in from_addr;
    int from_len = sizeof(from_addr);
    uint16_t block_number = 0;
    
    while (retries < max_retries && !received_ack) {
        // Send WRQ
        if (sendto(sock, buffer, wrq_size, 0, 
            (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            fclose(file);
            closesocket(sock);
            #ifdef _WIN32
                WSACleanup();
            #endif
            return -1;
        }
        
        // Wait for ACK or ERROR
        int recv_len = recvfrom(sock, buffer, sizeof(buffer), 0, 
            (struct sockaddr*)&from_addr, &from_len);
            
        if (recv_len >= 4) {  // Minimum size for ACK or ERROR packet
            uint16_t opcode = ntohs(*(uint16_t*)buffer);
            
            if (opcode == OPCODE_ACK && ntohs(*(uint16_t*)(buffer + 2)) == 0) {
                received_ack = 1;  // Got ACK for WRQ
                // Use this address for future packets (might be different from initial server_addr)
                server_addr = from_addr;
            } else if (opcode == OPCODE_ERROR) {
                // Server rejected WRQ with an error
                fclose(file);
                closesocket(sock);
                #ifdef _WIN32
                    WSACleanup();
                #endif
                return -1;
            }
        } else {
            retries++;
        }
    }
    
    if (!received_ack) {
        fclose(file);
        closesocket(sock);
        #ifdef _WIN32
            WSACleanup();
        #endif
        return -1;  // Couldn't get ACK for WRQ
    }
    
    // Start sending data blocks
    block_number = 1;
    int transfer_complete = 0;
    
    while (!transfer_complete) {
        // Reset buffer and prepare DATA packet
        memset(buffer, 0, sizeof(buffer));
        *(uint16_t*)buffer = htons(OPCODE_DATA);
        *(uint16_t*)(buffer + 2) = htons(block_number);
        
        // Read file block
        size_t bytes_read = fread(buffer + 4, 1, BUFFER_SIZE, file);
        
        // Send DATA packet with retries
        retries = 0;
        received_ack = 0;
        
        while (retries < max_retries && !received_ack) {
            // Send DATA
            if (sendto(sock, buffer, bytes_read + 4, 0,
                (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
                break;
            }
            
            // Wait for ACK
            int recv_len = recvfrom(sock, buffer, sizeof(buffer), 0,
                (struct sockaddr*)&from_addr, &from_len);
                
            if (recv_len >= 4) {
                uint16_t opcode = ntohs(*(uint16_t*)buffer);
                
                if (opcode == OPCODE_ACK && ntohs(*(uint16_t*)(buffer + 2)) == block_number) {
                    received_ack = 1;
                } else if (opcode == OPCODE_ERROR) {
                    fclose(file);
                    closesocket(sock);
                    #ifdef _WIN32
                        WSACleanup();
                    #endif
                    return -1;
                }
            } else {
                retries++;
            }
        }
        
        if (!received_ack) {
            fclose(file);
            closesocket(sock);
            #ifdef _WIN32
                WSACleanup();
            #endif
            return -1;
        }
        
        // Check if transfer is complete (less than full block sent)
        if (bytes_read < BUFFER_SIZE) {
            transfer_complete = 1;
        } else {
            block_number++;
        }
    }

    fclose(file);
    closesocket(sock);
    #ifdef _WIN32
        WSACleanup();
    #endif
    return 0;
}
