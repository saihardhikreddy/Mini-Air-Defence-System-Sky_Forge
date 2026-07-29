/*
 Auto Turret – 3 Servos + 1 Ultrasonic + LED + Buzzer
 - Radar servo    on D6: sweeps HC-SR04 0–180–0
 - Aim servo      on D5: points launcher toward target
 - Trigger servo  on D3: fires when target is close
 - HC-SR04 TRIG   -> D9
 - HC-SR04 ECHO   -> D10
 - LED            -> D12
 - Buzzer         -> D2
*/

#include <Servo.h>  



// ---------- Pins ----------
const int TRIG_PIN          = 9;
const int ECHO_PIN          = 10;
const int SERVO_RADAR_PIN   = 6;   
const int SERVO_AIM_PIN     = 5;  
const int SERVO_TRIGGER_PIN = 3;   
const int LED_PIN           = 12;  
const int BUZZER_PIN        = 2;   



// ---------- Sweep & Scan Settings ----------
const int SCAN_MIN_ANGLE    = 0;
const int SCAN_MAX_ANGLE    = 180;

// You set SCAN_STEP = 20:
const int SCAN_STEP         = 9;   
const int STEP_DELAY_MS     = 8;   
const int speed             = 100;
const int SAMPLES_PER_READ  = 2;
const int MAX_DISTANCE_CM   = 30;

// ---------- Detection distances ----------
const int DETECT_DISTANCE_CM = 30;  
const int FIRE_DISTANCE_CM   = 30;   
// ---------- Aiming & Trigger ----------
const int AIM_ANGLE_OFFSET   = 0;    

// Trigger servo angles – TUNE FOR YOUR MECHANISM
const int FIRE_REST_ANGLE    = 80;   
const int FIRE_SHOOT_ANGLE   = 15;   
const int FIRE_HOLD_MS       = 300;  

const int COOLDOWN_MS        = 1500; 

// ---------- Servos ----------
Servo servoRadar;
Servo servoAim;
Servo servoTrigger;

unsigned long lastShotTime = 0;

// Sweep state (0–180–0)
int scanAngle = SCAN_MIN_ANGLE;
int scanDir   = +1; 



// ---------- Ultrasonic helpers ----------
long readDistanceOnce() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  if (duration == 0) return MAX_DISTANCE_CM + 1;

  long cm = duration / 29 / 2;
  if (cm < 0) return MAX_DISTANCE_CM + 1;
  return cm;
}

long readDistanceAvg(int samples) {
  long sum = 0;
  int good = 0;

  for (int i = 0; i < samples; i++) {
    long d = readDistanceOnce();
    if (d > 0 && d <= MAX_DISTANCE_CM) {
      sum += d;
      good++;
    }
    delay(20);
  }

  if (good == 0) return MAX_DISTANCE_CM + 1;
  return sum / good;
}



// ---------- Smooth aim servo move ----------
void smoothMoveServo(Servo &s, int fromA, int toA, int stepDelay = 2) {
  fromA = constrain(fromA, 0, 180);
  toA   = constrain(toA,   0, 180);

  if (fromA < toA) {
    for (int a = fromA; a <= toA; a++) {
      s.write(a);
      delay(stepDelay);
    }
  } else {
    for (int a = fromA; a >= toA; a--) {
      s.write(a);
      delay(stepDelay);
    }
  }
}



// ---------- Engage sequence at given angle ----------
void engageAtAngle(int angleDeg, long distanceCm) {
  Serial.print("ENGAGE at angle ");
  Serial.print(angleDeg);
  Serial.print(" dist=");
  Serial.println(distanceCm);

  
  servoRadar.write(angleDeg);
  delay(80);

  
  int aimTarget = constrain(angleDeg + AIM_ANGLE_OFFSET, 0, 180);
  smoothMoveServo(servoAim, servoAim.read(), aimTarget, 2);

  
  long dConfirm = readDistanceAvg(SAMPLES_PER_READ);
  Serial.print("Confirm dist=");
  Serial.println(dConfirm);

  if (dConfirm <= FIRE_DISTANCE_CM) {
    unsigned long now = millis();
    if (now - lastShotTime > COOLDOWN_MS) {
      Serial.println("FIRE!");

      
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      
      servoTrigger.write(FIRE_SHOOT_ANGLE);
      delay(FIRE_HOLD_MS);
      servoTrigger.write(FIRE_REST_ANGLE);

      
      digitalWrite(LED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);

      lastShotTime = now;
    } else {
      Serial.println("Cooldown, not firing yet.");
    }
  } else {
    Serial.println("Target moved / out of fire range.");
  }

  delay(300); 
}



// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  servoRadar.attach(SERVO_RADAR_PIN);
  servoAim.attach(SERVO_AIM_PIN);
  servoTrigger.attach(SERVO_TRIGGER_PIN);

  servoRadar.write(scanAngle);
  servoAim.write(scanAngle + AIM_ANGLE_OFFSET);
  servoTrigger.write(FIRE_REST_ANGLE);

  delay(500);
  Serial.println("Auto Turret Ready (LED + Buzzer enabled)");
}



// ---------- Main loop ----------
void loop() {
  
  servoRadar.write(scanAngle);
  delay(STEP_DELAY_MS);

  
  long d1 = readDistanceAvg(2);
  long d2 = readDistanceAvg(2);
  long d3 = readDistanceAvg(2);

  long d = MAX_DISTANCE_CM + 1;  

  
  if (d1 <= DETECT_DISTANCE_CM && d2 <= DETECT_DISTANCE_CM) {
    d = (d1 + d2) / 2;
  } else if (d1 <= DETECT_DISTANCE_CM && d3 <= DETECT_DISTANCE_CM) {
    d = (d1 + d3) / 2;
  } else if (d2 <= DETECT_DISTANCE_CM && d3 <= DETECT_DISTANCE_CM) {
    d = (d2 + d3) / 2;
  }

  // -------- SEND DATA TO PROCESSING FOR RADAR VISUALIZER --------
  long dispDist = d1;  // choose d1 as the display distance
  if (dispDist < 0 || dispDist > 30) {
    dispDist = 0;     
  }

  Serial.print("scan a=");
  Serial.print(scanAngle);
  Serial.print(" d=");
  Serial.println(dispDist);
  // ---------------------------------------------------------------

  
  Serial.print("angle=");
  Serial.print(scanAngle);
  Serial.print(" d1=");
  Serial.print(d1);
  Serial.print(" d2=");
  Serial.print(d2);
  Serial.print(" d3=");
  Serial.print(d3);
  Serial.print(" -> d=");
  Serial.println(d);

  
  if (d <= DETECT_DISTANCE_CM) {
    engageAtAngle(scanAngle, d);
  }

  
  scanAngle += scanDir * SCAN_STEP;

  if (scanAngle >= SCAN_MAX_ANGLE) {
    scanAngle = SCAN_MAX_ANGLE;
    scanDir = -1;
  } else if (scanAngle <= SCAN_MIN_ANGLE) {
    scanAngle = SCAN_MIN_ANGLE;
    scanDir = +1;
  }
}