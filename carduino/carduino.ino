#include <SoftwareSerial.h>

char command;
int normalSpeed = 100;
int turningSpeed = 100;
String inputString = "";

// Batarya koruma bayrağı
bool batteryCutoffActive = false;

// SoftwareSerial için Bluetooth pinleri
#define BT_TX 4  // Arduino TX -> Bluetooth RX
#define BT_RX 5  // Arduino RX <- Bluetooth TX
SoftwareSerial bluetooth(BT_RX, BT_TX);

// Motor pinleri
#define solMotorPos 7
#define solMotorNeg 8
#define sagMotorPos 13
#define sagMotorNeg 12
#define solMotorPWM 9
#define sagMotorPWM 10

// Batarya ölçüm pin'i
#define BATTERY_PIN A0

// Voltaj bölücü oranı: R2 / (R1 + R2) = 10 / 25 = 0.4
// Eğer 12.5V sabit çıkıyorsa ve gerçek voltaj 7V ise:
// 7V / 12.5V = 0.56 (doğru oran)
#define DIVIDER_RATIO 0.56

// ADC referans voltajı
#define REF_VOLTAGE 5.0

// Batarya ölçüm zamanlayıcısı
unsigned long lastBatteryCheck = 0;
const unsigned long BATTERY_CHECK_INTERVAL = 3000; // 3 saniyede bir

// ------------------- CIRCULAR BUFFER -------------------
#define BUFFER_SIZE 64
char commandBuffer[BUFFER_SIZE];
int head = 0;
int tail = 0;
char lastCommand = '\0'; // Tekrar komutları önlemek için

bool isBufferEmpty() {
    return head == tail;
}

bool isBufferFull() {
    return ((head + 1) % BUFFER_SIZE) == tail;
}

void enqueueCommand(char cmd) {
    if (!isBufferFull()) {
        commandBuffer[head] = cmd;
        head = (head + 1) % BUFFER_SIZE;
    }
}

char dequeueCommand() {
    if (!isBufferEmpty()) {
        char cmd = commandBuffer[tail];
        tail = (tail + 1) % BUFFER_SIZE;
        return cmd;
    }
    return '\0';
}

void clearBuffer() {
    head = 0;
    tail = 0;
}
// --------------------------------------------------------

void setup() {
  Serial.begin(115200); // Debug için 115200 baud
  bluetooth.begin(9600); // HC-05 için 9600 baud

  pinMode(sagMotorPWM, OUTPUT);  // ENA (PWM)
  pinMode(sagMotorPos, OUTPUT);  // IN1
  pinMode(sagMotorNeg, OUTPUT);  // N2
  pinMode(solMotorPos, OUTPUT);  // IN3
  pinMode(solMotorNeg, OUTPUT);  // IN4
  pinMode(solMotorPWM, OUTPUT);  // ENB (PWM)
  
  Serial.println("Arduino başlatıldı - Bluetooth D4/D5");
}

void loop() {
  // Bluetooth'tan gelenleri buffer'a ekle
  while (bluetooth.available()) {
    char inChar = (char)bluetooth.read();
    if (inChar == '\n') {
      if (inputString.length() > 0) {
          char prefix = inputString.charAt(0);
          if (prefix == 'F' || prefix == 'B' || prefix == 'L' || prefix == 'R' || prefix == 'S') {
              enqueueCommand(prefix);
          } else if (prefix == 'N' || prefix == 'T') {
              parseCommand(inputString); // Speed komutları direkt işlenir
          }
          inputString = "";
      }
    } else {
      inputString += inChar;
    }
  }

  // Buffer'dan komut varsa işle
  if (!isBufferEmpty()) {
    char cmd = dequeueCommand();
    if (cmd != lastCommand) { // Aynı komutu tekrar etme
        executeMovement(cmd);
        lastCommand = cmd;
    }
  }
  
  // Batarya durumunu gönder
  sendBatteryStatus();
}

void parseCommand(String cmd) {
  if (cmd.length() == 0) return;

  char prefix = cmd.charAt(0);

  if (prefix == 'N') {
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
  // Batarya koruması aktifse motorları çalıştırma
  if (batteryCutoffActive && command != 'S') {
    return; // Sadece STOP komutuna izin ver
  }
  
  // Komutları işle
  switch (command) {
    case 'F': // İleri
      digitalWrite(sagMotorPos, HIGH);
      digitalWrite(sagMotorNeg, LOW);
      digitalWrite(solMotorPos, LOW);
      digitalWrite(solMotorNeg, HIGH);
      analogWrite(sagMotorPWM, normalSpeed);
      analogWrite(solMotorPWM, normalSpeed);
      break;

    case 'B': // Geri
      digitalWrite(sagMotorPos, LOW);
      digitalWrite(sagMotorNeg, HIGH);
      digitalWrite(solMotorPos, HIGH);
      digitalWrite(solMotorNeg, LOW);
      analogWrite(sagMotorPWM, normalSpeed);
      analogWrite(solMotorPWM, normalSpeed);
      break;

    case 'R': // Sağa
      digitalWrite(sagMotorPos, HIGH);
      digitalWrite(sagMotorNeg, LOW);
      digitalWrite(solMotorPos, HIGH);
      digitalWrite(solMotorNeg, LOW);
      analogWrite(sagMotorPWM, turningSpeed);
      analogWrite(solMotorPWM, turningSpeed);
      break;

    case 'L': // Sola
      digitalWrite(sagMotorPos, LOW);
      digitalWrite(sagMotorNeg, HIGH);
      digitalWrite(solMotorPos, LOW);
      digitalWrite(solMotorNeg, HIGH);
      analogWrite(sagMotorPWM, turningSpeed);
      analogWrite(solMotorPWM, turningSpeed);
      break;

    case 'S': // Dur
      clearBuffer();   // 
      digitalWrite(sagMotorPos, LOW);
      digitalWrite(sagMotorNeg, LOW);
      digitalWrite(solMotorPos, LOW);
      digitalWrite(solMotorNeg, LOW);
      analogWrite(sagMotorPWM, 0);
      analogWrite(solMotorPWM, 0);
      lastCommand = '\0'; // STOP sonrası tekrar işleme hazır
      break;
  }
}

// ------------------- BATARYA ÖLÇÜM FONKSİYONLARI -------------------

// Önceki okumayı sakla için
float lastStableVoltage = 0.0;

float readBatteryVoltage() {
  float sum = 0;
  float minVal = 1023.0;
  float maxVal = 0.0;

  // Daha stabil ölçüm için 20 örnek al ve outlier'ları temizle
  for (int i = 0; i < 20; i++) {
    float reading = analogRead(BATTERY_PIN);
    sum += reading;
    if (reading < minVal) minVal = reading;
    if (reading > maxVal) maxVal = reading;
    delay(2);
  }

  // En düşük ve en yüksek değerleri çıkar (outlier temizliği)
  sum = sum - minVal - maxVal;
  float avg = sum / 18.0; // 18 örnek (20-2)

  // Arduino pin voltajı
  float voltage = avg * (REF_VOLTAGE / 1023.0);

  // DEBUG: Ham değerleri yazdır
  Serial.print("DEBUG - Ham ADC: ");
  Serial.print(avg);
  Serial.print(", Pin Voltajı: ");
  Serial.print(voltage, 3);
  Serial.print("V, DIVIDER_RATIO: ");
  Serial.print(DIVIDER_RATIO, 3);

  // Gerçek batarya voltajı
  float batteryVoltage = voltage / DIVIDER_RATIO;
  
  Serial.print(", Gerçek Batarya Voltajı: ");
  Serial.print(batteryVoltage, 3);
  Serial.println("V");

  // Low-pass filter geçici olarak devre dışı (debug için)
  // if (lastStableVoltage > 0.0) {
  //   float diff = abs(batteryVoltage - lastStableVoltage);
  //   if (diff > 0.3) { // 0.3V'den fazla değişim varsa yumuşat
  //     batteryVoltage = (lastStableVoltage * 0.7) + (batteryVoltage * 0.3);
  //   }
  // }

  lastStableVoltage = batteryVoltage;
  return batteryVoltage;
}

int getBatteryPercentage(float voltage) {
  // 2S Li-ion için (6.0V - 8.5V)
  
  if (voltage >= 8.5) return 100;
  if (voltage >= 8.0) return map(voltage * 100, 800, 850, 80, 100);
  if (voltage >= 7.4) return map(voltage * 100, 740, 800, 50, 80);
  if (voltage >= 6.8) return map(voltage * 100, 680, 740, 20, 50);
  if (voltage >= 6.0) return map(voltage * 100, 600, 680, 5, 20);

  return 0;
}

void checkBatteryCutoff(float voltage) {
  // Batarya koruma cutoff - 6.3V altında sistemi kapat
  if (voltage < 6.3 && !batteryCutoffActive) {
    bluetooth.println("LOW_BATTERY_CUTOFF");
    batteryCutoffActive = true;
    
    // Tüm motorları durdur
    digitalWrite(sagMotorPos, LOW);
    digitalWrite(sagMotorNeg, LOW);
    digitalWrite(solMotorPos, LOW);
    digitalWrite(solMotorNeg, LOW);
    analogWrite(sagMotorPWM, 0);
    analogWrite(solMotorPWM, 0);
    
    // Buffer'ı temizle ve komutları durdur
    head = 0;
    tail = 0;
    lastCommand = '\0';
  }
  // Batarya yükselirse korumayı kaldır (6.5V üstü)
  else if (voltage > 6.5 && batteryCutoffActive) {
    batteryCutoffActive = false;
    bluetooth.println("BATTERY_PROTECTION_RESET");
  }
}

void sendBatteryStatus() {
  // Her 3 saniyede bir batarya durumunu gönder (sadece komut akmıyorken)
  if (millis() - lastBatteryCheck >= BATTERY_CHECK_INTERVAL && isBufferEmpty()) {
    lastBatteryCheck = millis();
    
    float voltage = readBatteryVoltage();
    int percentage = getBatteryPercentage(voltage);
    
    // Batarya korumasını kontrol et
    checkBatteryCutoff(voltage);
    
    // Qt uygulamasına batarya bilgisini gönder
    bluetooth.print("VOLT:");
    bluetooth.print(voltage, 2);
    bluetooth.print("V|BATT:");
    bluetooth.print(percentage);
    bluetooth.println("%");
  }
}