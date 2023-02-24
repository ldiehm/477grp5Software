/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>
#include "wifi.h"

//constant definitions

#define IPSIZE 16
#define ONE (38)
#define ZERO (19)
#define RS (0)
#define NUMLED (64)
#define LED_ARR_SIZE (24*NUMLED + 200)
int count = 0;

#include "stm32f0xx.h"


void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}


//BUFFER AT END FOR OFF CYCLE
uint8_t leds[LED_ARR_SIZE] = {0};

uint8_t off[24] = {19,19,19,19,19,19,19,19,
				  19,19,19,19,19,19,19,19,
				  19,19,19,19,19,19,19,19};
uint8_t red[24] = {19,19,19,19,19,19,19,19,
					19,19,19,19,38,38,38,38,
					19,19,19,19,19,19,19,19};
uint8_t green[24] = {19,19,19,19,38,38,38,38,
				  	  19,19,19,19,19,19,19,19,
					  19,19,19,19,19,19,19,19};
uint8_t blue[24] = { 19,19,19,19,19,19,19,19,
				  	  19,19,19,19,19,19,19,19,
					  19,19,19,19,38,38,38,38};
uint8_t white[24] = {38,38,38,38,38,38,38,38,
						38,38,38,38,38,38,38,38,
						38,38,38,38,38,38,38,38};
uint8_t reset[24] = {RS, RS, RS, RS, RS, RS, RS, RS,
						RS, RS, RS, RS, RS, RS, RS, RS,
						RS, RS, RS, RS, RS, RS, RS, RS};

void init_tim1_dma(void){
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER8);
    GPIOA->MODER |= GPIO_MODER_MODER8_1;
    GPIOA->AFR[1] &= ~(0xf<<0);
    GPIOA->AFR[1] |= (0x2<<(0*4));

    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    TIM1->BDTR |= TIM_BDTR_MOE;

    TIM1->PSC = 1 - 1;
    TIM1->ARR = 60 - 1;

    TIM1->CR2 |= TIM_CR2_CCDS; //cc DMA selection
    TIM1->DIER |= TIM_DIER_CC1DE | TIM_DIER_UDE | TIM_DIER_TDE; // CC 1 DMA request enable
    //TIM1->DIER |= TIM_DIER_UIE;


    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    //TIM1->CCMR1 |= TIM_CCMR1_OC1PE;
    TIM1->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE);
    TIM1->CCER |= TIM_CCER_CC1E;



    //DMA CONF
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
    DMA1_Channel2->CPAR = (uint32_t) (&(TIM1->CCR1)); /* (3) */
    DMA1_Channel2->CMAR = (uint32_t)(leds); /* (4) */
    DMA1_Channel2->CNDTR =  (uint16_t) LED_ARR_SIZE; /* (5) */
    DMA1_Channel2->CCR |= DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_TEIE | DMA_CCR_TCIE ; /* (6) */
    DMA1_Channel2->CCR |= DMA_CCR_DIR | DMA_CCR_CIRC;
    //DMA1_Channel2->CCR |= DMA_CCR_EN; /* (7) */

    //DMACSELR &= ~(0xf0);
    /* Configure NVIC for DMA */
    /* (1) Enable Interrupt on DMA Channel 1 */
    /* (2) Set priority for DMA Channel 1 */
    //NVIC_EnableIRQ(DMA1_Ch2_3_DMA2_Ch1_2_IRQn); /* (1) */
    //NVIC_SetPriority(DMA1_Ch2_3_DMA2_Ch1_2_IRQn,0); /* (2) */

    TIM1->CR1 |= TIM_CR1_CEN;
    return;

}
//I dont think this is even configured

//void DMA1_CH2_3_DMA2_CH1_2_IRQHandler(void){
//
//	//if(DMA1->ISR & DMA_ISR_TCIF2){
//
//	//DMA1_Channel2->CCR &= ~DMA_CCR_EN;
//	//TIM1->CCR1 = 0;
//	DMA1->IFCR |= 0xf0;
//
//		//DMA1_Channel2->CCR |= DMA_CCR_EN;
//		//count = 0;
//	//}
//
//}

void LED_write(uint8_t *leds){ //I dont think this works
	//enable DMA to transfer array (noncircular)
	DMA1_Channel2->CCR &= ~DMA_CCR_EN;
	DMA1_Channel2->CMAR = (uint32_t) leds;
	DMA1_Channel2->CCR |= DMA_CCR_EN;


	return;
}

void LED_alternate(uint8_t *leds, uint8_t *col1, uint8_t *col2){
	DMA1_Channel2->CCR &= ~DMA_CCR_EN;

	for(int i = 0; i < NUMLED; i++)
	{
		for(int j = 0; j < 24; j++){
			if(i % 2)
				leds[i*24 + j] = col1[j];
			else
				leds[i*24 + j] = col2[j];
		}
	}
	DMA1_Channel2->CCR |= DMA_CCR_EN;

}

//////////////////////////////////////////////////

//init_usart5
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
    DMA2_Channel2->CMAR = &(buf);
    DMA2_Channel2->CNDTR = size;

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

void configureUDP(char * IP){


	writeString("AT+CIPSTART=0,\"UDP\",\"");
	writeString(IP);
	writeString("\",80,80,1");
//	writeString(pass);
	writeString("\r\n");

}

void clear_buf(char *buf, int size){
	for(int i = 0; i < size; i++){
		buf[i] = 1;
	}
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

void establishConnection(char * IP){


	writeString("AT+CIPSTART=\"TCP\",\"");
	writeString(IP);
	writeString("\",80\r\n");

}
int main(void)
{
    char buf[200];

    clear_buf(buf, 200);
    init_usart5();
    enable_DMAinterrupt(buf, 200);

    writeString("\r\n");
    nano_wait(1000);
    writeString("AT+CWMODE=3\r\n");

    clear_buf(buf, 200);
    configureWifi("BOXFISH", "BOX");
    while(!strstr(buf, "WIFI GOT IP"));

    writeString("AT+CIPMUX=1\r\n");


    configureUDP("0.0.0.0");

  	LED_alternate(leds, off, off);
    init_tim1_dma();
   	//LED_write(leds);
   	//LED_alternate(leds, green, green);
	//LED_write(leds);



   	int count = 0;
    while(1){

    	if(count == 0){
    	  	LED_alternate(leds, green, red);

    	   	//LED_write(leds);
    	   	//LED_alternate(leds, off, green);
    	   	//LED_alternate(leds, red, blue);

    	}else if(count == 100000){
    	   	LED_alternate(leds, red, blue);

    	} else if(count > 200000) {
    		count = -1;
    	}
    	count++;

 	}
}

//
//int main() {

//    char *AT = "AT\r\n";
//    char *AT_CWSAP = "AT+CWSAP?\r\n";
//    char *AT_CWLAP = "AT+CWLAP\r\n";
//
//    disconnectWifi();

//
//    writeString("AT+CIFSR\r\n");
//
//    char * IP = "000.000.0.0";

//    for(;;){



//        configureWifi("BOXFISH", "BOX");
//        clear_buf(buf, 200);
//
//        writeString("AT+CIFSR\r\n");
//
//
////        char IP[IPSIZE] = {0};
////        getIP(IP, buf, 200);
//
//
//        establishConnection("192.168.4.1");
//        while (!(USART5->ISR & USART_ISR_RXNE));


////    }
//
//    return 0;
//}
//


