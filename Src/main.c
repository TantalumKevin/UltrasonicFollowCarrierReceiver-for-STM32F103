/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "IQmathLib.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/*时序标志
	0:待定
	Pin:该Pin率先拉起
		设计时：SEQL(左侧)=0x0400=1024,SEQR(右侧)=0x8000=2048
	1/3=L/R相对方位
	*/
uint16_t SEQ_flag = 0;
uint16_t delta_t = 0 ;
float TEMP_ADC[2]={0.0,0.0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t motor_init(void);
uint16_t sonic_init(void);
void Lumos(void);
void printUart(float data);
uint8_t gcInfo(uint8_t * cmpstr, uint8_t num);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//超声数据参考比较值
	uint16_t sonic_std=0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  //抢先关闭外部中断以防意外
  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	//等待串口拉起
  //HAL_UART_Transmit(&huart1,(uint8_t *)"s",1,0xFFFF);
  Lumos();
  //HAL_Delay(5000);
  Lumos();
	while (gcInfo((uint8_t *)"shelloe",7));
  HAL_UART_Transmit(&huart1,(uint8_t *)"shelloe",7,HAL_MAX_DELAY);
	if(motor_init())
  {
    Lumos();
		while(motor_init())
			Lumos();
  }
	Lumos(); 
  
	for(uint8_t Init_Sonic = 0; Init_Sonic < 5; Init_Sonic++)
	{
		sonic_std=sonic_init();
		printUart((float)sonic_std);
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //开启外部中断，进入常规工作模式
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//仅有SEQ_flag=1OR3时视为有声波信号传入，进行数据读取
		if (SEQ_flag == 1 || SEQ_flag == 3)
		{
      //关闭中断以防意外
      HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
			//原有思路是DMA读入数据，但由于DMA中断太快难以控制，转而改用轮询
			//此处原想用IQmath加速,但显然IQmath难以计算如此数量级的数字,
			//随便写一个buffer大小，实测再改
      float Temp_ADC[2]={0,0};
      for (uint8_t adc_i = 0; adc_i < 2200; adc_i++)
      {//输出100个周期(T=1/40k)ADC数值(22次采样≈1T)
        HAL_ADC_Start(&hadc1);
        if(HAL_OK == HAL_ADC_PollForConversion(&hadc1, 1))
          //输出一次ADC单通道值，下次输出为另一通道
          Temp_ADC[adc_i%2] += HAL_ADC_GetValue(&hadc1)/4095.0;
      }
			
			
			//判断角度和距离(计算方法)
			//float dst = 0.0 ,agl = 0.0;
			//_iq10 theta,alpha,delta_r ;
			//已知数据 Δt 单位0.1us Δl = 7cm
			//计算公式
			//theta = _IQ
			
			//串口输出
			printUart((float)delta_t*((float)SEQ_flag-1));
			printUart((float)Temp_ADC[0]*4095/1100.0);
			printUart((float)Temp_ADC[1]*4095/1100.0);
			
			//标志位清0
			SEQ_flag = 0;
		}
    //重新开启中断
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
		if(!SEQ_flag && (SEQ_flag - GPIO_Pin) != 0)
		{
			
			HAL_TIM_Base_Start(&htim2);
			__HAL_TIM_SetCounter(&htim2,0);
			SEQ_flag = GPIO_Pin;
		}
		else 
		{
			//关闭定时器,就是将定时器控制寄存器TIMx_CR1的CEN位置1	
			HAL_TIM_Base_Stop(&htim2); 
			//在运行时读取定时器的当前计数值,就是读取TIM2_CNT寄存器的值,注意单位0.1us;
			delta_t = __HAL_TIM_GET_COUNTER(&htim2); 
			__HAL_TIM_SET_COUNTER(&htim2, 0);
			//#define GPIO_PIN_10  ((uint16_t)0x0400)==1024
			//#define GPIO_PIN_15  ((uint16_t)0x8000)==32768
			//32768-1024=31744
			//结果=1 & 3
			//1:1015
			//3:1510
			SEQ_flag = ((int)SEQ_flag - (int)GPIO_Pin) / 31744 + 2;

		}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim2)
		if(SEQ_flag>10)
			SEQ_flag = 0;
}

uint8_t motor_init(void)
{
	HAL_UART_Transmit(&huart1,(uint8_t *)"steste",6,0xFFFF);
  HAL_Delay(300);
	char info[10]={0};
	while(1)
	{
		HAL_UART_Abort(&huart1);
		HAL_Delay(50);
		HAL_UART_Receive(&huart1, (uint8_t *)info, 3, 0xFFFF);
		HAL_UART_Abort(&huart1);
		HAL_Delay(50);
		if(!(strncmp(info,"s",1)||strncmp(info+2,"e",1)))
			return (uint8_t) info[1]-48;
		HAL_UART_Transmit(&huart1,(uint8_t *)"steste",6,0xFFFF);
		Lumos();
	}
}

uint16_t sonic_init(void)
{
		//uint16_t Temp_ADC[450];
		//控制5s不断记录数据
		//初始化ADC
		HAL_ADCEx_Calibration_Start(&hadc1);
		HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,0);   
    
		//闪灯
		for(uint16_t Sonic_Init_i = 0; Sonic_Init_i < 200; Sonic_Init_i ++)
		{
      for (uint8_t prd_i = 0; prd_i < 22; prd_i++)
      {//输出一个周期(T=1/40k)ADC数值
        for (uint8_t adc_i = 0; adc_i < 2; adc_i++)
        {//输出一组ADC数值
          HAL_ADC_Start(&hadc1);
          if(HAL_OK == HAL_ADC_PollForConversion(&hadc1, 1))
            //输出一次ADC单通道值，下次输出为另一通道
            TEMP_ADC[adc_i] += HAL_ADC_GetValue(&hadc1)/4095.0;
        }
      }
      __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,\
				(__HAL_TIM_GET_COMPARE(&htim3,TIM_CHANNEL_3)+5)%1000);
		}
		uint16_t Sonic_Init_Return = (uint16_t)(TEMP_ADC[0]+TEMP_ADC[1])*4095.0/8800.0;
		TEMP_ADC[0] = 0.0;
		TEMP_ADC[1] = 0.0;
		return Sonic_Init_Return;
}

void Lumos(void)
{
	//二极管闪亮一秒
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_3);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,0);
	HAL_Delay(500);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,1000);
	HAL_Delay(500);
	HAL_TIM_PWM_Stop(&htim3,TIM_CHANNEL_3);
}

void printUart(float data)
{
	HAL_UART_Transmit(&huart1,(uint8_t *)"s",1,0xFFFF);
  // 把浮点数data转换为字符串，存放在strdata中
  char strdata[6];
  sprintf(strdata,"%03.1f",data);
	HAL_UART_Transmit(&huart1,(uint8_t *)strdata,5,0xFFFF);
	HAL_UART_Transmit(&huart1,(uint8_t *)"e",1,0xFFFF);
  HAL_UART_Abort(&huart1);
}

uint8_t gcInfo(uint8_t * cmpstr, uint8_t num)
{
  //gc的意思是g:get,c:compare
  Lumos();
  uint8_t tempInfo_i;
  for(tempInfo_i = 0; tempInfo_i < num ; tempInfo_i++)
  {
    //Lumos();
    char ch=0;
    while(strncmp(&ch,(char *)cmpstr+tempInfo_i,1))
    {
      //Lumos();
      HAL_UART_Receive(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    }
  }
  HAL_UART_Abort(&huart1);
  HAL_Delay(50);
  return 0;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
