#include "MAXM10S.h"
#include "driver/i2c.h"
#include <esp_log.h>

static esp_err_t i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPS_I2C_SDA;
    conf.scl_io_num = GPS_i2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    conf.clk_flags = 0;

    i2c_param_config(I2C_NUM_1, &conf);
    return i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);
}

void maxm10s_init()
{
    // i2c init & scan
    if (i2c_master_init() != ESP_OK)
        ESP_LOGE("GPS", "i2c init failed\n");

    printf("i2c scan: \n");
    for (uint8_t i = 1; i < 127; i++)
    {
        int ret;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK)
        {
            printf("Found device at: 0x%2x\n", i);
        }
    }
}