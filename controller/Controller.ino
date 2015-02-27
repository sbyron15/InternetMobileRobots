#include <Bridge.h>
#include <FileIO.h>
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

String lastCommand = "";

const String FORWARD = "moveForward";
const String BACKWARD = "moveBackward";
const String LEFT = "leftTurn";
const String RIGHT = "rightTurn";
const String STOP = "stop";
const String SPEED_UP = "speedUp";
const String SPEED_DOWN = "speedDown";
const String SLOW = "setSlowSpeed";
const String MEDIUM = "setMediumSpeed";
const String FAST = "setFastSpeed";

const char* LOG_PATH = "/mnt/sd/arduino/www/controller/log.txt";
const String CLEAR_LOG = "clearLog";

void setup() 
{ 
    Bridge.begin();
    startWebcam();
    
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

    FileSystem.begin();
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
  analogWrite(B2M1, 255);
  analogWrite(B2M2, 0);
  
  lastCommand = RIGHT;
}

void turnLeft() {
  // Motor Controller 1 Forwards
  analogWrite(B1M1, 255);
  analogWrite(B1M2, 0);
  
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

void startWebcam() {
  Process p;
  p.runShellCommandAsynchronously("mjpg_streamer -i \"input_uvc.so -d /dev/video0 -r 320x240\" -o \"output_http.so -p 8080 -w /root\"");
}

void log(String msg){
    File msgLog = FileSystem.open(LOG_PATH, FILE_APPEND);
    msgLog.println(msg);
    msgLog.close();
}

void clearLog(){
    File msgLog = FileSystem.open(LOG_PATH, FILE_WRITE); // write clears file
    msgLog.close();
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
    log("Received command: " + command);
    Serial.println(command);
    
    if (command == FORWARD) {
      forward();
      client.print(command);
      
    } else if (command == BACKWARD) {
      backward();
      client.print(command);
      
    } else if (command == LEFT) {
      turnLeft();
      client.print(command);
      
    } else if (command == RIGHT) {
      turnRight();
      client.print(command);
      
    } else if (command == SPEED_UP) {
      speed = speed + 10;
      if (speed > 255) {
        speed = 255;
      }
      setSpeed();
      client.print("SPEED = ");
      client.print(speed);
      
    } else if (command == SPEED_DOWN) {
      speed = speed - 10;
      if (speed < 0) {
        speed = 0;
      }
      setSpeed();
      client.print("SPEED = ");
      client.print(speed);
    
    } else if (command == FAST) {
        speed = 255;
        setSpeed();
        client.print("SPEED = ");
        client.print(speed);
        
    } else if (command == SLOW) {
        speed = 150;
        setSpeed();
        client.print("SPEED = ");
        client.print(speed);
      
    } else if (command == MEDIUM) {
        speed = 200;
        setSpeed();
        client.print("SPEED = ");
        client.print(speed);
        
    } else if (command == STOP) {
      //speed = 0;
      //setSpeed();
      allStop();
      client.print("SPEED = ");
      client.print(speed);
    } else if (command == CLEAR_LOG) {
      clearLog();
      client.print("Log cleared");
    }
    
    client.stop();
  }
}
