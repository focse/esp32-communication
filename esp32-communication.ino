
//#include <AsyncMQTT_ESP32.h>

#include "common.h"

#include "esp_now_trans.h"
#include "udp_trans.h"
#include "mqtt_trans.h"

// WiFi network credentials
//TP-LINK_0D2246
const char* wifi_ssid = "412";
const char* wifi_password = "412412412";

bool b_send_over = 1; //发送完成标志位
bool b_recv_over = 0; //接受完成标志位
bool b_speak_out;     //0代表对外讲话，1代表收听

void setup(void)
{
    Serial.println("Init start!");

    Serial.begin(115200);

    //ESP-NOW初始化，用来传输音频
#if USE_TRANS_PROTOCOL == USE_ESP_NOW
    init_esp_now();
#elif USE_TRANS_PROTOCOL == USE_UDP
    init_udp();
#elif USE_TRANS_PROTOCOL == USE_MQTT
    init_mqtt();
#else
    Serial.println("Unknown Version");
    return;
#endif

    //I2S初始化 
    init_i2s();
    
    pinMode(BTN, INPUT_PULLUP);//按键
    pinMode(LED, OUTPUT);//指示灯
    digitalWrite(LED, HIGH);

    Serial.println("Init end!");
}

bool BtnisPressed(void)//按键是否按下
{
    return !digitalRead(BTN);   
}

void loop(void)
{
    if ( BtnisPressed() )//按下按键发射数据
    {
        b_speak_out = 0;

        digitalWrite(LED, HIGH); //发射时开灯

        uint32_t samples_read = local_i2s_read(samples_16bit, CUR_SAMPLE_COUNT); //读取数据

#if USE_TRANS_PROTOCOL == USE_ESP_NOW
        covert_data_bit(samples_16bit, samples_8bit, samples_read); //发送时转换为8位

        send_esp_now_data(samples_8bit, samples_read);  //发射数据
#elif USE_TRANS_PROTOCOL == USE_UDP
    #if USE_16BIT_DATA == 0
        covert_data_bit(samples_16bit, samples_8bit, samples_read); //发送时转换为8位
        
        send_udp_data(samples_8bit, samples_read);
    #elif USE_16BIT_DATA == 1
        send_udp_data((uint8_t *) samples_16bit, samples_read * sizeof(int16_t));
    #endif
#elif USE_TRANS_PROTOCOL == USE_MQTT
        send_mqtt_data(samples_16bit, samples_read);
#else
    Serial.println("Unknown Version");
    return;
#endif
    }
    else//接收数据，接受部分在回调函数中
    {
      delay(28);//经过一段延时再判断，接收数据并且播放也需要时间

      b_speak_out = 1;
      
      if (b_recv_over)
      {
          b_recv_over = 0;
          
          if(digitalRead(LED))  //接受数据时闪烁LED
              digitalWrite(LED, LOW);
          else
              digitalWrite(LED, HIGH);
      }
      else
      {
          digitalWrite(LED, LOW);  //没有接收到消息，也没有发射，关灯
          i2s_zero_dma_buffer(SPK_I2S_PORT);  //清空DMA中缓存的数据，你可以尝试一下不清空（注释这行）是什么效果

          sequenceNumber = 0;
      }
    }
}
