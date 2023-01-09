#include "src/data_structures/Hsv_to_rgb.h"
#include "src/calibrate/calibrate_data.h"
#include "src/data_structures/Point.h"
#include "src/output/icons/Icons.h"
#include "src/weather_client/WeatherClient.h"
#include "src/output/screens/CurrentWeatherScreen.h"
#include "src/output/screens/MainScreen.h"
#include "src/output/screens/Forecast12Screen.h"
#include "src/output/screens/FewDaysForecastScreen.h"
#include "src/output/screens/WiFiListScreen.h"
#include "src/output/screens/PasswordInputScreen.h"
#include "src/output/icons/ScreenPointItem.h"
#include "src/input/TouchScreen.h"


#include <EEPROM.h>
#include <TFT_eSPI.h> 
#include <SPI.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define EEPROM_SIZE 6*sizeof(String)+sizeof(uint8_t)

#define BACKGROUND_COLOR 0x10C4
#define X_SCREENS 3
#define Y_SCREENS 1
#define SCREEN_LIST 2
#define MINUTES_15 900000
#define MINUTE 60000
#define SECOND 1000

TFT_eSPI tft = TFT_eSPI();

//const char* ssid =  "bc772c"; //"bc772c"; //"Black Shark";   // "NETIASPOT-2,4GHz-69C140"; // bc772c
//const char* password =  "269929817"; //"269929817"; //"12345abc";   //"6epTdSSVW22X"; // 269929817
const String current_weather = "https://api.openweathermap.org/data/2.5/weather?lat=50.95709295&lon=17.290269769664455&units=metric&lang=pl&appid=";
const String key = "6a0b31b6c9c1f95d47860092dadc1f6c";

uint8_t number_of_tries = 0;

HTTPClient http;
bool get_http;

WeatherClient wclient(&http, MINUTES_15);
Weather* weather;
Forecast* forecast;

enum Move_idx
{
    UP,
    DOWN,
    RIGHT,
    LEFT
};

uint32_t lastTimeCheck = 0;
uint32_t last_15_minCheck = 0;

TouchScreen ts(&tft, calData);

MainScreen*** screens = new MainScreen**[X_SCREENS]{
    new MainScreen* [Y_SCREENS] {new CurrentWeatherScreen(&tft, BACKGROUND_COLOR)},  // [0][0]
    new MainScreen* [Y_SCREENS] {new Forecast12Screen(&tft, BACKGROUND_COLOR)},       // [1][0]
    new MainScreen* [Y_SCREENS] {new FewDaysForecastScreen(&tft, BACKGROUND_COLOR)}
};
ScreenPointItem sci(&tft, 150, 230, BACKGROUND_COLOR);

void refresh();

WiFiScreen** wifi_screens = new WiFiScreen* [2]{
    new WiFiListScreen (&tft, BACKGROUND_COLOR, refresh),
    new PasswordInputScreen(&tft, BACKGROUND_COLOR)
};


Point screen_idx(0,0);
uint8_t wifi_screen_idx = 0;

bool try_to_connect_to_wifi(String wifi_ssid)
{
    Serial.println("Connecting...");
    tft.println("Connecting to WiFi: "+wifi_ssid+" ");

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
        tft.println(".");
        number_of_tries++;

        if (number_of_tries == 6){
            Serial.println("[-] Failed to connect to WiFi.");
            tft.println("[-] Failed to connect to WiFi.");
            return false;
        }
    }
    Serial.println("[+] Connected to the Wifi");
    tft.println("[+] Connected to the Wifi");
    return true;
}

void print_touch()
{
    Serial.println("TOUCHING!!!");
}

void refresh()
{
    wifi_screens[0]->clear_buttons();
    wifi_screens[0]->scan();
    wifi_screens[0]->draw();
}

void up()
{

}

void down()
{

}

void left()
{
    move(LEFT);
}

void right()
{
    move(RIGHT);
}

void move(uint8_t move)
{
    if (screen_idx.y == 0)
    {
        switch(move)
        {
            case LEFT:
                screen_idx.x = screen_idx.x > 0 ? screen_idx.x - 1 : 2;
                break;
            case RIGHT:
                screen_idx.x = screen_idx.x < 2 ? screen_idx.x + 1: 0;
                break;
            default:
                break;
        }
        if (screen_idx.x == 0)
        {
            tft.fillScreen(BACKGROUND_COLOR);
            screens[screen_idx.x][screen_idx.y]->draw(weather, true);
        }
        else
        {
            tft.fillScreen(BACKGROUND_COLOR);
            screens[screen_idx.x][screen_idx.y]->draw(forecast, true);
        }
    }

    sci.draw(3,1,screen_idx.x+1,1);
}

void wifi_setup()
{
    int16_t* pos = ts.read_buttons();

    if(pos)
    {
        wifi_screens[wifi_screen_idx]->check(pos);
        if (wifi_screens[wifi_screen_idx]->change())
        {
            wifi_screen_idx = wifi_screen_idx == 0 ? 1 : 0;

            tft.fillScreen(BACKGROUND_COLOR);
            if (wifi_screen_idx)
            {
                wifi_screens[wifi_screen_idx]->draw(wifi_screens[0]->get_str());
            }
            else{
                wifi_screens[wifi_screen_idx]->draw();
            }
        }
        delete [] pos;
    }
}

void setup()
{
    EEPROM.begin(EEPROM_SIZE);
    Serial.begin(921600);
    tft.init();
    tft.setRotation(3);

    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(1);

    //******************************
    // Read from EEPROM saved wifis
    //
    uint8_t count = EEPROM.read(10);

    uint32_t address = 10;
    address += sizeof(uint8_t);
    String saved_ssid, saved_psw;

    for (uint8_t i=0; i<count; i++)
    {
        saved_ssid = EEPROM.readString(address);
        address += sizeof(saved_ssid);
        saved_psw = EEPROM.readString(address);
        address += sizeof(saved_psw);

        char* temp_ssid = new char[saved_ssid.length()+1];
        char* temp_psw = new char[saved_psw.length()+1];
        
        saved_ssid.toCharArray(temp_ssid, saved_ssid.length()+1);
        saved_psw.toCharArray(temp_psw, saved_psw.length()+1);

        WiFi.mode(WIFI_STA);
        WiFi.begin(temp_ssid, temp_psw);

        if(try_to_connect_to_wifi(temp_ssid))
        {
            break;
        }
    }


    ts.on_down(down);
    ts.on_left(left);
    ts.on_right(right);
    ts.on_up(up);

    //******************************
    //  Force a connection to WiFi
    //
    if(WiFi.status() != WL_CONNECTED)
    {
        wifi_screens[0]->scan();
        wifi_screens[0]->draw();

        while(!wifi_screens[1]->load_main()){wifi_setup();}

        String temp_ssid = wifi_screens[1]->get_str(), temp_pwd = wifi_screens[1]->get_str();

        address = 10;
        count = EEPROM.read(address);
        address+=sizeof(uint8_t);
        EEPROM.writeString(address, temp_ssid);
        EEPROM.commit();

        address += sizeof(temp_ssid);
        EEPROM.writeString(address, temp_pwd);
        EEPROM.commit();

        address += sizeof(temp_pwd);
        count++;
        EEPROM.write(10, count);
        EEPROM.commit();
    }

    get_http = wclient._init_("Oława");
    tft.println("GET_HTTP: "+String(get_http));

    while(!get_http)
    {
        tft.println("Retrying.");
        get_http = wclient._init_("Oława");
        delay(3500);
    }

    tft.fillScreen(BACKGROUND_COLOR);

    weather = new Weather;
    forecast = new Forecast;
    forecast->number_of_forecasts = NUMBER_OF_HOURS_TO_FORECAST;
    forecast->forecasted_weather = new Weather* [NUMBER_OF_HOURS_TO_FORECAST];
    for (uint8_t i=0;i<NUMBER_OF_HOURS_TO_FORECAST; i++)
    {
        forecast->forecasted_weather[i] = new Weather;
    }

    wclient.current_weather(weather);
    wclient.forecast_weather(forecast);

    screens[0][0]->init();

    EEPROM.end();

    tft.fillScreen(BACKGROUND_COLOR);
    screens[screen_idx.x][screen_idx.y]->draw(weather, true);
    sci.draw(3,1,1,1);
}


void loop()
{
    ts.read();

    if (millis() - lastTimeCheck> SECOND)
    {
        wclient.current_weather(weather);
        wclient.forecast_weather(forecast);
        // drawing main screen time data
        screens[0][0]->refresh();
        if (screen_idx.x == 0)
        {
            screens[0][0]->draw(weather, false);
        }
        lastTimeCheck = millis();
    }
}