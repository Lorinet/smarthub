#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP.h>
#define RelayPin 0
#define ControlPin 2
int CCS = LOW;
int OCS = LOW;
struct WiFiCredentials
{
  char ssid[64];
  char password[64];
}creds;
WiFiServer server(80);
String header;
String outputState = "off";
void setup() {
  pinMode(RelayPin, OUTPUT);
  pinMode(ControlPin, INPUT);
  digitalWrite(RelayPin, LOW);
  Serial.begin(115200);
  Serial.println();
  Serial.println("SmartLight v1.3");
  EEPROM.begin(512);
  EEPROM.get(0, creds);
  Serial.println("Loaded WiFi settings");
  Serial.print("SSID: ");
  Serial.println(creds.ssid);
  Serial.print("Password: ");
  Serial.println(creds.password);
  Serial.println("If you want to change WiFi configuration, type yes in 2 seconds:");
  delay(2000);
  String input = Serial.readString();
  input.trim();
  if(input == "yes")
  {
    Serial.println("Enter WiFi network name:");
    while(!Serial.available())
    {
    
    }
    String s = Serial.readString();
    s.trim();
    s.toCharArray(creds.ssid, 64);
    Serial.println("Enter password:");
    while(!Serial.available())
    {
    
    }
    s = Serial.readString();
    s.trim();
    s.toCharArray(creds.password, 64);
    EEPROM.put(0, creds);
    EEPROM.commit();
    Serial.println("WiFi settings saved.");
  }
  Serial.print("Connecting to " + getstr(creds.ssid) + " with password " + getstr(creds.password));
  WiFi.begin(creds.ssid, creds.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
void loop() {
  CCS = digitalRead(ControlPin);
  if(CCS != OCS)
  {
      if(CCS == LOW)
      {
        if(outputState == "off")
        {
          outputState = "on";
          digitalWrite(RelayPin, HIGH);
          Serial.println("Turned on with button");
        }
        else if(outputState == "on")
        {
          outputState = "off";
          digitalWrite(RelayPin, LOW);
          Serial.println("Turned off with button");
        }
      }
      OCS = CCS;
      delay(100);
  }
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /on") >= 0) 
            {
              Serial.println("Light on");
              outputState = "on";
              digitalWrite(RelayPin, HIGH);
            } 
            else if (header.indexOf("GET /off") >= 0) 
            {
              Serial.println("Light off");
              outputState = "off";
              digitalWrite(RelayPin, LOW);
            }
            else if(header.indexOf("GET /di") >= 0)
            {
              Serial.println("Device discovered by another SmartHub device.");
              client.println("smartlight12");
            }
            client.println("<!DOCTYPE html><html><head><title>SmartLight</title><link href=\"https://fonts.googleapis.com/css?family=Nunito\" rel=\"stylesheet\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><style>.button{background-color:#0061ff; padding: 15px; color: white; text-decoration:none; font-family: Nunito, Helvetica, sans-serif; font-size: 22px;} .button:hover{background-color: #116afa;} .button:active{background-color: #0055dd;} .heading{font-family: Nunito, Helvetica, sans-serif; color:white; font-size: 48px;} body{background-color:#040404; text-align:center; } </style></head><body><p class=\"heading\">Smart Light</p>");
            if(outputState == "off")
            {
              client.println("<a href=\"/on\" class=\"button\">Switch on</a>");
            }
            else if(outputState == "on")
            {
              client.println("<a href=\"/off\" class=\"button\">Switch off</a>");
            }
            client.println("</body></html>");
            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if(c != '\r')
        {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected");
    Serial.println("");
  }
}

String getstr(char a[])
{
  String s = "";
  for(int i = 0; i < 64; i++)
  {
    if(a[1] == 0x00 || a[i] == 0xFF)
    {
      break;
    }
    else
    {
      s += a[i];
    }
  }
  return s;
}

void externalIP()
{
  WiFiClient client;
  if (!client.connect("api.ipify.org", 80)) {
    Serial.println("Failed to connect with 'api.ipify.org' !");
  }
  else {
    int timeout = millis() + 5000;
    client.print("GET /?format=json HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    int size;
    while ((size = client.available()) > 0) {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = client.read(msg, size);
      Serial.write(msg, size);
      free(msg);
    }
  }
}
