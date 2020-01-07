/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include "SPI_TFT_ILI9341.h"
#include "Touch_ADS7843.h"
#include "fonts.h"
#include "SampleImage.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

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
SPI_TFT_ILI9341_DriverStruct TFT;
Touch_ADS7843_DriverStruct TP;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void TFT_spi_write_callback(uint8_t *data, uint16_t nbytes) { HAL_SPI_Transmit(&hspi6, data, nbytes, 10); }
void TFT_digitalOut_dc_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_DC_GPIO_Port, SPI_TFT_ILI9341_DC_Pin, high_low); }
void TFT_digitalOut_cs_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_CS_GPIO_Port, SPI_TFT_ILI9341_CS_Pin, high_low); }
void TFT_digitalOut_reset_callback(uint8_t high_low) { HAL_GPIO_WritePin(SPI_TFT_ILI9341_RESET_GPIO_Port, SPI_TFT_ILI9341_RESET_Pin, high_low); }
void TFT_waitms_callback(uint16_t ms) { HAL_Delay(ms); }

uint8_t touch_spi_write_callback(uint8_t data)
{
  static uint8_t ret;
  HAL_SPI_TransmitReceive(&hspi3, &data, &ret, 1, 10);
  return ret;
}
void touch_digitalOut_cs_callback(uint8_t high_low) { HAL_GPIO_WritePin(TOUCH_ADS7843_CS_GPIO_Port, TOUCH_ADS7843_CS_Pin, high_low); }
uint8_t touch_digitalIn_irq_callback(void) { return HAL_GPIO_ReadPin(TOUCH_ADS7843_IRQ_GPIO_Port, TOUCH_ADS7843_IRQ_Pin); }

////////////////////////////////flappy start
int x, y; // Variables for the coordinates where the display has been pressed
// Floppy Bird
int xP = 300;
int yP = 100;
int yB = 70;
int movingRate = 3;
int fallRateInt = 0;
float fallRate = 0;
int score = 0;
char s[64];
int lastSpeedUpScore = 0;
int highestScore;
bool screenPressed = false;
bool gameStarted = false;

void drawPilars(int x, int y);
void drawBird(int y);
void gameOver();
void initiateGame();

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
  MX_SPI3_Init();
  MX_SPI6_Init();
  /* USER CODE BEGIN 2 */

  //exercise 1 start
  SPI_TFT_ILI9341(&TFT,
                  &TFT_spi_write_callback,
                  &TFT_digitalOut_cs_callback,
                  &TFT_digitalOut_reset_callback,
                  &TFT_digitalOut_dc_callback,
                  &TFT_waitms_callback);
  Touch_ADS7843(&TP,
                &touch_spi_write_callback,
                &touch_digitalOut_cs_callback,
                &touch_digitalIn_irq_callback,
                &TFT_waitms_callback, &TFT);

  SPI_TFT_ILI9341_set_background(&TFT, Navy);  // set background to black
  SPI_TFT_ILI9341_set_foreground(&TFT, White); // set chars to white

  ////// Touchpanel calibration
  SPI_TFT_ILI9341_set_orientation(&TFT, 3);
  Touch_ADS7843_TouchPanelCalibrate(&TP, Arial12x12);

  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial12x12);

  SPI_TFT_ILI9341_cls(&TFT);
  SPI_TFT_ILI9341_locate(&TFT, 0, 20);
  SPI_TFT_ILI9341_printf(&TFT, "Done.\nTouch panel to read again.\n");
  while (!Touch_ADS7843_OnTouch(&TP))
    ;

  initiateGame(&TP); /////////// Initiate the game

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if (gameStarted)
    {

      if (Touch_ADS7843_OnTouch(&TP)) //when touch
      {
        if (Touch_ADS7843_ReadADS7843(&TP, NULL))
        {
          Touch_ADS7843_GetDisplayPoint(&TP);
          SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial12x12);
          SPI_TFT_ILI9341_locate(&TFT, 0, 0);
          SPI_TFT_ILI9341_printf(&TFT, "x:%03d", TP.display.x);
          SPI_TFT_ILI9341_locate(&TFT, 40, 0);
          SPI_TFT_ILI9341_printf(&TFT, "y:%03d", TP.display.y);

          /////Controlling the bird
          fallRate = -6; // Setting the fallRate negative will make the bird jump
        }
      }

      ///////////////////////////////continue looping
      xP = xP - movingRate; // xP - x coordinate of the pilars; range: 319 - (-51)
      drawPilars(xP, yP);   // Draws the pillars

      // yB - y coordinate of the bird which depends on value of the fallingRate variable
      yB += fallRateInt;
      fallRate = fallRate + 0.4; // Each inetration the fall rate increase so that we can the effect of acceleration/ gravity
      fallRateInt = (int)fallRate;

      // Checks for collision
      if (yB >= 215 || yB <= 15)
      { // top and bottom
        gameOver();
      }
      if ((xP <= 62) && (xP >= 0) && (yB <= yP + 6))
      { // upper pillar
        gameOver();
      }
      if ((xP <= 62) && (xP >= 0) && (yB >= yP + 74))
      { // lower pillar
        gameOver();
      }

      // Draws the bird
      drawBird(yB);

      // After the pillar has passed through the screen
      if (xP <= -50)
      {
        xP = 300;               // Resets xP to 319
        yP = rand() % 100 + 20; // Random number for the pillars height
        score++;                // Increase score by one
      }

      // After each five points, increases the moving rate of the pillars
      if ((score - lastSpeedUpScore) == 5)
      {
        lastSpeedUpScore = score;
        movingRate++;
      }

      ////////////////loop end
    }

    //exercise 2 end
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void drawPilars(int x, int y)
{

  // Draws blue rectangle right of the pillar
  SPI_TFT_ILI9341_fillrect(&TFT, x + 50, 15, x + 60, y, Navy);
  // Draws the black frame of the pillar
  //SPI_TFT_ILI9341_rect(&TFT, x, 0, x+50, y, Black);
  // Draws the pillar
  SPI_TFT_ILI9341_fillrect(&TFT, x, 15, x + 50, y, Red);
  // Draws the blue rectangle left of the pillar
  SPI_TFT_ILI9341_fillrect(&TFT, x - 10, 15, x - 1, y, Navy);

  // The bottom pillar
  // Draws blue rectangle right of the pillar
  SPI_TFT_ILI9341_fillrect(&TFT, x + 50, y + 80, x + 60, 215, Navy);
  // Draws the black frame of the pillar
  //SPI_TFT_ILI9341_rect(&TFT, x, y + 80, x+50, 204, Black);
  // Draws the pillar
  SPI_TFT_ILI9341_fillrect(&TFT, x, y + 80, x + 50, 215, Red);
  // Draws the blue rectangle left of the pillar
  SPI_TFT_ILI9341_fillrect(&TFT, x - 3, y + 80, x - 1, 215, Navy);

  // Draws the score
  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial12x12);
  SPI_TFT_ILI9341_set_orientation(&TFT, 3);
  SPI_TFT_ILI9341_locate(&TFT, 200, 0);
  SPI_TFT_ILI9341_printf(&TFT, "score: %03d", score);
}

//====== drawBird() - Custom Function
void drawBird(int y)
{
  //SPI_TFT_ILI9341_circle(&TFT, 80, 150, 33, White);
  SPI_TFT_ILI9341_fillcircle(&TFT, 50, y, 6, Yellow);

  // Draws blue rectangles above and below the bird in order to clear its previus state
  SPI_TFT_ILI9341_fillrect(&TFT, 44, y - 18, 57, y - 6, Navy);
  SPI_TFT_ILI9341_fillrect(&TFT, 44, y + 6, 57, y + 18, Navy);
}

//======== gameOver() - Custom Function
void gameOver()
{
  HAL_Delay(1000); // 1 second
  // Clears the screen and prints the text
  SPI_TFT_ILI9341_cls(&TFT);

  SPI_TFT_ILI9341_set_background(&TFT, Navy);  // set background to black
  SPI_TFT_ILI9341_set_foreground(&TFT, White); // set chars to white

  SPI_TFT_ILI9341_set_orientation(&TFT, 3);
  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial24x23);
  SPI_TFT_ILI9341_locate(&TFT, 65, 100);
  SPI_TFT_ILI9341_printf(&TFT, "GAME OVER");

  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial12x12);
  SPI_TFT_ILI9341_locate(&TFT, 65, 130);
  SPI_TFT_ILI9341_printf(&TFT, "score: %03d", score);

  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial12x12);
  SPI_TFT_ILI9341_locate(&TFT, 65, 150);
  SPI_TFT_ILI9341_printf(&TFT, "Restarting...");
  HAL_Delay(2000);

  // Resets the variables to start position values
  xP = 300;
  yB = 70;
  fallRate = 0;
  score = 0;
  lastSpeedUpScore = 0;
  movingRate = 3;
  gameStarted = false;
  // Restart game
  initiateGame();
}

void initiateGame()
{

  SPI_TFT_ILI9341_cls(&TFT);
  SPI_TFT_ILI9341_set_background(&TFT, Navy);  // set background to black
  SPI_TFT_ILI9341_set_foreground(&TFT, White); // set chars to white

  SPI_TFT_ILI9341_set_orientation(&TFT, 0);
  SPI_TFT_ILI9341_fillrect(&TFT, 0, 0, SPI_TFT_ILI9341_width(&TFT), SPI_TFT_ILI9341_height(&TFT), Navy);
  SPI_TFT_ILI9341_fillrect(&TFT, 215, 0, SPI_TFT_ILI9341_width(&TFT), SPI_TFT_ILI9341_height(&TFT), Green);

  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial12x12);
  SPI_TFT_ILI9341_set_orientation(&TFT, 3);
  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial24x23);
  SPI_TFT_ILI9341_locate(&TFT, 75, 100);
  SPI_TFT_ILI9341_printf(&TFT, "Flappy Bird");
  SPI_TFT_ILI9341_set_font(&TFT, (unsigned char *)Arial12x12);
  SPI_TFT_ILI9341_locate(&TFT, 105, 130);
  SPI_TFT_ILI9341_printf(&TFT, "TAP TO START");

  drawBird(yB); // Draws the bird //draw rect

  // Wait until we tap the sreen
  while (!gameStarted)
  {

    if (Touch_ADS7843_OnTouch(&TP))
    {
      if (Touch_ADS7843_ReadADS7843(&TP, NULL))
      {
        gameStarted = true;

        // Clears the text "TAP TO START" before the game start
        SPI_TFT_ILI9341_set_orientation(&TFT, 3);
        SPI_TFT_ILI9341_fillrect(&TFT, 50, 50, 290, 150, Navy);
      }
    }
  }

  
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

#ifdef USE_FULL_ASSERT
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
