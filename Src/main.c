/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "math.h"

#define NUM_LEDS          256 //How many LEDs are in the string of lights
#define LENGTH            24 //Length of information (r=8 bits, g=8 bits, b=8 bits) - NOT SENDING BRIGHTNESS
#define FREQ              10 //Update Frequency (Hz), make no smaller than 3000 to not mess with LED Reset code
#define START_R           255
#define START_G           0
#define START_B           0
#define START_H           4

#define TEXT_R						80
#define TEXT_G						12
#define TEXT_B						12
#define TEXT_BUFFER				10
#define TEXT_LENGTH				10
#define BG_R							255
#define BG_G							255
#define BG_B							255


#define A									2140180607
#define B									4287730030
#define E									4287730065
#define H 								4279243007
#define I									0
#define L									4278255873
#define M									4282401023
#define N									0
#define O									2122416510
#define T									2164228224
#define U									0
#define SPACE							0
#define FULL							4294967295


//unsigned int full_text[] = {B, T, H, O, SPACE, N, M};
unsigned int full_text[] = {B, T, H, O, SPACE, A, L, A, B, A, M, A, FULL, FULL, FULL};

unsigned int animPeriod = 2*FREQ;


#define ANIM_PERIOD   ((unsigned int)&animPeriod)

TIM_HandleTypeDef TimHandle;
TIM_HandleTypeDef FrameHandle;

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned int fc = 0; //FRAME COUNT

unsigned char i = 0;
int on = 0;
  
unsigned int numLEDs = NUM_LEDS; //50 LEDs in 1D strip
unsigned int il = 0;
  
unsigned int c = (START_H << 24) + (START_G << 16) + (START_R << 8) + START_B;
  
unsigned int colors[NUM_LEDS];
unsigned int b_colors[NUM_LEDS];

//Variables for LED animations
unsigned int lastChase = 0;
unsigned int lastTrail = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

static void LED_High(void);
static void LED_Low(void);
static void Use_Brightness(void);
static void setH(unsigned int index, unsigned char h);
static void setRGB(unsigned int index, unsigned char r, unsigned char g, unsigned char b);
static void swapValues(unsigned int l[], unsigned int index0, unsigned int index1);
static void setManyRGB(unsigned int index, unsigned int length, unsigned char r, unsigned char g, unsigned char b);
static void sendData(unsigned int l[]);
static void default_values(unsigned int l[]);
static void paintText(unsigned int framecount, unsigned int l[], unsigned int text_length, unsigned char r, unsigned char g, unsigned char b);
static void fillBG(unsigned char r, unsigned char g, unsigned char b);
static void translateGrid(unsigned int l[]);
static void EXTILine0_Config(void);
static void Button_GPIO_Config(void);
static void setRGB_XY(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  
  
  
  
  HAL_Init();
  //EXTILine0_Config();
  //Button_GPIO_Config();
  
  for(int o = 0; o < numLEDs; o++){
    colors[o] = c;
  }
  
  for(int o = 0; o < numLEDs; o++){
    b_colors[o] = c;
  }
  //setManyRGB(0, sizeof(A), 25, 0, 255);
  //setManyRGB(0, 50, 0, 0, 255);
  BSP_LED_Init(LED3);
  BSP_LED_Off(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Off(LED4);

  /* Configure the System clock to 180 MHz */
  SystemClock_Config();
  
  uint16_t uwPrescalerValue = 0;
  
  uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / 90000000) - 1;
  //uwPrescalerValue = 0;
  TimHandle.Instance = TIM3;
  TimHandle.Init.Period = 196 - 1;
  TimHandle.Init.Prescaler = uwPrescalerValue;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if(HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  if(HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
  {
    /* Starting Error */
    Error_Handler();
  }
  
  uint16_t t4PrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (60000*FREQ)) - 1;
  FrameHandle.Instance = TIM4;
  FrameHandle.Init.Period = 60000 - 1;
  FrameHandle.Init.Prescaler = t4PrescalerValue;
  FrameHandle.Init.ClockDivision = 0;
  FrameHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  FrameHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if(HAL_TIM_Base_Init(&FrameHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  if(HAL_TIM_Base_Start_IT(&FrameHandle) != HAL_OK)
  {
    /* Starting Error */
    Error_Handler();
  }
  /* Infinite loop */
  while (1)
  {
  }
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	fillBG(BG_R, BG_G, BG_B);
  paintText(fc, full_text, TEXT_LENGTH, TEXT_R, TEXT_G, TEXT_B);
	//setRGB_XY(fc%32,fc/4%8,0,0,255);
	
//	for(int j = 0; j < 32; j++){
//		if((2131333374 >> (31-j)) % 2){
//			setRGB(j, TEXT_R, TEXT_G, TEXT_B);
//		}
//	}
	
	Use_Brightness();
	//translateGrid(colors);
  sendData(colors);
	fc++;
  
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == KEY_BUTTON_PIN)
  {
    /* Toggle LED3 */
    BSP_LED_Toggle(LED4);
  }
}

static void LED_High(void){ //1 bit
  if(!on && __HAL_TIM_GET_COUNTER(&TimHandle) <= 98){
      BSP_LED_On(LED3);
    on = 1;
    }
  if(on && __HAL_TIM_GET_COUNTER(&TimHandle) > 98){
      BSP_LED_Off(LED3);
      i++;
    on = 0;
    }
}

static void LED_Low(void){ //0 bit
  if(!on && __HAL_TIM_GET_COUNTER(&TimHandle) <= 27){
      BSP_LED_On(LED3);
    on = 1;
    }
  if(on && __HAL_TIM_GET_COUNTER(&TimHandle) > 27){
      BSP_LED_Off(LED3);
    i++;
    on = 0;
    
    while(__HAL_TIM_GET_COUNTER(&TimHandle) < 98){} //delay so 1 bit doesn't trigger early
    }
}

void Use_Brightness(){
  for(int o = 0; o < numLEDs; o++){
    unsigned char h = ((colors[o]& 0xFF000000) >> 24) ;
    b_colors[o] = ((((((colors[o] & 0xFF0000) >> 16) * h)/255) << 16) & 0xFF0000) | ((((((colors[o] & 0xFF00) >> 8) * h)/255) << 8) & 0xFF00) | ((((colors[o] & 0xFF) * h)/255) & 0xFF);
    //b_colors[o] = b_colors[o] & 0xFFFF00FF;
  }
}

void default_values(unsigned int l[]){
  for(int o = 0; o < numLEDs; o++){
    l[o] = c;
  }
}
void setRGB(unsigned int index, unsigned char r, unsigned char g, unsigned char b){
  colors[index] = (colors[index] & 0xFF000000) | (g << 16) | (r << 8) | b;
}

void setRGB_XY(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b){
	if(x % 2){
		colors[8 * x + (7 - y)] = (colors[8 * x + (7 - y)] & 0xFF000000) | (g << 16) | (r << 8) | b;
	}else{
		colors[8 * x + y] = (colors[8 * x + y] & 0xFF000000) | (g << 16) | (r << 8) | b;
	}
}

void setH(unsigned int index, unsigned char h){
  colors[index] = (colors[index] & 0x00FFFFFF) | (h << 24);
}

void swapValues(unsigned int l[], unsigned int index0, unsigned int index1){
  unsigned int temp = l[index0];
  l[index0] = l[index1];
  l[index1] = temp;
}

void setManyRGB(unsigned int index, unsigned int length, unsigned char r, unsigned char g, unsigned char b){
  for(int o = index; o < index + length; o++){
    colors[o % numLEDs] = (colors[o % numLEDs] & 0xFF000000) | (g << 16) | (r << 8) | b;
  }
}
void paintText(unsigned int framecount, unsigned int l[], unsigned int text_length, unsigned char r, unsigned char g, unsigned char b){
	//unsigned int start_pos = framecount % 256;
//	for(int o = 0; o < text_length; o++){
//		for(int j = 0; j < 32; j++){
//			if((full_text[o] >> (31-j)) % 2){
//					setRGB((j-16*framecount+48*o)%(48*text_length+TEXT_BUFFER*8), TEXT_R, TEXT_G, TEXT_B);
//			}
//		}
//	}
	for(int o = 0; o < text_length; o++){
		//o = letter
		for(int j = 0; j < 32; j++){
			//j = LED of letter
			if((full_text[o] >> (31-j)) % 2){
				setRGB_XY((6*o-framecount+j/8)%(6*text_length+TEXT_BUFFER), j%8, TEXT_R, TEXT_G, TEXT_B);
			}
		}
	}
	
	
}
void fillBG(unsigned char r, unsigned char g, unsigned char b){
	for(int o = 0; o < numLEDs; o++){
    colors[o % numLEDs] = (colors[o % numLEDs] & 0xFF000000) | (g << 16) | (r << 8) | b;
  }
}

void translateGrid(unsigned int l[]){
	unsigned int temp = 0;
	if(fc % 2 == 1){
		for(int o = 0; o < numLEDs; o++){
	//		if(o%16 > 7 && o%16 < 12){
	//			temp = l[o];
	//			l[o] = l[o+7-2*(o%8)];
	//			l[o+7-2*(o%8)] = temp;
	//		}
//			temp = l[o];
//			l[o] = l[o+7-2*(o%8)];
//			l[o+7-2*(o%8)] = temp;
			temp = l[o];
			l[o] = l[o+8];
			l[o+8] = temp;
			l[o] = 0;
		}
	}
}

void sendData(unsigned int l[]){
	
  __HAL_TIM_SET_COUNTER(&TimHandle, 0);
  while(il < numLEDs){
    
    if(((b_colors[il] >> (LENGTH-1-i)) % 2)){ //7-i because it's most significant first (i think) - change if not
      LED_High();
    }else{
      LED_Low();
    }
    
    if(i >= LENGTH){
      i = 0;
      il++;
    }   
    
  } 
  BSP_LED_Off(LED3);
  il = 0;
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the Over-Drive mode */
  HAL_PWREx_EnableOverDrive();
 
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

static void EXTILine0_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOA clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* Configure PA0 pin as input floating */
  //GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

static void Button_GPIO_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOA clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* Configure PA0 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
  GPIO_InitStructure.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 
