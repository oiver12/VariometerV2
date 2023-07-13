#ifndef __GPIO_H__
#define __GPIO_H__

#include "esp_err.h"
#include "stdint.h"

#define ESP_INTR_FLAG_LEVEL1 (1 << 1)        ///< Accept a Level 1 interrupt vector (lowest priority)
#define ESP_INTR_FLAG_LEVEL2 (1 << 2)        ///< Accept a Level 2 interrupt vector
#define ESP_INTR_FLAG_LEVEL3 (1 << 3)        ///< Accept a Level 3 interrupt vector
#define ESP_INTR_FLAG_LEVEL4 (1 << 4)        ///< Accept a Level 4 interrupt vector
#define ESP_INTR_FLAG_LEVEL5 (1 << 5)        ///< Accept a Level 5 interrupt vector
#define ESP_INTR_FLAG_LEVEL6 (1 << 6)        ///< Accept a Level 6 interrupt vector
#define ESP_INTR_FLAG_NMI (1 << 7)           ///< Accept a Level 7 interrupt vector (highest priority)
#define ESP_INTR_FLAG_SHARED (1 << 8)        ///< Interrupt can be shared between ISRs
#define ESP_INTR_FLAG_EDGE (1 << 9)          ///< Edge-triggered interrupt
#define ESP_INTR_FLAG_IRAM (1 << 10)         ///< ISR can be called if cache is disabled
#define ESP_INTR_FLAG_INTRDISABLED (1 << 11) ///< Return with this interrupt disabled

typedef enum
{
    GPIO_NUM_NC = -1, /*!< Use to signal not connected to S/W */
    GPIO_NUM_0 = 0,   /*!< GPIO0, input and output */
    GPIO_NUM_1 = 1,   /*!< GPIO1, input and output */
    GPIO_NUM_2 = 2,   /*!< GPIO2, input and output */
    GPIO_NUM_3 = 3,   /*!< GPIO3, input and output */
    GPIO_NUM_4 = 4,   /*!< GPIO4, input and output */
    GPIO_NUM_5 = 5,   /*!< GPIO5, input and output */
    GPIO_NUM_6 = 6,   /*!< GPIO6, input and output */
    GPIO_NUM_7 = 7,   /*!< GPIO7, input and output */
    GPIO_NUM_8 = 8,   /*!< GPIO8, input and output */
    GPIO_NUM_9 = 9,   /*!< GPIO9, input and output */
    GPIO_NUM_10 = 10, /*!< GPIO10, input and output */
    GPIO_NUM_11 = 11, /*!< GPIO11, input and output */
    GPIO_NUM_12 = 12, /*!< GPIO12, input and output */
    GPIO_NUM_13 = 13, /*!< GPIO13, input and output */
    GPIO_NUM_14 = 14, /*!< GPIO14, input and output */
    GPIO_NUM_15 = 15, /*!< GPIO15, input and output */
    GPIO_NUM_16 = 16, /*!< GPIO16, input and output */
    GPIO_NUM_17 = 17, /*!< GPIO17, input and output */
    GPIO_NUM_18 = 18, /*!< GPIO18, input and output */
    GPIO_NUM_19 = 19, /*!< GPIO19, input and output */
    GPIO_NUM_20 = 20, /*!< GPIO20, input and output */
    GPIO_NUM_21 = 21, /*!< GPIO21, input and output */
    GPIO_NUM_26 = 26, /*!< GPIO26, input and output */
    GPIO_NUM_27 = 27, /*!< GPIO27, input and output */
    GPIO_NUM_28 = 28, /*!< GPIO28, input and output */
    GPIO_NUM_29 = 29, /*!< GPIO29, input and output */
    GPIO_NUM_30 = 30, /*!< GPIO30, input and output */
    GPIO_NUM_31 = 31, /*!< GPIO31, input and output */
    GPIO_NUM_32 = 32, /*!< GPIO32, input and output */
    GPIO_NUM_33 = 33, /*!< GPIO33, input and output */
    GPIO_NUM_34 = 34, /*!< GPIO34, input and output */
    GPIO_NUM_35 = 35, /*!< GPIO35, input and output */
    GPIO_NUM_36 = 36, /*!< GPIO36, input and output */
    GPIO_NUM_37 = 37, /*!< GPIO37, input and output */
    GPIO_NUM_38 = 38, /*!< GPIO38, input and output */
    GPIO_NUM_39 = 39, /*!< GPIO39, input and output */
    GPIO_NUM_40 = 40, /*!< GPIO40, input and output */
    GPIO_NUM_41 = 41, /*!< GPIO41, input and output */
    GPIO_NUM_42 = 42, /*!< GPIO42, input and output */
    GPIO_NUM_43 = 43, /*!< GPIO43, input and output */
    GPIO_NUM_44 = 44, /*!< GPIO44, input and output */
    GPIO_NUM_45 = 45, /*!< GPIO45, input and output */
    GPIO_NUM_46 = 46, /*!< GPIO46, input and output */
    GPIO_NUM_47 = 47, /*!< GPIO47, input and output */
    GPIO_NUM_48 = 48, /*!< GPIO48, input and output */
    GPIO_NUM_MAX,
    /** @endcond */
} gpio_num_t;

#define BIT2 0x00000004
#define BIT1 0x00000002
#define BIT0 0x00000001

#define GPIO_MODE_DEF_DISABLE (0)
#define GPIO_MODE_DEF_INPUT (BIT0)  ///< bit mask for input
#define GPIO_MODE_DEF_OUTPUT (BIT1) ///< bit mask for output
#define GPIO_MODE_DEF_OD (BIT2)     ///< bit mask for OD mode

#define IRAM_ATTR

typedef enum
{
    GPIO_MODE_DISABLE = GPIO_MODE_DEF_DISABLE,                                                         /*!< GPIO mode : disable input and output             */
    GPIO_MODE_INPUT = GPIO_MODE_DEF_INPUT,                                                             /*!< GPIO mode : input only                           */
    GPIO_MODE_OUTPUT = GPIO_MODE_DEF_OUTPUT,                                                           /*!< GPIO mode : output only mode                     */
    GPIO_MODE_OUTPUT_OD = ((GPIO_MODE_DEF_OUTPUT) | (GPIO_MODE_DEF_OD)),                               /*!< GPIO mode : output only with open-drain mode     */
    GPIO_MODE_INPUT_OUTPUT_OD = ((GPIO_MODE_DEF_INPUT) | (GPIO_MODE_DEF_OUTPUT) | (GPIO_MODE_DEF_OD)), /*!< GPIO mode : output and input with open-drain mode*/
    GPIO_MODE_INPUT_OUTPUT = ((GPIO_MODE_DEF_INPUT) | (GPIO_MODE_DEF_OUTPUT)),                         /*!< GPIO mode : output and input mode                */
} gpio_mode_t;

typedef enum
{
    GPIO_PULLUP_DISABLE = 0x0, /*!< Disable GPIO pull-up resistor */
    GPIO_PULLUP_ENABLE = 0x1,  /*!< Enable GPIO pull-up resistor */
} gpio_pullup_t;

typedef enum
{
    GPIO_PULLDOWN_DISABLE = 0x0, /*!< Disable GPIO pull-down resistor */
    GPIO_PULLDOWN_ENABLE = 0x1,  /*!< Enable GPIO pull-down resistor  */
} gpio_pulldown_t;

typedef enum
{
    GPIO_PULLUP_ONLY,     /*!< Pad pull up            */
    GPIO_PULLDOWN_ONLY,   /*!< Pad pull down          */
    GPIO_PULLUP_PULLDOWN, /*!< Pad pull up + pull down*/
    GPIO_FLOATING,        /*!< Pad floating           */
} gpio_pull_mode_t;

typedef enum
{
    GPIO_INTR_DISABLE = 0,    /*!< Disable GPIO interrupt                             */
    GPIO_INTR_POSEDGE = 1,    /*!< GPIO interrupt type : rising edge                  */
    GPIO_INTR_NEGEDGE = 2,    /*!< GPIO interrupt type : falling edge                 */
    GPIO_INTR_ANYEDGE = 3,    /*!< GPIO interrupt type : both rising and falling edge */
    GPIO_INTR_LOW_LEVEL = 4,  /*!< GPIO interrupt type : input low level trigger      */
    GPIO_INTR_HIGH_LEVEL = 5, /*!< GPIO interrupt type : input high level trigger     */
    GPIO_INTR_MAX,
} gpio_int_type_t;

typedef struct
{
    uint64_t pin_bit_mask;        /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    gpio_mode_t mode;             /*!< GPIO mode: set input/output mode                     */
    gpio_pullup_t pull_up_en;     /*!< GPIO pull-up                                         */
    gpio_pulldown_t pull_down_en; /*!< GPIO pull-down                                       */
    gpio_int_type_t intr_type;    /*!< GPIO interrupt type                                  */
} gpio_config_t;

typedef void (*gpio_isr_t)(void *arg);

esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level);
int gpio_get_level(gpio_num_t gpio_num);
esp_err_t gpio_config(const gpio_config_t *pGPIOConfig);
esp_err_t gpio_install_isr_service(int intr_alloc_flags);
esp_err_t gpio_isr_handler_add(gpio_num_t gpio_num, gpio_isr_t isr_handler, void *args);
esp_err_t gpio_isr_handler_remove(gpio_num_t gpio_num);

#endif