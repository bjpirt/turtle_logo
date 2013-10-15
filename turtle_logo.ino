#include <stdlib.h>

// COMMAND_COUNT specifies how many user commands we are going to be able to hold - you can reduce this to save memory
#define COMMAND_COUNT  12
#define INPUT_BUFFER_LENGTH 20
#define CMD_STACK_SIZE 100
#define REPEAT_DEPTH 10
// Define the different types of results
#define INTEGER_TYPE    1
#define FLOAT_TYPE      2
#define STRING_TYPE     3
#define NO_TYPE         4
#define REPEAT_TYPE     5
#define END_REPEAT_TYPE 6

typedef void (* fp) (void);

struct UserCmd {
  const char *cmd;
  unsigned char type;
  fp fn;
};

struct Command {
  char cmd;
  int  arg;
};

struct RepeatStack {
  byte pos;
  byte loops_remaining;
};

unsigned char fn_counter = 0;
char input_buffer[INPUT_BUFFER_LENGTH];
unsigned char input_buffer_pos = 0;
byte cmd_write_pos = 0;
byte cmd_read_pos = 0;
boolean running = false;
char repeat_pos = -1;

UserCmd user_cmds[COMMAND_COUNT];
Command cmd_stack[CMD_STACK_SIZE];
RepeatStack repeat_stack[REPEAT_DEPTH];


// This gets called each loop and is responsible for kicking off any commands
void processInput(){
  int arg;
  if (Serial.available() > 0){
    char incomingByte = Serial.read();
    if(incomingByte == '\r' || incomingByte == '\n'){
      // we have a message in the buffer
      input_buffer[input_buffer_pos] = '\0';
      _storeCmd(input_buffer);
      //reset the buffer because we've processed this line
      input_buffer_pos = 0;
    }else{
      // add the character to the buffer
      input_buffer[input_buffer_pos] = incomingByte;
      input_buffer_pos++;
    }
  }
}

// This allows you to register a callback funtion
void addUserCmd(char* cmd, unsigned char type, void (* fn) (void)){
  if (fn_counter == COMMAND_COUNT) { return; }
  //if(type == REPEAT_TYPE){
  user_cmds[fn_counter].cmd = cmd;
  user_cmds[fn_counter].type = type;
  user_cmds[fn_counter].fn = fn;
  fn_counter++;
}

// This is a private method which matches the command to our list of commands
char _extractCmd(char *buffer){
  for(unsigned char cmd = 0; cmd<COMMAND_COUNT; cmd++){
    for(unsigned char j = 0; j < input_buffer_pos; j++){
      //compare each letter until we hit a space or the end of the string to find a match
      //Serial.println(cmd_list[cmd][j]);
      if(buffer[j] == user_cmds[cmd].cmd[j]){
        if((buffer[j + 1] == ' ' || buffer[j + 1] == '\0') && user_cmds[cmd].cmd[j + 1] == '\0'){
          //This is a command match, so return the index of the user command
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
  char arg[INPUT_BUFFER_LENGTH];
  char arg_pos = -1;
  for (unsigned char i = 0; i < INPUT_BUFFER_LENGTH; i++){
    if(arg_pos == -1){
      if (buffer[i] == ' '){
        arg_pos++;
      }
    }else{
      if(buffer[i] != '\0' && buffer[i] != ' '){
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

void _storeCmd(char *buffer){
  char cmd = _extractCmd(input_buffer);
  if(cmd < 0){ return; }
  cmd_stack[cmd_write_pos].cmd = _extractCmd(buffer);
  if(user_cmds[cmd].type == INTEGER_TYPE || user_cmds[cmd].type == REPEAT_TYPE){
    cmd_stack[cmd_write_pos].arg = _extractIntArg(buffer);
  }
  cmd_write_pos++;
}

void processNextCmd(){
  if (!running) { return; }
  if (cmd_read_pos >= cmd_write_pos) { return; }
  if (cmd_write_pos == 0) { return; }
  
  if (user_cmds[cmd_stack[cmd_read_pos].cmd].type == REPEAT_TYPE) {
    // if it's a repeat, enter another level in the stack
  } else if (user_cmds[cmd_stack[cmd_read_pos].cmd].type == REPEAT_TYPE) {
    // if it's the end of a repeat, either loop or continue
  } else {
    // run the command
    ((void (*)(int))user_cmds[cmd_stack[cmd_read_pos].cmd].fn)(cmd_stack[cmd_read_pos].arg);
  }
  cmd_read_pos++;
}

/**
 *  Define the functions we are going to use to control the robot
 **/

void release(){
  //motor1.release();
  //motor2.release();
}

void move(int distance){
  Serial.println("Move");
  /*
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
  */
}

void forward(int distance){
  move(distance);
}

void backward(int distance){
  move(-distance);
}

void penUp(){
  Serial.println("PEN UP");
  //myservo.write(135);
}

void penDown(){
  Serial.println("PEN DOWN");
  //myservo.write(45);
}


void turn(int angle){
  Serial.println("Turn");
  /*
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
  */
}

void leftTurn(int angle){
  turn(-angle);
}

void rightTurn(int angle){
  turn(angle);
}

void pause(){
  running = false;
  Serial.println("PAUSE");
}

void play(){
  running = true;
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
  //motor1.setSpeed(50);  // 300 rpm
  //motor2.setSpeed(50);  // 300 rpm
  
  //set up the servo
  //myservo.attach(10);
  penDown();
  
  //add the commands
  addUserCmd("FD", INTEGER_TYPE, (fp) &forward);
  addUserCmd("BK", INTEGER_TYPE, (fp) &backward);
  addUserCmd("PD", NO_TYPE, (fp) &penDown);
  addUserCmd("PU", NO_TYPE, (fp) &penUp);
  addUserCmd("LT", INTEGER_TYPE, (fp) &leftTurn);
  addUserCmd("RT", INTEGER_TYPE, (fp) &rightTurn);
  addUserCmd("PAUSE", NO_TYPE, (fp) &pause);
  addUserCmd("PLAY", NO_TYPE, (fp) &play);
  addUserCmd("CLEAR", NO_TYPE, (fp) &clearBuffer);
  addUserCmd("PING", NO_TYPE, (fp) &ping);
  addUserCmd("REPEAT", REPEAT_TYPE, 0);
  addUserCmd("]", END_REPEAT_TYPE, 0);
}

void loop() {
  processInput();
  processNextCmd();
}
