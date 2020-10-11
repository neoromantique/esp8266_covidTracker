#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PASSWD"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "corona-stats.online";
const int httpsPort = 443;

const char fingerprint[] PROGMEM = "BE F3 A7 38 A8 0C EE B5 75 8E 1A 17 67 60 E3 9E 68 B1 30 3D";
StaticJsonDocument<4096> doc;

ESP8266WiFiMulti WiFiMulti;

String text;

//U8g2 Contructor
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);
// Alternative board version. Uncomment if above doesn't work.
// U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 4, /* clock=*/ 14, /* data=*/ 2);

u8g2_uint_t offset;     // current offset for the scrolling text
u8g2_uint_t width;      // pixel width of the scrolling text (must be lesser than 128 unless U8G2_16BIT is defined

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void setup(void) {
  Serial.begin(115200);


  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // text = "xyz";
  text = WiFi.localIP().toString();

  delay(500);
  
  u8g2.begin();

  u8g2.setFont(u8g2_font_5x7_tr); // set the target font to calculate the pixel width
  u8g2.setFontMode(0);    // enable transparent mode, which is faster
}



void loop(void) {

    int data_todayCases;
    // Use WiFiClientSecure class to create TLS connection
    WiFiClientSecure client;
    Serial.print("connecting to ");
    Serial.println(host);

    Serial.printf("Using fingerprint '%s'\n", fingerprint);
    client.setFingerprint(fingerprint);

    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");
      return;
    }

    String url = "/LT?format=json";
    Serial.print("requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "User-Agent: TestESP8266\r\n" +
                "Connection: close\r\n\r\n");

    Serial.println("request sent");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    String json = client.readStringUntil('\n');
    DeserializationError error = deserializeJson(doc, json);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

      Serial.println("json: ");
      Serial.println(json);
      JsonObject data = doc["data"][0];
      data_todayCases = data["todayCases"]; // 205
      Serial.println("cases: ");
      Serial.println(data_todayCases);


    u8g2.firstPage();
    do {
          u8g2.drawStr(0,10,"Current IP: ");
          u8g2.drawStr(65,10, text.c_str());
          u8g2.drawStr(0,20,"New Cases in Lithuania: ");
          u8g2.drawStr(40,30, String(data_todayCases).c_str());

        // u8g2.drawStr(0, 5, text);  // write something to the internal memory
        //u8g2.drawStr(0, 40, F("ORIGINAL IN FLASH").c_str());  // does not work
    } while (u8g2.nextPage());
    delay(600000);
}
