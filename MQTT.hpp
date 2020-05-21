#ifndef MQTT_H
#define MQTT_H


/************************************************
 *  Libraries
 ***********************************************/
#ifndef Adafruit_MQTT_Client
#include <Adafruit_MQTT_Client.h>
#endif

#ifndef WiFiClient
#include <ESP8266WiFi.h>
#endif

#ifndef Adafruit_MQTT_Publish
#include <Adafruit_MQTT.h>
#endif

#ifndef uint8_t
#include <stdint.h>  //has also uint16_t and uint32_t
#endif  //uint8_t

namespace Connection
{
    extern WiFiClient client;
    extern Adafruit_MQTT_Client mqtt;

    const uint8_t max_retries(10u);
    enum class SignalCode : uint8_t;

    #ifdef ESP8266
    inline namespace ESP
    {
        void connectWLAN();
        bool connectBrokerMQTT();

        class MqttListenDevice;
    }
    #endif //ESP8266
    
    
}

enum class Connection::SignalCode : uint8_t
{
    NO_ERROR = 0u,
    BAD_PAYLOAD,            //For Incorrect Data in Payload
    BAD_PAYLOAD_SIZE,       //For Incorrect Number of Variables
    UNKNOWN_ERROR

};

#ifdef ESP8266

class Connection::ESP::MqttListenDevice final
{
    public:
    explicit MqttListenDevice(const uint8_t &device_id, const std::string &feed_name);
    void init();
    bool sendError(const SignalCode &err_code);
    bool sendAckMsg();
    bool listenForMsg(const int16_t &timeout);
    bool retainConnection();
    const std::string getLastMsg() const;

    private:
    const uint8_t _k_device_id;
    std::string _last_msg;
    
    Adafruit_MQTT_Publish _error_feed{&mqtt, IO_USER IO_ERROR_FEED};
    Adafruit_MQTT_Subscribe _listen_feed;

};


#endif  //ESP8266


#endif  //MQTT_H
