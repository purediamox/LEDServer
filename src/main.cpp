#include <Arduino.h>
#include <U8g2lib.h>
#include <wifi.h>

//#define ASYNC_TCP_SSL_ENABLED 1  -- doesn't work on ESP32 yet.
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <SPIFFS.h>

#include <qrcode.h>

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

int g_fontht = 0;
wl_status_t wifi_status = WL_DISCONNECTED;

AsyncWebServer server(80);

void DrawQRCode(const char *url, int ox, int oy, int size)
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

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 WEB SERVER</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP32 WEB SERVER</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

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

void setup()
{
    // put your setup code here, to run once:
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);
    while (!Serial)
    {
    }
    Serial.println("ESP32 Startup");

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

    // WiFi.mode(WIFI_STA);

    int count = 0;
    while (wifi_status != WL_CONNECTED && count++ < 10)
    {
        g_OLED.clearBuffer();
        g_OLED.setCursor(0, g_fontht);
        g_OLED.printf("wifi_status %d", wifi_status);
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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send_P(200, "text/html", index_html); });
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/style.css", "text/css"); });

    server.begin();
}

void loop()
{
    // put your main code here, to run repeatedly:
}