#include <M5StickC.h>　
#include "TinyGPS++.h"        // Downloaded lib
#include <cmath>

TinyGPSPlus gps;
HardwareSerial GPSRaw(2);

static int displayRefreshWait = 3000;
int lastRefresh = 0;
int currentView = 0;
int secondarySelection = 0;

double destinationLat = 0.00;
double destinationLng = 0.00;
  
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

void drawNumber(uint16_t backColor, uint16_t foreColor, int cursorXPos, int cursorYPos, float posVal) {
  M5.Lcd.setTextColor(foreColor, backColor);
  M5.Lcd.setCursor(cursorXPos, cursorYPos);
  M5.Lcd.printf("%.1f", posVal);
}

void drawString(uint16_t backColor, uint16_t foreColor, int cursorXPos, int cursorYPos, char *text) {
  M5.Lcd.setTextColor(foreColor, backColor);
  M5.Lcd.setCursor(cursorXPos, cursorYPos);
  M5.Lcd.print(text);
}

void drawLatLngSelection(TinyGPSPlus gps) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10,10);
  
  // Selection is Lat (increase 0.1 (0), increase 10 (1), flip sign (2)), Lon (increase 0.1 (3), increase 10 (4), flip sign (5));
  // So 6 possible selections
  if(secondarySelection > 5) secondarySelection = 0;

  bool latHighlight = secondarySelection < 3;
  uint16_t selectionFore = BLACK;
  uint16_t selectionBack = WHITE;

  M5.Lcd.setTextSize(2);
  int leftIndent = 10;
  int secondIndent = 80;
  int topPad = 20;
  drawString(BLACK, WHITE, leftIndent, 5, "LAT");
  drawString(BLACK, WHITE, secondIndent, 5, "LON");
  if(latHighlight) {
    drawNumber(selectionBack, selectionFore, leftIndent, topPad, destinationLat);
    drawNumber(selectionFore, selectionBack, secondIndent, topPad, destinationLng);
  } else {
    drawNumber(selectionFore, selectionBack, leftIndent, topPad, destinationLat);
    drawNumber(selectionBack, selectionFore, secondIndent, topPad, destinationLng);
  }

  M5.Lcd.setTextSize(1);
  char *increaseTenthsText = "+0.1";
  char *increaseTensText = "+10";
  char *flipText = "*-1";
  char *toolMessages[3] = {
    increaseTenthsText, increaseTensText, flipText
  };
  int controlIndent = 30;
  int controlTopPad = 50;
  int toolSelection = secondarySelection % 3;
  for(int i = 0; i < 3; i++) {
    char *toolMsg = toolMessages[i];
    uint16_t fore = selectionBack;
    uint16_t back = selectionFore;
    if(i == toolSelection) {
      fore = selectionFore;
      back = selectionBack;
    }
    drawString(back, fore, controlIndent * (i + 1), controlTopPad, toolMsg);
  }

  M5.Lcd.setTextColor(WHITE, BLACK);
}

void capDestLatLng() {
  if(destinationLat < -90) destinationLat = -90;
  if(destinationLat > 90) destinationLat = 90;
  if(destinationLng < -180) destinationLng = -180;
  if(destinationLng > 180) destinationLng = 180;
}

bool btnAPressedSinceLastUiDraw() {
  float sinceChange = millis() - M5.BtnA.lastChange();
  // < 100 ensures no false positives
  return M5.BtnA.lastChange() > lastRefresh && sinceChange < 100;
}

void distanceToDestView(TinyGPSPlus gps) {
  if(btnAPressedSinceLastUiDraw()) {
    
    // Selection is Lat (increase (0), flip sign (1)), Lon (increase (2), flip sign (3));
    // So 4 possible selections
    float tenthChange = 0.1;
    float tenChange = 10.0;
    switch(secondarySelection) {
      case 0:
        destinationLat += tenthChange;
        break;
      case 1:
        destinationLat += tenChange;
        break;
      case 2:
        destinationLat *= -1.0;
        break;
      case 3:
        destinationLng += tenthChange;
        break;
      case 4:
        destinationLng += tenChange;
        break;
      case 5:
        destinationLng *= -1.0;
        break;
    }
    capDestLatLng();
  }
  
  drawLatLngSelection(gps);
    
  double distanceKm = gps.distanceBetween(
    gps.location.lat(),
    gps.location.lng(),
    destinationLat,
    destinationLng) / 1000.0;

  M5.Lcd.setCursor(50, 70);
  M5.Lcd.printf("Dist: %.3fkm", distanceKm);
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
      case 5:
        M5.Lcd.setTextSize(1.5);
        distanceToDestView(gps);
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

  // refresh on BtnA press to allow views to react. They have to do time comparisons though, as this resets the check (see drawLatLngSelection)
  if(M5.BtnA.wasPressed()) {
    uiLoop(gps);
  }
  
  // cycle screens when button pressed for > 1s
  if(M5.BtnA.pressedFor(1000)) {
    currentView++;
    secondarySelection = 0;
    M5.Lcd.fillScreen(BLACK);
    uiLoop(gps);
    delay(500); // cycle screens one every 0.5s whilst button is pressed
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
  }
  handleButtons(gps);
}
