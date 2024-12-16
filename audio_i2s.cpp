#include "audio_i2s.h"

int32_t *samples_32bit;//读出来的原始32位数据，长度=CUR_SAMPLE_COUNT
int16_t *samples_16bit;//转换后的16位数据，长度=CUR_SAMPLE_COUNT
uint8_t *samples_8bit ;//转换后的8位数据，长度=CUR_SAMPLE_COUNT
int16_t *recive_16bit;//接收后转换的16位数据，长度=CUR_SAMPLE_COUNT
int16_t *output_16bit;//发送给扬声器的16位数据，长度=CUR_SAMPLE_COUNT*2，因为传入数据是双声道所以*2

//mic读取音频配置
const i2s_config_t rec_i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,//单位是bits_per_sample
    //.use_apll = false,
    //.tx_desc_auto_clear = false,
    //.fixed_mclk = 0
};

//i2s扬声器播放配置
const i2s_config_t spk_i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
    .communication_format =(i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
};

const i2s_pin_config_t rec_pin_config = {
    .bck_io_num   = RX_I2S_BCLK,
    .ws_io_num    = RX_I2S_LRC,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = RX_I2S_DIN
};

const i2s_pin_config_t spk_pin_config = {
    .bck_io_num   = TX_I2S_BCLK,
    .ws_io_num    = TX_I2S_LRC,
    .data_out_num = TX_I2S_DOUT,
    .data_in_num  = I2S_PIN_NO_CHANGE
};  

void init_i2s()
{
    samples_32bit = (int32_t *)malloc(sizeof(int32_t) * CUR_SAMPLE_COUNT);
    samples_16bit = (int16_t *)malloc(sizeof(int16_t) * CUR_SAMPLE_COUNT);
    samples_8bit =  (uint8_t *)malloc(sizeof(uint8_t) * CUR_SAMPLE_COUNT);
    recive_16bit =  (int16_t *)malloc(sizeof(int16_t) * CUR_SAMPLE_COUNT);
    output_16bit =  (int16_t *)malloc(2 * sizeof(int16_t) * CUR_SAMPLE_COUNT);

    esp_err_t err;

    /* RX: I2S_NUM_0 */
    err = i2s_driver_install(REC_I2S_PORT, &rec_i2s_config, 0, NULL);
    CHECK(err);

    err = i2s_set_pin(REC_I2S_PORT, &rec_pin_config);
    CHECK(err);

    /* TX: I2S_NUM_1 */
    err = i2s_driver_install(SPK_I2S_PORT, &spk_i2s_config, 0, NULL);
    CHECK(err);

    err = i2s_set_pin(SPK_I2S_PORT, &spk_pin_config);
    CHECK(err);

    Serial.println("I2SInit over!");
}

uint32_t local_i2s_read(int16_t *samples, int count)// read from i2s
{
    size_t bytes_read = 0;
    
    if (count > CUR_SAMPLE_COUNT)
        count = CUR_SAMPLE_COUNT;//最少读取CUR_SAMPLE_COUNT
    
    i2s_read(REC_I2S_PORT, (char *)samples_32bit, sizeof(int32_t) * count, &bytes_read, portMAX_DELAY);

    uint32_t samples_read = bytes_read / sizeof(int32_t);

    //convert 1x 32 bit mono -> 1 x 16 bit mono
    for (int i=0; i<samples_read; i++)
    {
        int32_t temp = samples_32bit[i] >> 11;
        samples[i] = (temp > INT16_MAX) ? INT16_MAX : (temp < -INT16_MAX) ? -INT16_MAX : (int16_t)temp;
    }

    return samples_read;
}

void local_i2s_write(int16_t *samples, int count)//数据写入IIS
{
    size_t bytes_written;

    i2s_write(SPK_I2S_PORT, samples, sizeof(int16_t)*count*2, &bytes_written, portMAX_DELAY); 
    //Serial.println(bytes_written);
}

//16位数据转成8位
void covert_data_bit(int16_t *samples_16bit, uint8_t *samples_8bit, uint32_t len)
{ 
    for(uint32_t i=0; i<len; i++)
        samples_8bit[i] = (samples_16bit[i] + 32768) >> 8;    
}