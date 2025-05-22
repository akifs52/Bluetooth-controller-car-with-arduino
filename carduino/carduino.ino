char command;

#define solMotorPos 7
#define solMotorNeg 8
#define sagMotorPos 13
#define sagMotorNeg 12
#define solMotorPWM 9
#define sagMotorPWM 10
#define normalSpeed 175
#define turningSpeed 175

void setup() {
  Serial.begin(9600); // Bluetooth HC-05 9600 baud ile çalışır

  pinMode(sagMotorPWM, OUTPUT);  // ENA (PWM)
  pinMode(sagMotorPos, OUTPUT);  // IN1
  pinMode(sagMotorNeg, OUTPUT);  // IN2
  pinMode(solMotorPos, OUTPUT); // IN3
  pinMode(solMotorNeg, OUTPUT); // IN4
  pinMode(solMotorPWM, OUTPUT); // ENB (PWM)
}

void loop() {
  if (Serial.available()) {
    command = Serial.read();

    switch (command) {
      case 'F': // ileri
        digitalWrite(sagMotorPos, HIGH);
        digitalWrite(sagMotorNeg, LOW);
        digitalWrite(solMotorPos, HIGH);
        digitalWrite(solMotorNeg, LOW);
        analogWrite(sagMotorPWM, normalSpeed);  // Hız (0-255)
        analogWrite(solMotorPWM, normalSpeed);
        break;

      case 'B': // geri
        digitalWrite(sagMotorPos, LOW);
        digitalWrite(sagMotorNeg, HIGH);
        digitalWrite(solMotorPos, LOW);
        digitalWrite(solMotorNeg, HIGH);
        analogWrite(sagMotorPWM, normalSpeed);  // Hız (0-255)
        analogWrite(solMotorPWM, normalSpeed);
        break;

      case 'R': // sola
        digitalWrite(sagMotorPos, LOW);
        digitalWrite(sagMotorNeg, HIGH);
        digitalWrite(solMotorPos, HIGH);
        digitalWrite(solMotorNeg, LOW);
        analogWrite(sagMotorPWM, turningSpeed);  // Hız (0-255)
        analogWrite(solMotorPWM, turningSpeed);
        break;

      case 'L': // sağa
        digitalWrite(sagMotorPos, HIGH);
        digitalWrite(sagMotorNeg, LOW);
        digitalWrite(solMotorPos, LOW);
        digitalWrite(solMotorNeg, HIGH);
        analogWrite(sagMotorPWM, turningSpeed);  // Hız (0-255)
        analogWrite(solMotorPWM, turningSpeed);
        break;

      case 'S': // dur
        digitalWrite(sagMotorPos, LOW);
        digitalWrite(sagMotorNeg, LOW);
        digitalWrite(solMotorPos, LOW);
        digitalWrite(solMotorNeg, LOW);
        analogWrite(sagMotorPWM, 0);  // Hız (0-255)
        analogWrite(solMotorPWM, 0);
        break;
    }
  }
}
