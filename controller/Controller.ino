#include <Bridge.h>
#include <FileIO.h>
#include <YunServer.h>
#include <YunClient.h>

#define FRONT_SENSOR 0
#define BACK_SENSOR 1
#define TO_CM 29


//States
#define REMOTE_CONTROL 100
#define AIMODE1 101

struct HCSR04
{
  int trigPin;
  int echoPin; 
};
struct HCSR04 front_sensor;
struct HCSR04 back_sensor;

YunServer server;


int trigPinFront = 3;
int echoPinFront = 2;

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
const String STATUS = "status";

const String RC = "remoteControl";
const String AI1 = "aiMode1";

int mode = REMOTE_CONTROL;


const char* LOG_PATH = "/mnt/sd/arduino/www/controller/log.txt";
const String CLEAR_LOG = "clearLog";

void setup() 
{ 
    Bridge.begin();
    startWebcam();
    
    //Serial.begin(9600);
    server.begin();
    
    front_sensor.trigPin = 3;
    front_sensor.echoPin = 2;
    back_sensor.trigPin = 12;
    back_sensor.echoPin = 13;
    
    // Set up pins
    pinMode(B1M1, OUTPUT);
    pinMode(B1M2, OUTPUT);
  
    pinMode(B2M1, OUTPUT);
    pinMode(B2M2, OUTPUT);
    
    //setup front/back proximity sensors
    pinMode(front_sensor.trigPin, OUTPUT);
    pinMode(front_sensor.echoPin, INPUT);
    pinMode(back_sensor.trigPin, OUTPUT);
    pinMode(back_sensor.echoPin, INPUT);
    
    // Enable all four motors
    digitalWrite(B1E1, HIGH);
    digitalWrite(B1E2, HIGH);
  
    digitalWrite(B2E1, HIGH);
    digitalWrite(B2E2, HIGH);

    FileSystem.begin();
} 

int getDistanceCM(struct HCSR04 sensor) {
  int duration;
  digitalWrite(sensor.trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sensor.trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(sensor.trigPin, LOW);
  duration = pulseIn(sensor.echoPin, HIGH);
  return (duration/2) / TO_CM;
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
  if(mode == REMOTE_CONTROL) {
    if(lastCommand == FORWARD) {
      if(getDistanceCM(front_sensor) < 20) {
        allStop(); 
      } 
    }
    if(lastCommand == BACKWARD) {
      if(getDistanceCM(back_sensor) < 20) {
        allStop();
      } 
    }
  } else if(mode == AIMODE1) {
    AiMode1(); 
  }
  
  // There is a new client?
  if (client) {
    
    
    client.setTimeout(2); // Maximum amount of time to wait for the stream
    
    // read the command
    String command = client.readString();
    command.trim();        //kill whitespace
    log("Received command: " + command);
    Serial.println(command);
    if (command == RC) {
      mode = REMOTE_CONTROL;
      allStop();
    } else if (command == AI1) {
      mode = AIMODE1; 
      forward();
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
    } else if (command == STATUS){
        client.print(lastCommand + ":" + String(speed));
    }
    
    if(mode == REMOTE_CONTROL) {
      RemoteControlMode(command, client); 
    } else if (mode == AIMODE1) {
      AiMode1(); 
    }
    
    client.stop();
  }
}

void RemoteControlMode(String command, YunClient client) {

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
      
    } 
}

void AiMode1() {
   
   if(lastCommand == FORWARD) {
     if(getDistanceCM(front_sensor) < 20) {
       turnLeft();
     } 
   }
   else if(lastCommand == LEFT) {
     if(getDistanceCM(front_sensor) > 30) {
       forward();
     } 
   }
}
