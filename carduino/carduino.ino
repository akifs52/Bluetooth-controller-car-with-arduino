char command;
int normalSpeed = 175;
int turningSpeed = 175;
String inputString = "";

#define solMotorPos 7
#define solMotorNeg 8
#define sagMotorPos 13
#define sagMotorNeg 12
#define solMotorPWM 9
#define sagMotorPWM 10

void setup() {
  Serial.begin(9600); // HC-05 için 9600 baud

  pinMode(sagMotorPWM, OUTPUT);  // ENA (PWM)
  pinMode(sagMotorPos, OUTPUT);  // IN1
  pinMode(sagMotorNeg, OUTPUT);  // IN2
  pinMode(solMotorPos, OUTPUT);  // IN3
  pinMode(solMotorNeg, OUTPUT);  // IN4
  pinMode(solMotorPWM, OUTPUT);  // ENB (PWM)
}

void loop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      parseCommand(inputString);
      inputString = "";
    } else {
      inputString += inChar;
    }
  }
}

void parseCommand(String cmd) {
  if (cmd.length() == 0) return;

  char prefix = cmd.charAt(0);

  if (prefix == 'F' || prefix == 'B' || prefix == 'L' || prefix == 'R' || prefix == 'S') {
    executeMovement(prefix);
  }
  else if (prefix == 'N') {
    int newSpeed = cmd.substring(1).toInt();
    if (newSpeed >= 0 && newSpeed <= 255) {
      normalSpeed = newSpeed;
    }
  }
  else if (prefix == 'T') {
    int newSpeed = cmd.substring(1).toInt();
    if (newSpeed >= 0 && newSpeed <= 255) {
      turningSpeed = newSpeed;
    }
  }
}

void executeMovement(char command) {
  switch (command) {
    case 'F': // ileri
      digitalWrite(sagMotorPos, HIGH);
      digitalWrite(sagMotorNeg, LOW);
      digitalWrite(solMotorPos, HIGH);
      digitalWrite(solMotorNeg, LOW);
      analogWrite(sagMotorPWM, normalSpeed);
      analogWrite(solMotorPWM, normalSpeed);
      break;

    case 'B': // geri
      digitalWrite(sagMotorPos, LOW);
      digitalWrite(sagMotorNeg, HIGH);
      digitalWrite(solMotorPos, LOW);
      digitalWrite(solMotorNeg, HIGH);
      analogWrite(sagMotorPWM, normalSpeed);
      analogWrite(solMotorPWM, normalSpeed);
      break;

    case 'R': // sola
      digitalWrite(sagMotorPos, LOW);
      digitalWrite(sagMotorNeg, HIGH);
      digitalWrite(solMotorPos, HIGH);
      digitalWrite(solMotorNeg, LOW);
      analogWrite(sagMotorPWM, turningSpeed);
      analogWrite(solMotorPWM, turningSpeed);
      break;

    case 'L': // sağa
      digitalWrite(sagMotorPos, HIGH);
      digitalWrite(sagMotorNeg, LOW);
      digitalWrite(solMotorPos, LOW);
      digitalWrite(solMotorNeg, HIGH);
      analogWrite(sagMotorPWM, turningSpeed);
      analogWrite(solMotorPWM, turningSpeed);
      break;

    case 'S': // dur
      digitalWrite(sagMotorPos, LOW);
      digitalWrite(sagMotorNeg, LOW);
      digitalWrite(solMotorPos, LOW);
      digitalWrite(solMotorNeg, LOW);
      analogWrite(sagMotorPWM, 0);
      analogWrite(solMotorPWM, 0);
      break;
  }
}