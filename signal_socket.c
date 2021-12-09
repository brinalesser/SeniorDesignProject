#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080
#define GPIO_PIN 21

gpioPulse_t pulse[2]; /* only need two pulses for a square wave */
char data_to_send[4];
int bytes_sent;
int sock;

void alert_cb(int gpio, int level, uint32_t tick){
   if(gpio == GPIO_PIN && level == 1){ //rising edge of GPIO 21
      //send tick (number of us since boot) over network socket
      data_to_send[0] = (tick) & 0xFF;
      data_to_send[1] = (tick >> 8) & 0xFF;
      data_to_send[2] = (tick >> 16) & 0xFF;
      data_to_send[3] = (tick >> 24) & 0xFF;
      bytes_sent = send(sock, &data_to_send, 4, 0 );
      //if(bytes_sent != 1){printf("Error sending\n");}
   }
}

int main(int argc, char *argv[])
{
   /* Setup network socket */
   int server, valread;
   struct sockaddr_in address;
   int opt = 1;
   int addrlen = sizeof(address);
   if((server = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
           printf("socket failed\n");
       exit(EXIT_FAILURE);
   }
   printf("socket succeeded\n");
   if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
           printf("setsockopt failed\n");
           exit(EXIT_FAILURE);
   }
   printf("setsockopt succeeded\n");
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons( PORT );
   if(bind(server, (struct sockaddr *)&address, sizeof(address))<0) {
           printf("bind failed\n");
           exit(EXIT_FAILURE);
   }
   printf("bind succeeded\n");
   if(listen(server, 3) < 0) {
           printf("listen failed");
           exit(EXIT_FAILURE);
   }
   printf("listen succeeded\n");
   if((sock = accept(server, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
           printf("accept failed");
           exit(EXIT_FAILURE);
   }
   printf("accept succeeded\n");

   /* Setup square wave */
   int secs=60, us = 50;

   if (argc>1) us   = atoi(argv[1]); /* square wave micros */
   if (argc>2) secs = atoi(argv[2]); /* program run seconds */

   if (us<2) us = 2; /* minimum of 2 micros per pulse */

   if ((secs<1) || (secs>3600)) secs = 3600;

   if (gpioInitialise()<0) return 1;

   gpioSetMode(GPIO_PIN, PI_OUTPUT);

   pulse[0].gpioOn = (1<<GPIO_PIN); /* GPIO 21 high */
   pulse[0].gpioOff = 0;
   pulse[0].usDelay = us;

   pulse[1].gpioOn = 0;
   pulse[1].gpioOff = (1<<GPIO_PIN); /* GPIO 21 low */
      pulse[1].usDelay = us;

   gpioSetAlertFunc(GPIO_PIN, alert_cb);

   gpioWaveClear();

   gpioWaveAddGeneric(2, pulse);

   int id = gpioWaveCreate();
   gpioWaveTxSend(id, PI_WAVE_MODE_REPEAT);

   sleep(secs);

   gpioWaveTxStop();

   gpioTerminate();
}