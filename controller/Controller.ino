#include <Bridge.h>
#include <FileIO.h>
#include <YunServer.h>
#include <YunClient.h>

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
const String CLEAR_LOG = "clearLog";
const String GET_SID = "getSID";
const String SET_TIME = "setTime";

// Error constants
const int ERR_NONE = 0;    // indicates no error for internal use
const int ERR_BAD_CMD = 1; // malformed/unrecognized command
const int ERR_NO_SID = 2; // no sid provided when needed
const int ERR_BAD_SID = 3; // wrong sid provided
const int ERR_SID_EXP = 4;  // sid expired, no session is in progress
const int ERR_BAD_SID_REQ = 5; // getSID called, but unexpired sid alremilady exists

const unsigned long SESSION_TIMEOUT = 60000; // session times out in one minute
const long ADMIN_SID = 816845;

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

  // Start log
  FileSystem.begin();
  log("System started");

  randomSeed(analogRead(0)); // reading unused pin is fairly random
}

void loop()
{ 
  // There is a new client?
  YunClient client = server.accept();
  if (client) {
    client.setTimeout(2); // Maximum amount of time to wait for the stream
    int error = ERR_NONE;

    // read the command from the client
    String command = client.readString();
    command.trim(); //kill whitespace
    
    // expire session on timeout
    if (sid != -1 && (millis() - session_set_time > SESSION_TIMEOUT)) {
      log("Session expired");
      sid = -1;
    }

    if (command == STATUS) { // always allow status commands
      log("Received command: " + command);
      client.print(lastCommand + ":" + String(speed));
    } 
    else if (command == GET_SID) { // always allow attempts to get sid
      log("Received command: " + command);
      if (sid == -1) {
        sid = random(0x7FFFFFFFL);
        client.print(String(sid));
      } 
      else {
        error = ERR_BAD_SID_REQ;
      }
    }
    else {
      long receivedSID;

      int sidSlashIndex = command.lastIndexOf('/');
      if (sidSlashIndex == -1 || sidSlashIndex == command.length() - 1) error = ERR_NO_SID;
      
      // extract SID and command
      receivedSID = command.substring(sidSlashIndex + 1).toInt();
      command = command.substring(0, sidSlashIndex);
      
      if (receivedSID != ADMIN_SID){
        if (sid == -1) error = ERR_SID_EXP;
        else if (receivedSID != sid) error = ERR_BAD_SID;
      }

      if (error == ERR_NONE) {
        log("Received command: " + command);
        session_set_time = millis(); // refresh session
         
        if (command.startsWith(SET_TIME)) {
          unix_time = command.substring(command.indexOf('/') + 1).toInt();
          millis_time_set = millis();
          client.print("time=" + String(unix_time));
        } 
        else if (command == CLEAR_LOG) {
          clearLog();
          client.print("Log cleared");
        } else {
          // process any speed commands
          processSpeedCommand(command, client);
          // process any direction commands
          processDirectionCommand(command, client);
        }
      }
    }
    
    if (error != ERR_NONE) {
      client.print(getErrorString(error));
      log("Error id=" + String(error) + " on received command \"" + command + "\"");
    }
  }

  client.stop();
}

void processDirectionCommand(String command, YunClient client) {
  if (command.startsWith(FORWARD)) {
    forward();
    int slashIndex = command.lastIndexOf('/');
    if (slashIndex != -1) {
      long wait = command.substring(slashIndex + 1).toInt();
      delay(wait);
      allStop();
    }
    client.print(command);

  } else if (command.startsWith(BACKWARD)) {
    backward();
    int slashIndex = command.lastIndexOf('/');
    if (slashIndex != -1) {
      long wait = command.substring(slashIndex + 1).toInt();
      delay(wait);
      allStop();
    }
    client.print(command);

  } else if (command.startsWith(LEFT)) {
    turnLeft(255);
    int slashIndex = command.lastIndexOf('/');
    if (slashIndex != -1) {
      long wait = command.substring(slashIndex + 1).toInt();
      delay(wait);
      allStop();
    }
    client.print(command);

  } else if (command.startsWith(RIGHT)) {
    turnRight(255);
    int slashIndex = command.lastIndexOf('/');
    if (slashIndex != -1) {
      long wait = command.substring(slashIndex + 1).toInt();
      delay(wait);
      allStop();
    }
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
  analogWrite(B1M2, turn_speed);

  // Motor Controller 2 Backwards
  analogWrite(B2M1, turn_speed);
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
  analogWrite(B1M1, turn_speed);
  analogWrite(B1M2, 0);

  // Motor Controller 2 Backwards
  analogWrite(B2M1, 0);
  analogWrite(B2M2, turn_speed);

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
