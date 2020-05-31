#include "WS2812B.hpp"

static float calculateStep(const uint8_t &start_value,
                        const uint8_t &final_value,
                        const uint16_t &step);

static int16_t floatToInt16(const float &value);

namespace WS2812B
{
RingLED::RingLED(const uint8_t &led_count, const uint16_t &pin):
_led_array(std::vector<uint32_t>(led_count)),
_pixel_matrix(Adafruit_NeoPixel(led_count, pin, NEO_GRB + NEO_KHZ800)),
_k_led_count(led_count)
{
}


void RingLED::init()
{
  Serial.println(F("Lamp Initialization \n"));
  _pixel_matrix.begin();
  _pixel_matrix.show();
  _pixel_matrix.setBrightness(_brightness);
  
  setColorLED(0, _pixel_matrix.Color(153,0,0));
  
  _pixel_matrix.show();
}

void RingLED::setColorLED(const uint16_t &pixel, const uint32_t &color, const bool &show)
{
    _pixel_matrix.setPixelColor(pixel, color);
    _led_array[pixel] = color;
    
    if(show)
    {
        _pixel_matrix.show();
    }
}

void RingLED::setNewColorToAllPixels(const uint32_t &color,
                            const uint16_t &time_delay,
                            const uint8_t &steps_count)
{

    const uint8_t k_arr_size(3);

    int16_t step[k_arr_size];
    float temp;

    uint16_t led;
    uint8_t i, col;

    int16_t rgb_now[k_arr_size];

    //function is simplified to it actual use - faster solution and less memory is used
    for (i = 0; i < k_arr_size ; ++i)
    {
        temp = calculateStep(getColorComponent(_led_array[_k_led_count - 1], static_cast<COLOR>(i * 8)),
                            getColorComponent(color, static_cast<COLOR>(i * 8)),
                            steps_count);
                
        step[i] = floatToInt16(temp);

        rgb_now[i] = getColorComponent(_led_array[_k_led_count - 1], static_cast<COLOR>(i * 8)) * 100;
    }
    
    uint32_t color_now;
    for (i = 0; i < steps_count; ++i)
    {
        for (col = 0; col < k_arr_size; ++col)
        {
            rgb_now[col] += step[col];
            rgb_now[col] = (rgb_now[col] < 0) ? 0 : rgb_now[col]; 
            rgb_now[col] = (rgb_now[col] > 25500) ? 25500 : rgb_now[col]; // >0xff after conversion
        }

        color_now = _pixel_matrix.Color(rgb_now[2] / 100, rgb_now[1] / 100, rgb_now[0] / 100);

        for (led = 0; led < _k_led_count; ++led)
        {
            setColorLED(led, color_now);
        }
        delay(time_delay);
        _pixel_matrix.show();
    }  
}

uint8_t RingLED::getColorComponent(const uint32_t &color, const COLOR &component)
{
    uint32_t pattern(0xffffff);
    pattern &= (0xff << (static_cast<uint8_t>(component)));

    return static_cast<uint8_t>((color & pattern) >> (static_cast<uint8_t>(component)));
}

uint32_t RingLED::getColorFromPixel(const uint16_t &pixel) const
{
    return _led_array[pixel];
}


} //namespace WS2812B

static float calculateStep(const uint8_t &start_value,
                        const uint8_t &final_value,
                        const uint16_t &step)
{
    return static_cast<float>(final_value - start_value) / step;
}

static int16_t floatToInt16(const float &value)
{
    //Note: This function is example of very bad casting, but perfect for RGB purpouses.
    auto result = static_cast<int16_t>(value * 100);
    return result;
}
