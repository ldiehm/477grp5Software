#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>

//Important Functions
void init_usart5(void);
void enable_DMAinterrupt(char *buf, int size);
void writeString(char * input);
void configureWifiSystem(void);
void configureWifi(char * SSID, char * pass);

//Functions that could be removed later
void disconnectWifi(void);
void configureUDP(char * IP);
void clear_buf(char *buf, int size);
void getIP(char * IP, char * buf, int size);
void establishConnection_TCP(char * IP);
