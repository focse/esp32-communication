#include "esp_now_trans.h"

const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0XFF};  //广播地址

static void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status);
static void on_data_recv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len);

//esp-now初始化
void init_esp_now(void) {
    WiFi.disconnect();

    WiFi.mode(WIFI_STA);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel((wifi_second_chan_t)3, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    if (esp_now_init() == ESP_OK) {
        Serial.println("ESPNow Init Success");
    }
    else {
        Serial.println("ESPNow Init Failed");
        ESP.restart();
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, BROADCAST_MAC, 6);
    if (!esp_now_is_peer_exist(BROADCAST_MAC))
    {
        esp_err_t result;
      
        result = esp_now_add_peer(&peerInfo);
        if (result != ESP_OK)
            Serial.printf("Failed to add broadcast peer: %s\n", esp_err_to_name(result));
    }

    //绑定发射和接受回调函数
    esp_now_register_send_cb(on_data_sent);
    esp_now_register_recv_cb(on_data_recv);

    Serial.println("InitESPNow over!");
}

void send_esp_now_data(const uint8_t *data, uint16_t len) {
    //必须得等待上次发送完毕
    while (0 == b_send_over) 
      delay(1);
    
    b_send_over = 0;
    
    esp_now_send(BROADCAST_MAC, data, len);
}

void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {//数据发送完成回调函数
    b_send_over = 1;
    //if(status == ESP_NOW_SEND_SUCCESS )
    //    Serial.println("Delivery success");
    //else
    //    Serial.println("Delivery success");
}

//数据接受完成回调函数，将数据写入到扬声器I2S中
void on_data_recv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len) {
    if(0 == b_speak_out)
        return;  
    
    b_recv_over = 1;

    for (int i = 0; i < data_len; i++)//接收到信息后转换为16bit，补充左右声道，写入到I2S
    {
        recive_16bit[i] = (data[i] - 128) << 5;
        output_16bit[i * 2 ] = recive_16bit[i] * MAX98357_VOLUME;
        output_16bit[i * 2 + 1] = recive_16bit[i] * MAX98357_VOLUME;
    }
    
    local_i2s_write(output_16bit, data_len);
}
