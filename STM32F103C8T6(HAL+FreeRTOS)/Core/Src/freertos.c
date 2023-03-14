/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "semphr.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "string.h"
#include <stdio.h>
#include "usart1.h"
#include "dht11.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId LED_TASKHandle;
osThreadId KEY_TASKHandle;
osThreadId USART1_TASKHandle;
osSemaphoreId Usart1_Receive_BinSemaphoreHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);
void led_task(void const *argument);
void key_task(void const *argument);
void usart1_task(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* Create the semaphores(s) */
    /* definition and creation of Usart1_Receive_BinSemaphore */
    osSemaphoreDef(Usart1_Receive_BinSemaphore);
    Usart1_Receive_BinSemaphoreHandle = osSemaphoreCreate(osSemaphore(Usart1_Receive_BinSemaphore), 1);

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    xSemaphoreTake(Usart1_Receive_BinSemaphoreHandle, portMAX_DELAY);
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* definition and creation of defaultTask */
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    /* definition and creation of LED_TASK */
    osThreadDef(LED_TASK, led_task, osPriorityLow, 0, 128);
    LED_TASKHandle = osThreadCreate(osThread(LED_TASK), NULL);

    /* definition and creation of KEY_TASK */
    osThreadDef(KEY_TASK, key_task, osPriorityLow, 0, 128);
    KEY_TASKHandle = osThreadCreate(osThread(KEY_TASK), NULL);

    /* definition and creation of USART1_TASK */
    osThreadDef(USART1_TASK, usart1_task, osPriorityLow, 0, 256);
    USART1_TASKHandle = osThreadCreate(osThread(USART1_TASK), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument)
{
    /* USER CODE BEGIN StartDefaultTask */
    if (dht11_init() != 0)
    {
        DEBUG_PRINT("dht11_init() failed!!\r\n");
    }
    vTaskDelete(NULL);
    /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_led_task */
/**
 * @brief Function implementing the LED_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_led_task */
void led_task(void const *argument)
{
    /* USER CODE BEGIN led_task */
    /* Infinite loop */
    for (;;)
    {
#define TEST_DHT11
#ifdef TEST_DHT11
        if(g_dht11_device_st.get_dat11_data(&g_dht11_device_st.dht11_data_temprature,&g_dht11_device_st.dht11_data_humidity) == 0)
        {
            INFO_PRINT("wd:%d, sd:%d\r\n", g_dht11_device_st.dht11_data_temprature, g_dht11_device_st.dht11_data_humidity);
        }
        else
        {
            ERROR_PRINT("get dht11 data error\r\n");
        }
#endif /* TEST_MODE */
        LED0_TOGGLE;
        delay_ms(500);
    }
    /* USER CODE END led_task */
}

/* USER CODE BEGIN Header_key_task */
/**
 * @brief Function implementing the KEY_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_key_task */
void key_task(void const *argument)
{
    /* USER CODE BEGIN key_task */
    /* Infinite loop */
    for (;;)
    {

        osDelay(1);
    }
    /* USER CODE END key_task */
}

/* USER CODE BEGIN Header_usart1_task */
/**
 * @brief Function implementing the USART1_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_usart1_task */
void usart1_task(void const *argument)
{
    /* USER CODE BEGIN usart1_task */
    uint8_t len;
    /* Infinite loop */
    for (;;)
    {
        xSemaphoreTake(Usart1_Receive_BinSemaphoreHandle, portMAX_DELAY);
        len = USART_RX_STA & 0x3fff; // get lenth of the data
        printf("data:%s len:%d\r\n", USART_RX_BUF, len);
        memset(USART_RX_BUF, '\0', sizeof(USART_RX_BUF)); // initialization buf
        USART_RX_STA = 0;
        // osDelay(1);
        delay_ms(1);
    }
    /* USER CODE END usart1_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
