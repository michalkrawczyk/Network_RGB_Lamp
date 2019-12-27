#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Adafruit_NeoPixel.h>

/************************************************
 *  WLAN Access Setup
 ***********************************************/
#define WLAN_SSID       "SSID"
#define WLAN_PASS       "password"

/************************************************
 *  Broker Server Setup
 ***********************************************/
#define IO_SERVER       "io.adafruit.com"
#define IO_PORT         1883
#define IO_USER         "username"
#define IO_KEY          "4f8c88ab3a2741638a335fc1d2696fb9"

namespace mqtt_params       //For some Settings it's better to store like that
{
  int16_t read_timeout(5000);
  uint8_t conn_retries = 3;
}

/************************************************
 *  RGB Lamp Settings
 ***********************************************/
#define LED_PIN        D6
#define LED_COUNT      16

namespace color_params
{
  uint8_t brightness(150u);
  uint32_t led_array[LED_COUNT] = {};
}

/************************************************
 *  Init
 ***********************************************/

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client,
        IO_SERVER,
        IO_PORT,
        IO_USER,
        IO_KEY);

enum class COLOR
{
  WHITE = 1,
  RED = 2, 
  GREEN = 3, 
  BLUE = 4
};

Adafruit_NeoPixel pixel_matrix(LED_COUNT,
        LED_PIN,
        NEO_GRB + NEO_KHZ800);

int red(0), blue(0), green(0);  //White is not used in WS2812B. Int instead of uint8_t to handle some scanf issues


/************************************************
 *  Feeds
 ***********************************************/
Adafruit_MQTT_Subscribe rgb_color = Adafruit_MQTT_Subscribe(&mqtt, IO_USER "/feeds/lamp");


/************************************************
 *  Prototype Functions
 ***********************************************/
void MQTT_connect();

void setColorLED(const uint16_t &pixel, const uint32_t &color);

float calculateStep(const uint8_t &start_value,
        const uint8_t &final_value,
        const uint16_t &step);
        
uint8_t getColorComponent(const uint32_t &color, const COLOR &component);

void setNewColorToAllPixels(const uint32_t &color,
        const uint16_t &time_delay = 50,
        const uint8_t &steps_count = 100);



/************************************************
 *  CODE SETUP
 ***********************************************/
void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Lamp Initialization \n"));
  pixel_matrix.begin();
  pixel_matrix.show();
  pixel_matrix.setBrightness(color_params::brightness);

  setColorLED(0, pixel_matrix.Color(153,0,0));
  
  pixel_matrix.show();

  Serial.println(F("MQTT Connection Initialization"));
  Serial.print(("  Connecting to network:"));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
  mqtt.subscribe(&rgb_color); // Setup MQTT subscription
  setColorLED(0, pixel_matrix.Color(0,0,0));

  pixel_matrix.show();
}

/************************************************
 *  CODE LOOP
 ***********************************************/
void loop() {
  MQTT_connect();
  if(color_params::led_array[0] == 0)
  {
    setColorLED(0, pixel_matrix.Color(0,30,0));
          
    pixel_matrix.show();
  }

Adafruit_MQTT_Subscribe *subscription_ptr;
int result(0);

while ((subscription_ptr = mqtt.readSubscription(mqtt_params::read_timeout))) 
  {
    if (subscription_ptr == &rgb_color) 
    {
      Serial.print(F("Got: "));
      Serial.println((char *)rgb_color.lastread);
      
      result = sscanf((char *)rgb_color.lastread, "%d,%d,%d", &red, &green, &blue); //Avoiding %hhu, cause of possibilty of bad cast
    }
  }

  if(result == 3)  
  {
      if((red | blue | green) >= 0 && (red | blue | green) < 256) //If all are in range of uint8_t - change color
      {
        setNewColorToAllPixels(pixel_matrix.Color(red, green, blue), 20);
      }
  }
  else
  {
    if(!mqtt.ping())
    {
      mqtt.disconnect();
    }
  }
  
}

/************************************************
 *  Declarations of Prototype Functions
 ***********************************************/


void MQTT_connect() {
  /**
   * @brief
   *  Connects/Reconnects with MQTT Server
  */

  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Connecting to MQTT... "));
  
  uint8_t retries(mqtt_params::conn_retries);
  
  while (ret = mqtt.connect()) // mqtt.connect will return 0 if connected
  { 

    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();

    delay(5000);
    retries--;

    if (!retries) 
    {
      setColorLED(1, pixel_matrix.Color(255,163,0));
      pixel_matrix.show();

      while (1); // wait for reset from WDT
    }
  }
  Serial.println(F("Connected with MQTT"));
}


void setColorLED(const uint16_t &pixel, const uint32_t &color)
{
    pixel_matrix.setPixelColor(pixel, color);
    color_params::led_array[pixel] = color;
}



float calculateStep(const uint8_t &start_value,
        const uint8_t &final_value,
        const uint16_t &step)
{
  if ((final_value - start_value) == 0)
  {
    return 0.f;
  }
  return float(final_value-start_value)/step;
}



uint8_t getColorComponent(const uint32_t &color,
        const COLOR &component)
{
     /**
   * @brief
   * Return one component from color
   * 
   * @param color - color reference
   * @param component - component to be obtained
   * enum class COLOR{WHITE = 1, RED = 2, GREEN = 3, BLUE = 4 };
   */
    int shift = (int(COLOR::BLUE) - int(component) ) * 8;
    uint32_t pattern = (uint32_t(0xff) << shift);

    uint8_t result = (uint8_t(color & pattern) >> shift);
    

    return (uint8_t(color & pattern) >> shift);
}


void setNewColorToAllPixels(const uint32_t &color,
        const uint16_t &time_delay,
        const uint8_t &steps_count)
{
   /**
   * @brief
   * Changes in steps to desired color
   * 
   * @param color - desired color
   * @param time_delay - delay between each step
   * @param steps_count - describes how many steps would take to switch
   * Note: More steps_count and less time_delay - will effect with more smooth passing
   *
   */
    uint8_t final[4] = {
            getColorComponent(color, COLOR::WHITE),
            getColorComponent(color, COLOR::RED),
            getColorComponent(color, COLOR::GREEN),
            getColorComponent(color, COLOR::BLUE)
    };

    uint8_t  start[LED_COUNT][4];

    float step[LED_COUNT][4];

    for (uint16_t led = 0; led < LED_COUNT; led++)
    {
        start[led][0] = getColorComponent(color_params::led_array[led], COLOR::WHITE);
        start[led][1] = getColorComponent(color_params::led_array[led], COLOR::RED);
        start[led][2] = getColorComponent(color_params::led_array[led], COLOR::GREEN);
        start[led][3] = getColorComponent(color_params::led_array[led], COLOR::BLUE);

        step[led][0] = calculateStep(start[led][0], final[0], steps_count);
        step[led][1] = calculateStep(start[led][1], final[1], steps_count);
        step[led][2] = calculateStep(start[led][2], final[2], steps_count);
        step[led][3] = calculateStep(start[led][3], final[3], steps_count);
    }

    for (uint8_t cur_step = 0; cur_step < steps_count; cur_step++)
    {
        for (uint16_t led = 0; led < LED_COUNT; led++)
        {
            setColorLED(led, pixel_matrix.Color((uint8_t)(start[led][1] + step[led][1] * cur_step),
                                                (uint8_t)(start[led][2] + step[led][2] * cur_step),
                                                (uint8_t)(start[led][3] + step[led][3] * cur_step),
                                                (uint8_t)(start[led][0] + step[led][0] * cur_step)
            ));
        }

        delay(time_delay);
        pixel_matrix.show();
    }

    for (uint16_t led = 0; led < LED_COUNT; led++)
    {
        if(color_params::led_array[led] != color)
        {
            setColorLED(led, color);
        }
    }
    pixel_matrix.show();

}
