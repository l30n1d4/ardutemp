# ardutemp

Arduino UNO R4 WiFi with dht22 module for temperature and humidity and RTC time to make HTTP requests to endpoints


## Screenshots

![ardutemp](https://github.com/l30n1d4/ardutemp/blob/main/ardutemp_1.jpg)


## Features

- read temperature / humidity from DHT22 sensor
- display temperature on Arduino UNO R4 WiFi matrix led 12x8 (update every 10 seconds)
- use WiFi connection
- get RTC Time from NTP server
- HTTP POST request to hostname endpoint (every 10 minutes)


## Variables

To run this project, you will need to add the following variables to your arduino_secrets.h file

`SECRET_SSID`

`SECRET_PASS`


## Documentation

functions to display value in led matrix (value in format xx.x)
```cpp
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
```

function to get datetime in format YYYY-MM-DDTHH:MM:SS with zeros filled
```cpp
String getDateTime() {
  char datetime[19];
  RTCTime currentTime;
  RTC.getTime(currentTime); // get current time from RTC

  sprintf(datetime, "%04d-%02d-%02dT%02d:%02d:%02d", 
          currentTime.getYear(), 
          Month2int(currentTime.getMonth()), 
          currentTime.getDayOfMonth(), 
          currentTime.getHour(), 
          currentTime.getMinutes(), 
          currentTime.getSeconds());
  return datetime;
}
```

code to get tens, units and decimal to display in led matrix
```cpp
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
```
