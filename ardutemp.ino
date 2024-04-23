#include <DHT.h>
#include <Arduino_LED_Matrix.h>
#include <WiFiS3.h>
#include <RTC.h>
#include <NTPClient.h>
#include "fonts.h"
#include "arduino_secrets.h" 

#define DHTPIN 10 // connect DHT sensor digital pin 10
#define DHTTYPE DHT22 // type of DHT sensor
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)

unsigned long lastTime = 0; // the following variables are unsigned longs because the time, measured in milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long timerDelay = 590500; // 10 minutes (600000) | 5 seconds (5000)
float lastTemp = 0;

WiFiClient client;
int wifiStatus = WL_IDLE_STATUS;
WiFiUDP Udp; // a UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

int HTTP_PORT = 80;
String HTTP_METHOD = "GET"; // or POST
char HOST_NAME[] = "insert_here_HOST_NAME";
String PATH_NAME = "insert_here_PATH_NAME";

DHT dht(DHTPIN, DHTTYPE);
ArduinoLEDMatrix matrix;

uint8_t frame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void setup() {
  Serial.begin(9600);
  dht.begin();
  matrix.begin();
  pinMode(12, OUTPUT); // sets the digital pin 12 as output
  digitalWrite(12, HIGH); // sets the digital pin 12 on ( +5v )

  connectToWiFi();
  RTC.begin();
  Serial.println("\nStarting connection to server...");
  timeClient.begin();
  timeClient.update();

  auto timeZoneOffsetHours = 2; // get the current date and time from an NTP server and convert it to UTC +1 by passing the time zone offset in hours. You may change the time zone offset to your local one.
  auto unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
  Serial.print("Unix time = ");
  Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);

  RTCTime currentTime; // retrieve the date and time from the RTC and print them
  RTC.getTime(currentTime); 
  Serial.println("The RTC was just set to: " + String(currentTime));
}

void loop() {
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  String queryString = "datetime=" + getDateTime() + "&temp=" + getOneDecimal(temp) + "&humi=" + getOneDecimal(humi);
  //Serial.println(queryString);

  if ((millis() - lastTime) > timerDelay) { // send an HTTP POST request every xx minutes
    connectToWiFi();
    if (client.connect(HOST_NAME, HTTP_PORT)) { // connect to web server on port 80
      Serial.println("Connected to server");
      client.println(HTTP_METHOD + " " + PATH_NAME + "?" + queryString + " HTTP/1.1"); // send HTTP header
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println(); // end HTTP header

      //client.println(queryString); // send HTTP body

      while (client.connected()) {
        if (client.available()) {
          char c = client.read(); // read an incoming byte from the server and print it to serial monitor
          Serial.print(c);
        }
      }

      client.stop(); // the server's disconnected, stop the client
      Serial.println();
      Serial.println("disconnected");
    } else { // if not connected
      Serial.println("connection failed");
    }
    lastTime = millis();
  }

  if (lastTemp != temp) {
    lastTemp = temp;
    int th, tz, te;
    int ti = 10 * temp;
    th = ti / 100;
    tz = ti % 100 / 10;
    te = ti % 10;
    clear_frame();
    add_to_frame(th, -2);
    add_to_frame(tz, 2);
    add_to_frame(te, 7);
    frame[6][7] = 1;
    display_frame();
  }

  delay(10000);
}

void printWifiStatus() {
  Serial.print("SSID: "); // print the SSID of the network you're attached to
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP(); // print your board's IP address
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI(); // print the received signal strength
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectToWiFi() {
  if (WiFi.status() == WL_NO_MODULE) { // check for the WiFi module
    Serial.println("Communication with WiFi module failed!");
    while (true); // don't continue
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (wifiStatus != WL_CONNECTED) { // attempt to connect to WiFi network
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    wifiStatus = WiFi.begin(ssid, pass); // connect to WPA/WPA2 network. Change this line if using open or WEP network
    delay(10000); // wait 10 seconds for connection
  }

  Serial.println("Connected to WiFi");
  printWifiStatus();
}

String getOneDecimal(float value) {
  char outValue[19];

  sprintf(outValue, "%.1f", value);
  return outValue;
}

String getDateTime() {
  char datetime[19];
  RTCTime currentTime;
  RTC.getTime(currentTime); // get current time from RTC

  sprintf(datetime, "%04d-%02d-%02dT%02d:%02d:%02d", currentTime.getYear(), Month2int(currentTime.getMonth()), currentTime.getDayOfMonth(), currentTime.getHour(), currentTime.getMinutes(), currentTime.getSeconds());
  return datetime;
}

void clear_frame() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 12; col++) {
      frame[row][col] = 0;
    }
  }
}

void display_frame() {
  matrix.renderBitmap(frame, 8, 12);
}

void add_to_frame(int index, int pos) {
  for (int row = 0; row < 8; row++) {
    uint32_t temp = fonts[index][row] << (7 - pos);
    for (int col = 0; col < 12; col++) {
      frame[row][col] |= (temp >> (11 - col)) & 1;
    }
  }
}