#include <stdlib.h>
#include <AFMotor.h>
#include <Servo.h> 

// COMMAND_COUNT specifies how many commands we are going to be able to hold - you can reduce this to save memory
#define COMMAND_COUNT  10
#define BUFFER_LENGTH 20
// Define the different types of results
#define INTEGER_TYPE   1
#define FLOAT_TYPE     2
#define STRING_TYPE    3
#define NO_TYPE        4
#define REPEAT_TYPE    5

typedef void (* fp) (void);

const char *cmd_list[COMMAND_COUNT];
unsigned char type_list[COMMAND_COUNT];
fp cb_list[COMMAND_COUNT];
unsigned char cb_counter = 0;
char cmd_buffer[BUFFER_LENGTH];
unsigned char buffer_count = 0;

AF_Stepper motor1(48, 1);
AF_Stepper motor2(48, 2);

Servo myservo;

/**
 *  This set of functions provides a reasonably generic command parser.
 *  It allows you to register a callback function which will be called when a specific command
 *  is received. It will extract arguments from the command and pass them to the function.
 **/

// This gets called each loop and is responsible for kicking off any commands
void parseInput(){
  int arg;
  if (Serial.available() > 0){
    char incomingByte = Serial.read();
    if(incomingByte == '\r' || incomingByte == '\n'){
      // we have a message in the buffer
      cmd_buffer[buffer_count] = '\0';
      char cmd = _extractCmd(cmd_buffer);
      if(cmd >= 0){
        switch(type_list[cmd]){
          case NO_TYPE:
            //just call the function
            ((void (*)(void))cb_list[cmd])();
            break;
          case INTEGER_TYPE:
            //get argument as an int and call the handler with it
            arg = _extractIntArg(cmd_buffer);
            ((void (*)(int))cb_list[cmd])(arg);
            break;
          case FLOAT_TYPE:
          case STRING_TYPE:
            //not implemented yet   
            break;        
        }
      }else{
        Serial.println("NO MATCH");
      }
      Serial.println("OK");
      //reset the buffer because we've processed this line
      buffer_count = 0;
    }else{
      // add the character to the buffer
      cmd_buffer[buffer_count] = incomingByte;
      buffer_count++;
    }
  }
}

// This allows you to register a callback funtion
void addCallback(char* cmd, unsigned char type, void (* cb) (void)){
  if (cb_counter == COMMAND_COUNT) { return; }
  cmd_list[cb_counter] = cmd;
  type_list[cb_counter] = type;
  cb_list[cb_counter] = cb;
  cb_counter++;
}

// This is a private method which matches the command to our list of commands
char _extractCmd(char *buffer){
  for(unsigned char cmd = 0; cmd<COMMAND_COUNT; cmd++){
    for(unsigned char j = 0; j < buffer_count; j++){
      //compare each letter until we hit a space or the end of the string to find a match
      //Serial.println(cmd_list[cmd][j]);
      if(buffer[j] == cmd_list[cmd][j]){
        if((buffer[j + 1] == ' ' || buffer[j + 1] == '\0') && cmd_list[cmd][j + 1] == '\0'){
          //This is a command match, so let's extract the argument and call the function
          return cmd;
        }
      }else{
        break;
      }
    }
  }
  return -1;
}

// This extracts an int from the arguments
int _extractIntArg(char *buffer){
  //extract everthing after the space to the end of the line and convert to an integer
  char arg[BUFFER_LENGTH];
  char arg_pos = -1;
  for (unsigned char i = 0; i < BUFFER_LENGTH; i++){
    if(arg_pos == -1){
      if (buffer[i] == ' '){
        arg_pos++;
      }
    }else{
      if(buffer[i] != '\0'){
       arg[arg_pos] = buffer[i];
       arg_pos++; 
      }else{
        break;
      }
    }
  }
  arg[arg_pos] = '\0';
  return atoi(arg);
}

/**
 *  Define the functions we are going to use to control the robot
 **/

void release(){
  motor1.release();
  motor2.release();
}

void move(int distance){
  Serial.println("Move");
  char temp[6];
  Serial.println(itoa(distance, temp, 10));
  char dir = FORWARD;
  if(distance < 0){
    dir = BACKWARD;
    distance = -distance;
  }
  
  for(int i=0; i<distance; i++){
    motor1.step(1, dir, DOUBLE);
    motor2.step(1, dir, DOUBLE);
  }
  release();
}

void forward(int distance){
  move(distance);
}

void backward(int distance){
  move(-distance);
}

void penUp(){
  Serial.println("PEN UP");
  myservo.write(135);
}

void penDown(){
  Serial.println("PEN DOWN");
  myservo.write(45);
}


void turn(int angle){
  Serial.println("Turn");
  char temp[10];
  Serial.println(itoa(angle, temp, 10));
  char dir = BACKWARD;
  if(angle < 0){
    dir = FORWARD;
    angle = -angle;
  }
  
  for(int i=0; i<angle; i++){
    motor1.step(1, (dir == FORWARD ? BACKWARD : FORWARD), DOUBLE);
    motor2.step(1, (dir == FORWARD ? FORWARD : BACKWARD), DOUBLE);
  }
  release();
}

void leftTurn(int angle){
  turn(-angle);
}

void rightTurn(int angle){
  turn(angle);
}

void pause(){
  Serial.println("PAUSE");
}

void play(){
  Serial.println("PLAY");
}

void clearBuffer(){
  Serial.println("CLEAR");
}

void ping(){
  Serial.println("PONG");
}


/**
 *  Define our setup function and then loop
 **/
void setup(){
  Serial.begin(115200);           // set up Serial library at 11520 bps
  Serial.println("Stepper test!");
  
  //set up the steppers
  motor1.setSpeed(50);  // 300 rpm
  motor2.setSpeed(50);  // 300 rpm
  
  //set up the servo
  myservo.attach(10);
  penDown();
  
  //add the commands
  addCallback("FD", INTEGER_TYPE, (fp) &forward);
  addCallback("BK", INTEGER_TYPE, (fp) &backward);
  addCallback("PD", NO_TYPE, (fp) &penDown);
  addCallback("PU", NO_TYPE, (fp) &penUp);
  addCallback("LT", INTEGER_TYPE, (fp) &leftTurn);
  addCallback("RT", INTEGER_TYPE, (fp) &rightTurn);
  addCallback("PAUSE", NO_TYPE, (fp) &pause);
  addCallback("PLAY", NO_TYPE, (fp) &play);
  addCallback("CLEAR", NO_TYPE, (fp) &clearBuffer);
  addCallback("PING", NO_TYPE, (fp) &ping);
  //addCallback("REPEAT", REPEAT_TYPE, 0);
}

void loop() {
  parseInput();
}
