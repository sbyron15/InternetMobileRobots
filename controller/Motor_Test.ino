#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

YunServer server;

int B1E1 = 4;
int B1M1 = 5;
int B1E2 = 7;
int B1M2 = 6;

int B2E1 = 8;
int B2M1 = 9;
int B2E2 = 11;
int B2M2 = 10;

int speed = 0;
 
void setup() 
{ 
    Bridge.begin();
    server.begin();
    
    pinMode(B1M1, OUTPUT);
    pinMode(B1M2, OUTPUT);
  
    pinMode(B2M1, OUTPUT);
    pinMode(B2M2, OUTPUT);
    
    setSpeed(speed);
} 

void reverse() {
  setSpeed(speed);
  
  // Motor Controller 1 Backwards
  digitalWrite(B1M1, HIGH);
  digitalWrite(B1M2, LOW);
  // Motor Controller 2 Backwards
  digitalWrite(B2M1, HIGH);
  digitalWrite(B2M2, LOW);
}

void forward() {
  setSpeed(speed);
  
  // Motor Controller 1 Forwards
  digitalWrite(B1M1, LOW);
  digitalWrite(B1M2, HIGH);
  // Motor Controller 2 Forwards
  digitalWrite(B2M1, LOW);
  digitalWrite(B2M2, HIGH);
}

void allStop() {
  setSpeed(0);
  
  // Motor Controller 1 Stop
  digitalWrite(B1M1, LOW);
  digitalWrite(B1M2, LOW);
  // Motor Controller 2 Stop
  digitalWrite(B2M1, LOW);
  digitalWrite(B2M2, LOW);
}

void turnRight() {
  setSpeed(speed);
  
  // Motor Controller 1 Forwards
  digitalWrite(B1M1, LOW);
  digitalWrite(B1M2, HIGH);
  
  // Motor Controller 2 Backwards
  digitalWrite(B2M1, HIGH);
  digitalWrite(B2M2, LOW);
}

void turnLeft() {
  setSpeed(speed);
  
  // Motor Controller 1 Forwards
  digitalWrite(B1M1, HIGH);
  digitalWrite(B1M2, LOW);
  
  // Motor Controller 2 Backwards
  digitalWrite(B2M1, LOW);
  digitalWrite(B2M2, HIGH);
}

void setSpeed(int speed) {
    digitalWrite(B1E1, speed);
    digitalWrite(B1E2, speed);
  
    digitalWrite(B2E1, speed);
    digitalWrite(B2E2, speed);
}
 
void loop() 
{ 
  YunClient client = server.accept();
  
  // There is a new client?
  if (client) {
    client.setTimeout(2); // Maximum amount of time to wait for the stream
    
    // read the command
    String command = client.readString();
    command.trim();        //kill whitespace
    Serial.println(command);
    
    if (command == "moveForward") {
      forward();
      client.print("FORWARD");
    } else if (command == "moveBackward") {
      reverse();
      client.print("REVERSE");
    } else if (command == "stop") {
      allStop();
      client.print("STOP");
    } else if (command == "leftTurn") {
      turnLeft();
      client.print("LEFT TURN");
    } else if (command == "rightTurn") {
      turnRight();
      client.print("RIGHT TURN");
    } else if (command == "speedUp") {
      speed = speed + 1;
      setSpeed(speed);
      client.print("SPEED = ");
      client.print(speed);
    } else if (command == "speedDown") {
      speed = speed - 1;
      setSpeed(speed);
      client.print("SPEED = ");
      client.print(speed);
    }
    
    client.stop();
  }
}
