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
#define WifiCommBuffSIZE 200

char wifiCommunicationBuffer[WifiCommBuffSIZE];
int count = 0;

void nano_wait(unsigned int n) {
	asm( "        mov r0,%0\n"
			"repeat: sub r0,#83\n"
			"        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

//BUFFER AT END FOR OFF CYCLE
uint8_t leds[LED_ARR_SIZE] = { 0 };

uint8_t off[24] = { 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19 };
uint8_t red[24] = { 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 38, 38, 38,
		38, 19, 19, 19, 19, 19, 19, 19, 19 };
uint8_t green[24] = { 19, 19, 19, 19, 38, 38, 38, 38, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19 };
uint8_t blue[24] = { 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
		19, 19, 19, 19, 19, 38, 38, 38, 38 };
uint8_t white[24] = { 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
		38, 38, 38, 38, 38, 38, 38, 38, 38, 38 };
uint8_t reset[24] = { RS, RS, RS, RS, RS, RS, RS, RS,
RS, RS, RS, RS, RS, RS, RS, RS,
RS, RS, RS, RS, RS, RS, RS, RS };

void init_clock(void) {

	if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) /* (1) */
	{
		RCC->CFGR &= (uint32_t) (~RCC_CFGR_SW); /* (2) */
		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) /* (3) */
		{
			/* For robust implementation, add here time-out management */
		}
	}
	RCC->CR &= (uint32_t) (~RCC_CR_PLLON);/* (4) */
	while ((RCC->CR & RCC_CR_PLLRDY) != 0) /* (5) */
	{
		/* For robust implementation, add here time-out management */
	}

	RCC->CFGR = (RCC->CFGR & (~RCC_CFGR_PLLMUL)) | (RCC_CFGR_PLLMUL12);
	RCC->CR |= RCC_CR_PLLON;

	while ((RCC->CR & RCC_CR_PLLRDY) == 0) /* (8) */
	{
		/* For robust implementation, add here time-out management */
	}
	RCC->CFGR |= (uint32_t) (RCC_CFGR_SW_PLL); /* (9) */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) /* (10) */
	{
		/* For robust implementation, add here time-out management */
	}

}

void init_tim1_dma(void) {

//    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
//    GPIOA->MODER &= ~(GPIO_MODER_MODER8);
//    GPIOA->MODER |= GPIO_MODER_MODER8_0;
//    GPIOA->ODR |= 1<<8;

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODER8);
	GPIOA->MODER |= GPIO_MODER_MODER8_1;
	GPIOA->AFR[1] &= ~(0xf << 0);
	GPIOA->AFR[1] |= (0x2 << (0 * 4));

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

//    good stuff
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	DMA1_Channel2->CCR &= ~DMA_CCR_EN;
	DMA1_Channel2->CPAR = (uint32_t) (&(TIM1->CCR1)); /* (3) */
	DMA1_Channel2->CMAR = (uint32_t) (leds); /* (4) */
	DMA1_Channel2->CNDTR = (uint16_t) LED_ARR_SIZE; /* (5) */
	DMA1_Channel2->CCR |= DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_TEIE
			| DMA_CCR_TCIE; /* (6) */
	DMA1_Channel2->CCR |= DMA_CCR_DIR;    // | DMA_CCR_CIRC;
	DMA1_Channel2->CCR |= DMA_CCR_DIR | DMA_CCR_CIRC;

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

void LED_write(uint8_t *leds) { //I dont think this works
	//enable DMA to transfer array (noncircular)
	DMA1_Channel2->CCR &= ~DMA_CCR_EN;
	DMA1_Channel2->CMAR = (uint32_t) leds;
	DMA1_Channel2->CCR |= DMA_CCR_EN;

	return;
}

void LED_alternate(uint8_t *leds, uint8_t *col1, uint8_t *col2) {
	DMA1_Channel2->CCR &= ~DMA_CCR_EN;

	for (int i = 0; i < NUMLED; i++) {
		for (int j = 0; j < 24; j++) {
			if (i % 2)
				leds[i * 24 + j] = col1[j];
			else
				leds[i * 24 + j] = col2[j];
		}
	}
	DMA1_Channel2->CCR |= DMA_CCR_EN;

}

void init_tim7(void) {

	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN; // Enable Timer6 clock
	TIM7->PSC = 480000 - 1; // Set prescaler to generate 10ms tick
	TIM7->ARR = 10 - 1; // Auto-reload value to generate 10ms interrupt
	TIM7->DIER |= TIM_DIER_UIE; // Enable update interrupt
	NVIC_EnableIRQ(TIM7_IRQn); // Enable Timer7 interrupt in NVIC
	TIM7->CR1 |= TIM_CR1_CEN; // Enable Timer7 counter

}

void TIM7_IRQHandler(void) {

	TIM7->SR &= ~1;

	if (strstr(wifiCommunicationBuffer, "OK")) {
		transferData();
	}

}

void transferData(void) {
	char current;
	for (int i = 0; i < 3 * NUMLED; i++) { //add offset to buf
		current = wifiCommunicationBuffer[i];
		for (int j = 0; j < 8; j++) {
			if (current & (1 << j)) {
				leds[i * 8 + j] = 38;
			} else {
				leds[i * 8 + j] = 19;
			}
		}
	}
	clear_buf(wifiCommunicationBuffer, 200);
}

//////////////////////////////////////////////////

int main(void) {

	char *buf;
	buf = wifiCommunicationBuffer;

	init_clock();

	init_usart5();
	writeString("\r\n");
	writeString("AT+CWMODE=1\r\n");

	enable_DMAinterrupt(wifiCommunicationBuffer, WifiCommBuffSIZE);
	writeString("\r\n");
	writeString("AT+CWMODE=1\r\n");

	configureWifiSystem();

//    init_tim7();

	init_tim1_dma();
	LED_alternate(leds, blue, blue);

	int count = 0;
	clear_buf(wifiCommunicationBuffer, 200);

//    while(1){
//
//    	if(count == 0 && strstr(wifiCommunicationBuffer, "g")){
//    	  	LED_alternate(leds, green, green);
//    	  	clear_buf(wifiCommunicationBuffer, 200);
//
//    	}else if(count == 1000000 && strstr(wifiCommunicationBuffer, "IPD,0,3:b")){
//    	   	LED_alternate(leds, blue, blue);
//    	   	clear_buf(wifiCommunicationBuffer, 200);
//
//    	} else if(count > 2000000) {
//    		count = -1;
//    	}
//    	count++;
//
// 	}
	while (1) {

		if (count == 0) {
			LED_alternate(leds, green, red);

		} else if (count == 1000000) {
			LED_alternate(leds, red, green);

		} else if (count > 2000000) {
			count = -1;
		}
		count++;

	}
}

