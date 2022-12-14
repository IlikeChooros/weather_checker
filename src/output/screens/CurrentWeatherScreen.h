#ifndef CURRENT_WEATHER_SCREEN_H
#define CURRENT_WEATHER_SCREEN_H

#include "MainScreen.h"
#include "../items/WeatherItem.h"
#include "../items/TextItem.h"
#include "../items/DateItem.h"
#include "../items/WiFiItem.h"
#include "../icons/WeatherIcon.h"
#include "../icons/WindIcon.h"
#include "../../weather_client/Weather.h"
#include "../../data_structures/Hsv_to_rgb.h"


#define DEGREE '`'
#define NUMBER_OF_WEATHER_ITEMS 6
#define OFFSET 70



class CurrentWeatherScreen: public MainScreen
{
    WeatherItem** weather_items;
    WiFiItem* wifi;
    DateItem* date;

    public:
    CurrentWeatherScreen(TFT_eSPI * tft, uint16_t bg_c): MainScreen(tft,bg_c){
        set_text_colors();
        
        weather_items = new WeatherItem*[NUMBER_OF_WEATHER_ITEMS] {
            new WeatherIcon(this->_tft,185,30+OFFSET, 125, bg_c),
            new WindIcon(this->_tft, 5, 105+OFFSET, 20, bg_c),
            new TextTemp(this->_tft, 30, 30+OFFSET, 4, 2, TFT_WHITE, "%d `C" , bg_c),
            new TextFeelsLike(this->_tft, 35, 80+OFFSET, 2, 1, TFT_WHITE,"Feels like: %d `C", bg_c),
            new TextWind(this->_tft, 30, 105+OFFSET, 4, 1, 0x77F2, "%d km/h", bg_c),
            new TextPressure(this->_tft, 30, 130+OFFSET, 4, 1, 0xB41F, "%d hPa", bg_c)
        }; 

        wifi = new WiFiItem(this->_tft, 0,0,15,bg_c);
        date = new DateItem(this->_tft, 160, 10 , 45, bg_c);
    }
    void draw(Weather* weather, bool forceDraw);
    void draw(Forecast* forecast, bool forceDraw)
    {
        return;
    }

    void refresh(bool forceUpdate);
    void init() {
        date->init();
    }
};


#endif