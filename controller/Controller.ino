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

int speed = 170;

int lastCommand = 0;

const int FORWARD = 1;
const int BACKWARD = 2;
const int LEFT = 3;
const int RIGHT = 4;
const int STOP = 5;
 
void setup() 
{ 
    Bridge.begin();
    server.begin();
    
    // Set up pins
    pinMode(B1M1, OUTPUT);
    pinMode(B1M2, OUTPUT);
  
    pinMode(B2M1, OUTPUT);
    pinMode(B2M2, OUTPUT);
    
    // Enable all four motors
    digitalWrite(B1E1, HIGH);
    digitalWrite(B1E2, HIGH);
  
    digitalWrite(B2E1, HIGH);
    digitalWrite(B2E2, HIGH);
    
    //setSpeed(speed);
} 

void backward() {
  // Motor Controller 1 Backwards
  analogWrite(B1M1, speed);
  analogWrite(B1M2, 0);
  // Motor Controller 2 Backwards
  analogWrite(B2M1, speed);
  analogWrite(B2M2, 0);
  
  lastCommand = BACKWARD;
}

void forward() {
  // Motor Controller 1 Forwards
  analogWrite(B1M1, 0);
  analogWrite(B1M2, speed);
  // Motor Controller 2 Forwards
  analogWrite(B2M1, 0);
  analogWrite(B2M2, speed);
  
  lastCommand = FORWARD;
}

void allStop() {
  // Motor Controller 1 Stop
  analogWrite(B1M1, 0);
  analogWrite(B1M2, 0);
  // Motor Controller 2 Stop
  analogWrite(B2M1, 0);
  analogWrite(B2M2, 0);

  lastCommand = STOP;
}

void turnRight() {
  // Motor Controller 1 Forwards
  analogWrite(B1M1, 0);
  analogWrite(B1M2, 255);
  
  // Motor Controller 2 Backwards
  analogWrite(B2M1, 0);
  analogWrite(B2M2, 100);
  
  lastCommand = RIGHT;
}

void turnLeft() {
  // Motor Controller 1 Forwards
  analogWrite(B1M1, 0);
  analogWrite(B1M2, 100);
  
  // Motor Controller 2 Backwards
  analogWrite(B2M1, 0);
  analogWrite(B2M2, 255);
  
  lastCommand = LEFT;
}

void setSpeed() {
    if (lastCommand == FORWARD) {
      forward();
    } else if (lastCommand == BACKWARD) {
      backward();
    } else if (lastCommand == LEFT) {
      turnLeft();
    } else if (lastCommand == RIGHT) {
      turnRight();
    }
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
      backward();
      client.print("REVERSE");
      
    } else if (command == "stop") {
      //allStop();
      //client.print("STOP");
      speed = 0;
      setSpeed();
      client.print("SPEED = ");
      client.print(speed);
      
    } else if (command == "leftTurn") {
      turnLeft();
      client.print("LEFT TURN");
      
    } else if (command == "rightTurn") {
      turnRight();
      client.print("RIGHT TURN");
      
    } else if (command == "speedUp") {
      speed = speed + 10;
      if (speed > 255) {
        speed = 255;
      }
      setSpeed();
      client.print("SPEED = ");
      client.print(speed);
      
    } else if (command == "speedDown") {
      speed = speed - 10;
      if (speed < 0) {
        speed = 0;
      }
      setSpeed();
      client.print("SPEED = ");
      client.print(speed);
    
    } else if (command == "setFastSpeed") {
        speed = 255;
        setSpeed();
        client.print("SPEED = ");
        client.print(speed);
        
    } else if (command == "setSlowSpeed") {
        speed = 150;
        setSpeed();
        client.print("SPEED = ");
        client.print(speed);
      
    } else if (command == "setMediumSpeed") {
        speed = 200;
        setSpeed();
        client.print("SPEED = ");
        client.print(speed);
    }
    
    client.stop();
  }
}
