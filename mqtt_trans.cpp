#include "HardwareSerial.h"
#include <stdint.h>
#include <AsyncMQTT_ESP32.h>

#include "mqtt_trans.h"

/*
  124.223.103.23: 树莓的MQTT服务器地址, ok
  broker.emqx.io: EMQ的公共 MQTT 服务器, not ok. 应该是在max packet size上做了限制
  192.168.1.188： EMQX 开源版, ok
  192.168.1.105: 运行于本地主机上的NanoMQ, ok. 需要改为本机ip
*/
const char* mqtt_server = "192.168.1.105";
const int mqtt_port = 1883; // 非加密端口

const char* mqtt_user = "user";
const char* mqtt_password = "555555";

// 主题配置
const char* publish_topic = "esp32/publish";   // 发布主题
const char* subscribe_topic = "esp32/subscribe"; // 订阅主题

AsyncMqttClient mqttClient;

// 连接Wi-Fi
void setup_wifi() {
  delay(1000);
  Serial.println();
  Serial.print("连接到 ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  // 等待连接
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi 已连接");
  Serial.println("IP 地址: ");
  Serial.println(WiFi.localIP());
}

// MQTT 连接回调
void onMqttConnect(bool sessionPresent) {
  Serial.println("已连接到 MQTT Broker");
  // 订阅主题
  mqttClient.subscribe(subscribe_topic, 2);
}

// MQTT 断开连接回调
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.print("MQTT 断开连接，原因: ");
  Serial.println((int)reason);
  
  // 尝试重新连接
  if (WiFi.isConnected()) {
    mqttClient.connect();
  }
}

// MQTT 订阅回调
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.print("订阅成功，PacketId: ");
  Serial.println(packetId);
}

// MQTT 发布回调
void onMqttUnsubscribe(uint16_t packetId) {
  Serial.print("取消订阅成功，PacketId: ");
  Serial.println(packetId);
}

// MQTT 发布完成回调
void onMqttPublish(uint16_t packetId) {
  Serial.print("发布完成，PacketId: ");
  Serial.println(packetId);
}

// MQTT 消息接收回调
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    if (0 == b_speak_out)
        return;  
    
    b_recv_over = 1;

    if (len == 0 || len % sizeof(int16_t) != 0)
        return;
    
    if (strcmp(topic, subscribe_topic) == 0) 
    {
        int buffer_size = len / sizeof(int16_t);

        // 使用 std::vector 动态分配内存
        std::vector<int16_t> i2s_buffer_16(buffer_size);
        std::vector<int16_t> output_buffer(buffer_size * 2);

        // 将 payload 内容拷贝到 i2s_buffer_16 中
        memcpy(i2s_buffer_16.data(), payload, len);
        /*
        // 输出处理结果
        for (size_t i = 0; i < i2s_buffer_16.size(); ++i) {
            printf("%04X ", static_cast<int16_t>(i2s_buffer_16[i]));
        }
        Serial.println("\n");
        */
        for (int i = 0; i < buffer_size; i++) {
            output_buffer[2 * i] = i2s_buffer_16[i];         // 左通道
            output_buffer[2 * i + 1] = i2s_buffer_16[i];     // 右通道
        }

        local_i2s_write(output_buffer.data(), buffer_size);
    }
}

void init_mqtt() { 
  // 连接Wi-Fi
  setup_wifi();
  
  // 配置 MQTT 客户端
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  
  mqttClient.setServer(mqtt_server, mqtt_port);
  
  // 如果使用认证，设置用户名和密码
  //mqttClient.setCredentials(mqtt_user, mqtt_password);
  
  // 连接到 MQTT Broker
  mqttClient.connect();
}

void send_mqtt_data(int16_t *data, uint16_t len) {
    /*
    for (size_t i = 0; i < len; ++i) {
        Serial.printf("%04X ", static_cast<int16_t>(data[i]));
    } 
    Serial.println("\n");
    */
    if (mqttClient.connected()) {
        mqttClient.publish(subscribe_topic, 0, false, (const char*) data, len * sizeof(int16_t));
    }
}
