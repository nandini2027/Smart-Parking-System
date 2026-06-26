// SMART PARKING SYSTEM 

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <time.h>

// WIFI
const char* ssid = "Nandini";
const char* password = "12345678";

WiFiServer server(80);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// SERVO
Servo gateServo;

#define SERVO_PIN 4

int OPEN = 90;
int CLOSED = 25;

// IR SENSOR
#define IR_PIN 23

// SHIFT REGISTER
#define DATA 5
#define CLOCK 18
#define LATCH 19


// ULTRASONIC PINS


int trigPins[6] = {13, 14, 27, 26, 25, 33};
int echoPins[6] = {34, 35, 32, 12, 39, 36};

// SLOT NAMES
int slotNames[6] = {1, 3, 4, 5, 6, 2};

// VARIABLES
bool occupied[6];
bool reserved[6];

int blinkColor[6];

unsigned long leaveTimer[6];
unsigned long parkTimer[6];

bool previousIR = HIGH;

bool blinkState = false;

unsigned long previousBlink = 0;

bool lcdIdleShown = false;

int carCounter = 0;

// TIME VARIABLES

String checkInTime[6];
String checkOutTime[6];

unsigned long entryMillis[6];
unsigned long parkedDuration[6];

// DISTANCE FUNCTION

float getDistance(int trig, int echo) {

  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);

  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);

  if (duration == 0)
    return 999;

  return duration * 0.034 / 2;
}

// CURRENT TIME

String getCurrentTime() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {

    return "--:--:--";
  }

  char buffer[10];

  strftime(buffer,
           sizeof(buffer),
           "%H:%M:%S",
           &timeinfo);

  return String(buffer);
}

// SLOT PRIORITY
// S1 -> S2 -> S3 -> S6 -> S5 -> S4

int findNextSlot() {

  int priorityOrder[6] = {0, 5, 1, 4, 3, 2};

  for (int i = 0; i < 6; i++) {

    int idx = priorityOrder[i];

    if (!occupied[idx] &&
        !reserved[idx]) {

      return idx;
    }
  }

  return -1;
}

// LCD GRID


void displayGrid(int slotNumber) {

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("L1 ");

  if (slotNumber == 3)
    lcd.print("*");
  else
    lcd.print("3");

  lcd.print(" ");

  if (slotNumber == 2)
    lcd.print("*");
  else
    lcd.print("2");

  lcd.print(" ");

  if (slotNumber == 1)
    lcd.print("*");
  else
    lcd.print("1");

  lcd.setCursor(0, 1);

  lcd.print("L2 ");

  if (slotNumber == 4)
    lcd.print("*");
  else
    lcd.print("4");

  lcd.print(" ");

  if (slotNumber == 5)
    lcd.print("*");
  else
    lcd.print("5");

  lcd.print(" ");

  if (slotNumber == 6)
    lcd.print("*");
  else
    lcd.print("6");
}

// UPDATE LEDS


void updateLEDs() {

  if (millis() - previousBlink >= 500) {

    previousBlink = millis();

    blinkState = !blinkState;
  }

  byte sr1 = 0;
  byte sr2 = 0;

  
  // RESERVED BLINKING
  

  for (int i = 0; i < 6; i++) {

    if (reserved[i] && blinkState) {

      bool red = false;
      bool green = false;

      // GREEN

      if (blinkColor[i] == 0) {

        green = true;
      }

      // YELLOW

      else if (blinkColor[i] == 1) {

        green = true;
        red = true;
      }

      // RED

      else {

        red = true;
      }

      switch(i) {

        case 0:
          if (green) sr1 |= (1 << 0);
          if (red) sr1 |= (1 << 1);
          break;

        case 1:
          if (green) sr1 |= (1 << 2);
          if (red) sr1 |= (1 << 3);
          break;

        case 2:
          if (green) sr1 |= (1 << 4);
          if (red) sr1 |= (1 << 5);
          break;

        case 3:
          if (green) sr2 |= (1 << 0);
          if (red) sr2 |= (1 << 1);
          break;

        case 4:
          if (green) sr2 |= (1 << 2);
          if (red) sr2 |= (1 << 3);
          break;

        case 5:
          if (green) sr2 |= (1 << 4);
          if (red) sr2 |= (1 << 5);
          break;
      }
    }
  }

  
  // OCCUPIED RED
  

  for (int i = 0; i < 6; i++) {

    if (occupied[i]) {

      switch(i) {

        case 0:
          sr1 &= ~(1 << 0);
          sr1 |= (1 << 1);
          break;

        case 1:
          sr1 &= ~(1 << 2);
          sr1 |= (1 << 3);
          break;

        case 2:
          sr1 &= ~(1 << 4);
          sr1 |= (1 << 5);
          break;

        case 3:
          sr2 &= ~(1 << 0);
          sr2 |= (1 << 1);
          break;

        case 4:
          sr2 &= ~(1 << 2);
          sr2 |= (1 << 3);
          break;

        case 5:
          sr2 &= ~(1 << 4);
          sr2 |= (1 << 5);
          break;
      }
    }
  }

  digitalWrite(LATCH, LOW);

  shiftOut(DATA, CLOCK, MSBFIRST, sr2);
  shiftOut(DATA, CLOCK, MSBFIRST, sr1);

  digitalWrite(LATCH, HIGH);
}


// SENSOR CHECK


void checkParkingSensors() {

  for (int i = 0; i < 6; i++) {

    if (occupied[i] || reserved[i]) {

      float d =
        getDistance(trigPins[i],
                    echoPins[i]);

      bool detected = (d <= 6);

      
      // RESERVED -> OCCUPIED
      

      if (reserved[i] && detected) {

        if (parkTimer[i] == 0) {

          parkTimer[i] = millis();
        }

        // STAY FOR 2 SEC

        if (millis() - parkTimer[i] > 2000) {

          reserved[i] = false;

          occupied[i] = true;

          checkInTime[i] =
            getCurrentTime();

          checkOutTime[i] = "";

          entryMillis[i] =
            millis();

          updateLEDs();

          parkTimer[i] = 0;

          Serial.print("CAR PARKED AT SLOT ");

          Serial.println(slotNames[i]);
        }
      }

      else {

        parkTimer[i] = 0;
      }

      
      // OCCUPIED -> EMPTY
      

      if (occupied[i] && !detected) {

        if (leaveTimer[i] == 0) {

          leaveTimer[i] = millis();
        }

        if (millis() - leaveTimer[i] > 2000) {

          occupied[i] = false;

          leaveTimer[i] = 0;

          parkedDuration[i] =
            millis() - entryMillis[i];

          checkOutTime[i] =
            getCurrentTime();

          updateLEDs();

          Serial.print("CAR LEFT SLOT ");

          Serial.println(slotNames[i]);
        }
      }

      else if (occupied[i]) {

        leaveTimer[i] = 0;
      }
    }
  }
}

// SMART DELAY


void smartDelay(unsigned long ms) {

  unsigned long start = millis();

  while (millis() - start < ms) {

    updateLEDs();

    checkParkingSensors();

    delay(10);
  }
}


// GATE


void openGateGradually() {

  if (CLOSED < OPEN) {

    for (int pos = CLOSED;
         pos <= OPEN;
         pos++) {

      gateServo.write(pos);

      updateLEDs();

      checkParkingSensors();

      delay(5);
    }

  } else {

    for (int pos = CLOSED;
         pos >= OPEN;
         pos--) {

      gateServo.write(pos);

      updateLEDs();

      checkParkingSensors();

      delay(5);
    }
  }
}

void closeGateFast() {

  gateServo.write(CLOSED);
}


// SETUP


void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println("");

  Serial.println("WiFi Connected");

  Serial.print("IP Address: ");

  Serial.println(WiFi.localIP());

  server.begin();

  configTime(19800, 0, "pool.ntp.org");

  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.print("SMART PARKING");

  ESP32PWM::allocateTimer(0);

  gateServo.setPeriodHertz(50);

  gateServo.attach(SERVO_PIN,
                   500,
                   2400);

  gateServo.write(CLOSED);

  pinMode(IR_PIN, INPUT);

  pinMode(DATA, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);

  for (int i = 0; i < 6; i++) {

    pinMode(trigPins[i], OUTPUT);

    pinMode(echoPins[i], INPUT);

    occupied[i] = false;
    reserved[i] = false;

    leaveTimer[i] = 0;
    parkTimer[i] = 0;

    checkInTime[i] = "";
    checkOutTime[i] = "";

    parkedDuration[i] = 0;
  }
}


// LOOP


void loop() {

  updateLEDs();

  checkParkingSensors();

  
  // WEB SERVER
  

  WiFiClient client = server.available();

  if (client) {

    client.readStringUntil('\r');

    client.flush();

    int freeSlots = 0;

    for (int i = 0; i < 6; i++) {

      if (!occupied[i]) {

        freeSlots++;
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();

    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<head>");

    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");

    client.println("<meta http-equiv='refresh' content='2'>");

    client.println("<title>IoT Smart Parking</title>");

    
    // CSS
    

    client.println("<style>");

    client.println("body {");
    client.println("font-family: Arial;");
    client.println("background-color: #f2f2f2;");
    client.println("text-align: center;");
    client.println("padding: 20px;");
    client.println("}");

    client.println("h1 {");
    client.println("font-size: 52px;");
    client.println("}");

    client.println("h2 {");
    client.println("font-size: 34px;");
    client.println("margin-bottom: 30px;");
    client.println("}");

    client.println("table {");
    client.println("width: 95%;");
    client.println("margin: auto;");
    client.println("border-collapse: collapse;");
    client.println("background: white;");
    client.println("box-shadow: 0px 0px 10px gray;");
    client.println("}");

    client.println("th, td {");
    client.println("border: 1px solid gray;");
    client.println("padding: 16px;");
    client.println("font-size: 28px;");
    client.println("}");

    client.println("th {");
    client.println("background-color: #dddddd;");
    client.println("}");

    client.println(".occupied {");
    client.println("color: red;");
    client.println("font-weight: bold;");
    client.println("}");

    client.println(".available {");
    client.println("color: green;");
    client.println("font-weight: bold;");
    client.println("}");

    client.println(".reserved {");
    client.println("color: orange;");
    client.println("font-weight: bold;");
    client.println("}");

    client.println("</style>");

    client.println("</head>");

    client.println("<body>");

    client.println("<h1>IoT Smart Parking System</h1>");

    client.print("<h2>Total Free: ");

    client.print(freeSlots);

    client.println("</h2>");

    
    // TABLE
    

    client.println("<table>");

    client.println("<tr>");
    client.println("<th>Slot</th>");
    client.println("<th>Status</th>");
    client.println("<th>Check-In</th>");
    client.println("<th>Check-Out</th>");
    client.println("<th>Duration</th>");
    client.println("</tr>");

    
    // SLOT DATA
    

    for (int i = 0; i < 6; i++) {

      client.println("<tr>");

      // SLOT

      client.print("<td>");

      client.print(slotNames[i]);

      client.println("</td>");

      // STATUS

      client.print("<td>");

      if (occupied[i]) {

        client.println("<span class='occupied'>Occupied</span>");

      }

      else if (reserved[i]) {

        client.println("<span class='reserved'>Reserved</span>");

      }

      else {

        client.println("<span class='available'>Available</span>");
      }

      client.println("</td>");

      // CHECK IN

      client.print("<td>");

      if (checkInTime[i] != "") {

        client.print(checkInTime[i]);

      } else {

        client.print("--:--:--");
      }

      client.println("</td>");

      // CHECK OUT

      client.print("<td>");

      if (checkOutTime[i] != "") {

        client.print(checkOutTime[i]);

      } else {

        client.print("--:--:--");
      }

      client.println("</td>");

      // DURATION

      client.print("<td>");

      unsigned long duration;

      if (occupied[i]) {

        duration =
          millis() - entryMillis[i];

      } else {

        duration =
          parkedDuration[i];
      }

      unsigned long totalSeconds =
        duration / 1000;

      int hours =
        totalSeconds / 3600;

      int minutes =
        (totalSeconds % 3600) / 60;

      int seconds =
        totalSeconds % 60;

      char durationBuffer[15];

      sprintf(durationBuffer,
              "%02d:%02d:%02d",
              hours,
              minutes,
              seconds);

      client.print(durationBuffer);

      client.println("</td>");

      client.println("</tr>");
    }

    client.println("</table>");

    client.println("</body>");
    client.println("</html>");

    client.stop();
  }

  
  // IR SENSOR
  

  int irState =
    digitalRead(IR_PIN);

  if (irState == LOW &&
      previousIR == HIGH) {

    int targetIndex =
      findNextSlot();

    
    // PARKING FULL
    

    if (targetIndex == -1) {

      lcd.clear();

      lcd.print("PARKING FULL!");

      smartDelay(2000);

      lcdIdleShown = false;
    }

    else {

      reserved[targetIndex] = true;

      blinkColor[targetIndex] =
        carCounter % 3;

      carCounter++;

      int slotNo =
        slotNames[targetIndex];

      
      // WELCOME
      

      lcd.clear();

      lcd.setCursor(3, 0);

      lcd.print("WELCOME!");

      smartDelay(1000);

      
      // SLOT MESSAGE
      

      lcd.clear();

      if (slotNo == 1 ||
          slotNo == 2 ||
          slotNo == 3) {

        lcd.print("Park at L1 S");

      } else {

        lcd.print("Park at L2 S");
      }

      lcd.print(slotNo);

      lcd.setCursor(0, 1);

      if (blinkColor[targetIndex] == 0) {

        lcd.print("FOLLOW GREEN");
      }

      else if (blinkColor[targetIndex] == 1) {

        lcd.print("FOLLOW YELLOW");
      }

      else {

        lcd.print("FOLLOW RED");
      }

      smartDelay(2000);

      
      // GRID
      

      displayGrid(slotNo);

      smartDelay(2000);

      
      // GATE
      

      openGateGradually();

      smartDelay(700);

      closeGateFast();

      lcdIdleShown = false;
    }
  }

  previousIR = irState;

  
  // IDLE LCD
  

  if (!lcdIdleShown) {

    lcd.clear();

    lcd.setCursor(0, 0);

    lcd.print("SMART PARKING");

    lcdIdleShown = true;
  }

  delay(20);
}
