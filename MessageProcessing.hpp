#ifndef MESSAGE_PROCESSING_HPP
#define MESSAGE_PROCESSING_HPP

#ifndef string
#include <string>
#endif

#include "WS2812B.hpp"
#include "MQTT.hpp"

namespace Msg
{
    #ifdef ESP8266
    inline namespace Esp
    {
        bool dataProcessing(const std::string &msg,
                            WS2812B::RingLED &lamp,
                            Connection::Esp::MqttListenDevice &dev ,
                            Connection::SignalCode *p_sig_code);
    }// inline namespace Esp
    #endif //ESP8266
} //namespace Msg

#endif //MESSAGE_PROCESSING_HPP
