#include <stm32f4xx.h>
#include <stdio.h>
#include <stdbool.h>

#define mask(x) (1UL <<(x))

volatile int currentDigit;
volatile int whichDigit = 0;
void getter(int digit);
void SPI_write(int command, short data);
void create_delay(int ms);
void usart2_send(uint16_t data);
void usart2_init(uint32_t pclk);
void usart1_init(uint32_t pclk);
void usart12_init(uint32_t pclk);
void clear_7segs(void);
void set_7seg_for_err(void);
void getter(int digit);
int number_identify(int row);
void SPI_init(void);
void SPI_write(int command, short data);
void keypad_intrrupt_init(void);



int main(){
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOCEN;
	
	GPIOC->MODER=0x5400;

	RCC->APB2ENR |=RCC_APB2ENR_SYSCFGEN;

	SYSCFG->EXTICR[0] |=SYSCFG_EXTICR1_EXTI1_PC;
	SYSCFG->EXTICR[0] |=SYSCFG_EXTICR1_EXTI2_PC;
	SYSCFG->EXTICR[0] |=SYSCFG_EXTICR1_EXTI3_PC;
	SYSCFG->EXTICR[1] |=SYSCFG_EXTICR2_EXTI4_PC;
	
    __disable_irq();
	SPI_init();
	SPI_write(0x09, 0x0F);
	SPI_write(0x0B, 0x03);
	SPI_write(0x0A, 0x0F);
	SPI_write(0x0C, 0x01); 
	usart12_init(8);
	__enable_irq();

	keypad_intrrupt_init();
	
	GPIOC->ODR &=0xFF1F;
	GPIOC->ODR |=0x00E0;

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
// sender IRQ Handler 
void USART2_IRQHandler() {
	NVIC_ClearPendingIRQ(USART2_IRQn);
}
// send a data to cpu with usart 2
void usart2_send(uint16_t data) {
	USART2->DR |= (data & (uint16_t)0x01ff);	
	while(!READ_BIT(USART2->SR, USART_SR_TC)){}
}		
void usart1_init(uint32_t pclk){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;					// enable gpioa clock
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;					// enable usart1 clock
    GPIOA->MODER |= GPIO_MODER_MODER10_1;         			// Pin10 mode AF
	GPIOA->OTYPER |= 0x0400;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR10_1;
    GPIOA->AFR[1] |= 0x00000700;                 			//Set pin10 the AF to AF7(USART1);
    SET_BIT(RCC->APB2RSTR,RCC_APB2RSTR_USART1RST);
    CLEAR_BIT(RCC->APB2RSTR,RCC_APB2RSTR_USART1RST);
	USART1->CR1 |= USART_CR1_UE;            				// 		Enable USART
	USART1->CR2 = 0x0000; 									/* 1 stop bit */
	USART1->CR3 = 0x0000;
	CLEAR_BIT(USART1->CR1,USART_CR1_PEIE);					// no parity bit
	USART1->BRR = 0x0683; 									// BASED ON SLIDES /* 9600 baud @ 16 MHz */
    USART1->CR1 |= USART_CR1_RE;							// recieve enable
	NVIC_SetPriority(USART1_IRQn, 0);
	NVIC_ClearPendingIRQ(USART1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	USART1->CR1 |= USART_CR1_RXNEIE;						// RECIEVE INTRRUPT ENABLE
}
void usart2_init(uint32_t pclk){
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					// enable usart2 clock
    GPIOA->MODER |= GPIO_MODER_MODER2_1;         			// pin2 mode AF
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2_1;
    GPIOA->AFR[0] |= 0x00000700;                 			//Set the AF to AF7(USART1~3);
    SET_BIT(RCC->APB1RSTR,RCC_APB1RSTR_USART2RST);
    CLEAR_BIT(RCC->APB1RSTR,RCC_APB1RSTR_USART2RST);
	USART2->CR1 |= USART_CR1_UE;            				// Enable USART
	USART2->CR2 = 0x0000;                   				/* 1 stop bit */
	USART2->CR3 = 0x0000;
	CLEAR_BIT(USART2->CR1,USART_CR1_PEIE);					// no parity bit
	USART2->BRR = 0x0683;									// BASED ON SLIDES /* 9600 baud @ 16 MHz */
    USART2->CR1 |= USART_CR1_TE;							// transmit enable
  	NVIC_SetPriority(USART2_IRQn, 0);
	NVIC_ClearPendingIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);
	USART2->CR1 |= USART_CR1_TCIE;							// trasmit complete intrrupt enable
}


void usart12_init(uint32_t pclk){
	usart1_init(pclk);
	usart2_init(pclk);
}
// set all segmanet zero
void clear_7segs(){
	SPI_write(1,0);
	SPI_write(2,0);
	SPI_write(3,0);
	SPI_write(4,0);
}

void set_7seg_for_err(){
	SPI_write(1,12);
	SPI_write(2,12);
	SPI_write(3,12);
	SPI_write(4,12);
}
// get input from cpu and do the work required
void getter(int digit){
	if(digit >= 0 && digit <= 9){							// input is a valid digit
		SPI_write(whichDigit + 1, digit);					// write current valid input digit on 7 seg
		whichDigit = (whichDigit + 1) % 4;
	}else if (digit == 10 || digit == 12){					// clear --input is * OR #
		clear_7segs();
		whichDigit = 0;
	}else if(digit == 13){ 									// An error has occurred
		set_7seg_for_err();
	}
	
}
// Identify which key is pressed
int number_identify(int row){
	// first column
	GPIOC->ODR &= 0xFFDF;
	create_delay(100);
	if(!(GPIOC->IDR & mask(row))){
		GPIOC->ODR |= 0x00F0;
		if (row == 1){
			return 1;
		}else if(row == 2){
			return 4;
		}else if(row == 3){
			return 7;
		}else if(row == 4){
			return 10; //*
		}
	}
	// second column
	create_delay(100);
	GPIOC->ODR &= 0xFFBF;
	if(!(GPIOC->IDR & mask(row))){
		GPIOC->ODR |= 0x00F0;
				if (row == 1){
			return 2;
		}else if(row == 2){
			return 5;
		}else if(row == 3){
			return 8;
		}else if(row==4){
			return 0;
		}
	}
	// third column
	create_delay(100);
	GPIOC->ODR &= 0xFF7F;
	if(!(GPIOC->IDR & mask(row))){
		GPIOC->ODR |= 0x00F0;
		if (row == 1){
			return 3;
		}else if(row == 2){
			return 6;
		}else if(row == 3){
			return 9;
		}else if(row == 4){
			return 12; //#
		}
	}

	GPIOC->ODR |= 0x00F0;
	return -1;
	
}


// row A in keypad 
void EXTI1_IRQHandler(void){
	EXTI->PR |= mask(1);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	if(GPIOC->IDR & mask(1)){
		currentDigit=number_identify(1);
		usart2_send((uint16_t)currentDigit);
		create_delay(100000);
	}
}
// row B in keypad 
void EXTI2_IRQHandler(void){
	EXTI->PR |= mask(2);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
	if(GPIOC->IDR & mask(2)){
		currentDigit=number_identify(2);
		usart2_send((uint16_t)currentDigit);
		create_delay(100000);
	}
}
// row C in keypad 
void EXTI3_IRQHandler(void){
	EXTI->PR |= mask(3);
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
	if(GPIOC->IDR & mask(3)){	
		currentDigit=number_identify(3);
		usart2_send((uint16_t)currentDigit);	
		create_delay(100000);		
	}
}
// row D in keypad 
void EXTI4_IRQHandler(void){
	EXTI->PR |= mask(4);
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
	if(GPIOC->IDR & mask(4)){
		currentDigit=number_identify(4);
		usart2_send((uint16_t)currentDigit);	 
		create_delay(100000);
	}		
}

void SPI_init(){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; 				/* enable GPIOB clock */
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;					/* enable SPI1 clock */
	GPIOB->MODER &= ~0x00000CC0; 						/* clear pin mode */
	GPIOB->MODER |= 0x00000880; 						/* set pin alternate mode */
	GPIOB->AFR[0] &= ~0x00F0F000; 						/* clear alt mode */
	GPIOB->AFR[0] |= 0x00505000; 						/* set alt mode SPI1 */
	GPIOB->MODER &= ~0x00000300; 						/* clear pin mode */
	GPIOB->MODER |= 0x00000100; 						/* set pin output mode */
	SPI1->CR1 = 0x31C; 									/* set the Baud rate, 8-bit data frame */
	SPI1->CR2 = 0;   									
	SPI1->CR1 |= 0x40; 									/* enable SPI1 module */
}

void SPI_write(int command, short data){
	command &= 0x0FF;  									/* make sure command is 8-bit */
	data &= 0x0FF; 										/* make sure data is 8-bit */
	while (!(SPI1->SR & 2)) {} 							/* wait until Transfer buffer Empty */
	SPI1->DR = command; 								/* write command and upper 4 bits of data */
	GPIOB->BSRR = 0x00100000; 							/* deassert slave select */
	while (!(SPI1->SR & 2)) {} 							/* wait until Transfer buffer Empty */
	SPI1->DR = data;									/* write date */
	while (SPI1->SR & 0x80) {} 							/* wait for transmission done */
	GPIOB->BSRR = 0x00000010; 							/* assert slave select */
}

void keypad_intrrupt_init(){
	EXTI->IMR  |= (mask(1));
	EXTI->RTSR |= (mask(1));
	EXTI->IMR  |= (mask(2));
	EXTI->RTSR |= (mask(2));
	EXTI->IMR  |= (mask(3));
	EXTI->RTSR |= (mask(3));
	EXTI->IMR  |= (mask(4));
	EXTI->RTSR |= (mask(4));

	NVIC_SetPriority(EXTI1_IRQn,0);
	NVIC_SetPriority(EXTI2_IRQn,0);
	NVIC_SetPriority(EXTI3_IRQn,0);		
	NVIC_SetPriority(EXTI4_IRQn,0);	
	
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
	
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
}


