#include "WeatherClient.h"

//--------------------------
// 1. http = new HTPP
// 2. cacheTime in millis
//--------------------------
WeatherClient::WeatherClient(HTTPClient* http, uint32_t cacheTime)
{
    this->http = http;
    this->cacheTime = cacheTime;
}

bool WeatherClient::_init_(String city_name)
{
    http->begin("http://api.openweathermap.org/geo/1.0/direct?q=" + city_name + "&limit=1&appid="+APPID);

    int16_t http_code = http->GET();
    bool isSuccesful = http_code == 200;
    if (isSuccesful)
    {
        String payload = http->getString();

        StaticJsonDocument<100> filter;
        filter[0]["lat"] = true;
        filter[0]["lon"] = true;

        StaticJsonDocument<100> doc;
        deserializeJson(doc, payload, DeserializationOption::Filter(filter));

        _lat = doc[0]["lat"].as<double>();
        _lon = doc[0]["lon"].as<double>();
    }

    http->end();

    lastWeatherCheck = -2*cacheTime;
    lastForecastCheck = -2*cacheTime;
    return isSuccesful;
}

//-------------------------------------------------------------
// Must get already initcialized object
// 1. Weather weather = new Weather
// Returns boolean wheter obtaining weather info was successful
//--------------------------------------------------------------
bool WeatherClient::current_weather(Weather* weather)
{
    if (millis() - lastWeatherCheck < cacheTime)
    {
        return false;
    }
    
    http->begin("http://api.openweathermap.org/data/2.5/weather?lat="+String(_lat)+"&lon="+String(_lon)+"&units=metric&lang=pl&appid="+APPID);
    
    int16_t http_code = http->GET();
    String payload = http->getString();

    bool isSuccessfull = http_code == 200;

    if (isSuccessfull)
    {
        DynamicJsonDocument filter(440);
        filter["weather"][0]["main"] = true;
        filter["weather"][0]["icon"] = true;
        filter["main"]["temp"] = true;
        filter["main"]["feels_like"] = true;
        filter["main"]["pressure"] = true;
        filter["main"]["humidity"] = true;
        filter["wind"]["speed"] = true;
        filter["sys"]["sunrise"] = true;
        filter["sys"]["sunset"] = true;
        filter["dt"]=true;

        DynamicJsonDocument doc(440);
        deserializeJson(doc, payload, DeserializationOption::Filter(filter));
        weather
            ->feels_like(doc["main"]["feels_like"].as<double>())
            ->main(doc["weather"][0]["main"].as<String>())
            ->icon(doc["weather"][0]["icon"].as<String>())
            ->pressure(doc["main"]["pressure"].as<uint16_t>())
            ->humidity(doc["main"]["humidity"].as<uint8_t>())
            ->temp(doc["main"]["temp"].as<double>())
            ->wind_speed(doc["wind"]["speed"].as<double>())
            ->sunrise(doc["sys"]["sunrise"].as<uint32_t>())
            ->sunset(doc["sys"]["sunset"].as<uint32_t>())
            ->dt(doc["dt"].as<uint32_t>());
        doc.clear();
        filter.clear();
    }
    http->end();
    lastWeatherCheck = millis();

    return isSuccessfull;
}

//----------------------------------
// 1. Forecast must be init -> new Forecast
// 2. forecasted_weather must be init -> forecast->forecasted_weather = new ...
// 3. forecast->number_of_forecasts = NUMBER_OF_HOURS_TO_FORECAST;
// Returns boolean wheter obtaining weather info was successful
//----------------------------------
bool WeatherClient::forecast_weather(Forecast* forecast)
{
    if (millis() - lastForecastCheck < cacheTime)
    {
        return false;
    }

    http->begin("http://api.openweathermap.org/data/2.5/forecast?lat="+String(_lat)+"&lon="+String(_lon)+"&units=metric&lang=pl&appid="+APPID);
    
    int16_t http_code = http->GET();
    String payload = http->getString();

    bool isSuccessfull = http_code == 200;

    if (isSuccessfull)
    {
        DynamicJsonDocument filter(FORECAST_CAPACITY);
        for (uint8_t i=0;i<NUMBER_OF_HOURS_TO_FORECAST;i++)
        {
            filter["list"][i]["weather"][0]["main"] = true;
            filter["list"][i]["weather"][0]["icon"] = true;
            filter["list"][i]["main"]["temp"] = true;
            filter["list"][i]["main"]["feels_like"] = true;
            filter["list"][i]["main"]["pressure"] = true;
            filter["list"][i]["main"]["humidity"] = true;
            filter["list"][i]["wind"]["speed"] = true;
            filter["list"][i]["dt"] = true;
            filter["list"][i]["pop"] = true;

            DynamicJsonDocument doc(FORECAST_CAPACITY);
            DeserializationError err = deserializeJson(doc, payload, DeserializationOption::Filter(filter));


            forecast->forecasted_weather[i]
                ->feels_like(doc["list"][i]["main"]["feels_like"].as<double>())
                ->main(doc["list"][i]["weather"][0]["main"].as<String>())
                ->icon(doc["list"][i]["weather"][0]["icon"].as<String>())
                ->pressure(doc["list"][i]["main"]["pressure"].as<uint16_t>())
                ->humidity(doc["list"][i]["main"]["humidity"].as<uint8_t>())
                ->temp(doc["list"][i]["main"]["temp"].as<double>())
                ->wind_speed(doc["list"][i]["wind"]["speed"].as<double>())
                ->pop(doc["list"][i]["pop"].as<double>())
                ->dt(doc["list"][i]["dt"].as<uint32_t>());
        }
        filter.clear();
    }
    http->end();
    lastForecastCheck = millis();

    return isSuccessfull;
}