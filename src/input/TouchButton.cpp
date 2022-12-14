#include "TouchButton.h"

void do_nothing_button()
{
    return;
}

TouchButton::TouchButton(int16_t x, int16_t y, int16_t width, int16_t height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->color = 0x52AA;
    this->set_on_press(do_nothing_button);
}

void TouchButton::set_color(uint16_t color)
{
    this->color = color;
}

void TouchButton::set_on_press(void(*func_on_press)(void))
{
    this->on_press = func_on_press;
}


//********************************
// Checks if its button area has
// been touched
//********************************
bool TouchButton::check(int16_t x, int16_t y)
{
    if( (x >= this->x) && (x <= this->x + width))
    {
        if ((y >= this->y) && (y <= this->y + height))
        {
            on_touch();
            this->on_press();
            return true;
        }
    }
    return false;
}