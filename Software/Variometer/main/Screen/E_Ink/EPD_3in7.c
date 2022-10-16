/*****************************************************************************
* | File      	:   EPD_3IN7.C
* | Author      :   Waveshare team
* | Function    :   3.7inch e-paper
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2020-07-17
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "EPD_3in7.h"
#include "driver/spi_master.h"

#define SPIBUS_READ     (0x80)  /*!< addr | SPIBUS_READ  */
#define SPIBUS_WRITE    (0x7F)  /*!< addr & SPIBUS_WRITE */

spi_device_handle_t screen_spi_handle;

static const UBYTE lut_4Gray_GC[] =
{
0x2A,0x06,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//1
0x28,0x06,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//2
0x20,0x06,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3
0x14,0x06,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//4
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//5
0x00,0x02,0x02,0x0A,0x00,0x00,0x00,0x08,0x08,0x02,//6
0x00,0x02,0x02,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,//7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//8
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//9
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//10
0x22,0x22,0x22,0x22,0x22
};	

static const UBYTE lut_1Gray_GC[] =
{
0x2A,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//1
0x05,0x2A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//2
0x2A,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3
0x05,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//4
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//5
0x00,0x02,0x03,0x0A,0x00,0x02,0x06,0x0A,0x05,0x00,//6
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//8
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//9
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//10
0x22,0x22,0x22,0x22,0x22
};  

static const UBYTE lut_1Gray_DU[] =
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//1
0x01,0x2A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x0A,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//5
0x00,0x00,0x05,0x05,0x00,0x05,0x03,0x05,0x05,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//9
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x22,0x22,0x22,0x22,0x22
}; 

static const UBYTE lut_1Gray_A2[] =
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //1
0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //2
0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //3
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //4
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //5
0x00,0x00,0x03,0x05,0x00,0x00,0x00,0x00,0x00,0x00, //6
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //7
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //8
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //9
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //10
0x22,0x22,0x22,0x22,0x22
}; 

void EPD_3IN7_SPI_Init()
{
    spi_device_interface_config_t dev_config;
    dev_config.command_bits = 0;
    dev_config.address_bits = 0;
    dev_config.dummy_bits = 0;
    dev_config.mode = 0;
    dev_config.duty_cycle_pos = 128;  // default 128 = 50%/50% duty
    dev_config.cs_ena_pretrans = 0;  // 0 not used
    dev_config.cs_ena_posttrans = 0;  // 0 not used
    dev_config.clock_speed_hz = 20 * 1000 * 1000;
    dev_config.spics_io_num = EPD_CS_PIN;
    dev_config.flags = 0;  // 0 not used
    dev_config.queue_size = 1;
    dev_config.pre_cb = NULL;
    dev_config.post_cb = NULL;
    volatile esp_err_t err = spi_bus_add_device(SPI2_HOST, &dev_config, &screen_spi_handle);
}

void DEV_SPI_WriteByte(UBYTE value)
{
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.length = 8;
    transaction.rxlength = 0;
    transaction.user = NULL;
    transaction.tx_buffer = &value;
    transaction.rx_buffer = NULL;
    ESP_ERROR_CHECK(spi_device_transmit(screen_spi_handle, &transaction));
}

void DEV_SPI_WriteBytes(const UBYTE *value, size_t length)
{
    spi_transaction_t transaction;
    transaction.flags = 0;
    transaction.cmd = 0;
    transaction.length = length * 8;
    transaction.rxlength = 0;
    transaction.user = NULL;
    transaction.tx_buffer = value;
    transaction.rx_buffer = NULL;
    ESP_ERROR_CHECK(spi_device_transmit(screen_spi_handle, &transaction));
}

int DEV_Module_Init(void)
{
    gpio_config_t io_conf_output = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << EPD_DC_PIN) | (1ULL << EPD_RST_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf_output);
    gpio_config_t io_conf_input = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << EPD_BUSY_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf_input);
    DEV_Digital_Write(EPD_DC_PIN, 0);
    //DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    EPD_3IN7_SPI_Init();
	return 0;
}

void DEV_Module_Exit(void)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    //DEV_Digital_Write(EPD_CS_PIN, 0);
    //close 5V
    DEV_Digital_Write(EPD_RST_PIN, 0);
}

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_3IN7_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(300);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(10);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(300);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_3IN7_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    //DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    //DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_3IN7_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    //DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    //DEV_Digital_Write(EPD_CS_PIN, 1);
}

static void EPD_3IN7_SendDataStream(const UBYTE *Data, size_t size)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    int blockSize = 4090;
    for (size_t i = 0; i < size; i += blockSize)
    {
        if (i + blockSize > size)
        {
            //printf("Send Data lastd: %i, %i \n", i, size);
            DEV_SPI_WriteBytes(&Data[i], size - i);
        }
        else
        {
            //printf("Send Data: %i \n", i);
            DEV_SPI_WriteBytes(&Data[i], blockSize);
        }
    }
}

static void EPD_3IN7_ReadBusy_HIGH(void)
{
    printf("e-Paper busy\r\n");
    UBYTE busy;
    do {
        busy = DEV_Digital_Read(EPD_BUSY_PIN);
        DEV_Delay_ms(10);
    } while(busy);
    //DEV_Delay_ms(100);
    printf("e-Paper busy release\r\n");
}

/******************************************************************************
function :	set the look-up tables
parameter:
******************************************************************************/
void EPD_3IN7_Load_LUT(UBYTE lut)
{
    EPD_3IN7_SendCommand(0x32);
    if(lut == 0)
        EPD_3IN7_SendDataStream(lut_4Gray_GC, 105);
    else if(lut == 1)
        EPD_3IN7_SendDataStream(lut_1Gray_GC, 105);
    else if(lut == 2)
        EPD_3IN7_SendDataStream(lut_1Gray_DU, 105);
    else if(lut == 3)
        EPD_3IN7_SendDataStream(lut_1Gray_A2, 105);  
    else
        printf("There is no such lut \r\n");
//  UWORD i;
//   for (i = 0; i < 105; i++)
//   {
//     if(lut == 0)
//         EPD_3IN7_SendData(lut_4Gray_GC[i]);
//     else if(lut == 1)
//         EPD_3IN7_SendData(lut_1Gray_GC[i]);
//     else if(lut == 2)
//         EPD_3IN7_SendData(lut_1Gray_DU[i]);
//     else if(lut == 3)
//         EPD_3IN7_SendData(lut_1Gray_A2[i]);  
//     else
//         printf("There is no such lut \r\n");
//   }
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_3IN7_4Gray_Init(void)
{
    EPD_3IN7_Reset();
    
    EPD_3IN7_SendCommand(0x12);
    DEV_Delay_ms(300);
    
    EPD_3IN7_SendCommand(0x46); 
    EPD_3IN7_SendData(0xF7);
    EPD_3IN7_ReadBusy_HIGH();
    EPD_3IN7_SendCommand(0x47);
    EPD_3IN7_SendData(0xF7);
    EPD_3IN7_ReadBusy_HIGH(); 
    
    EPD_3IN7_SendCommand(0x01); // setting gaet number
    EPD_3IN7_SendData(0xDF);
    EPD_3IN7_SendData(0x01);
    EPD_3IN7_SendData(0x00);

    EPD_3IN7_SendCommand(0x03); // set gate voltage
    EPD_3IN7_SendData(0x00);

    EPD_3IN7_SendCommand(0x04); // set source voltage
    EPD_3IN7_SendData(0x41);
    EPD_3IN7_SendData(0xA8);
    EPD_3IN7_SendData(0x32);

    EPD_3IN7_SendCommand(0x11); // set data entry sequence
    EPD_3IN7_SendData(0x03);

    EPD_3IN7_SendCommand(0x3C); // set border 
    EPD_3IN7_SendData(0x00);

    EPD_3IN7_SendCommand(0x0C); // set booster strength
    EPD_3IN7_SendData(0xAE);
    EPD_3IN7_SendData(0xC7);
    EPD_3IN7_SendData(0xC3);
    EPD_3IN7_SendData(0xC0);
    EPD_3IN7_SendData(0xC0);  

    EPD_3IN7_SendCommand(0x18); // set internal sensor on
    EPD_3IN7_SendData(0x80);
     
    EPD_3IN7_SendCommand(0x2C); // set vcom value
    EPD_3IN7_SendData(0x44);
    
    EPD_3IN7_SendCommand(0x37); // set display option, these setting turn on previous function
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);  
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);  

    EPD_3IN7_SendCommand(0x44); // setting X direction start/end position of RAM
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x17);
    EPD_3IN7_SendData(0x01);

    EPD_3IN7_SendCommand(0x45); // setting Y direction start/end position of RAM
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0xDF);
    EPD_3IN7_SendData(0x01);

    EPD_3IN7_SendCommand(0x22); // Display Update Control 2
    EPD_3IN7_SendData(0xCF);
}

/******************************************************************************
function :  Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_3IN7_1Gray_Init(void)
{
    EPD_3IN7_Reset();
    
    EPD_3IN7_SendCommand(0x12);
    DEV_Delay_ms(300);
    
    EPD_3IN7_SendCommand(0x46); 
    EPD_3IN7_SendData(0xF7);
    EPD_3IN7_ReadBusy_HIGH();
    EPD_3IN7_SendCommand(0x47);
    EPD_3IN7_SendData(0xF7);
    EPD_3IN7_ReadBusy_HIGH(); 
    
    EPD_3IN7_SendCommand(0x01); // setting gaet number
    EPD_3IN7_SendData(0xDF);
    EPD_3IN7_SendData(0x01);
    EPD_3IN7_SendData(0x00);

    EPD_3IN7_SendCommand(0x03); // set gate voltage
    EPD_3IN7_SendData(0x00);

    EPD_3IN7_SendCommand(0x04); // set source voltage
    EPD_3IN7_SendData(0x41);
    EPD_3IN7_SendData(0xA8);
    EPD_3IN7_SendData(0x32);

    EPD_3IN7_SendCommand(0x11); // set data entry sequence
    EPD_3IN7_SendData(0x03);

    EPD_3IN7_SendCommand(0x3C); // set border 
    EPD_3IN7_SendData(0x00);

    EPD_3IN7_SendCommand(0x0C); // set booster strength
    EPD_3IN7_SendData(0xAE);
    EPD_3IN7_SendData(0xC7);
    EPD_3IN7_SendData(0xC3);
    EPD_3IN7_SendData(0xC0);
    EPD_3IN7_SendData(0xC0);  

    EPD_3IN7_SendCommand(0x18); // set internal sensor on
    EPD_3IN7_SendData(0x80);
     
    EPD_3IN7_SendCommand(0x2C); // set vcom value
    EPD_3IN7_SendData(0x44);
    
    EPD_3IN7_SendCommand(0x37); // set display option, these setting turn on previous function
    EPD_3IN7_SendData(0x00);     //can switch 1 gray or 4 gray
    EPD_3IN7_SendData(0xFF);
    EPD_3IN7_SendData(0xFF);
    EPD_3IN7_SendData(0xFF);
    EPD_3IN7_SendData(0xFF);  
    EPD_3IN7_SendData(0x4F);
    EPD_3IN7_SendData(0xFF);
    EPD_3IN7_SendData(0xFF);
    EPD_3IN7_SendData(0xFF);
    EPD_3IN7_SendData(0xFF);  

    EPD_3IN7_SendCommand(0x44); // setting X direction start/end position of RAM
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x17);
    EPD_3IN7_SendData(0x01);

    EPD_3IN7_SendCommand(0x45); // setting Y direction start/end position of RAM
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0xDF);
    EPD_3IN7_SendData(0x01);

    EPD_3IN7_SendCommand(0x22); // Display Update Control 2
    EPD_3IN7_SendData(0xCF);
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_3IN7_4Gray_Clear(void)
{
    uint64_t startTime = esp_timer_get_time();
    UWORD Width, Height;
    Width = (EPD_3IN7_WIDTH % 8 == 0)? (EPD_3IN7_WIDTH / 8 ): (EPD_3IN7_WIDTH / 8 + 1);
    Height = EPD_3IN7_HEIGHT;

    EPD_3IN7_SendCommand(0x49);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendCommand(0x4E);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendCommand(0x4F);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    
    EPD_3IN7_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
       for (UWORD i = 0; i < Width; i++) {
           EPD_3IN7_SendData(0xff);
       }
    }
    
    EPD_3IN7_SendCommand(0x4E);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
     
    EPD_3IN7_SendCommand(0x4F);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    
    EPD_3IN7_SendCommand(0x26);
    for (UWORD j = 0; j < Height; j++) {
       for (UWORD i = 0; i < Width; i++) {
           EPD_3IN7_SendData(0xff);
       }
    }
      
    EPD_3IN7_Load_LUT(0);
    EPD_3IN7_SendCommand(0x22);
    EPD_3IN7_SendData(0xC7);

    EPD_3IN7_SendCommand(0x20);
    EPD_3IN7_ReadBusy_HIGH();     
    printf("Update Time: %llu \n\r", (esp_timer_get_time() - startTime) / 1000);
}

/******************************************************************************
function :  Clear screen
parameter:
******************************************************************************/
void EPD_3IN7_1Gray_Clear(void)
{
    UWORD i;
    UWORD IMAGE_COUNTER = EPD_3IN7_WIDTH * EPD_3IN7_HEIGHT / 8;

    EPD_3IN7_SendCommand(0x4E);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendCommand(0x4F);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);

    EPD_3IN7_SendCommand(0x24);
    for (i = 0; i < IMAGE_COUNTER; i++)
    {
        EPD_3IN7_SendData(0xff);
    }
    
    EPD_3IN7_Load_LUT(2);
    
    EPD_3IN7_SendCommand(0x20);
    EPD_3IN7_ReadBusy_HIGH();    
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_3IN7_4Gray_Display(const UBYTE *Image)
{
    UDOUBLE i,j,k;
    UBYTE temp1,temp2,temp3;
    
    EPD_3IN7_SendCommand(0x49);
    EPD_3IN7_SendData(0x00);

    
    EPD_3IN7_SendCommand(0x4E);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    
    
    EPD_3IN7_SendCommand(0x4F);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    
    EPD_3IN7_SendCommand(0x24);
    for(i=0;i<16800;i++){
        temp3=0;
        for(j=0; j<2; j++) {
            temp1 = Image[i*2+j];
            for(k=0; k<2; k++) {
                temp2 = temp1&0xC0;
                if(temp2 == 0xC0)
                    temp3 |= 0x01;//white
                else if(temp2 == 0x00)
                    temp3 |= 0x00;  //black
                else if(temp2 == 0x80)
                    temp3 |= 0x00;  //gray1
                else //0x40
                    temp3 |= 0x01; //gray2
                temp3 <<= 1;

                temp1 <<= 2;
                temp2 = temp1&0xC0 ;
                if(temp2 == 0xC0)  //white
                    temp3 |= 0x01;
                else if(temp2 == 0x00) //black
                    temp3 |= 0x00;
                else if(temp2 == 0x80)
                    temp3 |= 0x00; //gray1
                else    //0x40
                    temp3 |= 0x01;	//gray2
                if(j!=1 || k!=1)
                    temp3 <<= 1;

                temp1 <<= 2;
            }

        }
        EPD_3IN7_SendData(temp3);
    }
    // new  data
    EPD_3IN7_SendCommand(0x4E);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
     
    
    EPD_3IN7_SendCommand(0x4F);
    EPD_3IN7_SendData(0x00);
    EPD_3IN7_SendData(0x00);
    
    EPD_3IN7_SendCommand(0x26);
    for(i=0; i<16800; i++) {
        temp3=0;
        for(j=0; j<2; j++) {
            temp1 = Image[i*2+j];
            for(k=0; k<2; k++) {
                temp2 = temp1&0xC0 ;
                if(temp2 == 0xC0)
                    temp3 |= 0x01;//white
                else if(temp2 == 0x00)
                    temp3 |= 0x00;  //black
                else if(temp2 == 0x80)
                    temp3 |= 0x01;  //gray1
                else //0x40
                    temp3 |= 0x00; //gray2
                temp3 <<= 1;

                temp1 <<= 2;
                temp2 = temp1&0xC0 ;
                if(temp2 == 0xC0)  //white
                    temp3 |= 0x01;
                else if(temp2 == 0x00) //black
                    temp3 |= 0x00;
                else if(temp2 == 0x80)
                    temp3 |= 0x01; //gray1
                else    //0x40
                    temp3 |= 0x00;	//gray2
                if(j!=1 || k!=1)
                    temp3 <<= 1;

                temp1 <<= 2;
            }
        }
        EPD_3IN7_SendData(temp3);
    }

    EPD_3IN7_Load_LUT(0);
    
    EPD_3IN7_SendCommand(0x22);
    EPD_3IN7_SendData(0xC7);
    
    EPD_3IN7_SendCommand(0x20);
    
    EPD_3IN7_ReadBusy_HIGH(); 
}

/******************************************************************************
function :  Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_3IN7_1Gray_Display(const UBYTE *Image)
{
  UWORD i;
  UWORD IMAGE_COUNTER = EPD_3IN7_WIDTH * EPD_3IN7_HEIGHT / 8;

  EPD_3IN7_SendCommand(0x4E);
  EPD_3IN7_SendData(0x00);
  EPD_3IN7_SendData(0x00);
  EPD_3IN7_SendCommand(0x4F);
  EPD_3IN7_SendData(0x00);
  EPD_3IN7_SendData(0x00);

  EPD_3IN7_SendCommand(0x24);
  EPD_3IN7_SendDataStream(Image, IMAGE_COUNTER);
//   for (i = 0; i < IMAGE_COUNTER; i++)
//   {
//     EPD_3IN7_SendData(Image[i]);
//   }

  EPD_3IN7_Load_LUT(2);
  EPD_3IN7_SendCommand(0x20);
  EPD_3IN7_ReadBusy_HIGH();  
}

/******************************************************************************
function :  Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_3IN7_1Gray_Display_Part(const UBYTE *Image, UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
  UWORD i, Width;
  Width = (Xend-Xstart)%8 == 0 ? (Xend-Xstart)/8 : (Xend-Xstart)/8+1;
  UWORD IMAGE_COUNTER = Width * (Yend-Ystart);

  EPD_3IN7_SendCommand(0x44);
  EPD_3IN7_SendData(Xstart & 0xff);
  EPD_3IN7_SendData((Xstart>>8) & 0x03);
  EPD_3IN7_SendData(Xend & 0xff);
  EPD_3IN7_SendData((Xend>>8) & 0x03);
  EPD_3IN7_SendCommand(0x45);
  EPD_3IN7_SendData(Ystart & 0xff);
  EPD_3IN7_SendData((Ystart>>8) & 0x03);
  EPD_3IN7_SendData(Yend & 0xff);
  EPD_3IN7_SendData((Yend>>8) & 0x03);

  EPD_3IN7_SendCommand(0x24);
//   for (i = 0; i < IMAGE_COUNTER; i++)
//   {
//     EPD_3IN7_SendData(Image[i]);
//   }
  EPD_3IN7_SendDataStream(Image, IMAGE_COUNTER);
  
  EPD_3IN7_Load_LUT(2);
  EPD_3IN7_SendCommand(0x20);
  EPD_3IN7_ReadBusy_HIGH();    
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_3IN7_Sleep(void)
{
    EPD_3IN7_SendCommand(0X50);
    EPD_3IN7_SendData(0xf7);
    EPD_3IN7_SendCommand(0X02);  	//power off
    EPD_3IN7_SendCommand(0X07);  	//deep sleep
    EPD_3IN7_SendData(0xA5);
}