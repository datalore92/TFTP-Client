#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H

#include <stdint.h>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

// TFTP OpCodes
#define OPCODE_RRQ   1
#define OPCODE_WRQ   2
#define OPCODE_DATA  3
#define OPCODE_ACK   4
#define OPCODE_ERROR 5

// Return codes
#define TFTP_SUCCESS       0
#define TFTP_ERROR_SOCKET -1
#define TFTP_ERROR_FILE   -2
#define TFTP_ERROR_SEND   -3
#define TFTP_ERROR_RECV   -4
#define TFTP_ERROR_TIMEOUT -5

/**
 * Initialize network subsystem (needed for Windows)
 * 
 * @return 0 on success, error code on failure
 */
int tftp_init();

/**
 * Cleanup network subsystem (needed for Windows)
 */
void tftp_cleanup();

/**
 * Send a file via TFTP protocol
 * 
 * @param server Server address
 * @param port Server port (usually 69 for TFTP)
 * @param filename Path to the file to send
 * @return 0 on success, error code on failure
 */
int tftp_send_file(const char *server, int port, const char *filename);

/**
 * Extract filename from path
 * 
 * @param path Full file path
 * @return Pointer to the filename part of the path
 */
const char* tftp_extract_filename(const char *path);

#endif // TFTP_CLIENT_H
