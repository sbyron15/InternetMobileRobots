#include <Bridge.h>
#include <FileIO.h>
#include <YunServer.h>
#include <YunClient.h>
#include <HttpClient.h>

// Sensor defines and constants
#define TO_CM 29

// States
#define REMOTE_CONTROL 100
#define AIMODE1 101
#define FOLLOW_LINE_MODE 102
#define FOLLOW_GREEN_MODE 103

struct HCSR04
{
  int trigPin;
  int echoPin;
};
struct HCSR04 front_sensor;
struct HCSR04 back_sensor;
struct HCSR04 left_sensor;
struct HCSR04 right_sensor;

const int leftSensorPower = 5;
const int rightSensorPower = 2;

// Global server object
YunServer server;

// Command constants
String lastCommand = "";
const String START_WEBCAM = "startWebcam";
const String RC = "remoteControl";
const String AI1 = "aiMode1";
const String FOLLOW_LINE = "followLine";
const String FOLLOW_GREEN = "followGreen";
const String FORWARD = "moveForward";
const String BACKWARD = "moveBackward";
const String LEFT = "leftTurn";
const String RIGHT = "rightTurn";
const String STOP = "stop";

const String CONST_SID = "816845";

// Start off in remote control mode
int mode = REMOTE_CONTROL;

// Log file location
const char* LOG_PATH = "/mnt/sd/arduino/www/log.txt";

void setup() {
  Bridge.begin();
  startWebcam();

  server.begin();

  front_sensor.trigPin = 9;
  front_sensor.echoPin = 8;
  back_sensor.trigPin = 10;
  back_sensor.echoPin = 11;
  left_sensor.trigPin = 6;
  left_sensor.echoPin = 7;
  right_sensor.trigPin = 4;
  right_sensor.echoPin = 3;


  //setup front/back proximity sensors
  pinMode(front_sensor.trigPin, OUTPUT);
  pinMode(front_sensor.echoPin, INPUT);
  pinMode(back_sensor.trigPin, OUTPUT);
  pinMode(back_sensor.echoPin, INPUT);
  pinMode(left_sensor.trigPin, OUTPUT);
  pinMode(left_sensor.echoPin, INPUT);
  pinMode(right_sensor.trigPin, OUTPUT);
  pinMode(right_sensor.echoPin, INPUT);
  pinMode(rightSensorPower, OUTPUT);
  pinMode(leftSensorPower, OUTPUT);

  digitalWrite(rightSensorPower, HIGH);
  digitalWrite(leftSensorPower, HIGH);

  log("System started");

}

void loop() {
  if (mode == AIMODE1) {
    aiMode1();
  } else if (mode == FOLLOW_LINE_MODE) {
    lineFollowingMode();
  } else if (mode == FOLLOW_GREEN_MODE) {
    followGreenMode();
  }

  // There is a new client?
  YunClient client = server.accept();
  if (client) {
    client.setTimeout(2); // Maximum amount of time to wait for the stream

    // read the command from the client
    String command = client.readString();
    command.trim(); //kill whitespace
    log("Received: " + command);

    // detect a mode switch
    switchMode(command, client);

    if (command == START_WEBCAM) {
      startWebcam();
    }

  }

  client.stop();
}

/**
***   Command Interpreters
**/
void switchMode(String command, YunClient client) {
  if (command == RC) {
    mode = REMOTE_CONTROL;
    client.print("Remote Control");
    allStop();

  } else if (command == AI1) {
    mode = AIMODE1;
    client.print("AI Mode");
    forward("");
    setFastSpeed();

  } else if (command == FOLLOW_LINE) {
    allStop();
    setFastSpeed();
    mode = FOLLOW_LINE_MODE;
    client.print("Line Following");

  } else if (command == FOLLOW_GREEN) {
    allStop();
    setFastSpeed();
    mode = FOLLOW_GREEN_MODE;
    client.print("Follow Green");
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

  if (result[0] == 'F') {
    log("LF: F");

    if (lastCommand == LEFT || lastCommand == RIGHT) {
      forward("200");
    } else {
      forward("600");
    }

  } else if (result[0] == 'R') {
    turnRight("800");
    log("LF: R");
  } else if (result[0] == 'L') {
    turnLeft("800");
    log("LF: L");
  } else if (result[0] == 'S') {
    allStop();
    log("LF: S");
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

  if (result[0] == 'F') {
    forward("800");
    log("FG: F");
  } else if (result[0] == 'R') {
    turnRight("800");
    log("FG: R");
  } else if (result[0] == 'L') {
    turnLeft("800");
    log("FG: L");
  } else if (result[0] == 'S') {
    allStop();
    log("FG: S");
  }
}

void aiMode1() {
  int distance = 10;
  delay(2500);

  long fDistance = getDistanceCM(front_sensor);
  delay(100);
  long bDistance = getDistanceCM(back_sensor);
  delay(100);
  long lDistance = getDistanceCM(left_sensor);
  delay(100);
  long rDistance = getDistanceCM(right_sensor);

  log(String(fDistance) + " " + String(bDistance) + " " + String(lDistance) + " " + String(rDistance));

  if (fDistance == 0) {
    fDistance = distance + 1;
  }
  if (bDistance == 0) {
    bDistance = distance + 1;
  }
  if (lDistance == 0) {
    lDistance = distance + 1;
  }
  if (rDistance == 0) {
    rDistance = distance + 1;
  }

  if (fDistance < distance || bDistance < distance || lDistance < distance || rDistance < distance) {
    allStop();
  }
  
  /*Checking for only 1 tripped */
  if (fDistance <= distance && bDistance > distance && lDistance > distance && rDistance > distance) {
    backward("");
    log("FS Trig. B");
  } else if (fDistance > distance && bDistance <= distance && lDistance > distance && rDistance > distance) {
    forward("");
    log("BS Trig. F");
  } else if (fDistance > distance && bDistance > distance && lDistance <= distance && rDistance > distance) {
    turnRight("");
    log("LS Trig. R");
  } else if (fDistance > distance && bDistance > distance && lDistance > distance && rDistance <= distance) {
    turnLeft("");
    log("RS Trig. L");
    /* 2 tripped */
  } else if (fDistance <= distance && bDistance > distance && lDistance <= distance && rDistance > distance) {
    turnRight("");
    log("FS & LS. R");
  } else if (fDistance <= distance && bDistance > distance && lDistance > distance && rDistance <= distance) {
    turnLeft("");
    log("FS & RS. L");
    /* 3 tripped */
  } else if (fDistance <= distance && bDistance > distance && lDistance <= distance && rDistance <= distance) {
    backward("");
    log("LS, FS, RS. B");
  } else {
    forward("");
  }

}

/**
***   Misc Methods
**/
void startWebcam() {
  Process p;
  p.runShellCommandAsynchronously("mjpg_streamer -i \"input_uvc.so -d /dev/video0 -r 160x120 -f 8 -q 75\" -o \"output_http.so -p 8080 -w /root\"");
}

/**
***   Sensors
**/
int getDistanceCM(struct HCSR04 sensor) {
  long duration;
  digitalWrite(sensor.trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sensor.trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(sensor.trigPin, LOW);
  duration = pulseIn(sensor.echoPin, HIGH, 20000);
  return (duration / 60);
}

bool checkSensorsForObstacle(int distance) {
  if (getDistanceCM(front_sensor) < distance) {
    return true;
  }

  if (getDistanceCM(back_sensor) < distance) {
    return true;
  }

  if (getDistanceCM(left_sensor) < distance) {
    return true;
  }

  if (getDistanceCM(right_sensor) < distance) {
    return true;
  }

  return false;
}

/**
***   REST calls to the other board
**/
void backward(String wait) {
  Process p;
  String command = "curl http://192.168.0.16/arduino/moveBackward/";
  command.concat(wait);
  command.concat("/");
  command.concat(CONST_SID);
  p.runShellCommand(command);
  lastCommand = BACKWARD;
}

void forward(String wait) {
  Process p;
  String command = String("curl http://192.168.0.16/arduino/moveForward/");
  command.concat(wait);
  command.concat("/");
  command.concat(CONST_SID);
  p.runShellCommand(command);
  lastCommand = FORWARD;
}

void allStop() {
  Process p;
  String command = String("curl http://192.168.0.16/arduino/stop/");
  command.concat(CONST_SID);
  p.runShellCommand(command);
  lastCommand = STOP;
}

void turnRight(String wait) {
  Process p;
  String command = String("curl http://192.168.0.16/arduino/rightTurn/");
  command.concat(wait);
  command.concat("/");
  command.concat(CONST_SID);
  p.runShellCommand(command);
  lastCommand = RIGHT;
}

void turnLeft(String wait) {
  Process p;
  String command = String("curl http://192.168.0.16/arduino/leftTurn/");
  command.concat(wait);
  command.concat("/");
  command.concat(CONST_SID);
  p.runShellCommand(command);
  lastCommand = LEFT;
}

void setFastSpeed() {
  Process p;
  String command = String("curl http://192.168.0.16/arduino/setFastSpeed/");
  command.concat(CONST_SID);
  p.runShellCommand(command);
}


/**
***   Logging
**/
void log(String msg) {
  File msgLog = FileSystem.open(LOG_PATH, FILE_APPEND);
  msgLog.println(msg);
  msgLog.close();
}
