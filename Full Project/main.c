#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>
#include "wifi.h"

//constant definitions

#define IPSIZE 16
#define ONE (7)
#define ZERO (3)
#define RS (0)
#define NUMLED (64)
#define LED_ARR_SIZE (24*NUMLED + 200)
#define WifiCommBuffSIZE (250)




volatile char wifiCommunicationBuffer[WifiCommBuffSIZE];
int count = 0;

void nano_wait(unsigned int n) {
	asm( "        mov r0,%0\n"
			"repeat2: sub r0,#83\n"
			"        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

//BUFFER AT END FOR OFF CYCLE
uint8_t leds[LED_ARR_SIZE] = { 0 };

uint8_t off[24] = { ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
		ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO };
uint8_t red[24] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7, 7, 7,
		7, 3, 3, 3, 3, 3, 3, 3, 3 };
//uint8_t red[24] = { ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ONE, ONE, ONE,
//		ONE, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO };
uint8_t green[24] = { ZERO, ZERO, ZERO, ZERO, ONE, ONE, ONE, ONE, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
		ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO };
uint8_t blue[24] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 7, 7, 7, 7 };
//uint8_t blue[24] = { ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO,
//		ZERO, ZERO, ZERO, ZERO, ZERO, ONE, ONE, ONE, ONE };
uint8_t white[24] = { ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE,
		ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE, ONE };
//uint8_t reset[24] = { RS, RS, RS, RS, RS, RS, RS, RS,
//RS, RS, RS, RS, RS, RS, RS, RS,
//RS, RS, RS, RS, RS, RS, RS, RS };




void init_clock(void) {



//	CRS->CR &= ~CRS_CR_CEN;
//	CRS->CFGR |= CRS_CFGR_SYNCSRC_0;
//	CRS->CR |= CRS_CR_CEN;

//	 (++) Modify the CPU clock source, using "RCC_SYSCLKConfig()" function
//	         (++) If needed, modify the CPU clock prescaler by using "RCC_HCLKConfig()" function
//	         (++) Check that the new CPU clock source is taken into account by reading
//	              the clock source status, using "RCC_GetSYSCLKSource()" function

	RCC_HCLKConfig(0); //Clear all bits of HPRE
//	   pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
//	      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
//	      pllmull = ( pllmull >> 18) + 2;

	//disable pll by pllon 0
	//wait until
	RCC->CR &= (uint32_t)(~RCC_CR_PLLON);/* (4) */
	while((RCC->CR & RCC_CR_PLLRDY) != 0); /* (5) */

	RCC->CFGR |= RCC_CFGR_PLLMUL6;
	RCC->CFGR2 &= ~0xF; // Clears lowest 4 bits of CFGR2 (PREDIV)
	RCC->CFGR |= RCC_CFGR_PLLSRC_0;
	RCC->CR |= RCC_CR_PLLON;
//	RCC->CFGR &= ~(0x/3);
	int count = 0;

	while(!(RCC->CR & RCC_CR_PLLRDY))
	{
	 break;
	}

//	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	//uint8_t clksrc = RCC_GetSYSCLKSource();
	//GPIOC->ODR |= 1<<8;
	/* Clear SW[1:0] bits */
	  RCC->CFGR &= ~RCC_CFGR_SW;

	  /* Set SW[1:0] bits according to RCC_SYSCLKSource value */
	  RCC->CFGR |= RCC_SYSCLKSource_PLLCLK;

	count = 0;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){
	 break;
	}


	SystemCoreClockUpdate();
//	RCC_PLLConfig(RCC_PLLSource_HSI, RCC_CFGR_PLLMUL_2);

//	RCC->CFGR |= 0x3;
	return;

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
	TIM1->ARR = 10 - 1;

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
//	DMA1_Channel2->CCR |= DMA_CCR_DIR;    // | DMA_CCR_CIRC;
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

void transferData(void) {
	char current;
	for (int i = 0; i < (3 * NUMLED); i++) { //add offset to buf
		current = wifiCommunicationBuffer[i + 13];
		for (int j = 0; j < 8; j++) {
			if ((current & (1 << j)) != 0) {
				leds[(i * 8) + 7 - j] = ONE;
			} else {
				leds[(i * 8) + 7 - j] = ZERO;
			}
		}
	}
	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
}

//////////////////////////////////////////////////

int main(void) {

	char *buf;

	volatile int i;

	for(i = 0; i < 1000000; i++){
	    asm("nop");
	}

	buf = wifiCommunicationBuffer;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	GPIOC->MODER |= 1<< 16 | 1 << 14;
	GPIOC->ODR |= 1<<7;
	GPIOC->ODR |= 1<<8;


	//uint32_t clk = SystemCoreClock;
//	init_clock();
//	GPIOC->ODR |= (1<<7) & ((RCC->CFGR & RCC_CFGR_SWS_1) << 4);
//	GPIOC->ODR |= 1<<8;

	//clk = SystemCoreClock;
	init_usart5();
//	while(1){
//	while(!(USART5->ISR & USART_ISR_TXE)) { }
//	        //nano_wait(100);
//			USART_SendData(USART5, 'J');
////	        USART5->TDR = 'J';
////	        USART_ClearFlag(USART5, USART_FLAG_TC);
//	    	GPIOC->ODR &= ~1<<7;
////	        while(!(USART5->ISR & USART_ISR_TC));
//
//	}
	enable_DMAinterrupt(wifiCommunicationBuffer, WifiCommBuffSIZE);
	init_tim1_dma();

	LED_alternate(leds, blue, red);

	configureWifiSystem();

	//nano_wait(1000000);
	//nano_wait(1000000);



	LED_alternate(leds, blue, green);

	GPIOC->ODR |= 1<<7;

	int count = 0;
//	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);

// 	while(1);
//    while(1){
//
//    	if(count == 0 && strstr(wifiCommunicationBuffer, "g")){
//    	  	LED_alternate(leds, green, green);
//    	  	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
//
//    	}else if(count == 1000000 && strstr(wifiCommunicationBuffer, "IPD,0,3:b")){
//    	   	LED_alternate(leds, blue, blue);
//    	   	clear_buf(wifiCommunicationBuffer, WifiCommBuffSIZE);
//
//    	} else if(count > 2000000) {
//    		count = -1;
//    	}
//    	count++;
//
// 	}

	while(1) {
  	    if(strstr(wifiCommunicationBuffer, "IPD,0,193:")) {
	    	while(wifiCommunicationBuffer[205] != 'K');
	    	transferData();
	    }

	}

//	while (1) {
//
//		if (count == 0) {
//			LED_alternate(leds, green, red);
//
//		} else if (count == 1000000) {
//			LED_alternate(leds, red, green);
//
//		} else if (count > 2000000) {
//			count = -1;
//		}
//		count++;
//
//	}
}
