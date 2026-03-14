#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN 18
#define DATA_PIN 23
#define CS_PIN 5

MD_Parola display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

/* WLAN */

const char* ssid = "A1-68CB0A11";
const char* password = "vnNeoRJwXVm7k5";

/* Firebase */

String firebaseHost = "https://lea-led-display-default-rtdb.firebaseio.com/display.json";

/* Status Variablen */

String message = "HELLO";
bool displayOn = true;
int brightness = 5;

/* Timing */

unsigned long lastCheck = 0;
int checkInterval = 10000;

/* WLAN verbinden */

void connectWiFi()
{
  WiFi.begin(ssid, password);

  display.displayText("WIFI", PA_CENTER, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }

  display.displayText("ONLINE", PA_CENTER, 50, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

/* Firebase lesen */

void checkFirebase()
{
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;

  http.begin(firebaseHost);

  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String payload = http.getString();

    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      message = doc["message"].as<String>();
      brightness = doc["brightness"];
      displayOn = doc["on"];
    }
  }

  http.end();
}

/* Display aktualisieren */

void updateDisplay()
{
  display.setIntensity(brightness);

  if (!displayOn)
  {
    display.displayClear();
    return;
  }

  if (display.displayAnimate())
  {
    display.displayText(
      message.c_str(),
      PA_CENTER,
      100,
      0,
      PA_SCROLL_LEFT,
      PA_SCROLL_LEFT
    );
  }
}

void setup()
{
  display.begin();
  display.setIntensity(5);
  display.displayClear();
  display.displayText("TEST", PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  connectWiFi();
}

void loop()
{
  if (millis() - lastCheck > checkInterval)
  {
    checkFirebase();
    lastCheck = millis();
  }

  updateDisplay();
}