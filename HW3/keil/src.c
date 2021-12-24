#include <stm32f4xx.h>
#include <cstdlib>
#include <ctime>

#define mask(x) (1UL <<(x))

// VARS!
volatile int goalNumber;
volatile int A = 0;
volatile int B = 0;
volatile int C = 0;

volatile int operation = 0;

volatile int submit_hint_button = 0;

volatile int inputAiIsValid = 0;
volatile int inputBiIsValid = 0;
volatile int inputCiIsValid = 0;



// // function for delay 
// void delay(volatile int sec){
//   volatile int i = 0;
// 	for(i = 0 ; i < sec ; i++);
// }

void delay(int n) {
	int i;
	/* Configure SysTick */
	SysTick->LOAD = 16000; /* reload with number of clocks per millisecond */
	SysTick->VAL = 0; /* clear current value register */
	SysTick->CTRL = 0x5; /* Enable the timer */
	for(i = 0; i < n; i++) {
	while((SysTick->CTRL & 0x10000) == 0) /* wait until the COUNTFLAG is set */
	{ }
	}
	SysTick->CTRL = 0; /* Stop the timer (Enable = 0) */
}

// int buttonIsPressed(){
// 	// configure PA6 as input of TIM3 CH1
// 	RCC->AHB1ENR |= 1; /* enable GPIOA clock */
// 	GPIOA->MODER &= ~0x00003000; /* clear pin mode */
// 	GPIOA->MODER |= 0x00002000; /* set pin to alternate function */
// 	GPIOA->AFR[0] &= ~0x0F000000; /* clear pin AF bits */
// 	GPIOA->AFR[0] |= 0x02000000; /* set pin to AF2 for TIM3 CH1 */
// 	// configure TIM3 to do input capture with prescaler ...
// 	RCC->APB1ENR |= 2; /* enable TIM3 clock */
// 	TIM3->PSC = 16000 - 1; /* divided by 16000 */
// 	TIM3->CCMR1 = 0x41; /* set CH1 to capture at every edge */
// 	TIM3->CCER = 0x0B; /* enable CH 1 capture both edges */
// 	TIM3->CR1 = 1; /* enable TIM3 */
	
// }

int buttonIsPressed(int n) {
	int i;
	int bipms = 0;
	/* Configure SysTick */
	SysTick->LOAD = 16000; /* reload with number of clocks per millisecond */
	SysTick->VAL = 0; /* clear current value register */
	SysTick->CTRL = 0x5; /* Enable the timer */
	for(i = 0; i < n; i++) {
	while(((SysTick->CTRL & 0x10000) == 0) && (GPIOC->IDR & mask(0))) /* wait until the COUNTFLAG is set */
	{bipms++; }
	}
	SysTick->CTRL = 0; /* Stop the timer (Enable = 0) */
	if (bipms >= n){
		return 1;
	}else
	{
		return 0;
	}
	
}



int numberDiagnose(volatile int row){
    // set PK1 = 0
	delay(200);

    GPIOC->ODR &= 0xFFDF;
	if(!(GPIOC->IDR & mask(row))){
    // adad az soton aval hast va ba tavajoh be row yeki az maqadir [1,4,7] ra darad !
		GPIOC->ODR |= 0x00E0;
		if (row == 1){
			return 1;
		}else if(row == 2){
			return 4;
		}else if(row == 3){
			return 7;
		}
	}
    // set PK2 = 0
	GPIOC->ODR &= 0xFFBF;
	if(!(GPIOC->IDR & mask(row))){
    // adad az soton dovom hast va ba tavajoh be row yeki az maqadir [2,5,8] ra darad !
		GPIOC->ODR |= 0x00E0;
		if (row == 1){
			return 2;
		}else if(row == 2){
			return 5;
		}else if(row == 3){
			return 8;
		}
	}
    // set PK3 = 0
	GPIOC->ODR &= 0xFF7F;
	if(!(GPIOC->IDR & mask(row))){
    // adad az soton sevom hast va ba tavajoh be row yeki az maqadir [1,3,9] ra darad !
		GPIOC->ODR |= 0x00E0;
		if (row == 1){
			return 3;
		}else if(row == 2){
			return 6;
		}else if(row == 3){
			return 9;
		}
	}

	GPIOC->ODR |= 0x00F0;
	return 0;
}

// baraye button hast 
void EXTI0_IRQHandler(void){
	EXTI->PR |= mask(0);
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	// if(GPIOC->IDR & mask(0) && submit_hint_button == 0 && inputCiIsValid == 1){
	// 	volatile int temp = goalNumber;
    //     GPIOA->ODR &= 0xF000;
    //     GPIOA->ODR |= temp % 10;
	// 	temp /= 10;
	// 	GPIOA->ODR |= (temp%10 << 4);
	// 	temp /= 10;
    //     GPIOA->ODR |= (temp%10 << 8);
	// 	submit_hint_button++;
	// 	// operation = operation % 4;
	// }
	if(GPIOC->IDR & mask(0) && submit_hint_button == 0 && inputCiIsValid == 1){
        GPIOA->ODR &= 0xF000;
		GPIOA->ODR |= A << 8;
		GPIOA->ODR |= (B << 4);
        GPIOA->ODR |= (C << 0);
		submit_hint_button++;
		GPIOB->ODR &= 0xFFF0;
		GPIOB->ODR |= submit_hint_button;
	}
	// //proximity led !
	// else if(GPIOC->IDR & mask(0) && submit_hint_button == 1){
	// 	int temp = buttonIsPressed(1000);
	// 	if (temp == 0){

	// 	}
	// 	submit_hint_button++;
	// 	GPIOB->ODR &= 0xFFF0;
	// 	GPIOB->ODR |= submit_hint_button;
	// }
	// //
	// else if(GPIOC->IDR & mask(0) && submit_hint_button == 2){
	// 	int temp = buttonIsPressed(1000);
	// 	if (temp == 1){
	// 		GPIOA->ODR &= 0xF0FF;
	// 		GPIOA->ODR |= A << 8;
	// 		temp = buttonIsPressed(1000);
	// 		if(temp == 1){
	// 			GPIOA->ODR &= 0xFF0F;
	// 			GPIOA->ODR |= (B << 4);
	// 			temp = buttonIsPressed(1000);
	// 			if(temp == 1){
	// 				GPIOA->ODR &= 0xFFF0;
	// 				GPIOA->ODR |= (C << 0);	
	// 			}
	// 		}
	// 	}
	// 	}else{

	// 	}
	// }
}


// row 1
void EXTI1_IRQHandler(void){
	EXTI->PR |= mask(1);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
    // agar kelidi az in row feshorde shode bashe 
	if(GPIOC->IDR & mask(1)){
        // agar adad aval vared shode bashe 
		if(inputAiIsValid==0){
			inputAiIsValid=1;
			A=numberDiagnose(1); 
			delay(500000);
      // namayesh adad aval bar royey 7seg samte chap
	    // GPIOA->ODR &=0xF0FF;
	    // GPIOA->ODR |= A<<8;
		}else if(inputBiIsValid == 0){
			inputBiIsValid=1;
			B=numberDiagnose(1);
			// output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    // GPIOA->ODR &=0x0F00;
	    // GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    // GPIOA->ODR |= (output%10);
		//dah gan
	    // GPIOA->ODR |= (output/10)<<4;
	    delay(500000);
      // 0 kardan 7seg ha
	    GPIOA->ODR &=0x0000;
		}else if(inputCiIsValid == 0){
			inputCiIsValid=1;
			C=numberDiagnose(1);
			// delay(1000000);
			GPIOA->ODR &=0x0000;
		}
	}
}

// row 2 
void EXTI2_IRQHandler(void){
	EXTI->PR |= mask(2);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
    // agar kelidi az in row feshorde shode bashe 
	if(GPIOC->IDR & mask(2)){
		// agar adad aval vared shode bashe 
		if(inputAiIsValid==0){
			inputAiIsValid=1;
			A=numberDiagnose(2);
			delay(500000);
      // namayesh adad aval bar royey 7seg samte chap
			// GPIOA->ODR &=0xF0FF;
	    // GPIOA->ODR |= A<<8;
		}else if(inputBiIsValid == 0){
      inputBiIsValid=1;
			B=numberDiagnose(2);
			// output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    // GPIOA->ODR &=0x0F00;
	    // GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    // GPIOA->ODR |= (output%10);
		//dah gan
	    // GPIOA->ODR |= (output/10)<<4;
	    delay(500000);
      // 0 kardan 7seg ha
	    GPIOA->ODR &=0x0000;
		} else if(inputCiIsValid == 0){
			inputCiIsValid=1;
			C=numberDiagnose(2);
			// delay(1000000);
			GPIOA->ODR &=0x0000;
		}
	}
}

// row 3
void EXTI3_IRQHandler(void){
	EXTI->PR |= mask(3);
	NVIC_ClearPendingIRQ(EXTI3_IRQn);
    // agar kelidi az in row feshorde shode bashe 
	if(GPIOC->IDR & mask(3)){
		// agar adad aval vared shode bashe 
		if(inputAiIsValid==0){
			inputAiIsValid=1;
			A=numberDiagnose(3);
			delay(500000);
      // namayesh adad aval bar royey 7seg samte chap
			// GPIOA->ODR &=0xF0FF;
	    // GPIOA->ODR |= A<<8;
		}else if(inputBiIsValid == 0){
      inputBiIsValid=1;
			B=numberDiagnose(3);
			// output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    // GPIOA->ODR &=0x0F00;
	    // GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    // GPIOA->ODR |= (output%10);
		//dah gan
	    // GPIOA->ODR |= (output/10)<<4;
	    delay(500000);
      // 0 kardan 7seg ha
	    GPIOA->ODR &=0x0000;
		}else if(inputCiIsValid == 0){
			inputCiIsValid=1;
			C=numberDiagnose(3);
			// delay(1000000);
			GPIOA->ODR &=0x0000;
		}
	}
}

// row 4
void EXTI4_IRQHandler(void){
	EXTI->PR |= mask(4);
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
    // agar kelidi az in row feshorde shode bashe 
	if(GPIOC->IDR & mask(4)){
		// agar adad aval vared shode bashe 
		if(inputAiIsValid==0){
			inputAiIsValid=1;
			A=0; // chon az radife akhar faqat 0 mitonim bekhonim
			delay(500000);
      // namayesh adad aval bar royey 7seg samte chap
			// GPIOA->ODR &=0xF0FF;
	    // GPIOA->ODR |= A<<8;
		}else if(inputBiIsValid == 0){
      inputBiIsValid=1;
			B=0; // chon az radife akhar faqat 0 mitonim bekhonim
			// output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    // GPIOA->ODR &=0x0F00;
	    // GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    // GPIOA->ODR |= (output%10);
		//dah gan
	    // GPIOA->ODR |= (output/10)<<4;
	    delay(500000);
      // 0 kardan 7seg ha
	    GPIOA->ODR &=0x0000;
		}else if(inputCiIsValid == 0){
			inputCiIsValid=1;
			C=0;
			// delay(1000000);
			GPIOA->ODR &=0x0000;
		}
	}
}



int main(){
	goalNumber = (rand() % 999) + 1;

	
	__disable_irq();
		RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN;
		RCC->AHB1ENR |=RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR |=RCC_AHB1ENR_GPIOCEN;
	
		GPIOA->MODER=0x55555555;
    GPIOB->MODER=0x55;
		GPIOC->MODER=0x5400;
	
		RCC->APB2ENR |=RCC_APB2ENR_SYSCFGEN;
	
		SYSCFG->EXTICR[0] |=SYSCFG_EXTICR1_EXTI0_PC;
		SYSCFG->EXTICR[0] |=SYSCFG_EXTICR1_EXTI1_PC;
		SYSCFG->EXTICR[0] |=SYSCFG_EXTICR1_EXTI2_PC;
		SYSCFG->EXTICR[0] |=SYSCFG_EXTICR1_EXTI3_PC;
	  	SYSCFG->EXTICR[1] |=SYSCFG_EXTICR2_EXTI4_PC;
	
		EXTI->IMR  |= (mask(0)) | (mask(1)) | (mask(2)) | (mask(3)) | (mask(4));
		EXTI->RTSR |= (mask(0)) | (mask(1)) | (mask(2)) | (mask(3)) | (mask(4));	

	GPIOB->AFR[0] |= 0x00100000; /* PA5 pin for tim2 */
	GPIOB->MODER &= ~0x00000C00;
	GPIOB->MODER |= 0x00000800;
	/* setup TIM2 */
	RCC->APB1ENR |= 1; /* enable TIM2 clock */
	TIM2->PSC = 10 - 1; /* divided by 10 */
	TIM2->ARR = 10 - 1; /* divided by 26667 */
	TIM2->CNT = 0;
	TIM2->CCMR1 = 0x0060; /* PWM mode */
	TIM2->CCER = 1; /* enable PWM Ch1 */
	TIM2->CCR1 = 10 - 1; /* pulse width 1/3 of the period */
	TIM2->CR1 = 1; /* enable timer */

		__enable_irq();

		NVIC_SetPriority(EXTI0_IRQn,0);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI0_IRQn);

		NVIC_SetPriority(EXTI1_IRQn,0);
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);

	  NVIC_SetPriority(EXTI2_IRQn,0);
    NVIC_ClearPendingIRQ(EXTI2_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);

		NVIC_SetPriority(EXTI3_IRQn,0);
    NVIC_ClearPendingIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);

		NVIC_SetPriority(EXTI4_IRQn,0);	
		NVIC_ClearPendingIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);


		GPIOA->ODR &=0x0000;
		GPIOB->ODR &=0x0000;	
		GPIOC->ODR &=0xF1F;
		GPIOC->ODR |=0xE0;
		
		while(1);
}
