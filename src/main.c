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
#include "tty.h"
#include "fifo.h"

//init_usart5
void init_usart5(void)
{

    RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN);
    GPIOC->MODER &= ~GPIO_MODER_MODER12;
    GPIOC->MODER |= GPIO_MODER_MODER12_1;
    GPIOD->MODER &= ~GPIO_MODER_MODER2;
    GPIOD->MODER |= GPIO_MODER_MODER2_1;

    GPIOC->AFR[1] &= ~GPIO_AFRH_AFR12;
    GPIOC->AFR[1] |= 2 << (4 * 4);

    GPIOD->AFR[0] &= ~GPIO_AFRL_AFR2;
    GPIOD->AFR[0] |= 2 << (4 * 2);

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;

    USART5->CR1 &= ~USART_CR1_UE; // disabling usart5 for time being
    USART5->CR1 &= ~(1<<12 | 1<<28); // word length of 8, making m0 and m1 0
    USART5->CR2 &= ~USART_CR2_STOP; //making stop bit of 1
    USART5->CR2 &= ~USART_CR1_PCE; //disabling parity
    USART5->CR1 &= ~USART_CR1_OVER8; // oversample of 16

    USART5->BRR = 0x1A1;
    USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    USART5->CR1 |= USART_CR1_UE;

    while(!(USART5->ISR & USART_ISR_TEACK) && !(USART5->ISR & USART_ISR_REACK));


}

//#define STEP21
        #if defined(STEP21)
        int main(void)
        {
            init_usart5();
            for(;;) {
                while (!(USART5->ISR & USART_ISR_RXNE)) { }
                char c = USART5->RDR;
                while(!(USART5->ISR & USART_ISR_TXE)) { }
                USART5->TDR = c;
            }
        }
        #endif


//#define STEP22
#if defined(STEP22)
#include <stdio.h>
int __io_putchar(int c) {

    if(c == '\n')
            {
                while(!(USART5->ISR && USART_ISR_TXE));
                USART5->TDR = '\r';
            }

    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
     while (!(USART5->ISR & USART_ISR_RXNE)) { }
     char c = USART5->RDR;
     if(c == '\r') {
         c = '\n';
     }

     __io_putchar(c);
     return c;
}

int main() {
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

//#define STEP23
#if defined(STEP23)
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
int __io_putchar(int c) {

    if(c == '\n')
            {
                while(!(USART5->ISR && USART_ISR_TXE));
                USART5->TDR = '\r';
            }

    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
     /*while (!(USART5->ISR & USART_ISR_RXNE)) { }
     char c = USART5->RDR;
     if(c == '\r') {
         c = '\n';
     }*/

     //__io_putchar(c);

    char c = line_buffer_getchar();
     return c;
}

int main() {
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif


#define STEP24
#if defined(STEP24)
#include <stdio.h>
#include "fifo.h"
#include "tty.h"

#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

int __io_putchar(int c) {

    if(c == '\n')
            {
                while(!(USART5->ISR && USART_ISR_TXE));
                USART5->TDR = '\r';
            }

    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
     /*while (!(USART5->ISR & USART_ISR_RXNE)) { }
     char c = USART5->RDR;
     if(c == '\r') {
         c = '\n';
     }*/

     //__io_putchar(c);

    char c = interrupt_getchar();
     return c;
}
void enable_tty_interrupt(void) {

    USART5->CR1 |= USART_CR1_RXNEIE;
    USART5->CR3 |= USART_CR3_DMAR;

    NVIC_EnableIRQ(USART3_8_IRQn);

    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->RMPCR |= DMA_RMPCR2_CH2_USART5_RX;

    DMA2_Channel2->CCR &= ~DMA_CCR_EN;  // First make sure DMA is turned off

    DMA2_Channel2->CPAR = &(USART5->RDR);
    DMA2_Channel2->CMAR = &(serfifo);
    DMA2_Channel2->CNDTR = FIFOSIZE;

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

void USART3_4_5_6_7_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

int interrupt_getchar(void) {

    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi");
    }
    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

int main() {
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif
