#ifndef _ESP_TRANSPORT_H_
#define _ESP_TRANSPORT_H_

#include <esp_now.h>
#include <esp_wifi.h>

#include "common.h"

void init_esp_now(void);
void send_esp_now_data(const uint8_t *data, uint16_t len);

#endif  //_ESP_TRANSPORT_H_