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
#include "oled.h"
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityRealtime,
};
/* Definitions for LED_TASK */
osThreadId_t LED_TASKHandle;
const osThreadAttr_t LED_TASK_attributes = {
    .name = "LED_TASK",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for USART1_TASK */
osThreadId_t USART1_TASKHandle;
const osThreadAttr_t USART1_TASK_attributes = {
    .name = "USART1_TASK",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for SCREEN */
osThreadId_t SCREENHandle;
const osThreadAttr_t SCREEN_attributes = {
    .name = "SCREEN",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for Usart1_Receive_BinSemaphore */
osSemaphoreId_t Usart1_Receive_BinSemaphoreHandle;
const osSemaphoreAttr_t Usart1_Receive_BinSemaphore_attributes = {
    .name = "Usart1_Receive_BinSemaphore"};
/* Definitions for en_refresh_screen */
osSemaphoreId_t en_refresh_screenHandle;
const osSemaphoreAttr_t en_refresh_screen_attributes = {
    .name = "en_refresh_screen"};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void led_task(void *argument);
void usart1_task(void *argument);
void thread_screen(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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
    /* creation of Usart1_Receive_BinSemaphore */
    Usart1_Receive_BinSemaphoreHandle = osSemaphoreNew(1, 1, &Usart1_Receive_BinSemaphore_attributes);

    /* creation of en_refresh_screen */
    en_refresh_screenHandle = osSemaphoreNew(1, 1, &en_refresh_screen_attributes);

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
    /* creation of defaultTask */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    /* creation of LED_TASK */
    LED_TASKHandle = osThreadNew(led_task, NULL, &LED_TASK_attributes);

    /* creation of USART1_TASK */
    USART1_TASKHandle = osThreadNew(usart1_task, NULL, &USART1_TASK_attributes);

    /* creation of SCREEN */
    SCREENHandle = osThreadNew(thread_screen, NULL, &SCREEN_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    /* USER CODE BEGIN StartDefaultTask */
    osStatus ret;
    INFO_PRINT("init oled\r\n");
    OLED_Init();
    g_oled_device_st.oled_show_string(0, 0, "dht11 init...", 16);
    if (dht11_init() != 0)
    {
        DEBUG_PRINT("dht11_init() failed!!\r\n");
        g_oled_device_st.oled_show_string(0, 0, "dht11 init failed!", 16);
        delay_xms(1000);
    }

    g_oled_device_st.oled_show_string(0, 0, "system init success", 16);
    delay_xms(1000);
    g_oled_device_st.oled_clear_screen();

    g_oled_device_st.oled_refresh_type_em = SCREEN_REFRESH_MAIN_PAGE;
    ret = osSemaphoreRelease(en_refresh_screenHandle);
    if (ret != osOK)
    {
        ERROR_PRINT("give semaphore failure![%d]\r\n", ret);
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
void led_task(void *argument)
{
    /* USER CODE BEGIN led_task */
    BaseType_t ret = pdFALSE;
    /* Infinite loop */
    for (;;)
    {
#define TEST_DHT11
#ifdef TEST_DHT11
        if (g_dht11_device_st.get_dat11_data(&g_dht11_device_st.dht11_data_temprature, &g_dht11_device_st.dht11_data_humidity) == 0)
        {
            INFO_PRINT("wd:%d, sd:%d\r\n", g_dht11_device_st.dht11_data_temprature, g_dht11_device_st.dht11_data_humidity);
        }
        else
        {
            ERROR_PRINT("get dht11 data error\r\n");
        }
#endif /* TEST_MODE */
        LED0_TOGGLE;
        g_oled_device_st.oled_refresh_type_em = SCREEN_REFRESH_MAIN_PAGE_DATA;
        ret = osSemaphoreRelease(en_refresh_screenHandle);
        if (ret != osOK)
        {
            ERROR_PRINT("give semaphore failure![%d]\r\n", ret);
        }
        delay_ms(500);
    }
    /* USER CODE END led_task */
}

/* USER CODE BEGIN Header_usart1_task */
/**
 * @brief Function implementing the USART1_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_usart1_task */
void usart1_task(void *argument)
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

/* USER CODE BEGIN Header_thread_screen */
/**
 * @brief Function implementing the SCREEN thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_thread_screen */
void thread_screen(void *argument)
{
    /* USER CODE BEGIN thread_screen */
    osStatus ret;
    /* Infinite loop */
    for (;;)
    {
        ret = osSemaphoreAcquire(en_refresh_screenHandle, osWaitForever);
        if (ret != osOK)
        {
            ERROR_PRINT("get semaphore failure![%d]\r\n", ret);
        }
        /* 错误判断避免出现空指针!! */
        if (g_oled_device_st.oled_clear_screen != NULL ||
            g_oled_device_st.updata_main_page != NULL ||
            g_oled_device_st.updata_main_page_data != NULL)
        {
            switch (g_oled_device_st.oled_refresh_type_em)
            {
            case SCREEN_NONE:
                break;
            case SCREEN_CLREAR:
                g_oled_device_st.oled_clear_screen();
                break;
            case SCREEN_REFRESH_MAIN_PAGE:
                g_oled_device_st.oled_clear_screen();
                g_oled_device_st.updata_main_page(&g_oled_device_st);
                break;
            case SCREEN_REFRESH_MAIN_PAGE_DATA:
                g_oled_device_st.updata_main_page_data(&g_oled_device_st);
                break;
            default:
                ERROR_PRINT("unknown scrrn type!!\r\n");
                break;
            }
        }
        else
        {
            ERROR_PRINT("oled_XXX() is null!!\r\n");
        }
        g_oled_device_st.oled_refresh_type_em = SCREEN_NONE;
        INFO_PRINT("oled refresh\r\n");
        osDelay(1);
    }
    /* USER CODE END thread_screen */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
