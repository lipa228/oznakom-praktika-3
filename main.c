#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <curl/curl.h>

#define PORT 8080
#define BUFFER_SIZE 2048

void handle_client(int);
void parse_query(const char *, char *);

int main(int argc, char* argv[]) {
    // address struct
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    // create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket...");
        exit(EXIT_FAILURE);
    }

    // binding socket
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Failed to bind socket...");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // listening socket
    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen socket...");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // HTTP server loop
    while (1) {
        // accepting connection
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}

void url_decode_with_curl(char *dst, const char *src) {
    char *decoded = curl_easy_unescape(NULL, src, 0, NULL);
    if (decoded) {
        strcpy(dst, decoded);
        curl_free(decoded);
    } else {
        strcpy(dst, src);
    }
}

void parse_query(const char *request, char *message) {
    // looking for start of query
    const char *query_start = strchr(request, '?');
    if (!query_start) {
        strcpy(message, "No message provided");
        return;
    }

    // looking for message=
    const char *msg_param = strstr(query_start, "message=");
    if (!msg_param) {
        strcpy(message, "Message parameter not found");
        return;
    }

    msg_param += 8;     // skipping message=
    char encoded_msg[1024] = {0};
    int i = 0;
    while (msg_param[i] != ' ' && msg_param[i] != '\0' && msg_param[i] != '&' && i < 255) {
        encoded_msg[i] = msg_param[i];
        i++;
    }
    encoded_msg[i] = '\0';
    url_decode_with_curl(message, encoded_msg);
}

void handle_client(int client_socket) {
    // creating buffer
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE-1, 0);
    
    if (bytes_read < 0) {
        perror("recv failed");
        close(client_socket);
        return;
    }

    // extracting message
    char message[1024] = {0};
    parse_query(buffer, message);

    // html response
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "\r\n"
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><title>Student</title></head>\n"
        "<body>\n"
        "   <p>%s</p>\n"
        "   <img\n"
        "       src=\"https://www.mirea.ru/upload/medialibrary/c1a/MIREA_Gerb_Colour.jpg\"\n"
        "       title=\"MIREA\"\n"
        "       width=\"300\"\n"
        "       height=\"300\"\n"
        "   >\n"
        "</body>\n"
        "</html>",
        message);
    
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}
