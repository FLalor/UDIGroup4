#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ---------------- USER HARDWARE ----------------
#define seatPin A0
#define redLED 7
#define yellowLED 8
#define greenLED 9
#define buzzerPin 6
#define calibrationPin 0

Servo brakeServo;
int servoPin = 10;

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
// ------------------------------------------------

bool previousSeatState = true;
bool currentSeatState;

const unsigned short engageTime = 5000;
unsigned short sensorThreshold = 60;
unsigned long pressureCheckInterval = 200;

long previousCycleTime;
unsigned long seatTime;
unsigned long seatChangeStateTime;

// ---- BUZZER CONFIG ----
unsigned long buzzerCycleStart = 0;
const unsigned long buzzerOnTime = 2000;     // 2 sec ON
const unsigned long buzzerPeriod = 30000;    // 30 sec cycle
// -----------------------------------------------------------

bool checkSeat();

// =======================================================
void setup() {
  Serial.begin(9600);

  pinMode(seatPin, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(calibrationPin, INPUT_PULLUP);

  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(buzzerPin, LOW);

  brakeServo.attach(servoPin);
  brakeServo.write(90);   // brake OFF

  // LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // -------------------- STARTUP MESSAGE --------------------
  lcd.print("Tracto-Brake:");
  lcd.setCursor(0, 1);
  lcd.print("Stay Safe");
  delay(5000);
  lcd.clear();
  // ---------------------------------------------------------

  lcd.print("System Engaged");

  unsigned long startTime = millis();
  previousCycleTime = -1 * pressureCheckInterval;
  seatChangeStateTime = startTime;

  Serial.println("System Engaged");
}
// =======================================================

void loop() {
  long newCycleTime = millis();

  if ((newCycleTime - previousCycleTime) >= pressureCheckInterval) {

    currentSeatState = checkSeat();

    if (currentSeatState == previousSeatState) {
      seatTime = newCycleTime - seatChangeStateTime;

      // ------------------------------------------------------
      // ---------------- PERSON SITTING ----------------------
      // ------------------------------------------------------
      if (currentSeatState == true) {

        digitalWrite(greenLED, HIGH);
        digitalWrite(yellowLED, LOW);
        digitalWrite(redLED, LOW);
        analogWrite(buzzerPin, 100);   // buzzer OFF when sitting

        lcd.clear();
        lcd.print("Safety Brake:");
        lcd.setCursor(0, 1);
        lcd.print("OFF");

        brakeServo.write(0);

        if (digitalRead(calibrationPin) == LOW){
            sensorThreshold = analogRead(seatPin) * 0.9;
          }


      // ------------------------------------------------------
      // ------------ PERSON LEFT (CHECK TIME) ----------------
      // ------------------------------------------------------
      } else {

        // ---------------- BRAKE ENGAGE ----------------------
        if (seatTime >= engageTime) {

          digitalWrite(greenLED, LOW);
          digitalWrite(yellowLED, LOW);
          digitalWrite(redLED, HIGH);

          // --------------- BUZZER CYCLING --------------------
          unsigned long now = millis();
          unsigned long cyclePos = (now - buzzerCycleStart) % buzzerPeriod;

          if (cyclePos < buzzerOnTime) {
            analogWrite(buzzerPin, 100);    // ON for 2 seconds
          } else {
            digitalWrite(buzzerPin, LOW);     // OFF for the rest
          }
          // ----------------------------------------------------

          lcd.clear();
          lcd.print("Safety Brake:");
          lcd.setCursor(0, 1);
          lcd.print("ON");

          if (digitalRead(calibrationPin) == LOW){
            sensorThreshold = analogRead(seatPin) * 0.9;
          }

          brakeServo.write(90);


        // ---------------- WAITING (YELLOW LED) --------------
        } else {

          digitalWrite(greenLED, LOW);
          digitalWrite(yellowLED, HIGH);
          digitalWrite(redLED, LOW);

          digitalWrite(buzzerPin, LOW);   // buzzer OFF during waiting

          lcd.clear();
          lcd.print("Please Wait");
        }
      }

    } else {
      previousSeatState = currentSeatState;
      seatChangeStateTime = newCycleTime;
      seatTime = 0;
    }

    previousCycleTime = newCycleTime;
  }

  previousSeatState = currentSeatState;
}
// ====================================================


// ====================================================
// ------------------- SEAT CHECK ----------------------
// ====================================================
bool checkSeat() {
  int sensorValue = analogRead(seatPin);
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);

  if (sensorValue >= sensorThreshold) {
    Serial.println("Person Detected");
    return true;
  } else {
    Serial.println("Person NOT Detected");
    return false;
  }
}