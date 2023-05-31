

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wiringPi.h>

#define pin 18
#define pwmrng 100
#define port 8080

int main(){
    wiringPiSetupGpio();
    pinMode(pin,PWM_OUTPUT);

    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(pwmrng);
    pwmSetClock(192);

    int sfd,newsoc,rd;
    struct sockaddr_in addr;
    int opt=1;
    int addrlen=sizeof(addr);
    char buff[1024]={0};

    char *wp = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
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
                  "<br><br>"
                  "<input type=\"range\" min=\"0\" max=\"100\" value=\"0\" name=\"brightness\">"
                  "<br>"
                  "<input type=\"submit\" value=\"Set Brightness\">"
                  "</form>"
                  "</body></html>";

    if((sfd=socket(AF_INET,SOCK_STREAM,0))==0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEport,&opt,sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(port);

    if(bind(sfd,(struct sockaddr*)&addr,sizeof(addr))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, 3)<0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int led_state=0;
    int brightness=0;

    while(1){
        if((newsoc=accept(sfd,(struct sockaddr*)&addr,(socklen_t*)&addrlen))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        rd=read(newsoc,buff,1024);

        char *ledstat=strstr(buff,"LED=");
        if(ledstat!=NULL){
            ledstat+=4;
            if(strncmp(ledstat,"ON",2)==0 && led_state==0){
                digitalWrite(pin, HIGH);
                led_state = 1;
                printf("LED is ON\n");
            } 
            else if(strncmp(ledstat, "OFF", 3)==0 && led_state==1){
                digitalWrite(pin, LOW);
                led_state = 0;
                printf("LED is OFF\n");
            }
        }

        char *bristat=strstr(buff,"brightness=");
        if(bristat!=NULL){
            bristat+=11;
            brightness=atoi(bristat);

            if(brightness<0)
                brightness=0;
            else if(brightness>pwmrng)
                brightness=pwmrng;

            pwmWrite(pin,brightness);
            printf("Brightness set to %d\n",brightness);
        }

        write(newsoc,wp,strlen(wp));
        memset(buff,0,sizeof(buff));
        close(newsoc);
    }
    return 0;
}


