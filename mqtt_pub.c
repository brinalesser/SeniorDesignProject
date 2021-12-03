#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

int gui_value1 = 1;
int gui_value2 = 2;
int gui_value3 = 3;

int get_mem_at_location(char * loc){
        int * p = (int *)strtol(loc, NULL, 16);
        int val =  *p;
        return val;
}

int main(){
	int rc;
	struct mosquitto * mosq;

	mosquitto_lib_init();

	mosq = mosquitto_new("publisher-test", true, NULL);

	rc = mosquitto_connect(mosq, "localhost", 5672, 60);
	if(rc != 0){
		printf("Client could not connect to broker! Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}
	printf("We are now connected to the broker!\n");

        char mem_loc_str[100];
        printf("Enter a memory location: ");
        fgets(mem_loc_str, sizeof mem_loc_str, stdin);

        int val_to_send = get_mem_at_location(mem_loc_str);

        char char_val[6];
        sprintf(char_val, "%d", val_to_send);
        mosquitto_publish(mosq, NULL, "test/t1", 6, char_val, 0, false);

        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);

        mosquitto_lib_cleanup();
        return 0;
}
