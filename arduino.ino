#include <WiFiEspClient.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>
#include "SoftwareSerial.h"
#include <Adafruit_NeoPixel.h>
#define PIN 6   /*定义了控制LED的引脚，6表示Microduino的D6引脚，可通过Hub转接出来，用户可以更改 */
#define PIN_NUM 1 //定义允许接的led灯的个数

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIN_NUM, PIN, NEO_GRB + NEO_KHZ800); //该函数第一个参数控制串联灯的个数，第二个是控制用哪个pin脚输出，第三个显示颜色和变化闪烁频率
#define clientId "xxxxxxxx"//设备ID
#define username "xxxxx"//产品ID
#define password "xxxxxxxxxx"// 这里是产品的api-key
#define WIFI_AP "so cute"//自己的WiFi
#define WIFI_PASSWORD "meiyoumima"//自己的WiFi密码

void callback(char* topic, byte* payload, unsigned int length);
char MqttServer[] = "183.230.40.39";//onenet mqtt服务器地址（固定）
int port=6002; //onenet mqtt服务器端口（固定）

// 初始化以太网客户端对象 -- WiFiEspClient.h
WiFiEspClient espClient;

// 初始化DHT11传感器
//DHT dht(DHTPIN, DHTTYPE);

// 初始化MQTT库PubSubClient.h的对象
PubSubClient client(espClient);

SoftwareSerial soft(2, 3); // RX, TX

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup() {
  Serial.begin(9600);
    strip.begin(); //准备对灯珠进行数据发送
  strip.show(); //初始化LED为关的状态
//  dht.begin();

  InitWiFi();                                // 连接WiFi
  client.setServer( MqttServer, port );      // 连接WiFi之后，连接MQTT服务器
  client.setCallback(callback);
  lastSend = 0;
}

void loop() {
  if ( !client.connected() ) {
    reconnect();
  }
  client.loop();
}

void InitWiFi()
{
  // 初始化软串口，软串口连接ESP模块
  soft.begin(9600);
  // 初始化ESP模块
  WiFi.init(&soft);
  // 检测WiFi模块在不在，宏定义：WL_NO_SHIELD = 255,WL_IDLE_STATUS = 0,
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }
  Serial.println("[InitWiFi]Connecting to AP ...");
  // 尝试连接WiFi网络
  while ( status != WL_CONNECTED) {
    Serial.print("[InitWiFi]Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_AP);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    delay(500);
  }
  Serial.println("[InitWiFi]Connected to AP");
}
/**
 * 
 * MQTT客户端断线重连函数
 * 
 */
 void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  //测试数据是否成功读取

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    strip.setPixelColor(0, strip.Color(255, 0, 0));//红光
    strip.show();   //LED显示 
    Serial.print("ON\n");
  } else {
    strip.setPixelColor(0, strip.Color(0, 0, 0));//无光
    strip.show();
    Serial.print("OFF\n");
  }

}

void reconnect() {
  // 一直循环直到连接上MQTT服务器
  while (!client.connected()) {
    Serial.print("[reconnect]Connecting to MQTT Server ...");
    // 尝试连接connect是个重载函数 (clientId, username, password)
    if ( client.connect(clientId, username, password) ) {
      Serial.println( "[DONE]" );
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print( "[FAILED] [ mqtt connect error code = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );// Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}
