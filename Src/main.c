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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "IQmathLib.h"
#include "stdio.h"
#include "string.h"
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
//这里DMAflag初始值设置为250的用意是，TIM1每中断一次时间为0.02ms，控制中断250次即可达到5ms控制时间
uint16_t DMA_flag = 250*200;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t motor_init(void);
uint16_t sonic_init(void);
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
	uint16_t std=0;
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	//呼吸灯呼吸2min
	
	
	
	if(motor_init())
		while(1)
		{
			//闪灯
			HAL_Delay(500);
			
			HAL_Delay(500);
		}
	//闪灯
	HAL_Delay(500);
	
	HAL_Delay(500);

	std=sonic_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//仅有SEQ_flag=1OR3时视为有声波信号传入，进行数据读取
		if (SEQ_flag == 1 || SEQ_flag == 3)
		{
				//DMA读入数据
				uint16_t Temp_ADC[2]={0,0},Max_ADC[2]={0,0};
				HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&Temp_ADC,2);
				//控制5ms不断比较峰值数据
				HAL_TIM_Base_Start_IT(&htim1);
				while(DMA_flag)
				{
					if(Temp_ADC[0]>Max_ADC[0]) Max_ADC[0] = Temp_ADC[0];
					if(Temp_ADC[1]>Max_ADC[1]) Max_ADC[1] = Temp_ADC[1];
				}
				HAL_TIM_Base_Stop_IT(&htim1);
				HAL_ADC_Stop_DMA(&hadc1);
				
				//判断角度和距离(计算方法)
				float dst = 0.0 ,agl = 0.0;
				_iq r1 ;
				
				r1 = _IQ(std+10);
				agl = _IQtoF(r1);
				//串口输出
				printf("%03.1f",dst);
				printf("%03.1f",agl);
				
				//标志位清零
				SEQ_flag = 0;
				//这里DMAflag初始值设置为250的用意是，TIM1每中断一次时间为0.02ms，控制中断250次即可达到5ms控制时间
				DMA_flag = 250;
		}
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

  /** Initializes the CPU, AHB and APB busses clocks 
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
  /** Initializes the CPU, AHB and APB busses clocks 
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
		if(!SEQ_flag)
			SEQ_flag = GPIO_Pin;
		else 
			SEQ_flag = (SEQ_flag - GPIO_Pin) / 1024 + 2;	
}

uint8_t motor_init(void)
{
	printf("steste");
	char info[10];
	gets(info);
	if(!(strncmp(info,"sxe\n",1)||strncmp(info+2,"e\n",1)))
		return info[1]-48;
	return 255;
}

uint16_t sonic_init(void)
{
		uint16_t Temp_std[100];
		uint64_t Temp = 0, time = 0;
		//控制5s不断记录数据
		HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&Temp_std,100);
		HAL_TIM_Base_Start_IT(&htim1);
		HAL_Delay(1);
		//大约读入80组数据
		//最好能拿板子打断点测试一下
		for(uint8_t i = 0; i < 5 ; i++)
			while(DMA_flag)
			{
				for(uint8_t j = 0; j <= 1; j++)
				{
					for(int k=0; k<50 ;k++)
					{
						Temp += Temp_std[50*j+k];
						time++;
					}
					//闪灯
					HAL_Delay(1);
				}
			}
		HAL_TIM_Base_Stop_IT(&htim1);
		HAL_ADC_Stop_DMA(&hadc1);
		DMA_flag = 250;
		return (uint16_t)Temp/time;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
