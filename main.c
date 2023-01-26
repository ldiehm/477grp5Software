#include "stm32f0xx.h"
#include <math.h>   // for M_PI
#include <string.h>


void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

#define ONE (38)
#define ZERO (19)

uint16_t buf[24] = {38,38,38,38,38,38,38,38,
				  19,19,19,19,19,19,19,19,
				  19,19,19,19,19,19,19,19};

uint16_t red[24] = {19,19,19,19,19,19,19,19,
					38,38,38,38,38,38,38,38,
					19,19,19,19,19,19,19,19};
uint16_t green[24] = {38,38,38,38,38,38,38,38,
				  	  19,19,19,19,19,19,19,19,
					  19,19,19,19,19,19,19,19};
uint16_t blue[24] = { 19,19,19,19,19,19,19,19,
				  	  19,19,19,19,19,19,19,19,
					  38,38,38,38,38,38,38,38};

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


    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;// | TIM_CCMR1_OC2M);
//    TIM1->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M);

    TIM1->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);// | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1);
//    TIM1->CCMR2 |= (TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1);

    TIM1->CCER |= TIM_CCER_CC1E;// | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E);



    //DMA CONF
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel2->CPAR = (uint32_t) (&(TIM1->CCR1)); /* (3) */
    DMA1_Channel2->CMAR = (uint32_t)(buf); /* (4) */
    DMA1_Channel2->CNDTR =  (uint16_t) 24; /* (5) */
    DMA1_Channel2->CCR |= DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_TEIE | DMA_CCR_TCIE ; /* (6) */
    DMA1_Channel2->CCR |= DMA_CCR_DIR | DMA_CCR_CIRC;
    DMA1_Channel2->CCR |= DMA_CCR_EN; /* (7) */
    //DMACSELR &= ~(0xf0);
    /* Configure NVIC for DMA */
    /* (1) Enable Interrupt on DMA Channel 1 */
    /* (2) Set priority for DMA Channel 1 */
    //NVIC_EnableIRQ(DMA1_Channel2_IRQn); /* (1) */
    //NVIC_SetPriority(DMA1_Channel2_IRQn,0); /* (2) */

    TIM1->CR1 |= TIM_CR1_CEN;
    return;

}


int main(void)
{

    // Demonstrate part 1
//#define TEST_TIMER3
#ifdef TEST_TIMER3
    setup_tim3();
    for(;;) { }
#endif


    init_tim1_dma();
    //init_tim7();
    int st = 0;
    while(1){
    	/*
    	//do color stuff here
    	if(st == 0){
        	memcpy(buf, red, 24);
        	st++;
    	}if(st == 1){
        	memcpy(buf, green, 24);
        	st++;
        }if(st == 2){
        	memcpy(buf, blue, 24);
			st++;
        }
        nano_wait(50000);
		*/
	}
}




