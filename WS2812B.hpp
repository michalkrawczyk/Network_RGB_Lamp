#ifndef WS2812B_H
#define WS2812B_H

#ifndef uint8_t
#include <stdint.h> //has also uint16_t and uint32_t
#endif  //uint8_t

#ifndef Adafruit_NeoPixel
#include <Adafruit_NeoPixel.h>
#endif  //Adafruit_NeoPixel

#ifndef D6
static const uint8_t D6   = 12;
#endif

#include <vector>

namespace WS2812B
{
    class RingLED;
    enum class COLOR : uint8_t
    {
        RED = 16, 
        GREEN = 8, 
        BLUE = 0
    };
} //namespace WS2812B

class WS2812B::RingLED final
{
public:
    explicit RingLED(const uint8_t &led_count = 16,
                     const uint16_t &pin = D6);
    
    

    void init();
    void setBrightness(const uint8_t &brightness, const bool &show = false);
    void setColorLED(const uint16_t &pixel, const uint32_t &color, const bool &show = false);
    void setNewColorToAllPixels(const uint32_t &color,
                                const uint16_t &time_delay = 50,
                                const uint8_t &steps_count = 100);

    static uint8_t getColorComponent(const uint32_t &color, const COLOR &component);
    uint32_t getColorFromPixel(const uint16_t &pixel) const;

private:
    uint8_t _brightness{150u};
    Adafruit_NeoPixel _pixel_matrix;
    std::vector<uint32_t> _led_array; //Spotted problems with array from Adafruit library
    const uint16_t _k_led_count;

};

#endif  //WS2812_H
