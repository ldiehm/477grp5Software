#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>

#define WifiCommBuffSIZE 250
extern char wifiCommunicationBuffer[WifiCommBuffSIZE];

void init_usart5(void)
{

    RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN);
    GPIOC->MODER &= ~GPIO_MODER_MODER12;
    GPIOC->MODER |= GPIO_MODER_MODER12_1; //PC12 - USART5_TX
    GPIOD->MODER &= ~GPIO_MODER_MODER2;
    GPIOD->MODER |= GPIO_MODER_MODER2_1; //PD2 - USART5_RX

    GPIOC->AFR[1] &= ~GPIO_AFRH_AFR12;
    GPIOC->AFR[1] |= 2 << (4 * 4);

    GPIOD->AFR[0] &= ~GPIO_AFRL_AFR2;
    GPIOD->AFR[0] |= 2 << (4 * 2);
    //done with alternate function stuff

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

    USART5->CR1 &= ~USART_CR1_UE; // disabling usart5 for time being
    USART5->CR1 &= ~(USART_CR1_M | 1 << 28);//(1<<12 | 1<<28); // word length of 8, making m0 and m1 0
    USART5->CR2 &= ~USART_CR2_STOP; //making stop bit of 1
    USART5->CR2 &= ~USART_CR1_PCE; //disabling parity
    USART5->CR1 &= ~USART_CR1_OVER8; // oversample of 16

    USART5->BRR = 0x1A1; // 115.2k baud rate
    USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    //USART5->RTOR |= USART_RTOR_RTO;
    USART5->CR1 |= USART_CR1_UE;

    //waiting for receive enable acknowledge flag, Transmit enable acknowledge flag
    while(!(USART5->ISR & USART_ISR_TEACK) && !(USART5->ISR & USART_ISR_REACK));

}

void enable_DMAinterrupt(char *buf, int size) {

    USART5->CR1 |= USART_CR1_RXNEIE;
    USART5->CR3 |= USART_CR3_DMAR;

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->RMPCR |= DMA_RMPCR2_CH2_USART5_RX;

    DMA2_Channel2->CCR &= ~DMA_CCR_EN;  // First make sure DMA is turned off

    DMA2_Channel2->CPAR = &(USART5->RDR);
    DMA2_Channel2->CMAR = &(wifiCommunicationBuffer);
    DMA2_Channel2->CNDTR = WifiCommBuffSIZE;

    DMA2_Channel2->CCR &= ~DMA_CCR_DIR;
    DMA2_Channel2->CCR &= ~(DMA_CCR_TEIE | DMA_CCR_HTIE);

    DMA2_Channel2->CCR &= ~DMA_CCR_MSIZE;
    DMA2_Channel2->CCR &= ~DMA_CCR_PSIZE;

    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;

    DMA2_Channel2->CCR |= DMA_CCR_CIRC;

    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;

    DMA2_Channel2->CCR |= DMA_CCR_PL;

    DMA2_Channel2->CCR |= DMA_CCR_EN;

}


void writeString(char * input) {
//	char *OK = "AT\r\n";
//	for(int i = 0; i < sizeof(input); i++){
	int i = 0;
	while(input[i] != '\0'){
//        while (!(USART5->ISR & USART_ISR_RXNE)) { }
//        char c =  (USART5->RDR);
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = input[i];
		i++;
    }
}

void disconnectWifi(void){

	writeString("AT+CWQAP\r\n");

}

void configureWifi(char * SSID, char * pass){


	writeString("AT+CWJAP=\"");
	writeString(SSID);
	writeString("\",\"");
	writeString(pass);
	writeString("\"\r\n");

}

void configureWifiSystem(void){
	writeString("\r\n");

	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
	writeString("AT+CWMODE=1\r\n");

	for(int i = 0; i < 1000000; i++);

	while(!strstr(wifiCommunicationBuffer, "OK"));
	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
	configureWifi("BOXFISH", "BOX");

	while(!strstr(wifiCommunicationBuffer, "OK"));

	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
	writeString("AT+CIPMUX=1\r\n");
	for(int i = 0; i < 100000; i++);

	while(!strstr(wifiCommunicationBuffer, "OK"));

	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
	configureUDP("0.0.0.0");

	for(int i = 0; i < 10000000; i++);

	while(!strstr(wifiCommunicationBuffer, "OK") && !strstr(wifiCommunicationBuffer, "ERROR"));
	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
}

void configureUDP(char * IP){


	writeString("AT+CIPSTART=0,\"UDP\",\"");
	writeString(IP);
	writeString("\",80,80,1");
//	writeString(pass);
	writeString("\r\n");

}

void clear_buf(char *buf, int size){

	DMA2_Channel2->CCR &= ~DMA_CCR_EN;

	for(int i = 0; i < size; i++){
		buf[i] = 1;
	}

    DMA2_Channel2->CNDTR = WifiCommBuffSIZE;
	DMA2_Channel2->CCR |= DMA_CCR_EN;
}


//Long term problem - buf potentially circling to end of buffer w/ IP
void getIP(char * IP, char * buf, int size){
	int i;
	int j = 0;
	char isIP = 0;
	for(i = 0; i < size; i++){

		if(buf[i] == '"'){
			if(isIP){
				break;
			}
			isIP = 1;
		}
		else if(isIP){
			IP[j] = buf[i];
			j++;
		}
	}

}

void establishConnection_TCP(char * IP){

	writeString("AT+CIPSTART=\"TCP\",\"");
	writeString(IP);
	writeString("\",80\r\n");

}
