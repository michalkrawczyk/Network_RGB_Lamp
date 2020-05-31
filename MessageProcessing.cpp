#include "MessageProcessing.hpp"


#ifdef ESP8266
  static bool checkID(const std::string &id_str, const Connection::Esp::MqttListenDevice &dev);


  static bool msgHandlerTurnOff(const std::string &msg);
#endif //ESP8266

  static bool msgHandlerRGB(const std::string &msg, 
                            WS2812B::RingLED &lamp, 
                            Connection::SignalCode *p_sig_code);
  
  static uint8_t getUint8FromMsg(const std::string &msg);

namespace Msg
{
  bool Esp::dataProcessing(const std::string &msg,
                            WS2812B::RingLED &lamp,
                            Connection::Esp::MqttListenDevice &dev ,
                            Connection::SignalCode *p_sig_code)
  {
      uint16_t pos(msg.size() + 1);
  
      for(uint16_t c = 0; c < msg.size(); ++c)
      {
          if (msg[c] == '#' || msg[c] == '!' || msg[c] == '@')
          {
              pos = c;
              break;
          }
      }
      
      if(pos > 3 || !checkID(msg.substr(0, pos), dev) || pos >= msg.size()) //id will not have more than 3 digits
      {
          *p_sig_code = Connection::SignalCode::BAD_DEVICE_ID;
          return false;
      }
      std::string command(msg.substr(pos + 1, msg.size()));
  
      switch (msg[pos])
      {
          case '#':
          {
              return msgHandlerRGB(command, lamp, p_sig_code);
          }
          case '!':
          {
              return msgHandlerTurnOff(command);
          }
          default:
              break;
      }
  
      *p_sig_code = Connection::SignalCode::UNKNOWN_ERROR;
      return false;
  }
}// namespace Msg



static uint8_t getUint8FromMsg(const std::string &msg)
{
    uint8_t num(0);

    while (num < msg.size())
    {
        if(num > 2 || !isdigit(msg[num]))
        {
            break;
        }
        ++num;
    }
    uint16_t temp(strtoul(msg.substr(0, num).c_str(), nullptr, 10));
    
    return (temp > 0xff) ? 0xff : temp;
}

static bool msgHandlerRGB(const std::string &msg, WS2812B::RingLED &lamp, Connection::SignalCode *p_sig_code)
{
    uint8_t r(0),
            g(0),
            b(0);

    if(!(msg.size() <= 11 && msg.size() >= 5 ))
    {
        *p_sig_code = Connection::SignalCode::BAD_PAYLOAD_SIZE;
        return false;
    }

    if(!isdigit(msg[0]))
    {
        *p_sig_code = Connection::SignalCode::BAD_PAYLOAD;
        Serial.println("NOT DIGIT");
        return false;
    }
    
    uint8_t dot_positions[2] = {12,12};
    uint8_t dot_index(0);

    for(uint8_t pos = 0; pos < msg.size() -1 ; ++pos)
    {
        if(msg[pos] == ',' && isdigit(msg[pos + 1]))
        {
            dot_positions[dot_index] = pos;
            ++dot_index;
        }
    }

    if(dot_positions[1] != 12)
    {
        r = getUint8FromMsg(msg.substr(0, dot_positions[0]));
        g = getUint8FromMsg(msg.substr(dot_positions[0] + 1, dot_positions[1]));
        b = getUint8FromMsg(msg.substr(dot_positions[1] + 1, msg.size()));

        lamp.setNewColorToAllPixels(Adafruit_NeoPixel::Color(r, g, b), 20);

        *p_sig_code = Connection::SignalCode::NO_ERROR;
        return true;
    }
    
    *p_sig_code = Connection::SignalCode::BAD_PAYLOAD;
    return false;
}



#ifdef ESP8266
  static bool checkID(const std::string &id_str, const Connection::Esp::MqttListenDevice &dev)
  {
      return dev.compareID(getUint8FromMsg(id_str));
  }

    static bool msgHandlerTurnOff(const std::string &msg)
    {
        //TODO: Rewrite Later
        if(msg[0] == '0')
        {
            ESP.deepSleep(ESP.deepSleepMax());
            
            return true;
        }

    }
#endif //ESP8266
