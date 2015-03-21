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

const int trigPinFront = 3;
const int echoPinFront = 2;

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

// Start off in remote control mode
int mode = REMOTE_CONTROL;

// Log file location
const char* LOG_PATH = "/mnt/sd/arduino/www/log.txt";

void setup() {
  Bridge.begin();
  startWebcam();

  server.begin();

  front_sensor.trigPin = 3;
  front_sensor.echoPin = 2;
  back_sensor.trigPin = 12;
  back_sensor.echoPin = 13;

  //setup front/back proximity sensors
  pinMode(front_sensor.trigPin, OUTPUT);
  pinMode(front_sensor.echoPin, INPUT);
  pinMode(back_sensor.trigPin, OUTPUT);
  pinMode(back_sensor.echoPin, INPUT);

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
    log("Received command: " + command);

    // detect a mode switch
    switchMode(command, client);

    if (command == START_WEBCAM) {
      startWebcam();
    }

    /*if (mode == AIMODE1) {
      aiMode1();
    } else if (mode == FOLLOW_LINE_MODE) {
      lineFollowingMode();
    } else if (mode == FOLLOW_GREEN_MODE) {
      followGreenMode();
    }*/

  }

  client.stop();
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
    setFastSpeed();

  } else if (command == FOLLOW_LINE) {
    allStop();
    setFastSpeed();
    mode = FOLLOW_LINE_MODE;
    client.print("Current Mode: Line Following Mode");

  } else if (command == FOLLOW_GREEN) {
    allStop();
    setFastSpeed();
    mode = FOLLOW_GREEN_MODE;
    client.print("Current Mode: Follow Green Mode");
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
    forward();
    log("Line Following: Forward");

    if (lastCommand == LEFT || lastCommand == RIGHT) {
      delay(200);
    } else {
      delay(600);
    }

    allStop();
  } else if (result[0] == 'R') {
    turnRight();
    log("Line Following: Right");
    delay(800);
    allStop();
  } else if (result[0] == 'L') {
    turnLeft();
    log("Line Following: Left");
    delay(800);
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

  if (result[0] == 'F') {
    forward();
    log("Follow Green Mode: Forward");
    delay(800);
    allStop();
  } else if (result[0] == 'R') {
    turnRight();
    log("Follow Green Mode: Right");
    delay(800);
    allStop();
  } else if (result[0] == 'L') {
    turnLeft();
    log("Follow Green Mode: Left");
    delay(800);
    allStop();
  } else if (result[0] == 'S') {
    allStop();
    log("Follow Green Mode: Stop");
  }
}

void aiMode1() {
  // TODO
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

/**
***   REST calls to the other board
**/
void backward() {
  Process p;
  p.runShellCommandAsynchronously("wget http://192.168.0.16/arduino/moveBackward");
  lastCommand = BACKWARD;
}

void forward() {
  Process p;
  p.runShellCommandAsynchronously("wget http://192.168.0.16/arduino/moveForward");
  lastCommand = FORWARD;
}

void allStop() {
  Process p;
  p.runShellCommandAsynchronously("wget http://192.168.0.16/arduino/stop");
  lastCommand = STOP;
}

void turnRight() {
  Process p;
  p.runShellCommandAsynchronously("wget http://192.168.0.16/arduino/rightTurn");
  lastCommand = RIGHT;
}

void turnLeft() {
  Process p;
  p.runShellCommandAsynchronously("wget http://192.168.0.16/arduino/leftTurn");
  lastCommand = LEFT;
}

void setFastSpeed() {
  Process p;
  p.runShellCommandAsynchronously("wget http://192.168.0.16/arduino/setFastSpeed");
}


/**
***   Logging
**/
void log(String msg) {
  File msgLog = FileSystem.open(LOG_PATH, FILE_APPEND);
  msgLog.println(msg);
  msgLog.close();
}
