// we recieve in usart1 and send in usart2 when we are in CPUSide
//  * : 10 
//  # : 12
#include <stm32f4xx.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define mask(x) (1UL <<(x))

volatile int a , b , c;
volatile int whichNumber = 0;
void getter(int input);
void usart2_send(uint16_t data);
void usart2_init(uint32_t pclk);
void usart1_init(uint32_t pclk);
void usart12_init(uint32_t pclk);
void show_result_in_cpuside(int number);
void send_result_to_terminal(int number);
int calculate_integral(void);
int shift_and_plus(int number, int plus);
void set_zero(void);
void GPIOS_enable(void);

int main(){
	GPIOS_enable();

	RCC->APB2ENR |=RCC_APB2ENR_SYSCFGEN;
	GPIOB->MODER &= 0x00000000;
	GPIOB->MODER |= 0x55555555;
	GPIOC->MODER |= 0x55555555;

	__disable_irq();
   	usart12_init(8);	
	__enable_irq();

	while(1);
}


void create_delay(int ins){
	volatile int i = 0;
	for(i = 0; i < ins; i++);
}

// reciever IRQ Handler
void USART1_IRQHandler() {
	if (READ_BIT(USART1->SR, USART_SR_RXNE)) {
		NVIC_ClearPendingIRQ(USART1_IRQn);
	  	getter((uint16_t)(USART1->DR & (uint16_t)0x01FF));
	}
}

// sender IEQ Handler
void USART2_IRQHandler() {
	NVIC_ClearPendingIRQ(USART2_IRQn);
}

// sent data to terminal with usart2
void usart2_send(uint16_t data) {
	USART2->DR |= (data & (uint16_t)0x01ff);	
	while(!READ_BIT(USART2->SR, USART_SR_TC)){}
}		
void usart1_init(uint32_t pclk){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;						// enable GPIOA clock
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;						// enable usart1 clock
    GPIOA->MODER |= GPIO_MODER_MODER10_1;         				//PinA10 mode AF
	GPIOA->OTYPER |= 0x0400;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10_1;
    GPIOA->AFR[1] |= 0x00000700;                 				//Set pin10 the AF to AF7(USART1);
    SET_BIT(RCC->APB2RSTR,RCC_APB2RSTR_USART1RST);
    CLEAR_BIT(RCC->APB2RSTR,RCC_APB2RSTR_USART1RST);
	USART1->CR1 |= USART_CR1_UE;            					// Enable USART1
	USART1->CR2 = 0x0000; 										/* 1 stop bit */
	USART1->CR3 = 0x0000;
	CLEAR_BIT(USART1->CR1,USART_CR1_PEIE);						// no parity bit
	USART1->BRR = 0x0683;										// BASED ON SLIDES /* 9600 baud @ 16 MHz */
    USART1->CR1 |= USART_CR1_RE;								// recieve enable
	NVIC_SetPriority(USART1_IRQn, 0);
	NVIC_ClearPendingIRQ(USART1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	USART1->CR1 |= USART_CR1_RXNEIE;							// RECIEVE INTRRUPT ENABLE
}
void usart2_init(uint32_t pclk){
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;						// enable usart2 clock
    GPIOA->MODER |= GPIO_MODER_MODER2_1;         				//PinA2 mode AF
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2_1;
    GPIOA->AFR[0] |= 0x00000700;                 				//Set the AF to AF7(USART1~3);
    SET_BIT(RCC->APB1RSTR,RCC_APB1RSTR_USART2RST);
    CLEAR_BIT(RCC->APB1RSTR,RCC_APB1RSTR_USART2RST);
	USART2->CR1 |= USART_CR1_UE;            					// Enable USART2
	USART2->CR2 = 0x0000;                   					/* 1 stop bit */
	USART2->CR3 = 0x0000;					
	CLEAR_BIT(USART2->CR1,USART_CR1_PEIE);						// no parity bit
	USART2->BRR = 0x0683;										// BASED ON SLIDES /* 9600 baud @ 16 MHz */
    USART2->CR1 |= USART_CR1_TE;								// transmit enable
  	NVIC_SetPriority(USART2_IRQn, 0);
	NVIC_ClearPendingIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);
	USART2->CR1 |= USART_CR1_TCIE;								// trasmit complete intrrupt enable
}

void usart12_init(uint32_t pclk){
	usart1_init(pclk);	
	usart2_init(pclk);
}

void show_result_in_cpuside(int number){
	//send from left most digit to right most 
	int currentDigit;
	GPIOC->ODR = 0x0;
	currentDigit = (int)(number / 1000);
	number %= 1000;
	GPIOC->ODR |= currentDigit << 12;

	currentDigit = (int)(number / 100);
	number %= 100;
	GPIOC->ODR |= currentDigit << 8;

	currentDigit = (int)(number / 10);
	number %= 10;
	GPIOC->ODR |= currentDigit << 4;

	currentDigit = (int)number;
	GPIOC->ODR |= currentDigit;
}

void send_result_to_terminal(int number){
	//send from left most digit to right most 
	int currentDigit;
	volatile int i = 0;
	show_result_in_cpuside(number);
	for (i = 3; i >= 0; i--){
		currentDigit = (int)(number / (int)pow(10, i));
		number %= (int)pow(10, i);
		usart2_send((uint16_t)currentDigit);
		create_delay(15000);
	}
}

int calculate_integral(){
	// calculat integral of (a^3 * x) + (b ^ 2 * x) + (c * x) in range of [1, 2]
	return (int)(((15 / 4) * a) + ((7 / 3) * b) + ((3 / 2) * c));
}

int shift_and_plus(int number, int plus){
	return (number * 10) + plus;
}

void set_zero(){
	a = 0;
	b = 0;
	c = 0;
}


void getter(int input){
	usart2_send((uint16_t)input); 				// send back input 	
		
	create_delay(60000);
	
	if(whichNumber==0){
		if(input>=0 && input<=9){				
			a = shift_and_plus(a, input);					// set input as right most digit of a
		}else if(input==10){				
			a = 0;											// clear number a (input is *)
		}else if(input==12){
			whichNumber=1; 									// end of number a (input is #)
		}
	}
	else if(whichNumber==1){
		if(input>=0 && input<=9){				
			b = shift_and_plus(b, input);					// set input as right most digit of a
		}else if(input==10){			
			b = 0;											// clear number a (input is *)
		}else if(input==12){					
			whichNumber=2;									// end of number a (input is #)	
		}
	}
	else if(whichNumber==2){
		if(input>=0 && input<=9){
			c = shift_and_plus(c, input);					// set input as right most digit of a
		}else if(input==10){
			c = 0;											// clear number a (input is *)
		}else if(input==12){								// end of number a (input is #)
			int integral = calculate_integral();
			send_result_to_terminal(integral);				//send integral to terminal side
			whichNumber=0;									// end of get input
			set_zero();										// set a, b, c = 0
		}
	}

	// show whichnumber variable in 7seg [GPIOB PIN0 - PIN3]
	GPIOB->ODR &= 0xFFF0;
	GPIOB->ODR |=  whichNumber;

}

void GPIOS_enable(){
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOCEN;
}


