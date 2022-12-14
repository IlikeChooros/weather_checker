#include "WiFiListScreen.h"

void WiFiListScreen::init()
{
    read_from_eeprom_wifis();
}

//-----------------------------------
// Scans for WiFis, creates new WiFi
// List Objects with WiFis data
// make sure to call clear_buttons()
// before using it
//----------------------------------
void WiFiListScreen::scan()
{
    change_ = false;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    number_of_networks = WiFi.scanNetworks();
    wifis = 0;

    if (number_of_networks <= 0)
    {
        return;
    }

    uint16_t x = 10, y = 20;
    uint8_t size_count = 1;

    number_of_networks = number_of_networks < 6 ? number_of_networks : 6;

    wifis = new WiFiListItem* [number_of_networks];
    
    for (uint8_t i=0;i<number_of_networks;i++)
    {
        String ssid = WiFi.SSID(i);
        bool is_saved = false;
        for (uint8_t j=0; j<number_of_saved_wifis; j++)
        {
            // If saved wifi has the same ssid as the scanned wifi
            Serial.println("FOUND WIFI: "+ssid + " SAVED: "+saved_wifi_info[j][0]);
            if (saved_wifi_info[j][0] == ssid)
            {
                is_saved = true;
            }
        }
        wifis[i] = new WiFiListItem(tft, x, y + i *(HEIGHT+OFFSET), WIDTH, HEIGHT, ssid, is_saved,WiFi.RSSI(i), bg_c);
    }
}

void WiFiListScreen::check(int16_t* pos)
{
    for (uint8_t i=0; i<number_of_networks; i++)
    {
        if (wifis[i]->check(pos[0], pos[1]))
        {
            picked_wifi = wifis[i]->get_ssid();
            if (wifis[i]->is_saved())
            {
                connect_to_wifi();
                return;
            }
            change_ = true;
            return;
        }
    }

    // Already got refresh function
    refresh_button->check(pos[0], pos[1]);
}

void WiFiListScreen::connect_to_wifi()
{
    String psw;

    // Look for saved wifi
    for (uint8_t i=0; i<number_of_saved_wifis; i++)
    {
        if (saved_wifi_info[i][0] == picked_wifi)
        {
            psw = saved_wifi_info[i][1];
            break;
        }
    }

    char* pass = new char [psw.length()+1];
    char* ssid = new char [picked_wifi.length()+1];

    psw.toCharArray(pass, psw.length()+1);
    picked_wifi.toCharArray(ssid, picked_wifi.length()+1);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    load_main_ = draw_connecting_to_wifi(picked_wifi, ssid, pass);
    change_ = !load_main_;
}


//*************************************
// Reads form eeprom saved 
// ssids and passwords
// then saves it as a String** pointer
//*************************************
void WiFiListScreen::read_from_eeprom_wifis()
{
    EEPROM.begin(EEPROM_SIZE);
    uint32_t address = 10;
    uint8_t count = EEPROM.read(address);
    this->number_of_saved_wifis = count;
    address += sizeof(uint8_t);
    Serial.println("COUNT: " +String(count));
    if (count)
    {
        saved_wifi_info = new String* [count];
    }
    

    String saved_ssid, saved_psw;

    for (uint8_t i=0; i<count; i++)
    {
        saved_ssid = EEPROM.readString(address);
        address += sizeof(saved_ssid);
        saved_psw = EEPROM.readString(address);
        address += sizeof(saved_psw);
        Serial.println(String(i)+". SSID: " + saved_ssid + " PASS: "+saved_psw);
        saved_wifi_info[i] = new String[2]{
            saved_ssid,
            saved_psw
        };

        Serial.println(String(i)+". SAVED SSID: " + saved_wifi_info[i][0] + " PASS: "+saved_wifi_info[i][1]);
    }

    EEPROM.end();
}

//---------------------------------
// returns true if this screen has to be
// changed to another one
//---------------------------------
bool WiFiListScreen::change()
{
    return this->change_;
}


//------------------------
// Before using it, make sure
// envoke scan() method
//------------------------
void WiFiListScreen::draw()
{
    for (uint8_t i=0; i<number_of_networks; i++)
    {
        wifis[i]->draw();
    }

    refresh_button->draw();
}

//*****************************
// Deletes all WiFi List Items
//*****************************
void WiFiListScreen::clear_buttons()
{
    if (number_of_networks<1)
    {
        return;
    }
    for (uint8_t i=0; i<number_of_networks; i++)
    {
        delete wifis[i];
    }
    delete [] wifis;
}