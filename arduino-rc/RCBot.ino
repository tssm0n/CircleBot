#define STATUS_LED 2

#define LEFT_ENABLE 6
#define RIGHT_ENABLE 11
#define LEFT_FORWARD 8
#define LEFT_BACK 7
#define RIGHT_FORWARD 9
#define RIGHT_BACK 10

#define RECEIVER_1 4
#define RECEIVER_2 3

// In microseconds
#define TICK_RATE 400

#define STOP() drive(0,0,true,true)

unsigned long time;

void setup() {
   pinMode(LEFT_ENABLE, OUTPUT);
   pinMode(RIGHT_ENABLE, OUTPUT);
   pinMode(LEFT_FORWARD, OUTPUT);
   pinMode(LEFT_BACK, OUTPUT);
   pinMode(RIGHT_FORWARD, OUTPUT);
   pinMode(RIGHT_BACK, OUTPUT);

   pinMode(STATUS_LED, OUTPUT);

   STOP();

   digitalWrite(STATUS_LED, HIGH);

   time = millis();

   Serial.begin(9600); 
   Serial.println("--- Ready! ");
}

void loop() {
  /*delay(1000);
  drive(226,226,true,true);
  delay(3000);
  drive(0, 226, true, false);
  delay(1000);
  STOP();
  drive(226, 0, false, true);
  delay(1000);
  drive(226,226,false,false);
  delay(2000);
  STOP();
  delay(3000);*/
  
  
  doManualControl();
}

void doManualControl(){
  int steering = pulseIn(RECEIVER_1, HIGH);
  int driving = pulseIn(RECEIVER_2, HIGH);
  
  if(steering == 0 || driving == 0){
    drive(0,0,true,true);
    delay(100);
    return;
  }
  
  boolean forward = (driving < 1530);
  
  int speedMod = 0;
  if(forward){
    speedMod = 1530 - driving;
  } else {
    speedMod = driving - 1530;
  }

  if(speedMod < 30){
    drive(0,0,true,true);
    delay(100);
    return;
  }
  
  speedMod -= 30;
  
  if(speedMod > 255){
    speedMod = 255;
  }
  

  
  byte left = (byte) speedMod;
  byte right = (byte) speedMod;
  
  int turnMod = (1530 - steering);
  boolean leftForward = forward;
  boolean rightForward = forward;
  
  if((turnMod > 0 && turnMod < 30) ||
     (turnMod < 0 && turnMod > -30)){
       turnMod = 0;
  }
  
  if(turnMod > 255){
    turnMod = 255;
  }
  if(turnMod < -255){
    turnMod = -255;
  }
  
  if(turnMod > 0){
    if(turnMod > left){
      left = turnMod - left;
      leftForward = !forward;
    } else {
      left = left - turnMod;
    }  
  } 
  
  if(turnMod < 0){
    turnMod = (-1 * turnMod);
    if(turnMod > right){
      right = turnMod - right;
      rightForward = !forward;
    } else {
      right = right - turnMod;
    }  
  }
  
  Serial.print(left);
  Serial.print(" - ");
  Serial.println(right);
    
  if(left > 255){
    left = 255;
  }
  
  if(right > 255){
    right = 255;
  }
  
  drive(left, right, leftForward, rightForward);
  delay(100);
}



void drive(byte leftSpeed, byte rightSpeed, boolean leftForward, boolean rightForward){
  digitalWrite(LEFT_FORWARD, leftForward);
  digitalWrite(LEFT_BACK, !leftForward);
  digitalWrite(RIGHT_FORWARD, rightForward);
  digitalWrite(RIGHT_BACK, !rightForward);
  analogWrite(LEFT_ENABLE, leftSpeed);
  analogWrite(RIGHT_ENABLE, rightSpeed);
}
