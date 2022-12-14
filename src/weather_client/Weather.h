#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

class Weather
{
    public:

    String _main,
            _icon;
    double _temp,
            _feels_like,
            _wind_speed,
            _pop;
    uint8_t _humidity;
    uint16_t _pressure;
    uint32_t _sunrise,
            _sunset,
            _dt;
    
    Weather* main(String main);
    Weather* icon(String icon);
    Weather* temp(double temp);
    Weather* wind_speed(double wind_speed);
    Weather* feels_like(double feels_like);
    Weather* humidity(uint8_t humidity);
    Weather* pressure(uint16_t pressure);
    Weather* sunrise(uint32_t sunrise);
    Weather* sunset(uint32_t sunset);
    Weather* dt(uint32_t dt);
    Weather* pop(double pop);
};


#endif