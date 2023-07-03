#ifndef __I2C_H__
#define __I2C_H__

#include <gpio.h>

typedef enum
{
    I2C_NUM_0 = 0, /*!< I2C port 0 */
    I2C_NUM_1,     /*!< I2C port 1 */
    I2C_NUM_MAX,   /*!< I2C port max */
} i2c_port_t;

#endif