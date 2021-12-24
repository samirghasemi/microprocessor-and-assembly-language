#include <stm32f4xx.h>

#define mask(x) (1UL <<(x))

// VARS!
volatile int A = 0;
volatile int B = 0;

volatile int output = 0;

volatile int operation = 0;

volatile int inputAiIsValid = 0;



// function for delay 
void delay(volatile int sec){
  volatile int i = 0;
	for(i = 0 ; i < sec ; i++);
}

// calculate result of operation
int performOperations(){
  volatile int result;
	if(operation==0){
		result = A + B;
	}
	else if(operation==1){
		result = A - B;
	}
	else if(operation==2){
		result = A * B;
	}
	else if(operation==3){
		result = A / B;
	}
    return result;
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
	if(GPIOC->IDR & mask(0)){
		operation++;
		operation = operation % 4;
	}
    // namayesh adad operation bar royey 7seg
    GPIOB->ODR &= 0xFFF0;
		GPIOB->ODR |= operation;
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
			delay(1000000);
      // namayesh adad aval bar royey 7seg samte chap
	    GPIOA->ODR &=0xF0FF;
	    GPIOA->ODR |= A<<8;
		}else{
			inputAiIsValid=0;
			B=numberDiagnose(1);
			output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    GPIOA->ODR &=0x0F00;
	    GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    GPIOA->ODR |= (output%10);
		//dah gan
	    GPIOA->ODR |= (output/10)<<4;
	    delay(3500000);
      // 0 kardan 7seg ha
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
			delay(1000000);
      // namayesh adad aval bar royey 7seg samte chap
			GPIOA->ODR &=0xF0FF;
	    GPIOA->ODR |= A<<8;
		}else{
      inputAiIsValid=0;
			B=numberDiagnose(2);
			output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    GPIOA->ODR &=0x0F00;
	    GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    GPIOA->ODR |= (output%10);
		//dah gan
	    GPIOA->ODR |= (output/10)<<4;
	    delay(3500000);
      // 0 kardan 7seg ha
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
			delay(1000000);
      // namayesh adad aval bar royey 7seg samte chap
			GPIOA->ODR &=0xF0FF;
	    GPIOA->ODR |= A<<8;
		}else{
			inputAiIsValid=0;
			B=numberDiagnose(3);
			output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    GPIOA->ODR &=0x0F00;
	    GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    GPIOA->ODR |= (output%10);
		//dah gan
	    GPIOA->ODR |= (output/10)<<4;
	    delay(3500000);
      // 0 kardan 7seg ha
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
			delay(1000000);
      // namayesh adad aval bar royey 7seg samte chap
			GPIOA->ODR &=0xF0FF;
	    GPIOA->ODR |= A<<8;
		}else{
			inputAiIsValid=0;
			B=0; // chon az radife akhar faqat 0 mitonim bekhonim
			output=performOperations();
      // namayesh adad dovom bar royey 7seg
	    GPIOA->ODR &=0x0F00;
	    GPIOA->ODR |= B<<12;
      // namayesh khoroji
	  //yekan
	    GPIOA->ODR |= (output%10);
		//dah gan
	    GPIOA->ODR |= (output/10)<<4;
	    delay(3500000);
      // 0 kardan 7seg ha
	    GPIOA->ODR &=0x0000;
		}	 
	}
}



int main(){
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
