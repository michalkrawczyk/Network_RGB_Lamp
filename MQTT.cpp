#include "MQTT.hpp"
#include "Esp.h"
#include <sstream>

#include "Network_Settings.h" // this file contains Network and MQTT Setup Settings
/* Note: That file includes: 
*(#define) WLAN_SSID, WLAN_PASS, IO_SERVER, IO_PORT, IO_USER, IO_KEY, IO_ERROR_FEED
*/


static std::string feedToString(const std::string &feed_name);

template <typename T>
static std::string to_string(const T &n) //Compiler Bug causes not finding std::to_string even in <string>
{
    std::ostringstream out_stream ;
    out_stream<< n ;

    return out_stream.str() ;
}

namespace Connection{
    extern WiFiClient client{};
    extern Adafruit_MQTT_Client mqtt(&client,
                                    IO_SERVER,
                                    IO_PORT,
                                    IO_USER,
                                    IO_KEY);
    

#ifdef ESP8266
    inline namespace ESP
    {
        void connectWLAN()
        {
            Serial.print(("  Connecting to network:"));
            Serial.println(WLAN_SSID);

            WiFi.begin(WLAN_SSID, WLAN_PASS);
            uint8_t retries(max_retries);

            while (WiFi.status() != WL_CONNECTED) 
            {
                delay(500);
                Serial.print(".");
        
                retries--;

                if(!retries)
                {
                    Serial.println();
                    Serial.println("Break for sleep - 5 s");
                    //TODO: Configure Watchdog
                    // ESP.deepSleep(5000000); //go to deep sleep for 5 s

                    Serial.print("Retrying to establish connection");
                    // retries = max_retries;

                    while(1);   //Wait for watchdog (temporary)
                }
        
            }

            Serial.println();
            Serial.println(F("WiFi connected"));
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
        }

        bool connectBrokerMQTT()
        {
            int8_t ret;

            if (mqtt.connected()) 
            {
                return true;
            }

            Serial.print(F("Connecting with MQTT... "));
            
            uint8_t retries(max_retries);
            
            while (ret = mqtt.connect()) // mqtt.connect will return 0 if connected
            { 

                Serial.println(mqtt.connectErrorString(ret));
                Serial.println("Retrying MQTT connection in 5 seconds...");
                mqtt.disconnect();

                // ESP.wdtFeed(); //feed to avoid reseting during reconnect
                delay(5000); //TODO: Consider changing to sleep function
                retries--;
                

                if (!retries) 
                {
                    // lamp.setColorLED(1, Adafruit_NeoPixel::Color(255,163,0), true);
                    // while (1); // wait for reset from WDT
                    return false;
                }
            }
            Serial.println(F("Connected!"));
            return true;
        }
        
        MqttListenDevice::MqttListenDevice(const uint8_t &device_id, const std::string &feed_name):
        _k_device_id(device_id),
        _listen_feed(&mqtt, feedToString(feed_name).c_str())
        {
          
        }

        void MqttListenDevice::init()
        {
            mqtt.subscribe(&_listen_feed);
        }

        bool MqttListenDevice::sendError(const SignalCode &err_code)
        {
            
            std::string msg(to_string(_k_device_id));
            //TODO: ADD KEY HERE
            msg += '/' + to_string(static_cast<uint8_t>(err_code));

            Serial.print("Signal Sent:");
            Serial.println(msg.c_str());
            
            return _error_feed.publish(msg.c_str());
        }

        bool MqttListenDevice::sendAckMsg()
        {
            return sendError(SignalCode::NO_ERROR);
        }

        bool MqttListenDevice::listenForMsg(const int16_t &timeout)
        {
            Adafruit_MQTT_Subscribe *subscription_ptr;

            while ((subscription_ptr = mqtt.readSubscription(timeout))) 
            {
                if (subscription_ptr == &_listen_feed) 
                {
                Serial.print(F("Got: "));
                Serial.println((char *)_listen_feed.lastread);

                _last_msg = reinterpret_cast<char *>(_listen_feed.lastread);

                return true;
                }
            }
            return false;
            //how to make system ready for dynamic number of variables and chars?
            //and do dynamicly operations with them?
        }

        bool MqttListenDevice::retainConnection()
        {
            bool result(false);

            if (!(result = mqtt.ping()))
            {
                mqtt.disconnect();
            }

            return result;
        }

        const std::string MqttListenDevice::getLastMsg() const
        {
            return _last_msg;
        }
    } //inline namespace ESP

}//namespace Connection
#endif //ifdef ESP8266
    
static std::string feedToString(const std::string &feed_name)
{
    if(!feed_name.empty() && feed_name[0] == '/')
    {
        return std::string(IO_USER + feed_name);
    }

    std::string feed(IO_USER);
    feed += '/' + feed_name;
    return feed;
}
