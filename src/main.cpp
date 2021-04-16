#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets

#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>

WebSocketsClient webSocket;
const char *ssid = "OOOOOOOO";
const char *password = "oooooooo";
const char *ip_host = "th-chart.herokuapp.com";
const uint16_t port = 443;
const int LED = 2;
const int BTN = 0;
unsigned long lastTime = 0;
unsigned long timerUpdateData = 60000;
unsigned long timerDelay = 30000;
const int DHTPIN = 4;
const int DHTTYPE = DHT11;

DHT dht(DHTPIN, DHTTYPE);

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);
  }
  break;
  case WStype_TEXT:
    Serial.printf("[WSc] get text: %s\n", payload);
    if (strcmp((char *)payload, "LED_ON") == 0)
    {
      Serial.printf("[WSc] turn on LED");

      digitalWrite(LED, HIGH); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
    }
    else if (strcmp((char *)payload, "LED_OFF") == 0)
    {
      digitalWrite(LED, LOW); // Khi client phát sự kiện "LED_OFF" thì server sẽ tắt LED
    }
    break;
  case WStype_BIN:
    Serial.printf("[WSc] get binary length: %u\n", length);
    break;
  }
}
void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT);
  dht.begin();
  Serial.begin(115200);
  Serial.println("ESP8266 Websocket Client");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  webSocket.beginSSL(ip_host, port);
  webSocket.onEvent(webSocketEvent);
}
void loop()
{
  webSocket.loop();
  if ((millis() - lastTime) > timerDelay)
  {

    static bool isPressed = false;
    if (!isPressed && digitalRead(BTN) == 0)
    { //Nhấn nút nhấn GPIO0
      isPressed = true;
      webSocket.sendTXT("BTN_PRESSED");
    }
    else if (isPressed && digitalRead(BTN))
    { //Nhả nút nhấn GPIO0
      isPressed = false;
      webSocket.sendTXT("BTN_RELEASE");
    }

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    Serial.println(t);

    if (t != NULL && h != NULL)
    {
      StaticJsonDocument<200> th;
      th["temperature"] = t;
      th["humidity"] = h;

      String jsonTH;
      serializeJson(th, jsonTH);
      webSocket.sendTXT(jsonTH);
    }

    lastTime = millis();
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, password);
    delay(5000);
    Serial.print(".");
  }
}