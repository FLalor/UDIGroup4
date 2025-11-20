//DOING -> TRACK HOW LONG SEAT HAS BEEN IN CURRENT STATE FOR

#define seatPin A0
#define redLED 7
#define yellowLED 8
#define greenLED 9

bool previousSeatState = true; // assumes person sitting down at the start (lazy)
bool currentSeatState;

const unsigned short engageTime = 5000; // 5 seconds of no one being on the seat

unsigned short sensorThreshold =  85; // change depending on sensor response
// ADD CALIBRATION (press button whilst seated to read how much the person + seat weighs)
unsigned long pressureCheckInterval = 200; // making these unsigned shorts is bad practice probably maybe but it'll be fine

// const int arraySize = engageTime/pressureCheckInterval;
// bool prevSeatStates[arraySize];

long previousCycleTime;
unsigned long seatTime;
unsigned long seatChangeStateTime;

bool checkSeat();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(seatPin, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT); 

  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);

  unsigned long startTime = millis();
  previousCycleTime = -1 * pressureCheckInterval;
  seatChangeStateTime = startTime;
  
  while (!Serial) {
    ;
  }

  Serial.println("System Engaged");
  Serial.println("Initial Conditions: ");
  Serial.print("Time: ");
  Serial.println(startTime);
  }

void loop() {
  // put your main code here, to run repeatedly:
  long newCycleTime = millis(); 
  Serial.println(newCycleTime);

  // CHECK SEAT EVERY pressureCheckInterval MILLISECONDS
  if ((newCycleTime - previousCycleTime) >= pressureCheckInterval) {
    Serial.print("New Active Cycle at Time: ");
    Serial.println(newCycleTime);
    currentSeatState = checkSeat();  
    //check if state is different than before
    if (currentSeatState == previousSeatState) { // same state as before
      seatTime = newCycleTime - seatChangeStateTime;

      if ((currentSeatState == true)) { // state is positive (person is sitting)
        digitalWrite(greenLED, HIGH);
        digitalWrite(yellowLED, LOW);
        digitalWrite(redLED, LOW);

      } else { // state is negative (person not sitting)
        // now check if its been engageTime milliseconds
        if (seatTime >= engageTime){ // seat has been off for more than engageTime milliseconds
          digitalWrite(greenLED, LOW);
          digitalWrite(yellowLED, LOW);
          digitalWrite(redLED, HIGH);
        } else { // seat has been off but not long enough
          digitalWrite(greenLED, LOW);
          digitalWrite(yellowLED, HIGH);
          digitalWrite(redLED, LOW);
        }
      }
    } else { // different state than before
      previousSeatState = currentSeatState; // <- should do nothing
      seatChangeStateTime = newCycleTime; 
      seatTime = 0; // seat just changed state and has been in new state for 0 milliseconds
    }
    
    previousCycleTime = newCycleTime;
  }
  //prepare for next iteration
  previousSeatState = currentSeatState;
  Serial.print("SeatTime: "); 
  Serial.println(seatTime);
}

bool checkSeat() {
  int sensorValue = analogRead(seatPin); // -> resistance directly(?) proportional to force applied 
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);
  if(sensorValue >= sensorThreshold) {
    Serial.print("Person Detected, Value: ");
    Serial.println(sensorValue);
    return true;
  } else {
    Serial.print("Person NOT Detected, Value: ");
    Serial.println(sensorValue);
    return false;
  }
}
