#include <Esp.h>

#include "WS2812B.hpp"
#include "MQTT.hpp"
#include "MessageProcessing.hpp"

/************************************************
 *  RGB Lamp Settings
 ***********************************************/
#define LED_PIN        D6
#define LED_COUNT      16

WS2812B::RingLED lamp(LED_COUNT, LED_PIN);

/************************************************
 *  MQTT INIT
 ***********************************************/

Connection::MqttListenDevice dev(1, "feeds/lamp");

/************************************************
 *  CODE SETUP
 ***********************************************/
void setup() {
  Serial.begin(9600);
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
  if(!Connection::connectBrokerMQTT())
  {
    lamp.setColorLED(1, Adafruit_NeoPixel::Color(255,163,0), true); //orange color to show mqtt error
    return;
  }
  if(!lamp.getColorFromPixel(0)) //if there's no color set
  {
    lamp.setColorLED(0, Adafruit_NeoPixel::Color(0,30,0), true); //set one to green color to show availabilty
    lamp.setColorLED(1, Adafruit_NeoPixel::Color(0,0,0), true); //reset orange led if turned on
  }

  int result(0);

  if(dev.listenForMsg(5000))
  {
    if(Msg::dataProcessing(dev.getLastMsg(), lamp, dev, &Connection::last_signal))
    {
      dev.sendAckMsg();
    }
    else
    {
      if(Connection::last_signal != Connection::SignalCode::BAD_DEVICE_ID)
      {
        dev.sendError(Connection::last_signal);
      }
    }  
  }
  else
  {
    dev.retainConnection();
  }
}
