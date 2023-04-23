#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/adc.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * Defines for WIFI connection
 */
#define SSID "xxx"
#define PASSWORD "xxx"
#define HOST_IP "192.168.1.xxx" 
#define PORT_STRING "8888"
#define ID "152" 

/**
 * Defines for Temperature sensor
 */
#define ONE_WIRE_BUS 4 // connected to pin 4 of temperature sensor
#define TEMPERATURE_PRECISION 10 // 9 to 12 bits
#define TEMPERATURE_REFRESH_TIME 10000 // in milliseconds

/**
 * URL variables used by the http post requests
 */
static const char *LINK_TEMP = "http://" HOST_IP ":" PORT_STRING "/temp/" ID;

/**
 * Variables for sending data
 */
//static bool transmitNow = false;
static HTTPClient http;

/**
 * Variables for Temperature sensor
 */
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);
static DeviceAddress outsideThermometer = {0x21, 0x22, 0x32, 0x13, 0xA, 0x0, 0x0, 0x11}; 
static float current_temp;

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("MY IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  wifiSetup();
  sensors.begin();
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);
}

void loop() {
  unsigned long currentMillis = millis();
  static unsigned long previous_temperature_time = 0;

  // Send temp data every 10 seconds
  if (currentMillis - previous_temperature_time >= TEMPERATURE_REFRESH_TIME) {
    previous_temperature_time = currentMillis;

    sensors.requestTemperatures();  // request the temperature reading from the sensor
    current_temp = sensors.getTempC(outsideThermometer); // get the temperature in Celsius
    
    http.begin(LINK_TEMP);
    http.addHeader("Content-Type", "text/plain");
    
    int httpResponseCode = http.POST(String(current_temp)); // send the temperature data to the server
    
    if (httpResponseCode <= 0) {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}
