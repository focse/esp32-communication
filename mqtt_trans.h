
#pragma once

#ifndef _MQTT_TRANSPORT_H_
#define _MQTT_TRANSPORT_H_

#include <AsyncMQTT_ESP32.hpp>

#include "common.h"

extern AsyncMqttClient mqtt_client;

void init_mqtt();
void mqtt_reconnect();

void send_mqtt_data(int16_t *data, uint16_t len);

#endif  //_MQTT_TRANSPORT_H_