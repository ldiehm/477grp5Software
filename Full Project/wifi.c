#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>
#include "wifi.h"

#define WifiCommBuffSIZE 250
extern volatile char wifiCommunicationBuffer[WifiCommBuffSIZE];


//void init_usart6(void)
//{
//
//    RCC->AHBENR |= (RCC_AHBENR_GPIOAEN);
//    GPIOA->MODER &= ~GPIO_MODER_MODER2;
//    GPIOA->MODER |= GPIO_MODER_MODER2_1; //PA2 - USART6_TX
//    GPIOA->MODER &= ~GPIO_MODER_MODER2;
//    GPIOA->MODER |= GPIO_MODER_MODER3_1; //PA3 - USART6_RX
//
//    GPIOA->AFR[0] &= ~GPIO_AFRL_AFR2;
//    GPIOA->AFR[0] |= 1 << (4 * 2);
//
//    GPIOA->AFR[0] &= ~GPIO_AFRL_AFR3;
//    GPIOA->AFR[0] |= 1 << (4 * 3);
//    //done with alternate function stuff
//
//    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
//
//    USART5->CR1 &= ~USART_CR1_UE; // disabling usart5 for time being
//    USART5->CR1 &= ~(USART_CR1_M | 1 << 28);//(1<<12 | 1<<28); // word length of 8, making m0 and m1 0
//    USART5->CR2 &= ~USART_CR2_STOP; //making stop bit of 1
//    USART5->CR2 &= ~USART_CR1_PCE; //disabling parity
//    USART5->CR1 &= ~USART_CR1_OVER8; // oversample of 16
//
//    USART5->BRR = 0x1A1; // 115.2k baud rate
//    USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE);
//
//    //USART5->RTOR |= USART_RTOR_RTO;
//    USART5->CR1 |= USART_CR1_UE;
//
//    //waiting for receive enable acknowledge flag, Transmit enable acknowledge flag
//    while(!(USART5->ISR & USART_ISR_TEACK) && !(USART5->ISR & USART_ISR_REACK));
//
//}

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
//
//void USART3_4_5_6_7_8_IRQHandler(void){
//	int count = 0;
//	while(wifiCommunicationBuffer[count] != 0){
//		count++;
//	}
//	wifiCommunicationBuffer[count] = USART5->RDR;
//}

void enable_DMAinterrupt(char *buf, int size) {

    USART5->CR1 |= USART_CR1_RXNEIE;// | USART_CR1_RE;

//	NVIC_EnableIRQ(USART3_8_IRQn);

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
        //nano_wait(100);
        USART5->TDR = input[i];
		i++;
    }
		//GPIOC->ODR |= 1 << 7;
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
	//int wait = 1000000;



	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
	//nano_wait(100000);



	writeString("AT+CWMODE=1\r\n");

	//nano_wait(10000000000);
	while(!strstr(wifiCommunicationBuffer, "OK"));
	//nano_wait(100000);
	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
	//nano_wait(100000);
	configureWifi("BOXFISH", "BOX");
	//for(long long int i = 0; i < 20000000; i++);
//	nano_wait(10000000000000);

	while(!strstr(wifiCommunicationBuffer, "OK"));
//	nano_wait(100000);
	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
//	nano_wait(100000);
	writeString("AT+CIPMUX=1\r\n");
	//nano_wait(100000);
	while(!strstr(wifiCommunicationBuffer, "OK"));
	//nano_wait(100000);
	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
	//nano_wait(100000);
	configureUDP("0.0.0.0");
	//nano_wait(100000);
	while(!strstr(wifiCommunicationBuffer, "OK") && !strstr(wifiCommunicationBuffer, "ERROR"));
	//nano_wait(100000);
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


	for(int i = 0; i < size; i++){
		buf[i] = 0;
	}

	DMA2_Channel2->CCR &= ~DMA_CCR_EN;


    DMA2_Channel2->CNDTR = WifiCommBuffSIZE;
	DMA2_Channel2->CCR |= DMA_CCR_EN;
}

int match_string(char *str, char *substr, int len){
	int count = 0;
	char *b = substr;
	char *a = str;
	while(count < len){
		if (*str != *b) {
				str++;
				count++;
			    continue;
		}
		a = str;
		while (1) {
			if (*b == 0) {
				return 1;
			}
			if (*a++ != *b++) {
				break;
			}
		}
		b = substr;
		str++;
		count++;
	}
	return 0;
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
