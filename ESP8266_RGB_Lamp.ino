#include <ESP8266WiFi.h>
#include <Esp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "LED_WS2812B.hpp"
#include "MQTT.hpp"


/************************************************
 *  RGB Lamp Settings
 ***********************************************/
#define LED_PIN        D6
#define LED_COUNT      16

WS2812B::RingLED lamp(LED_COUNT, LED_PIN);

int red(0), blue(0), green(0);  //White is not used in WS2812B. Int instead of uint8_t to handle some scanf issues

/************************************************
 *  MQTT INIT
 ***********************************************/

Connection::MqttListenDevice dev(1, "feeds/lamp");

/************************************************
 *  CODE SETUP
 ***********************************************/
void setup() {
  Serial.begin(115200);
  delay(10);

  lamp.init();
  
  Connection::connectWLAN();

  dev.init();
  lamp.setColorLED(0, Adafruit_NeoPixel::Color(0,0,0), true); // led 0 is set red on init
}

/************************************************
 *  CODE LOOP
 ***********************************************/
void loop() {
  Connection::connectBrokerMQTT();
  if(!lamp.getColorFromPixel(0)) //if there's no color set
  {
    lamp.setColorLED(0, Adafruit_NeoPixel::Color(0,30,0), true); //set one to green color to show availabilty
  }

  int result(0);

  if(dev.listenForMsg(5000))
  {
    result = sscanf(dev.getLastMsg().c_str(), "%d,%d,%d", &red, &green, &blue); //not elegant, but sscanf is much faster than streams

    if(result == 3)  
    {
      if((red | blue | green) >= 0 && (red | blue | green) < 256) //If all are in range of uint8_t - change color
      {
        lamp.setNewColorToAllPixels(Adafruit_NeoPixel::Color(red, green, blue), 30);
      }
      else
      {
        dev.sendError(Connection::SignalCode::BAD_PAYLOAD);
      }
    }
    else
    {
      dev.sendError(Connection::SignalCode::BAD_PAYLOAD_SIZE);
    }
  }
  else
  {
    dev.retainConnection();
  }
  
}
