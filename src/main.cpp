#include <Arduino.h>
#include <U8g2lib.h>
#include <wifi.h>

//#define ASYNC_TCP_SSL_ENABLED 1  -- doesn't work on ESP32 yet.
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <SPIFFS.h>

#include <qrcode.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>


#define OLED_CLOCK 15 // Pins for the OLED display
#define OLED_DATA 4
#define OLED_RESET 16
U8G2_SSD1306_128X64_NONAME_F_HW_I2C g_OLED(U8G2_R2, OLED_RESET, OLED_CLOCK, OLED_DATA);

#ifndef WIFI_PWD
#pragma message "WIFI_PWD not defined!"
#endif

#ifndef WIFI_SSID2
#pragma message "WIFI_SSID2 not defined!"
#endif
const char WIFI_SSID[] = WIFI_SSID2;
const char WIFI_PASSWORD[] = WIFI_PWD;

// TODO - look at Async WiFi connection and connection manager - https://github.com/khoih-prog/ESPAsync_WiFiManager#features


//#include "ledgfx.h"         // helper functions from Dave Plummer
#include "comet.h"


/**
 * Parses color value "#RRGGBB" into a CGRB value
 */
CRGB ParseRGBA(const char* rrggbb)
{
    int i_hex = std::strtol(rrggbb + 1, nullptr, 16);
    return CRGB(i_hex);
}

int g_fontht = 0;
wl_status_t wifi_status = WL_DISCONNECTED;

AsyncWebServer server(80);



/*********************
 * Draws QR code for the specified URL at the specific location on the screen 
 * @param size size of QR code - typically 3.
 ******/
void DrawQRCode(const char *url, int ox, int oy, int size = 3)
{
    QRCode qrcode;

    // Allocate a chunk of memory to store the QR code
    uint8_t qrcodeBytes[qrcode_getBufferSize(3)];

    qrcode_initText(&qrcode, qrcodeBytes, 3, ECC_LOW, url);

    for (int y = 0; y < qrcode.size; y++)
    {
        for (int x = 0; x < qrcode.size; x++)
        {
            if (qrcode_getModule(&qrcode, x, y))
            {
                g_OLED.drawBox(ox + (x * size), oy + (y * size), size, size);
            }
        }
    }
}

//const char index_html[] PROGMEM = R"rawliteral(
//<!DOCTYPE HTML><html>
//<head>
//)rawliteral";

void scanNetworks()
{
    g_OLED.clearBuffer();
    int n = WiFi.scanNetworks();

    if (n == 0)
    {
        g_OLED.setCursor(0, g_fontht);
        g_OLED.print("Searching networks.");
    }
    else
    {

        for (int i = 0; i < n; ++i)
        {
            // Print SSID for each network found
            g_OLED.setCursor(0, g_fontht * (i + 1));
            char currentSSID[64];
            WiFi.SSID(i).toCharArray(currentSSID, 64);
            g_OLED.print(currentSSID);
        }
    }
    g_OLED.sendBuffer();
    // Wait a bit before scanning again
    delay(5000);
}


void HandleGetEffects(AsyncWebServerRequest *request)
{
    // See: https://arduinojson.org/ - for details on constructing JSON
    Serial.println("geteffects");
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument doc(1024);
    for (int i = 0; i < CFX._effects.size(); i++) {
        doc["effects"][i]["n"] = i;
        doc["effects"][i]["name"] = CFX._effects.at(i)->name;
    }
        
    serializeJson(doc, *response);
    request->send(response);    
}


void SendEffectProperties(AsyncResponseStream *response, CEffect *pEffect)
{
    DynamicJsonDocument doc(1024);

    PROPINFO * pProp = pEffect->getPropinfo();

    if (pProp != NULL) {
        int i = 0;
        while (pProp->_type != PropEnd) 
        {
            doc["props"][i]["n"] = i;
            doc["props"][i]["name"] = pProp->_name;
            doc["props"][i]["range"] = pProp->_range;
            doc["props"][i]["type"] = pProp->_type;
            doc["props"][i]["offset"] = pProp->_offset;
            if (pProp->_type == PropInteger) {
                int valInt = *((int*)(((byte*)pEffect) + pProp->_offset)); // do pointer arithmetic in bytes, but cast pointer back to correct type before de-ref.
                doc["props"][i]["value"] = valInt;
            } else if (pProp->_type == PropColor) {
                CRGB valClr = *((CRGB*)(((byte*)pEffect) + pProp->_offset));
                int intClr = (valClr.r <<16) | (valClr.g << 8) | valClr.b;
                char hex[10];
                sprintf(hex, "#%6.6X", intClr);
                doc["props"][i]["value"] = hex;
            }
            ++pProp;  // move to next
            ++i;
        }
    }
    serializeJson(doc, *response);
}


/****
 * Returns value of numeric parameter or -1 if invalid
 * @param request HTTP request
 * @param param name of parameter
 */
int GetNumericParam(AsyncWebServerRequest *request, const char* param) {
    if (request->hasParam("id"))
    {
        int id =  request->getParam("id")->value().toInt();   // toInt() returns 0 for non-valid input. 
        return id;        
    }        
    else
        return -1;
}

void HandleGetEffectProperties(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    SendEffectProperties(response, CFX.getActiveEffect());
    request->send(response);   
}

void HandleSetEffect(AsyncWebServerRequest *request) 
{
    int id = GetNumericParam(request, "id");
    if (id >= 0)
    {
        if (CFX.setActiveEffect(id))
        {
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            SendEffectProperties(response, CFX.getActiveEffect());
            request->send(response);   
        }
        return;
    }
    // fall thru - send an error
    request->send(400, "bad id parameter");
}


void HandleSetColor(AsyncWebServerRequest *request)
{
    // List all parameters
    int params = request->params();
    for (int i = 0; i < params; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isFile())
        { // p->isPost() is also true
            Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
        }
        else if (p->isPost())
        {
            Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
        else
        {
            Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
        request->send(200, "OK");
    }

    if (request->hasParam("color"))
    {
        const char* color_code = request->getParam("color")->value().c_str();
        CFX.color = ParseRGBA(color_code);
    }

}

void setup()
{
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);

    // start serial port for logging
    Serial.begin(9600);
    while (!Serial)
    {
    }
    Serial.println("ESP32 Startup");

    // start SPIFFS file system for webserver
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    g_OLED.begin();                         // One-time startup
    g_OLED.clear();                         // Clear the screen
    g_OLED.setFont(u8g2_font_profont15_tf); // Choose a suitable font
    g_fontht = g_OLED.getFontAscent() - g_OLED.getFontDescent();
 
    digitalWrite(LED_BUILTIN, LOW); // Turn off the Onboard LED
    int count = 0;
    while (wifi_status != WL_CONNECTED && count++ < 10)
    {
        g_OLED.clearBuffer();
        g_OLED.setCursor(0, g_fontht);
        g_OLED.printf("Connecting %.*s", count, "..........");
        g_OLED.sendBuffer();
        wifi_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        delay(3000);
    }
    Serial.println("Connected!\n");

    // Print the IP that was received via DHCP
    IPAddress ip = WiFi.localIP();

    g_OLED.clearBuffer();

    if (wifi_status == WL_CONNECTED)
    {
        digitalWrite(LED_BUILTIN, HIGH); // Turn on the Onboard LED when we connect
        const char *ipaddr = ip.toString().c_str();
        g_OLED.setCursor(64, g_fontht);

        g_OLED.printf("IP: %s", ipaddr);
        Serial.println(ipaddr);
        DrawQRCode(ipaddr, 0, 0, 2);
    }
    else
    {
        g_OLED.setCursor(0, g_fontht);
        g_OLED.print("not connected");
    }
    g_OLED.sendBuffer();

    // set up WebServer
    server.on("/api/setcolor", HTTP_GET, HandleSetColor);
    server.on("/api/geteffects", HTTP_GET, HandleGetEffects);
    server.on("/api/seteffect", HTTP_GET, HandleSetEffect); 
    server.on("/api/geteffectprops", HTTP_GET, HandleGetEffectProperties);
   
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
          { request->send(SPIFFS, "/index.html", "text/html"); });
    
    server.serveStatic("/scripts", SPIFFS, "/scripts");

// see: https://github.com/me-no-dev/ESPAsyncWebServer#respond-with-content-coming-from-a-file
// server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
//           { request->send_P(200, "text/html", index_html); });
// server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
//          { request->send(SPIFFS, "/index.html", "text/html"); });

// server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
//           { request->send(SPIFFS, "/style.css", "text/css"); });

    server.begin();

    CFX.init(30);           // initialize with 30 LEDs



}


void loop()
{
    CFX.getActiveEffect()->Draw();
    FastLED.delay(50);
}