#include "LED_WS2812B.hpp"

WS2812B::RingLED::RingLED(const uint8_t &led_count, const uint16_t &pin):
_led_array(std::vector<uint32_t>(led_count)),
_pixel_matrix(Adafruit_NeoPixel(led_count, pin, NEO_GRB + NEO_KHZ800)),
_k_led_count(led_count)
{
}



void WS2812B::RingLED::init()
{
  Serial.println(F("Lamp Initialization \n"));
  _pixel_matrix.begin();
  _pixel_matrix.show();
  setBrightness(_brightness);

  setColorLED(0, _pixel_matrix.Color(153,0,0));
  
  _pixel_matrix.show();
}

void WS2812B::RingLED::setBrightness(const uint8_t &brightness, const bool &show)
{
    _brightness = brightness;
    _pixel_matrix.setBrightness(_brightness);

    if(show)
    {
        _pixel_matrix.show();
    }
}

void WS2812B::RingLED::setColorLED(const uint16_t &pixel, const uint32_t &color, const bool &show)
{
    if(pixel < _led_array.size())
    {
        _pixel_matrix.setPixelColor(pixel, color);
        _led_array[pixel] = color;

        if(show)
        {
            _pixel_matrix.show();
        }
    }
}

void WS2812B::RingLED::setNewColorToAllPixels(const uint32_t &color,
                                              const uint16_t &time_delay,
                                              const uint8_t &steps_count)
{
    const uint8_t k_arr_size(3);
uint8_t final[k_arr_size] = {
            getColorComponent(color, COLOR::RED),
            getColorComponent(color, COLOR::GREEN),
            getColorComponent(color, COLOR::BLUE)
    };

    uint8_t  start[_k_led_count][k_arr_size];

    float step[_k_led_count][k_arr_size];

    for (uint16_t led = 0; led < _k_led_count; led++)
    {
        start[led][1] = getColorComponent(_led_array[led], COLOR::RED);
        start[led][2] = getColorComponent(_led_array[led], COLOR::GREEN);
        start[led][3] = getColorComponent(_led_array[led], COLOR::BLUE);

        step[led][0] = calculateStep(start[led][0], final[0], steps_count);
        step[led][1] = calculateStep(start[led][1], final[1], steps_count);
        step[led][2] = calculateStep(start[led][2], final[2], steps_count);
    }

    for (uint8_t cur_step = 0; cur_step < steps_count; cur_step++)
    {
        for (uint16_t led = 0; led < _k_led_count; led++)
        {
            setColorLED(led, _pixel_matrix.Color(static_cast<uint8_t>(start[led][0] + step[led][0] * cur_step),
                                                 static_cast<uint8_t>(start[led][1] + step[led][1] * cur_step),
                                                 static_cast<uint8_t>(start[led][2] + step[led][2] * cur_step)
            ));
        }

        delay(time_delay);
        _pixel_matrix.show();
    }

    for (uint16_t led = 0; led < _k_led_count; led++)
    {
        if(_led_array[led] != color)
        {
            setColorLED(led, color);
        }
    }

    _pixel_matrix.show();
}

uint8_t WS2812B::RingLED::getColorComponent(const uint32_t &color, const COLOR &component)
{

    uint8_t shift = (static_cast<uint8_t>(COLOR::BLUE) - static_cast<uint8_t>(component) ) * 8;
    uint32_t pattern = (static_cast<uint32_t>(0xff) << shift);

    uint8_t result = (static_cast<uint8_t>(color & pattern) >> shift);
    

    return (static_cast<uint8_t>(color & pattern) >> shift);
}

uint32_t WS2812B::RingLED::getColorFromPixel(const uint16_t &pixel) const
{
    return _led_array[pixel];
}

//private
float WS2812B::RingLED::calculateStep(const uint8_t &start_value,
                        const uint8_t &final_value,
                        const uint16_t &step)
{
  if ((final_value - start_value) == 0)
  {
    return 0.f;
  }
  
  return static_cast<float>(final_value - start_value) / step;
}
