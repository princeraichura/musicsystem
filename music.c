#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define MUSIC_DIR "/home/ayushi/musicsystem"
#define PORT 8080

pid_t child_pid;
int music_playing = 0; // to track if music is currently playing

void play_music(char *filename) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", MUSIC_DIR, filename);

    child_pid = fork();
    if (child_pid == 0) {
        // Child process

       // execlp("mpg123", "mpg123", "-q", path, NULL);
       execlp("mpg123", "mpg123", "-q", "--buffer", "512", path, NULL);
        fprintf(stderr, "execlp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (child_pid < 0) {
 
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else {
        music_playing = 1; // to set when music is playing
    }
}

void stop_music() {
    if (child_pid > 0) {
       
        if (kill(child_pid, SIGTERM) == -1) {
            perror("kill failed");
            exit(EXIT_FAILURE);
        } else {
            music_playing = 0; // Reset music playing
        }
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    char *html_response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
                          "<html><head>"
                          "<title>Control Music</title>"
                          "<style>"
                          "body {background-color: #F0FFFF; text-align: center;}"
                          "h1 {font-size: 36px;}"
                          ".button-container {margin: 20px;}"
                          ".button-container button {border: 2px solid #555555; background-color: #ffffff; padding: 10px 20px; font-size: 24px; margin: 10px;}"
                          ".button-container button:hover {background-color: #555555; color: #ffffff;}"
                          ".play-icon::before {content: '\\25B6';}" // Play icon
                          ".pause-icon::before {content: '\\275A\\275A';}" // Pause icon
                          "</style></head><body>"
                          "<h1>Control Music</h1>"
                          "<div class=\"button-container\">"
                          "<button onclick=\"location.href='/music/play'\"><span class=\"play-icon\"></span>Play Music</button>"
                          "<button onclick=\"location.href='/music/stop'\"><span class=\"pause-icon\"></span>Stop Music</button>"
                          "</div>"
                          "<script>"
                          "var playButton = document.querySelector('.play-icon');"
                          "var pauseButton = document.querySelector('.pause-icon');"
                          "if (%d) { playButton.style.display = 'none'; pauseButton.style.display = 'inline'; }" // Set initial state based on music_playing flag
                          
                          "</script>"
                          "</body></html>";

       if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");//fail if soccket is not created
        exit(EXIT_FAILURE);
    }

   
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

       if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
              if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
	valread = read(new_socket, buffer, 1024);

              if (strncmp(buffer, "GET /music/play", 15) == 0) {
            play_music("Song1.mp3");
        } else if (strncmp(buffer, "GET /music/stop", 15) == 0) {
            stop_music();
        }

        // Send HTML response to the client
        char response[2048];
        sprintf(response, html_response, music_playing);

        send(new_socket, response, strlen(response), 0);

        close(new_socket);
    }

    return 0;
}
