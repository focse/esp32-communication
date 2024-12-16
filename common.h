#pragma once

#ifndef _COMMON_H_
#define _COMMON_H_

#include <WiFi.h>

#include "audio_i2s.h"

#define USE_ESP_NOW 0
#define USE_UDP     1
#define USE_MQTT    2

#define USE_TRANS_PROTOCOL  USE_ESP_NOW

#define USE_16BIT_DATA  0

#if USE_TRANS_PROTOCOL == USE_MQTT
#define CUR_SAMPLE_COUNT  512
#else
#define CUR_SAMPLE_COUNT  128
#endif
extern const char* wifi_ssid;
extern const char* wifi_password;

extern bool b_send_over;
extern bool b_recv_over;
extern bool b_speak_out;

#endif  //_COMMON_H_