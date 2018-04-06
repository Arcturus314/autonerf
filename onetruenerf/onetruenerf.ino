/*
 * IO:
 * OUTPUT
 * Flywheels: 5
 * Pusher   : 6
 * 
 * CONTROL
 * Trigger  : 3
 * Spin up  : 4
 * Pusher In: 2
 * 
 * SETUP
 * RPM select      : 2 7 right
 * Velocity select : 3 8 center
 * Burst select    : 4 9 left
 * 
 * RPM display     : 15 right
 * Velocity display: 16 center
 * Burst display   : 14 left
 * 
 * SYSTEM HEALTH
 * Battery voltage : 18 / A0
 * Health LED      : 10
 */

#define FLYWHEEL_MOTOR 5
#define PUSHER_MOTOR   6
#define FIRE_TRIGGER   3
#define SPIN_TRIGGER   4
#define PUSHER_IN      2 //HIGH when pusher is in
#define RPM_SEL        7
#define VEL_SEL        8
#define BURST_SEL      9
#define HEALTH_DISPLAY 10
#define VEL_DISPLAY    16
#define BURST_DISPLAY  14
#define BAT_VOLT       18
#define RPM_DISPLAY    15 

#define LOW_RPM   150 //analogWrite value for low RPM mode
#define LOW_VEL   150 //analogWrite value for high RPM mode

#define BATTERY_THRESHOLD 350 //equates to about 3.0V with 2x voltage divider 

int pusher_extended = 0; //0 when pusher is retracted, 1 when pusher is extended
int flywheels_on = 0; //0 when flywheels not spinning, 1 when flywheels are spinning

int rpm = 255; //analogWrite value for pusher motor
int vel = 255; //analogWrite value for flywheels

boolean burst = false;


void setup() {
  //setting up digital pins
  //outputs
  pinMode(FLYWHEEL_MOTOR, OUTPUT);
  pinMode(PUSHER_MOTOR,   OUTPUT);
  pinMode(RPM_DISPLAY,    OUTPUT);
  pinMode(VEL_DISPLAY,    OUTPUT);
  pinMode(BURST_DISPLAY,  OUTPUT);
  pinMode(HEALTH_DISPLAY, OUTPUT);
  //inputs
  pinMode(FIRE_TRIGGER, INPUT_PULLUP);
  pinMode(SPIN_TRIGGER, INPUT_PULLUP);
  pinMode(PUSHER_IN,    INPUT_PULLUP);
  pinMode(RPM_SEL,      INPUT_PULLUP);
  pinMode(VEL_SEL,      INPUT_PULLUP);
  pinMode(BURST_SEL,    INPUT_PULLUP);

  digitalWrite(HEALTH_DISPLAY, HIGH);
  delay(500);
  digitalWrite(HEALTH_DISPLAY, LOW);

  //debugging
  Serial.begin(9600);
}

void loop() {
  /*//motor testing
  digitalWrite(FLYWHEEL_MOTOR, HIGH);
  digitalWrite(PUSHER_MOTOR, LOW);
  delay(1000);
  digitalWrite(FLYWHEEL_MOTOR, LOW);
  digitalWrite(PUSHER_MOTOR, HIGH);
  delay(1000);*/

  //actual code

  //fully automatic
  if(burst == false) {
    Serial.println("Full auto");
    //Pusher control
    if(digitalRead(PUSHER_IN) == LOW) {
      Serial.println("Pusher extended");
      set_pusher(true);
      pusher_extended = 1;
    }
    else if(digitalRead(FIRE_TRIGGER) == LOW) {
      Serial.println("Firing");
      if(flywheels_on == 0) {
        set_flywheels(true);
        flywheels_on = 1;
        delay(200);
      }
      set_pusher(true);
      pusher_extended = 1;
    }
    else {
      set_pusher(false);
      pusher_extended = 0;
    }
  }

  //Flywheel control
  if(digitalRead(SPIN_TRIGGER) == LOW && flywheels_on == 0) {
    Serial.println("Trigger pulled");
    set_flywheels(true);
    flywheels_on = 1;
  }
  else if(digitalRead(FIRE_TRIGGER) == HIGH && digitalRead(SPIN_TRIGGER) == HIGH) {
    Serial.println("Turning off flywheels");
    set_flywheels(false);
    flywheels_on = 0;
  }

  //3-round burst mode
  if(burst == true && digitalRead(FIRE_TRIGGER) == LOW) {
    Serial.println("3 round burst mode, firing");
    //setting flywheels
    if(flywheels_on == 0) {
      Serial.println("Turning on flywheels");
      set_flywheels(true);
      flywheels_on = 1;
      delay(200);
    }
    //firing
    for(int num = 0; num < 3; num++) {
      Serial.print("firing number ");
      Serial.print(num);
      Serial.println("");
      if(pusher_extended == 0) {
        set_pusher(true);
        pusher_extended = 1;
        while(digitalRead(PUSHER_IN) == LOW) {} //moving pusher in (start of cycle)
      }
      while(digitalRead(PUSHER_IN) == HIGH) {} //moving pusher out 
      while(digitalRead(PUSHER_IN) == LOW) {}  //moving pusher back in
    }
    //stopping flywheels and pusher
    set_flywheels(false);
    flywheels_on = 0;
    while(digitalRead(PUSHER_IN) == LOW) {}
    set_pusher(false);
    pusher_extended = 0;
    while(digitalRead(FIRE_TRIGGER) == LOW) {} // waiting until the trigger is no longer pressed
  }

  //General tasks
  set_config();
  check_health();
  
}

void set_flywheels(boolean val) {
  //val == HIGH to spin up; val == LOW to spin down
  analogWrite(FLYWHEEL_MOTOR, vel);
}

void set_pusher(boolean val) {
  //val == HIGH to spin up; val == LOW to spin down
  analogWrite(PUSHER_MOTOR, rpm);
}

void set_config() {
  //reading buttons, writing to displays
  if(digitalRead(RPM_SEL) == LOW) {
    digitalWrite(RPM_DISPLAY, HIGH);
    rpm = 255;
  }
  else {
    digitalWrite(RPM_DISPLAY, LOW);
    rpm = LOW_RPM;
  }

  if(digitalRead(VEL_SEL) == LOW) {
    digitalWrite(VEL_DISPLAY, HIGH);
    vel = 255;
  }
  else {
    digitalWrite(VEL_DISPLAY, LOW);
    vel = LOW_VEL;
  }

  if(digitalRead(BURST_SEL) == LOW) {
    digitalWrite(BURST_DISPLAY, HIGH);
    burst = true;
  }
  else {
    digitalWrite(BURST_DISPLAY, LOW);
    burst = false;
  }
}

void check_health() {
  if(analogRead(BAT_VOLT) < BATTERY_THRESHOLD) {
    digitalWrite(HEALTH_DISPLAY, HIGH);
  }
  else {
    digitalWrite(HEALTH_DISPLAY, LOW);
  }
}
