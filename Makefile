all: mqtt_pub mqtt_sub
	echo "Build finish!"

mqtt_pub: mqtt_pub.c
	gcc mqtt_pub.c -o mqtt_pub.o -lpigpio -lmosquitto -lm
