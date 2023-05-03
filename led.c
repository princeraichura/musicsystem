#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wiringPi.h>

#define LED_PIN 0
#define PORT 8080

int main() {
    wiringPiSetup();
    pinMode(LED_PIN, OUTPUT);

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
              "<html><head>"
              "<title>Control LED</title>"
              "<style>"
              "body {background-color: #e6e6fa; text-align: center;}"
              "h1 {font-size: 36px;}"
              "button {border: none; border-radius: 20px; padding: 20px 40px; font-size: 24px; margin: 20px;}"
              "button#on {background-color: #4CAF50; color: white;}"
              "button#off {background-color: #f44336; color: white;}"
              "</style>"
              "</head><body>"
              "<h1>Control LED</h1>"
              "<form action=\"/\" method=\"POST\">"
              "<button id=\"on\" name=\"LED\" type=\"submit\" value=\"ON\">TURN ON</button>"
              "<button id=\"off\" name=\"LED\" type=\"submit\" value=\"OFF\">TURN OFF</button>"
              "</form>"
              "</body></html>";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

      if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

      if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
int led_state = 0;
        while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

               valread = read(new_socket, buffer, 1024);

              char *LED_status = strstr(buffer, "LED=");
        if (LED_status != NULL) {
            LED_status += 4;
            if (strncmp(LED_status, "ON", 2) == 0 && led_state == 0) {
              
            digitalWrite(LED_PIN, HIGH);//to turn on led
    led_state=1;
    printf("LED is ON\n");
            } else if (strncmp(LED_status, "OFF", 3) == 0 && led_state == 1) {
               
            digitalWrite(LED_PIN, LOW);
    led_state=0;
    printf("LED is OFF\n");
            }
        }

                write(new_socket, hello, strlen(hello));//send response
    memset(buffer, 0, sizeof(buffer));
                close(new_socket);
    }
    return 0;
}
