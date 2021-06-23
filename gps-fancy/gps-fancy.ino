#include <M5StickC.h>　
#include "TinyGPS++.h"        // Downloaded lib

TinyGPSPlus gps;
HardwareSerial GPSRaw(2);

static int displayRefreshWait = 3000;
int lastRefresh = 0;
int currentView = 0;
int secondarySelection = 0;

void setup() {
  M5.begin(); 
  Serial.begin(9600);
  
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  GPSRaw.begin(9600, SERIAL_8N1, 33, 32);
}

void cellPosition(int row, int column, int cellHeight, int cellWidth, int result[]) {
  result[0] = column * cellWidth;
  result[1] = row * cellHeight;
}

void cellTextPosition(int row, int column, int cellHeight, int cellWidth, int textXOffset, int textYOffset, int result[]) {
  int cellPos[2] = {};
  cellPosition(row, column, cellHeight, cellWidth, cellPos);
  cellPos[0] += textXOffset;
  cellPos[1] += textYOffset;
}

void cellTextPosition(int cellPos [], int textXOffset, int textYOffset, int result[]) {
  result[0] = cellPos[0] + textXOffset;
  result[1] = cellPos[1] + textYOffset;
}

void drawTable(int columnCount, int rowCount, String values[]) {
  int cellHeight = 15;
  int textOffsetY = 5;
  int textOffsetX = 5;
  int cellWidth = 42;

  for(int row = 0; row < rowCount; row++) {
    for(int column = 0; column < columnCount; column++) {
      int currCellPosition[2] = {};
      cellPosition(row, column, cellHeight, cellWidth, currCellPosition);
      M5.Lcd.drawRect(currCellPosition[0], currCellPosition[1], cellWidth, cellHeight, WHITE);
      
      int cellTextPos[2] = {0, 0};
      cellTextPosition(currCellPosition, textOffsetX, textOffsetY, cellTextPos);
      M5.Lcd.setCursor(cellTextPos[0], cellTextPos[1]);
      M5.Lcd.printf(values[row * columnCount + column].c_str());
    }
  }
}

void pendingMessage() {
  int dotCount = (millis() / 1000) % 4;
  int x = 140;
  int y = 70;
  M5.Lcd.setCursor(x, y);
    M5.Lcd.print("    ");
  M5.Lcd.setCursor(x, y);
  for(int i = 0; i < dotCount; i++) {
    M5.Lcd.print(".");
  }
}

void refreshTime(int x, int y, TinyGPSPlus gps) {
  M5.Lcd.setCursor(x, y);
  M5.Lcd.printf("            ");
  M5.Lcd.setCursor(x, y);
  M5.Lcd.printf("%02i:%02i:%02i", gps.time.hour(), gps.time.minute(), gps.time.second());
}

void refreshTable(TinyGPSPlus gps) {

  String empty[] = { "Lat", "Lon", "Alt(m)", "Sats"
                     "      ", "      ", "      ", "      " };

  drawTable(3, 2, empty);
  
  String values[] = { 
                              "Lat",                           "Lon",                    "Alt(m)",                    "Sats",
                      String(gps.location.lat()), String(gps.location.lng()), String(gps.altitude.meters()), String(gps.satellites.value())
                    };
  drawTable(4, 2, values);
}

void largeSpeed(TinyGPSPlus gps) {
  if(secondarySelection > 3) secondarySelection = 0;


  float speedVal = 0.0;
  char* speedName = "?";
  switch(secondarySelection) {
      case 0:
          speedVal = gps.speed.knots();
          speedName = "kts";
        break;
      case 1:
          speedVal = gps.speed.mph();
          speedName = "mph";
        break;
      case 2:
          speedVal = gps.speed.mps();
          speedName = "m/s";
        break;
      case 3:
          speedVal = gps.speed.kmph();
          speedName = "km/h";
        break;
  }
  
  M5.Lcd.setCursor(0,0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(3);
  M5.Lcd.println("");
  M5.Lcd.printf("   %.2f",speedVal);
  M5.Lcd.setTextSize(1);
  M5.Lcd.println("");
  M5.Lcd.println(speedName);
}

void largeCourse(TinyGPSPlus gps) {
  M5.Lcd.setCursor(0,0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(3);
  M5.Lcd.println("");
  M5.Lcd.printf(" %.2f",gps.course.deg());
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.println("course (deg)");
}

void batteryFullScreen() {
  float maxVoltage = 4.183;
  float minVoltage = 3;
  float voltage = M5.Axp.GetBatVoltage();
  float amountAboveMin = voltage - minVoltage;
  float voltageSpread = maxVoltage - minVoltage;
  float percent = (amountAboveMin / voltageSpread);
  // 80 * 160 resolution
  float width = 160 * percent;
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.fillRect(0, 0, width, 80, GREEN);

  M5.Lcd.setCursor(0,0);
  M5.Lcd.printf("Bat:\r\n  V: %.3fv  I: %.3fma\r\n (%.2fpc)", voltage, M5.Axp.GetBatCurrent(), percent);
}

void uiLoop(TinyGPSPlus gps) {
  
  switch (currentView) {
      case 0: // Table view
        M5.Lcd.setTextSize(1);
        refreshTable(gps);
        refreshTime(0, 70, gps);
        break;
      case 1: // speed, big
        largeSpeed(gps);
        refreshTime(0, 70, gps);
        break;
      case 2: // direction, big
        largeCourse(gps);
        refreshTime(0, 70, gps);
        break;
      case 3: // time, big
        M5.Lcd.setTextSize(3);
        refreshTime(10, 25, gps);
        break;
      case 4: // battery, big
        M5.Lcd.setTextSize(1);
        batteryFullScreen();
        break;
      default:
        currentView = 0;
        uiLoop(gps);
        break;
    }
  lastRefresh = millis();
}

bool canRefreshDisplay() {
  return lastRefresh + displayRefreshWait < millis();
}

void handleButtons(TinyGPSPlus gps) {
  M5.update();
  if(M5.BtnA.wasPressed()) {
    currentView++;
    secondarySelection = 0;
    M5.Lcd.fillScreen(BLACK);
    uiLoop(gps);
  }
  
  if(M5.BtnB.wasPressed()) {
    secondarySelection++;
    M5.Lcd.fillScreen(BLACK);
    uiLoop(gps);
  }
}

void loop() {
  while(GPSRaw.available()) {
    char x = GPSRaw.read();
    gps.encode(x);
    Serial.print(x); // output NMEA over serial
    
    if (gps.location.isUpdated() || gps.time.isUpdated()) {
      if(canRefreshDisplay()) {
        uiLoop(gps);
      }
    }
    handleButtons(gps);
  }
}
