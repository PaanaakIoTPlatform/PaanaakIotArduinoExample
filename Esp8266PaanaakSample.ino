#include <ESP8266WiFi.h>                                                                  // ESP8266 WiFi Library
#include <WiFiClient.h>                                                                   // WiFi client Library
#include <ESP8266HTTPClient.h>                                                            // HTTP Client Library

#include <OneWire.h>                                                                      // OneWire Library used in Dallas Temperature
#include <DallasTemperature.h>                                                            // Dallas Temperature Library


#define ONE_WIRE_BUS D1                                                                   // Temperature Sensor Pin D1
OneWire oneWire(ONE_WIRE_BUS);                                                            // Initialize OneWire                              
DallasTemperature sensors(&oneWire);                                                      // Initialize Temp Sensor


#define RLY1 D5                                                                           // RLY1 Pin D3
#define RLY2 D6                                                                           // RLY2 Pin D4

#define User_SSID  "MobinNet1122"                                                              // WiFi Router SSID
#define User_Pass  "44117517"                                                             // WiFi Router Password
#define ServerAddress  "http://paanaak-cloud.ir/api/device/devicecall?"                   // Server Address (Fixed)
#define SecretKey "17334a71-2988-4234-bf93-59c0b3a7713d"                                  // Device Secret Key (Unique for each device)
 

void setup(void)
{ 
  Serial.begin(115200);                                                                   // Begin Serial Port to Debug
  pinMode(RLY1,OUTPUT);                                                                   // define RLY1 Pin As Output
  pinMode(RLY2,OUTPUT);                                                                   // define RLY2 Pin As Output
  Serial.println("Dallas Temperature IC Control Library Demo");
  sensors.begin();                                                                        // Begin Sensor
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(User_SSID);
  WiFi.mode(WIFI_STA);                                                                    // Set WiFi Mode to Station
  WiFi.begin(User_SSID, User_Pass);                                                       // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {                                                 // Wait for WiFi to Connect
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");                                                       // WiFi Connected
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());                                                         // Print Local IP Address
}

void loop(void)
{ 
  String URL;

  // Read Temperature
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();                                                        // Send Command To Get Temperatures
  Serial.println("DONE");
  float tempC = sensors.getTempCByIndex(0);                                             // Read Temperature
  
  if(tempC != DEVICE_DISCONNECTED_C)                                                    // Successful Temperature Read
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);
    
    WiFiClient client;

    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    URL=ServerAddress;
    URL+="uniqueId=";
    URL+=UniqueId;
    URL+="&temp=";
    URL+=tempC;
    Serial.println(URL);
    if (http.begin(client, URL))    
    { 
        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();                                                        // HTTP GET for URL
        if (httpCode > 0) 
        {
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)        // HTTP GET Successsful
          {
            String payload = http.getString();                                            // Get Payload
            Serial.println(payload);                                                      // Print Payload
            if(payload[0]=='@' && payload[3]=='#')                                        // Check Relays' State and execute 
            {
                if(payload[1]=='1')                                                       // RLY1 is ON
                  digitalWrite(RLY1,HIGH);
                else if(payload[1]=='0')                                                  // RLY1 is OFF
                  digitalWrite(RLY1,LOW);

                if(payload[2]=='1')                                                       // RLY2 is ON
                  digitalWrite(RLY2,HIGH);
                else if(payload[2]=='0')                                                  // RLY2 is OFF
                  digitalWrite(RLY2,LOW);       
            }
          }
        } 
        else                                                                              // HTTP has Error
        {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();                                                                       // HTTP END
    } 
    else                                                                                  // HTTP Cannot Connect
    {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  } 
  else                                                                                    // Unsuccessful Temperature Read
  {
    Serial.println("Error: Could not read temperature data");
  }
  delay(6000);
}
