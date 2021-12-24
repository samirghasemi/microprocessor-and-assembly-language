#include "stm32f4xx.h"
#include <cstdlib>
#include <ctime>
#include <stdlib.h>
volatile int goalNumber = 0;
volatile int gu = 150;
int main(void) {
	goalNumber = (rand() % 999) + 1;
__disable_irq(); /* global disable IRQs */
RCC->AHB1ENR |= 2; /* enable GPIOA clock */
GPIOB->MODER &= ~0x00000FF0;
GPIOB->MODER |= 0x00000AA0;
GPIOB->AFR[0] |= 0x00020000; /* PA5 pin for tim2 */

/* setup TIM2 */
RCC->APB1ENR |= 2; /* enable TIM2 clock */
TIM3->PSC = 1600 - 1; /* divided by 16000 */
TIM3->ARR = goalNumber - 1; /* divided by 1000 */
	TIM3->CNT = 0;
	TIM3->CCMR1 = 0x0060;
	TIM3->CCER = 1;
	TIM3->CCR1 = abs(gu-goalNumber) - 1;
TIM3->CR1 = 1; /* enable counter */
TIM3->DIER |= 1; /* enable UIE */
NVIC_EnableIRQ(TIM3_IRQn); /* enable interrupt in NVIC */
__enable_irq(); /* global enable IRQs */
while(1) {
}
}
long long i =0;
void TIM3_IRQHandler(void) {
TIM3->SR = 0; /* clear UIF */
}
