#include "ButtonInput.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "Screen/Vario_Screen.h"
#include <esp_log.h>

#define DEBOUNCE_DELAY 50 // Debounce delay in milliseconds

#define Button_Enter 39 //Button 1
#define Button_Menu 38 //Button 2
#define Button_Up 9     //Button 3
#define Button_Down 48 //Button 4

#define ButtonEnter_Short (1<<0)
#define ButtonEnter_Long (1<<1)
#define ButtonMenu_Short (1<<2)
#define ButtonMenu_Long (1<<3)
#define ButtonUp_Short (1<<4)
#define ButtonUp_Long (1<<5)
#define ButtonDown_Short (1<<6)
#define ButtonDown_Long (1<<7)

#define TAG "ButtonInput"
#define LONG_PRESS_TIME_MS 500

typedef enum {
    BUTTON_STATE_IDLE,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_DEBOUNCING
} button_state_t;

//make a queue, which get messages from the interrupt
QueueHandle_t button_press_queue;

button_state_t button_state_enum = BUTTON_STATE_IDLE;

//EventGroup für kommunikation zwischen Interrupt und Task
EventGroupHandle_t button_press_EventGroup;

SemaphoreHandle_t button_press_Enter_semaphore;
SemaphoreHandle_t button_press_Menu_semaphore;
SemaphoreHandle_t button_press_Up_semaphore;
SemaphoreHandle_t button_press_Down_semaphore;

IRAM_ATTR static void buttonEnter_ISR(void* params)
{
    int button_state = gpio_get_level((gpio_num_t)Button_Enter);
    //gedrückt
    if(button_state == 0)
    {
        xTimerStartFromISR(button_press_Enter_semaphore, 0);
    }
    //released
    else
    {
        if(xTimerIsTimerActive(button_press_Enter_semaphore) == pdTRUE)
        {
            //short press
            xTimerStopFromISR(button_press_Enter_semaphore, 0);
            xEventGroupSetBitsFromISR(button_press_EventGroup, ButtonEnter_Short, 0);
        }
    }
}

void buttonEnter_TimerCallback(TimerHandle_t xTimer)
{
    //long Press
    xEventGroupSetBitsFromISR(button_press_EventGroup, ButtonEnter_Long, 0);
}

IRAM_ATTR static void buttonMenu_ISR(void* params)
{
    gpio_isr_handler_remove((gpio_num_t)Button_Menu);
    int button_state = gpio_get_level((gpio_num_t)Button_Menu);
    static char message[20];
    snprintf(message, 20,"B:%i", button_state);
    xQueueSendFromISR(button_press_queue, &message, 0);
    //gedrückt
    if(button_state == 0)
    {
        if (button_state_enum == BUTTON_STATE_IDLE) 
        {
            button_state_enum = BUTTON_STATE_DEBOUNCING;
            xTimerStartFromISR(button_press_Menu_semaphore, 0);
        }
    }
}

void buttonMenu_TimerCallback(TimerHandle_t xTimer)
{
    int button_state = gpio_get_level((gpio_num_t)Button_Menu);
    if(button_state == 0)
    {
        button_state_enum = BUTTON_STATE_PRESSED;
        xEventGroupSetBitsFromISR(button_press_EventGroup, ButtonMenu_Short, 0);
    }
    else
        button_state_enum = BUTTON_STATE_IDLE;

    gpio_isr_handler_add((gpio_num_t)Button_Menu, buttonMenu_ISR, NULL);
}

IRAM_ATTR static void buttonUP_ISR(void* params)
{
    int button_state = gpio_get_level((gpio_num_t)Button_Up);
    static char message[20];
    snprintf(message, 20,"B:%i", button_state);
    xQueueSendFromISR(button_press_queue, &message, 0);
    //gedrückt
    // if(button_state == 0)
    // {
    //     xTimerStartFromISR(button_press_Up_semaphore, 0);
    // }
    // //released
    // else
    // {
    //     if(xTimerIsTimerActive(button_press_Up_semaphore) == pdTRUE)
    //     {
    //         //short press
    //         xEventGroupSetBitsFromISR(button_press_EventGroup, ButtonUp_Short, 0);
    //         xTimerStopFromISR(button_press_Up_semaphore, 0);
    //     }
    // }
}

void buttonUP_TimerCallback(TimerHandle_t xTimer)
{
    static char message[20];
    snprintf(message, 20, "Timer");
    xQueueSendFromISR(button_press_queue, &message, 0);
    //long Press
    xEventGroupSetBitsFromISR(button_press_EventGroup, ButtonUp_Long, 0);
}

IRAM_ATTR static void buttonDown_ISR(void* params)
{
    int button_state = gpio_get_level((gpio_num_t)Button_Down);
    //gedrückt
    if(button_state == 0)
    {
        xTimerStartFromISR(button_press_Down_semaphore, 0);
    }
    //released
    else
    {
        if(xTimerIsTimerActive(button_press_Down_semaphore) == pdTRUE)
        {
            //short press
            xEventGroupSetBitsFromISR(button_press_EventGroup, ButtonDown_Short, 0);
            xTimerStopFromISR(button_press_Down_semaphore, 0);
        }
    }
}

void buttonDown_TimerCallback(TimerHandle_t xTimer)
{
    //long Press
    xEventGroupSetBitsFromISR(button_press_EventGroup, ButtonDown_Long, 0);
}

void ButtonInput_Task(void *params)
{
    while(1)
    {
        // char buffer;
        // xQueueReceive(button_press_queue, &buffer, portMAX_DELAY);
        // ESP_LOGI(TAG, "%s", &buffer);
        //warte bis irgendein Bit gesetzt wurde => bis irgend ein Button gedrückt wurde lang oder kurz. Nach dem auslesen werden die Bits automatsich gelöscht
        EventBits_t bits = xEventGroupWaitBits(button_press_EventGroup, ButtonEnter_Short | ButtonEnter_Long | ButtonMenu_Short | ButtonMenu_Long | ButtonUp_Short | ButtonUp_Long | ButtonDown_Short | ButtonDown_Long, pdTRUE, pdFALSE, portMAX_DELAY);
        // if(bits & ButtonEnter_Short)
        // {
        //     ESP_LOGI(TAG, "Button Enter Short Press");
        // }
        // if(bits & ButtonEnter_Long)
        // {
        //     ESP_LOGI(TAG, "Button Enter Long Press");
        // }
        if(bits & ButtonMenu_Short)
        {
            ESP_LOGI(TAG, "Button Menu Short Press");
        }
        if(bits & ButtonMenu_Long)
        {
            ESP_LOGI(TAG, "Button Menu Long Press");
        }
        // if(bits & ButtonUp_Short)
        // {
        //     ESP_LOGI(TAG, "Button Up Short Press");
        // }
        // if(bits & ButtonUp_Long)
        // {
        //     ESP_LOGI(TAG, "Button Up Long Press");
        // }
        // if(bits & ButtonDown_Short)
        // {
        //     ESP_LOGI(TAG, "Button Down Short Press");
        // }
        // if(bits & ButtonDown_Long)
        // {
        //     ESP_LOGI(TAG, "Button Down Long Press");
        // }
    }
}

void PrintTask(void *params)
{
    while(1)
    {
        char buffer;
        xQueueReceive(button_press_queue, &buffer, portMAX_DELAY);
        ESP_LOGI(TAG, "%s", &buffer);
    }
}

void ButtonInput_Init()
{
    //Set Button pins as input and enable interrupt
    gpio_config_t gpio_config_INT_Buttons = {
        .pin_bit_mask = (1ULL << Button_Enter) | (1ULL << Button_Menu) | (1ULL << Button_Up) | (1ULL << Button_Down),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&gpio_config_INT_Buttons);

    button_press_Enter_semaphore = xTimerCreate("Button_Enter", LONG_PRESS_TIME_MS / portTICK_PERIOD_MS, pdFALSE, (void*) 0, buttonEnter_TimerCallback);
    button_press_Menu_semaphore = xTimerCreate("Button_Menu", DEBOUNCE_DELAY / portTICK_PERIOD_MS, pdFALSE, (void*) 0, buttonMenu_TimerCallback);
    button_press_Up_semaphore = xTimerCreate("Button_Up", LONG_PRESS_TIME_MS / portTICK_PERIOD_MS, pdFALSE, (void*) 0, buttonUP_TimerCallback);
    button_press_Down_semaphore = xTimerCreate("Button_Down", LONG_PRESS_TIME_MS / portTICK_PERIOD_MS, pdFALSE, (void*) 0, buttonDown_TimerCallback);

    //Install ISR for Buttons
    gpio_isr_handler_add((gpio_num_t)Button_Enter, buttonEnter_ISR, NULL);
    gpio_isr_handler_add((gpio_num_t)Button_Menu, buttonMenu_ISR, NULL);
    gpio_isr_handler_add((gpio_num_t)Button_Up, buttonUP_ISR, NULL);
    gpio_isr_handler_add((gpio_num_t)Button_Down, buttonDown_ISR, NULL);

    button_press_queue = xQueueCreate(5, sizeof(char*));
    button_press_EventGroup = xEventGroupCreate();
    xEventGroupClearBits(button_press_EventGroup, ButtonEnter_Short | ButtonEnter_Long | ButtonMenu_Short | ButtonMenu_Long | ButtonUp_Short | ButtonUp_Long | ButtonDown_Short | ButtonDown_Long);
    xTaskCreate(ButtonInput_Task, "ButtonInput_Task", 1024 * 3, NULL, 5, NULL);
    xTaskCreate(PrintTask, "Print Task Button", 1024 * 3, NULL, 5, NULL);
}