#pragma once

#ifndef _AUDIOI2S_H_
#define _AUDIOI2S_H_

#include <stdint.h>
#include <Arduino.h>
#include <driver/i2s.h>

#include "common.h"

#define BTN 0

#if defined(CONFIG_IDF_TARGET_ESP32)
#define LED 2          // ESP32的LED引脚
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define LED 44         // ESP32-S3的LED引脚
#endif

#define LED_FREQ 2000
#define LED_RESOLUTION 8

#define SAMPLE_RATE 44100

#define CHECK(err)   if (err != ESP_OK) {Serial.printf("Failed installing driver: %d\n", err);while (true);}

#if defined(CONFIG_IDF_TARGET_ESP32)
#define TX_I2S_DOUT   17    // connect with external DAC
#define TX_I2S_BCLK   18
#define TX_I2S_LRC    19

#define RX_I2S_DIN    23    // connect with I2S microphone
#define RX_I2S_BCLK   21
#define RX_I2S_LRC    22
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#define TX_I2S_DOUT   16    // connect with external DAC
#define TX_I2S_BCLK   17
#define TX_I2S_LRC    18

#define RX_I2S_DIN    5    // connect with I2S microphone
#define RX_I2S_BCLK   6
#define RX_I2S_LRC    4
#endif

#define MAX98357_VOLUME 1

const i2s_port_t REC_I2S_PORT = I2S_NUM_0;
const i2s_port_t SPK_I2S_PORT = I2S_NUM_1;

extern int32_t *samples_32bit;//读出来的原始32位数据，长度128
extern int16_t *samples_16bit;//转换后的16位数据，长度128
extern uint8_t *samples_8bit ;//转换后的8位数据，长度128
extern int16_t *recive_16bit;//接收后转换的16位数据，长度128
extern int16_t *output_16bit;//发送给扬声器的16位数据，长度256

void init_i2s();

uint32_t local_i2s_read(int16_t *samples, int count);
void local_i2s_write(int16_t *samples, int count);

void covert_data_bit(int16_t *samples_16bit, uint8_t *samples_8bit, uint32_t len);

#endif   //_AUDIOI2S_H_
