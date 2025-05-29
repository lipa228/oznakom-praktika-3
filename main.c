#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 2048
#define MAX_MSG 256

void decode_url(char* output, const char* input) {
    char high, low;
    while (*input) {
        if (*input == '%' && (high = input[1]) && (low = input[2]) && isxdigit(high) && isxdigit(low)) {
            high = high >= 'a' ? high - 32 : high;
            high = high >= 'A' ? high - 55 : high - 48;
            low = low >= 'a' ? low - 32 : low;
            low = low >= 'A' ? low - 55 : low - 48;
            *output++ = (high << 4) | low;
            input += 3;
        } else {
            *output++ = *input++;
        }
    }
    *output = 0;
}

void setup_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 5);
    printf("Server listening on port %d\n", SERVER_PORT);

    while(1) {
        int client = accept(sock, 0, 0);
        
        char data[BUFFER_SIZE];
        read(client, data, BUFFER_SIZE);
        
        char* param_pos = strstr(data, "message=");
        char encoded[MAX_MSG] = {0};
        char decoded[MAX_MSG] = {0};

        if (param_pos) {
            sscanf(param_pos + 8, "%255[^& \n]", encoded);
            decode_url(decoded, encoded);
        }

        char page[BUFFER_SIZE];
        snprintf(page, BUFFER_SIZE,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n\r\n"
            "<!DOCTYPE html><html><head><title>Server</title></head>"
            "<body><div>%s</div>"
            "<img src='https://www.mirea.ru/upload/medialibrary/c1a/MIREA_Gerb_Colour.jpg' width='300'>"
            "</body></html>",
            decoded);
            
        write(client, page, strlen(page));
        close(client);
    }
}

int main() {
    setup_server();
    return 0;
}
