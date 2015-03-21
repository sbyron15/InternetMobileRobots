#include <Bridge.h>
#include <FileIO.h>
#include <YunServer.h>
#include <YunClient.h>

// States
#define REMOTE_CONTROL 100
#define AIMODE1 101
#define FOLLOW_LINE_MODE 102
#define FOLLOW_GREEN_MODE 103

// Sensor defines and constants
#define TO_CM 29

struct HCSR04
{
  int trigPin;
  int echoPin;
};
struct HCSR04 front_sensor;
struct HCSR04 back_sensor;

const int trigPinFront = 3;
const int echoPinFront = 2;

// Motor controller constants
const int B1E1 = 4;
const int B1M1 = 5;
const int B1E2 = 7;
const int B1M2 = 6;

const int B2E1 = 8;
const int B2M1 = 9;
const int B2E2 = 11;
const int B2M2 = 10;

// Global server object
YunServer server;

// Initial speed
int speed = 200;

// Command constants
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
const String START_WEBCAM = "startWebcam";
const String RC = "remoteControl";
const String AI1 = "aiMode1";
const String FOLLOW_LINE = "followLine";
const String FOLLOW_GREEN = "followGreen";
const String CLEAR_LOG = "clearLog";
const String GET_SID = "getSID";
const String SET_TIME = "setTime";

// Start off in remote control mode
int mode = REMOTE_CONTROL;

// Error constants
const int ERR_NONE = 0;    // indicates no error for internal use
const int ERR_BAD_CMD = 1; // malformed/unrecognized command
const int ERR_NO_SID = 2; // no sid provided when needed
const int ERR_BAD_SID = 3; // wrong sid provided
const int ERR_SID_EXP = 4;  // sid expired, no session is in progress
const int ERR_BAD_SID_REQ = 5; // getSID called, but unexpired sid already exists

const int SESSION_TIMEOUT = 60000; // session times out in one minute

long sid = -1;
unsigned long session_set_time = 0; // indicates time from millis() that session was last refreshed

// time keeping
long unix_time = -1;
unsigned long millis_time_set = 0; // indicates how long (in ms) the system was on when unix_time was last set

// Log file location
const char* LOG_PATH = "/mnt/sd/arduino/www/controller/log.txt";

void setup()
{
  Bridge.begin();
  startWebcam();

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

  // Start log
  FileSystem.begin();
  log("System started");

  randomSeed(analogRead(0)); // reading unused pin is fairly random
}

void loop()
{
  if (checkSensorsForObstacle()) {
    //allStop();
  } else if (mode == AIMODE1) {
    aiMode1();
  } else if (mode == FOLLOW_LINE_MODE) {
    lineFollowingMode();
  } else if (mode == FOLLOW_GREEN_MODE) {
    followGreenMode();
  }

  // There is a new client?
  YunClient client = server.accept();
  if (client) {
    int error = ERR_NONE;
    client.setTimeout(2); // Maximum amount of time to wait for the stream

    // read the command from the client
    String command = client.readString();
    command.trim(); //kill whitespace

    // expire session on timeout
    if (millis() - session_set_time > SESSION_TIMEOUT) sid = -1;

    if (command == STATUS) { // always allow status commands
      log("Received command: " + command);
      client.print(lastCommand + ":" + String(speed));
    } else if (command == GET_SID) { //always allow attempts to get sid
      log("Received command: " + command);
      if (sid == -1) {
        sid = random(0x7FFFFFFFL);
        client.print(String(sid));
      } else {
        error = ERR_BAD_SID_REQ;
      }
    } else {
      /*long receivedSID;

      int sidSlashIndex = command.lastIndexOf('/');
      if (sidSlashIndex == -1 || sidSlashIndex == command.length() - 1) error = ERR_NO_SID;
      else if (sid == -1) error = ERR_SID_EXP;
      else {
        // extract sid from rest of command
        receivedSID = command.substring(sidSlashIndex + 1).toInt();
        command = command.substring(0, sidSlashIndex);
        if (receivedSID != sid) error = ERR_BAD_SID;
      }*/

      if (error == ERR_NONE) {
        log("Received command: " + command);
        session_set_time = millis(); // refresh session

        // detect a mode switch
        switchMode(command, client);

        // misc commands

        if (command == START_WEBCAM) {
          startWebcam();
        } else if (command.startsWith(SET_TIME)) {
          unix_time = command.substring(command.indexOf('/') + 1).toInt();
          millis_time_set = millis();
          client.print("time=" + String(unix_time));
        } else if (command == CLEAR_LOG) {
          clearLog();
          client.print("Log cleared");
        }

        // process any speed commands
        processSpeedCommand(command, client);

        if (mode == REMOTE_CONTROL) {
          // process any direction commands
          processDirectionCommand(command, client);
        } else if (mode == AIMODE1) {
          aiMode1(); // Is this really necessary? We call aiMode1() at the begining of loop
        }
      }
    }

    if (error != ERR_NONE) {
      client.print(getErrorString(error));
      log("Error id=" + String(error) + " on received command \"" + command + "\"");
    }
    client.stop();
  }
}

/**
***   Command Interpreters
**/
void switchMode(String command, YunClient client) {
  if (command == RC) {
    mode = REMOTE_CONTROL;
    client.print("Current Mode: Remote Control Mode");
    allStop();

  } else if (command == AI1) {
    mode = AIMODE1;
    client.print("Current Mode: AI Mode");
    forward();

  } else if (command == FOLLOW_LINE) {
    allStop();
    mode = FOLLOW_LINE_MODE;
    client.print("Current Mode: Line Following Mode");

  } else if (command == FOLLOW_GREEN) {
    allStop();
    mode = FOLLOW_GREEN_MODE;
    client.print("Current Mode: Follow Green Mode");
  }
}

void processDirectionCommand(String command, YunClient client) {
  if (command == FORWARD) {
    forward();
    client.print(command);

  } else if (command == BACKWARD) {
    backward();
    client.print(command);

  } else if (command == LEFT) {
    turnLeft(255);
    client.print(command);

  } else if (command == RIGHT) {
    turnRight(255);
    client.print(command);
  }
}

void processSpeedCommand(String command, YunClient client) {
  bool printSpeed = false;

  if (command == SPEED_UP) {
    speed = speed + 10;
    if (speed > 200) {
      speed = 200;
    }
    printSpeed = true;
    replayLastDirection();

  } else if (command == SPEED_DOWN) {
    speed = speed - 10;
    if (speed < 0) {
      speed = 0;
    }
    printSpeed = true;
    replayLastDirection();

  } else if (command == FAST) {
    speed = 200;
    printSpeed = true;
    replayLastDirection();

  } else if (command == SLOW) {
    speed = 140;
    printSpeed = true;
    replayLastDirection();

  } else if (command == MEDIUM) {
    speed = 170;
    printSpeed = true;
    replayLastDirection();

  } else if (command == STOP) {
    allStop();
  }

  if (printSpeed) {
    client.print("SPEED = ");
    client.print(speed);
    client.print("\n");
  }
}

/**
***   Modes
**/
void lineFollowingMode() {
  Process p;
  p.runShellCommand("nice -n -19 python /root/image-processing/follow-line.pyo");

  char result[1];
  if (p.available() > 0) {
    result[0] = p.read();
  }

  p.close();
  speed = 200;

  if (result[0] == 'F') {
    forward();
    log("Line Following: Forward");
    delay(800);
    allStop();
  } else if (result[0] == 'R') {
    turnRight(255);
    log("Line Following: Right");
    delay(1500);
    allStop();
  } else if (result[0] == 'L') {
    turnLeft(255);
    log("Line Following: Left");
    delay(1500);
    allStop();
  } else if (result[0] == 'S') {
    allStop();
    log("Line Following: Stop");
  }
}

void followGreenMode() {
  Process p;
  p.runShellCommand("nice -n -19 python /root/image-processing/follow-green.pyo");

  char result[1];
  if (p.available() > 0) {
    result[0] = p.read();
  }

  p.close();
  speed = 200;

  if (result[0] == 'F') {
    forward();
    log("Follow Green Mode: Forward");
    delay(800);
    allStop();
  } else if (result[0] == 'R') {
    turnRight(255);
    log("Follow Green Mode: Right");
    delay(800);
    allStop();
  } else if (result[0] == 'L') {
    turnLeft(255);
    log("Follow Green Mode: Left");
    delay(800);
    allStop();
  } else if (result[0] == 'S') {
    allStop();
    log("Follow Green Mode: Stop");
  }
}

void aiMode1() {

  if (lastCommand == FORWARD) {
    if (getDistanceCM(front_sensor) < 20) {
      turnLeft(255);
    }
  }
  else if (lastCommand == LEFT) {
    if (getDistanceCM(front_sensor) > 30) {
      forward();
    }
  }
}

/**
***   Motor Controls
**/
void backward() {
  pinMode(B1M1, OUTPUT);
  pinMode(B1M2, OUTPUT);
  pinMode(B2M1, OUTPUT);
  pinMode(B2M2, OUTPUT);
  
  digitalWrite(B1E1, HIGH);
  digitalWrite(B1E2, HIGH);
  digitalWrite(B2E1, HIGH);
  digitalWrite(B2E2, HIGH);
  
  // Motor Controller 1 Backwards
  analogWrite(B1M1, speed + 55);
  analogWrite(B1M2, 0);
  // Motor Controller 2 Backwards
  analogWrite(B2M1, speed - 55);
  analogWrite(B2M2, 0);

  lastCommand = BACKWARD;
}

void forward() {
  pinMode(B1M1, OUTPUT);
  pinMode(B1M2, OUTPUT);
  pinMode(B2M1, OUTPUT);
  pinMode(B2M2, OUTPUT);
  
  digitalWrite(B1E1, HIGH);
  digitalWrite(B1E2, HIGH);
  digitalWrite(B2E1, HIGH);
  digitalWrite(B2E2, HIGH);
  
  // Motor Controller 1 Forwards
  analogWrite(B1M1, 0);
  analogWrite(B1M2, speed + 55);
  // Motor Controller 2 Forwards
  analogWrite(B2M1, 0);
  analogWrite(B2M2, speed - 55);

  lastCommand = FORWARD;
}

void allStop() {
  pinMode(B1M1, OUTPUT);
  pinMode(B1M2, OUTPUT);
  pinMode(B2M1, OUTPUT);
  pinMode(B2M2, OUTPUT);
  
  digitalWrite(B1E1, HIGH);
  digitalWrite(B1E2, HIGH);
  digitalWrite(B2E1, HIGH);
  digitalWrite(B2E2, HIGH);
  
  // Motor Controller 1 Stop
  analogWrite(B1M1, 0);
  analogWrite(B1M2, 0);
  // Motor Controller 2 Stop
  analogWrite(B2M1, 0);
  analogWrite(B2M2, 0);

  lastCommand = STOP;
}

void turnRight(int turn_speed) {
  pinMode(B1M1, OUTPUT);
  pinMode(B1M2, OUTPUT);
  pinMode(B2M1, OUTPUT);
  pinMode(B2M2, OUTPUT);
  
  digitalWrite(B1E1, HIGH);
  digitalWrite(B1E2, HIGH);
  digitalWrite(B2E1, HIGH);
  digitalWrite(B2E2, HIGH);
  
  // Motor Controller 1 Forwards
  analogWrite(B1M1, 0);
  analogWrite(B1M2, turn_speed + 55);

  // Motor Controller 2 Backwards
  analogWrite(B2M1, turn_speed - 55);
  analogWrite(B2M2, 0);

  lastCommand = RIGHT;
}

void turnLeft(int turn_speed) {
  pinMode(B1M1, OUTPUT);
  pinMode(B1M2, OUTPUT);
  pinMode(B2M1, OUTPUT);
  pinMode(B2M2, OUTPUT);
  
  digitalWrite(B1E1, HIGH);
  digitalWrite(B1E2, HIGH);
  digitalWrite(B2E1, HIGH);
  digitalWrite(B2E2, HIGH);
  
  // Motor Controller 1 Forwards
  analogWrite(B1M1, turn_speed + 55);
  analogWrite(B1M2, 0);

  // Motor Controller 2 Backwards
  analogWrite(B2M1, 0);
  analogWrite(B2M2, turn_speed - 55);

  lastCommand = LEFT;
}

void replayLastDirection() {
  if (lastCommand == FORWARD) {
    forward();
  } else if (lastCommand == BACKWARD) {
    backward();
  } else if (lastCommand == LEFT) {
    turnLeft(255);
  } else if (lastCommand == RIGHT) {
    turnRight(255);
  }
}

/**
***   Misc Methods
**/
void startWebcam() {
  Process p;
  p.runShellCommandAsynchronously("mjpg_streamer -i \"input_uvc.so -d /dev/video0 -r 160x120 -f 8 -q 75\" -o \"output_http.so -p 8080 -w /root\"");
}

void log(String msg) {
  File msgLog = FileSystem.open(LOG_PATH, FILE_APPEND);
  if (unix_time == -1)
    msgLog.println(msg);
  else
    msgLog.println(String(unix_time + (millis() - millis_time_set) / 1000) + ": " + msg);
  msgLog.close();
}

void clearLog() {
  File msgLog = FileSystem.open(LOG_PATH, FILE_WRITE); // write clears file
  msgLog.close();
}

String getErrorString(int err_id) {
  return String("err=") + String(err_id);
}

/**
***   Sensors
**/
int getDistanceCM(struct HCSR04 sensor) {
  int duration;
  digitalWrite(sensor.trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sensor.trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(sensor.trigPin, LOW);
  duration = pulseIn(sensor.echoPin, HIGH);
  return (duration / 2) / TO_CM;
}

bool checkSensorsForObstacle() {
  if (getDistanceCM(front_sensor) < 20) {
    return true;
  }

  if (getDistanceCM(back_sensor) < 20) {
    return true;
  }

  return false;
}
