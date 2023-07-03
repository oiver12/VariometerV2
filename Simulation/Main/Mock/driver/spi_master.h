#ifndef __SPI_MASTER_H__
#define __SPI_MASTER_H__

#include <stdint.h>
#include <stdio.h>
#include "esp_err.h"

/**
 * @brief This is a configuration for a SPI slave device that is connected to one of the SPI buses.
 */
typedef struct
{
    uint8_t command_bits;     ///< Default amount of bits in command phase (0-16), used when ``SPI_TRANS_VARIABLE_CMD`` is not used, otherwise ignored.
    uint8_t address_bits;     ///< Default amount of bits in address phase (0-64), used when ``SPI_TRANS_VARIABLE_ADDR`` is not used, otherwise ignored.
    uint8_t dummy_bits;       ///< Amount of dummy bits to insert between address and data phase
    uint8_t mode;             /**< SPI mode, representing a pair of (CPOL, CPHA) configuration:
                                   - 0: (0, 0)
                                   - 1: (0, 1)
                                   - 2: (1, 0)
                                   - 3: (1, 1)
                               */
    uint16_t duty_cycle_pos;  ///< Duty cycle of positive clock, in 1/256th increments (128 = 50%/50% duty). Setting this to 0 (=not setting it) is equivalent to setting this to 128.
    uint16_t cs_ena_pretrans; ///< Amount of SPI bit-cycles the cs should be activated before the transmission (0-16). This only works on half-duplex transactions.
    uint8_t cs_ena_posttrans; ///< Amount of SPI bit-cycles the cs should stay active after the transmission (0-16)
    int clock_speed_hz;       ///< SPI clock speed in Hz. Derived from `clock_source`.
    int input_delay_ns;       /**< Maximum data valid time of slave. The time required between SCLK and MISO
  valid, including the possible clock delay from slave to master. The driver uses this value to give an extra
  delay before the MISO is ready on the line. Leave at 0 unless you know you need a delay. For better timing
  performance at high frequency (over 8MHz), it's suggest to have the right value.
  */
    int spics_io_num;         ///< CS GPIO pin for this device, or -1 if not used
    uint32_t flags;           ///< Bitwise OR of SPI_DEVICE_* flags
    int queue_size;           ///< Transaction queue size. This sets how many transactions can be 'in the air' (queued using spi_device_queue_trans but not yet finished using spi_device_get_trans_result) at the same time
} spi_device_interface_config_t;

typedef enum
{
    // SPI1 can be used as GPSPI only on ESP32
    SPI1_HOST = 0, ///< SPI1
    SPI2_HOST = 1, ///< SPI2
    SPI_HOST_MAX,  ///< invalid host value
} spi_host_device_t;

typedef struct spi_device_handle_t
{
    uint8_t dummy;
} spi_device_handle_t;

typedef struct spi_transaction_t
{
    uint32_t flags;  ///< Bitwise OR of SPI_TRANS_* flags
    uint16_t cmd;    /**< Command data, of which the length is set in the ``command_bits`` of spi_device_interface_config_t.
                      *
                      *  <b>NOTE: this field, used to be "command" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF 3.0.</b>
                      *
                      *  Example: write 0x0123 and command_bits=12 to send command 0x12, 0x3_ (in previous version, you may have to write 0x3_12).
                      */
    uint64_t addr;   /**< Address data, of which the length is set in the ``address_bits`` of spi_device_interface_config_t.
                      *
                      *  <b>NOTE: this field, used to be "address" in ESP-IDF 2.1 and before, is re-written to be used in a new way in ESP-IDF3.0.</b>
                      *
                      *  Example: write 0x123400 and address_bits=24 to send address of 0x12, 0x34, 0x00 (in previous version, you may have to write 0x12340000).
                      */
    size_t length;   ///< Total data length, in bits
    size_t rxlength; ///< Total data length received, should be not greater than ``length`` in full-duplex mode (0 defaults this to the value of ``length``).
    void *user;      ///< User-defined variable. Can be used to store eg transaction ID.
    union
    {
        const void *tx_buffer; ///< Pointer to transmit buffer, or NULL for no MOSI phase
        uint8_t tx_data[4];    ///< If SPI_TRANS_USE_TXDATA is set, data set here is sent directly from this variable.
    };
    union
    {
        void *rx_buffer;    ///< Pointer to receive buffer, or NULL for no MISO phase. Written by 4 bytes-unit if DMA is used.
        uint8_t rx_data[4]; ///< If SPI_TRANS_USE_RXDATA is set, data is received directly to this variable
    };
} spi_transaction_t; // the rx data should start from a 32-bit aligned address to get around dma issue.

typedef struct
{
    union
    {
        int mosi_io_num;  ///< GPIO pin for Master Out Slave In (=spi_d) signal, or -1 if not used.
        int data0_io_num; ///< GPIO pin for spi data0 signal in quad/octal mode, or -1 if not used.
    };
    union
    {
        int miso_io_num;  ///< GPIO pin for Master In Slave Out (=spi_q) signal, or -1 if not used.
        int data1_io_num; ///< GPIO pin for spi data1 signal in quad/octal mode, or -1 if not used.
    };
    int sclk_io_num; ///< GPIO pin for SPI Clock signal, or -1 if not used.
    union
    {
        int quadwp_io_num; ///< GPIO pin for WP (Write Protect) signal, or -1 if not used.
        int data2_io_num;  ///< GPIO pin for spi data2 signal in quad/octal mode, or -1 if not used.
    };
    union
    {
        int quadhd_io_num; ///< GPIO pin for HD (Hold) signal, or -1 if not used.
        int data3_io_num;  ///< GPIO pin for spi data3 signal in quad/octal mode, or -1 if not used.
    };
    int data4_io_num;    ///< GPIO pin for spi data4 signal in octal mode, or -1 if not used.
    int data5_io_num;    ///< GPIO pin for spi data5 signal in octal mode, or -1 if not used.
    int data6_io_num;    ///< GPIO pin for spi data6 signal in octal mode, or -1 if not used.
    int data7_io_num;    ///< GPIO pin for spi data7 signal in octal mode, or -1 if not used.
    int max_transfer_sz; ///< Maximum transfer size, in bytes. Defaults to 4092 if 0 when DMA enabled, or to `SOC_SPI_MAXIMUM_BUFFER_SIZE` if DMA is disabled.
    uint32_t flags;      ///< Abilities of bus to be checked by the driver. Or-ed value of ``SPICOMMON_BUSFLAG_*`` flags.
    int intr_flags;      /**< Interrupt flag for the bus to set the priority, and IRAM attribute, see
                          *  ``esp_intr_alloc.h``. Note that the EDGE, INTRDISABLED attribute are ignored
                          *  by the driver. Note that if ESP_INTR_FLAG_IRAM is set, ALL the callbacks of
                          *  the driver, and their callee functions, should be put in the IRAM.
                          */
} spi_bus_config_t;

typedef enum
{
    SPI_DMA_DISABLED = 0, ///< Do not enable DMA for SPI
    SPI_DMA_CH_AUTO = 3,  ///< Enable DMA, channel is automatically selected by driver
} spi_dma_chan_t;

esp_err_t spi_bus_add_device(spi_host_device_t host_id, const spi_device_interface_config_t *dev_config, spi_device_handle_t *handle);
esp_err_t spi_device_transmit(spi_device_handle_t handle, spi_transaction_t *trans_desc);
esp_err_t spi_bus_initialize(spi_host_device_t host_id, const spi_bus_config_t *bus_config, spi_dma_chan_t dma_chan);

#endif