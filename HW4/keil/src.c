#include "stm32f4xx.h"
#include "stdio.h"
#define ADCBUFSIZE 200
void DMA2_init(void); /* Initialize DMA2 controller */
void DMA2_Stream0_setup(unsigned int src, unsigned int dst, int len); /* set up a DMA transfer for ADC1 */
void TIM2_init(void); /* initialize TIM2 */
void ADC1_init(void); /* setup ADC */
volatile int done = 1;
volatile char adcbuf[ADCBUFSIZE]; /* buffer to receive DMA data transfers from ADC conversion results */
volatile char out[ADCBUFSIZE];



/* Initialize ADC
ADC1 is configured to do 8-bit data conversion and triggered by
the rising edge of timer TIM2 channel 2 output.
*/
void ADC1_init(void) {
    RCC->AHB1ENR |= 1; /* enable GPIOA clock */
    GPIOA->MODER |= 3; /* PA0 analog */
    RCC->APB2ENR |= 0x0100; /* enable ADC1 clock */
    ADC1->CR1 = 0x2000000; /* 8-bit conversion */
    ADC1->CR2 = 0x13000000; /* exten rising edge, extsel 3 = tim2.2 */
    ADC1->CR2 |= 0x400; /* enable setting EOC bit after each conversion */
    ADC1->CR2 |= 1; /* enable ADC1 */
}
/* Initialize TIM2
Timer TIM2 channel 2 is configured to generate PWM at 1 kHz. The output of
the timer signal is used to trigger ADC conversion.
*/
void TIM2_init(void) {
    RCC->AHB1ENR |= 2; /* enable GPIOB clock */
    GPIOB->MODER |= 0x80; /* PB3 timer2.2 out */
    GPIOB->AFR[0] |= 0x1000; /* set pin for timer output mode */
    RCC->APB1ENR |= 1; /* enable TIM2 clock */
    TIM2->PSC = 160 - 1; /* divided by 160 */
    TIM2->ARR = 100 - 1; /* divided by 100, sample at 1 kHz */
    TIM2->CNT = 0;
    TIM2->CCMR1 = 0x6800; /* pwm1 mode, preload enable */
    TIM2->CCER = 0x10; /* ch2 enable */
    TIM2->CCR2 = 50 - 1;
}
/* Initialize DMA2 controller
* DMA2 controller's clock is enabled and also the DMA interrupt is
* enabled in NVIC.
*/
void DMA2_init(void) {
    RCC->AHB1ENR |= 0x00400000; /* DMA2 controller clock enable */
    DMA2->LIFCR = 0x003F; /* clear all interrupt flags of Stream 0 */
    NVIC_EnableIRQ(DMA2_Stream0_IRQn); /* DMA interrupt enable at NVIC */
	NVIC_ClearPendingIRQ(DMA2_Stream0_IRQn);
	NVIC_SetPriority(DMA2_Stream0_IRQn,0);
}


/* Set up a DMA transfer for ADC
* The ADC1 is connected to DMA2 Stream 0. This function sets up the peripheral register address,
* memory address, number of transfers, data size, transfer direction, and DMA interrupts are enabled.
* At the end, the DMA controller is enabled, the ADC conversion complete is used to trigger DMA data
* transfer, and the timer used to trigger ADC is enabled.
*/
void DMA2_Stream0_setup(unsigned int dst, unsigned int src, int len) {
    DMA2_Stream0->CR &= ~1; /* disable DMA2 Stream 0 */
    while (DMA2_Stream0->CR & 1) {} /* wait until DMA2 Stream 0 is disabled */
    DMA2->LIFCR = 0x003F; /* clear all interrupt flags of Stream 0 */
    DMA2_Stream0->PAR = src;
    DMA2_Stream0->M0AR = dst;
    DMA2_Stream0->NDTR = len;
    DMA2_Stream0->CR = 0x00000000; /* ADC1_0 on DMA2 Stream0 Channel 0 */
    DMA2_Stream0->CR |= 0x00000400; /* data size byte, mem incr, peripheral-to-mem */
    DMA2_Stream0->CR |= 0x16; /* enable interrupts DMA_IT_TC | DMA_IT_TE | DMA_IT_DME */
    DMA2_Stream0->FCR = 0; /* direct mode, no FIFO */
    DMA2_Stream0->CR |= 1; /* enable DMA2 Stream 0 */
    ADC1->CR2 |= 0x0100; /* enable ADC conversion complete DMA data transfer */
    TIM2->CR1 = 1; /* enable timer2 */
}
/* DMA2 Stream0 interrupt handler
This function handles the interrupts from DMA2 controller Stream0. The error interrupts
have a placeholder for error handling code. If the interrupt is from DMA data
transfer complete, the DMA controller is disabled, the interrupt flags are
cleared, the ADC conversion complete DMA trigger is turned off and the timer
that triggers ADC conversion is turned off too.
*/
void DMA2_Stream0_IRQHandler(void)
{
    if (DMA2->LISR & 0x000C){ /* if an error occurred */
        while(1) {} /* substitute this by error handling */
    }
    DMA2_Stream0->CR = 0; /* disable DMA2 Stream 0 */
    DMA2->LIFCR = 0x003F; /* clear DMA2 interrupt flags */
    ADC1->CR2 &= ~0x0100; /* disable ADC conversion complete DMA */
    TIM2->CR1 &= ~1; /* disable timer2 */
    done = 1;
}


void DMA2_Stream1_setup(unsigned int src, unsigned int dst, int len){ 
    DMA2_Stream1->CR &= ~1; /* disable DMA2 Stream 10 */
    while (DMA2_Stream1->CR & 1) {} /* wait until DMA2 Stream 1 is disabled */
    DMA2->LIFCR = 0x003F; /* clear all interrupt flags of Stream 1 */
    DMA2_Stream1->PAR = dst;
    DMA2_Stream1->M0AR = src;
    DMA2_Stream1->NDTR = len;
    DMA2_Stream1->CR |= 0x00000040; /* half word size byte, mem incr, mem-to-pripheral */
    DMA2_Stream1->CR |= 0x16; /* enable interrupts DMA_IT_TC | DMA_IT_TE | DMA_IT_DME */
    DMA2_Stream1->FCR = 0; /* direct mode, no FIFO */
    DMA2_Stream1->CR |= 1; /* enable DMA2 Stream 1 */
}


void DMA2_Stream1_IRQHandler(void)
{
	if (DMA2->LISR & 0x000C) /* if an error occurred */
		while(1) {} /* substitute this by error handling */
	DMA2_Stream1->CR = 0; /* disable DMA2 Stream 0 */
	DMA2->LIFCR = 0x003F; /* clear DMA2 interrupt flags */
    done = 1;
	
	
}


int main(void) {
	int i;
    int j;
	
  __disable_irq();
 	
	DMA2_init();
	ADC1_init();
	TIM2_init();

	__enable_irq();


	
    while(1) {
        done = 0; /* clear done flag */
        /* start a DMA of ADC data transfer */
        DMA2_Stream0_setup((uint32_t)adcbuf, (uint32_t)&(ADC1->DR), ADCBUFSIZE);
        while (done == 0) {} /* wait for ADC DMA transfer complete */
    }

    for(j = 0;j<ADCBUFSIZE;j++){
        out[ADCBUFSIZE - 1 - j] = adcbuf[j];
    }
    
    while(1) {
        done = 0; /* clear done flag */
        /* start a DMA of ADC data transfer */
        DMA2_Stream1_setup((uint32_t)out, (uint32_t)&(GPIOA->ODR), ADCBUFSIZE);
        while (done == 0) {} /* wait for ADC DMA transfer complete */
    }
}
