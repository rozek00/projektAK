/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#define RECHEIGHT 10
#define RECWIDTH 4
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int licznik_punktow=0;
char point_buffer[50];
int estimated_pos;
volatile bool newInput = false;
volatile char inputChar = '\0';

int __io_getchar(void) {
    if (newInput) {
        newInput = false;
        return (int)inputChar;
    }
    return -1;
}

// Callback do odbioru danych przez UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    newInput = true;
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&inputChar, 1); // Odbiór danych asynchroniczny
}

static int x = (SSD1306_WIDTH / 2) -10;
static int y = SSD1306_HEIGHT / 2;
int dx = 2;
int dy = 2;
struct paletka
{
	int x_pos;
	int y_pos;
};
struct paletka gracz;
struct paletka komputer;
void SetDefault()
{
    dx=2;
    dy=2;
    x=SSD1306_WIDTH / 2;
    y=SSD1306_HEIGHT / 2;
}
void Update_Screen()
 {
	 ssd1306_Fill(Black);
	 ssd1306_DrawRectangle(gracz.x_pos , gracz.y_pos , gracz.x_pos + RECWIDTH , gracz.y_pos + RECHEIGHT , White);
     ssd1306_DrawRectangle(komputer.x_pos , komputer.y_pos , komputer.x_pos + RECWIDTH , komputer.y_pos + RECHEIGHT , White);
     ssd1306_DrawRectangle(x, y, x+5, y+5, White);
     ssd1306_UpdateScreen();
}
void receive_uart_command() {
    char direction = __io_getchar();
    switch(direction) {
        case 'w':
            if(gracz.y_pos > 0) gracz.y_pos--;
            break;
        case 's':
            if(gracz.y_pos < SSD1306_HEIGHT - RECHEIGHT) gracz.y_pos++;
            break;
    }
}
void updateSquarePosition()
 {
    x += dx;
    y += dy;
    if (x < gracz.x_pos + RECWIDTH &&
        x + 5 > gracz.x_pos &&
        y < gracz.y_pos + RECHEIGHT &&
        y + 5 > gracz.y_pos)
    {
        x = gracz.x_pos + RECWIDTH;
        dx = -dx;
    }
    if (x + 5 > komputer.x_pos &&
        x < komputer.x_pos + RECWIDTH &&
        y < komputer.y_pos + RECHEIGHT &&
        y + 5 > komputer.y_pos)
    {
        x = komputer.x_pos - 5;
        dx = -dx;
    }
    if (x < 3 && dx < 0) {
    	licznik_punktow++;
    	sprintf(point_buffer, "Ilosc punktow: %d", licznik_punktow);
    	HAL_UART_Transmit(&point_buffer, (uint8_t *)point_buffer, strlen(point_buffer), HAL_MAX_DELAY);
    	SetDefault();
    }
    if (x > SSD1306_WIDTH-7 && dx > 0) {
    	x = SSD1306_WIDTH-7;
        dx = -dx;
    }

    if (y < 0 && dy < 0) {
    	y = 0;
        dy = -dy;
    }
    if (y > SSD1306_HEIGHT-7 && dy > 0) {
    	y = SSD1306_HEIGHT-7;
        dy = -dy;
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == B1_Pin)
  {
	  SetDefault();
  }
}
void Estimate_Pos() {

if (dx > 0) { // Piłka porusza się w kierunku komputera

int distance_x = komputer.x_pos - x;

if (distance_x > 0) { // Tylko jeśli odległość jest dodatnia

int time_to_reach = distance_x / dx;

int estimated_y = y + dy * time_to_reach;

// Ogranicz estimated_y do zakresu ekranu

if (estimated_y < 0) {

estimated_y = 0;

} else if (estimated_y > SSD1306_HEIGHT - RECHEIGHT) {

estimated_y = SSD1306_HEIGHT - RECHEIGHT;

}

estimated_pos = estimated_y;

}

}

// Poruszaj paletką komputera w kierunku estimated_pos

if (komputer.y_pos < estimated_pos && komputer.y_pos < SSD1306_HEIGHT - RECHEIGHT) {

komputer.y_pos++;

} else if (komputer.y_pos > estimated_pos && komputer.y_pos > 0) {

komputer.y_pos--;

}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  ssd1306_Init();
  gracz.x_pos=0;
  komputer.x_pos=SSD1306_WIDTH-RECWIDTH-1;
  HAL_UART_Receive_IT(&huart2, (uint8_t *)&inputChar, 1);
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  updateSquarePosition();
	  receive_uart_command();
	  Estimate_Pos();
	  Update_Screen();
	  HAL_Delay(10);
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00100D14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
