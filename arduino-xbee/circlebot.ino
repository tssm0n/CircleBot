#include <SoftwareSerial.h> 
#include <NewPing.h>
#include"xbeecom.h"
#include"commands.h"

#define XBEE_ADDRESS 1
#define XBEE_MASTER_ADDRESS 0

#define STATUS_LED 2

#define LEFT_ENABLE 6
#define RIGHT_ENABLE 11
#define LEFT_FORWARD 8
#define LEFT_BACK 7
#define RIGHT_FORWARD 9
#define RIGHT_BACK 10

#define BUMP_SENSOR A0
#define PING_SENSOR A1

#define USE_BUMP_SENSOR 0
#define USE_PING_SENSOR 1

#define MAX_DISTANCE 250

#define CONTROL_JUMPER 1

#define RECEIVER_1 4
#define RECEIVER_2 3

// TX pin to the Xbee's RX
#define XBEE_RX 12
#define XBEE_TX 13

// In milliseconds
#define TICK_RATE 400

#define STOP() drive(0,0,true,true)

#define ALLOW_MANUAL 0
#define STARTUP_TEST 1

struct Status {
   // sensors
   boolean bumped;
   unsigned int distance;
   
   // TODO: measure battery voltage...
};

Status status;

SoftwareSerial xbee(XBEE_RX, XBEE_TX);

NewPing* sonar;

XbeeCom xbeeCom;
byte outgoingBuffer[XBEE_OUTGOING_BUFFER];

unsigned long time;

boolean manualControl = false;

void setup() {
   status.bumped = false;

   pinMode(LEFT_ENABLE, OUTPUT);
   pinMode(RIGHT_ENABLE, OUTPUT);
   pinMode(LEFT_FORWARD, OUTPUT);
   pinMode(LEFT_BACK, OUTPUT);
   pinMode(RIGHT_FORWARD, OUTPUT);
   pinMode(RIGHT_BACK, OUTPUT);

   pinMode(BUMP_SENSOR, INPUT);

   pinMode(STATUS_LED, OUTPUT);
   
   pinMode(CONTROL_JUMPER, INPUT);

   pinMode(RECEIVER_1, INPUT);
   pinMode(RECEIVER_2, INPUT);

   STOP();

   NewPing sonarSensor(PING_SENSOR, PING_SENSOR, MAX_DISTANCE); 
   sonar = &sonarSensor;

   time = millis();   

   digitalWrite(STATUS_LED, HIGH);

   xbee.begin(9600);

   Serial.begin(9600);
  
   if(digitalRead(CONTROL_JUMPER) == HIGH){
     manualControl = true;
     Serial.println("Using manual control");
   }  
   
   Serial.println("Ready");
   
   if(STARTUP_TEST){
     runStartupTest();
   }
}

void loop() {
   if(!ALLOW_MANUAL || !manualControl){
     doXbeeControl();
   } else {
     doManualControl();
   }
}

void doManualControl(){
  int steering = pulseIn(RECEIVER_1, HIGH);
  int driving = pulseIn(RECEIVER_2, HIGH);
  
  //Serial.println(steering);
  //Serial.println(driving);
  
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
  
  int turnMod = (1530 - steering)/2;
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
  
  if(left > 255){
    left = 255;
  }
  
  if(right > 255){
    right = 255;
  }
  
  drive(left, right, leftForward, rightForward);
  delay(100);
}

void runStartupTest(){
  drive(200,200,1,1);
  delay(2000);
  drive(200,200,0,0);
  delay(2000);
  STOP();
}

void doXbeeControl(){
  while (xbee.available())
     xbeeCom.xbeeReceiveByte(xbee.read());
    
   if(xbeeCom.xbeeIsPacketAvailable())
     handleXbeeReceive();
     
   unsigned long newTime = millis();
   if((newTime >= (time + TICK_RATE)) ||
      (time > newTime)){
           
        time = newTime;    
        updateAndTransmitSensors();
   }
}  
  

void handleXbeeReceive(){
  struct XbeePacket packet = xbeeCom.xbeeNextPacket();
  if(packet.dataLength == 0 || packet.destination != XBEE_ADDRESS){
    return; 
  }

  byte command = packet.data[0];
  
  Serial.println((int) command);
  
  switch(command){
    case PING: {
      byte data[] = { ACK };
      transmit(packet.source, 1, data);
      break;
    }
    case ACK:
    case STATUS:
      // No op...
      break;
    case STATUSREQ:
      updateAndTransmitSensors();
      break;
    case STOP_DRIVE:
      STOP();
      break;
    case DRIVE:
      if(packet.dataLength >= 5)
        drive(packet.data[1], packet.data[2], packet.data[3], packet.data[4]);
      break;
    case TOGGLE_STATUS_LED:  
      if(packet.dataLength >= 2)
         digitalWrite(STATUS_LED, packet.data[1]);
      break; 
    default:
      Serial.print("Unknown Command: ");
      Serial.println((int)command);   
  }
  
}

void transmit(byte destination, byte length, byte* data){
    byte len = xbeeCom.xbeeSend(XBEE_ADDRESS, destination, length, data, outgoingBuffer);
    
    byte count;
    for(count = 0; count < len; count++){
      xbee.write(outgoingBuffer[count]); 
   }
}

void updateAndTransmitSensors(){
  status.bumped = (analogRead(BUMP_SENSOR) > 1000);
  
  if(USE_PING_SENSOR){
    status.distance = (*sonar).convert_cm((*sonar).ping());
  }
  
  // Safety
  if(USE_BUMP_SENSOR && status.bumped){
    STOP();
  }  
  
  byte data[] = { STATUS, status.bumped, highByte(status.distance), lowByte(status.distance) };
  transmit(XBEE_MASTER_ADDRESS, 2, data);
}

void readXbee(){
  byte nextVal = 0;
  if (xbee.available())
    nextVal = xbee.read();
    Serial.write(nextVal);
    xbeeCom.xbeeReceiveByte(nextVal);
}

void drive(byte leftSpeed, byte rightSpeed, boolean leftForward, boolean rightForward){
  digitalWrite(LEFT_FORWARD, leftForward);
  digitalWrite(LEFT_BACK, !leftForward);
  digitalWrite(RIGHT_FORWARD, rightForward);
  digitalWrite(RIGHT_BACK, !rightForward);
  analogWrite(LEFT_ENABLE, leftSpeed);
  analogWrite(RIGHT_ENABLE, rightSpeed);
}

