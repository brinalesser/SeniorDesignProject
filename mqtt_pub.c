#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <mosquitto.h>
#include <pigpio.h>

#define DEFAULT_HOST "wblvd.duckdns.org"
#define DEFAULT_PORT 1883
#define DEFAULT_UN "newadmin"
#define DEFAULT_PW "12345"
#define GPIO_PIN 21

struct mosquitto * mosq;
int gui_value1 = 1;
int gui_value2 = 2;
int gui_value3 = 3;

double * mem_loc;
char mem_loc_str[20];
char data_to_send[4];

gpioPulse_t pulse[2];

void gpio_cb(int gpio, int level, uint32_t tick){
   if(gpio == GPIO_PIN && level == 1 && mem_loc != NULL){ //rising edge of GPIO 21
        /**
      //number of us since boot
      data_to_send[0] = (tick) & 0xFF;
      data_to_send[1] = (tick >> 8) & 0xFF;
      data_to_send[2] = (tick >> 16) & 0xFF;
      data_to_send[3] = (tick >> 24) & 0xFF;
        **/

        snprintf(data_to_send, 50, "%f", *mem_loc);
        mosquitto_publish(mosq, NULL, "GUI_VAR", sizeof data_to_send, data_to_send, 0, false);

        //update value for testing purposes
         *mem_loc = (*mem_loc) + sin(tick);
   }
}

void cleanup(){
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
        printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);
        bool match = 0;
        mosquitto_topic_matches_sub("Message", message->topic, &match);
        if (match) {
                mem_loc = (double *)strtol(message->payload, NULL, message->payloadlen);
                printf("setting mem loc to %d", mem_loc);
        }
}

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
	printf("ID: %d\n", * (int *) obj);
	if(rc) {
		printf("Error with result code: %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, "Message", 0);
}

int main(int argc, char *argv[]){
        int rc, opt;
        char un[20] = DEFAULT_UN;
        char pw[20] = DEFAULT_PW;
        char host[50] = DEFAULT_HOST;
        int port = DEFAULT_PORT;

        while((opt = getopt(argc, argv, ":u:w:p:h:")) != -1)
    {
        switch(opt)
        {
                case 'u':
                        sprintf(un, "%s", optarg);
                        break;
                case 'w':
                        sprintf(pw, "%s", optarg);
                        break;
                case 'p':
                        port = atoi(optarg);
                        break;
                case 'h':
                        sprintf(host, "%s", optarg);
                        break;
            case ':':
                printf("arg missing value\n");
                break;
            case '?':
                printf("unknown rg: %c\n", optopt);
                break;
        }
    }

    for(; optind < argc; optind++){
        printf("too many args: %s\n", argv[optind]);
    }

        mosquitto_lib_init();

        mosq = mosquitto_new("test", true, NULL);
	
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, message_callback);
	
        mosquitto_username_pw_set(mosq, un, pw);
        rc = mosquitto_connect(mosq, host, port, 60);

        if(rc != 0){
                printf("Client could not connect to broker! Error Code: %d\n", rc);
                mosquitto_destroy(mosq);
                return -1;
        }
        mosquitto_loop_start(mosq);

        //square wave for periodic read
        int secs = 60;
        int us = 50;
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

   gpioSetAlertFunc(GPIO_PIN, gpio_cb);
   gpioWaveClear();
   gpioWaveAddGeneric(2, pulse);

   int id = gpioWaveCreate();
   gpioWaveTxSend(id, PI_WAVE_MODE_REPEAT);

   sleep(secs);
   gpioWaveTxStop();
   gpioTerminate();

        atexit(cleanup);
        return 0;
}
