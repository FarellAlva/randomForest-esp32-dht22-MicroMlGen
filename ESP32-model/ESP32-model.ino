int pot1Pin = 34;
int pot2Pin = 35;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int pot1Raw = analogRead(pot1Pin);
  int pot2Raw = analogRead(pot2Pin);

  float pot1Percent = (pot1Raw / 4095.0) * 100.0;
  float pot2Percent = (pot2Raw / 4095.0) * 100.0;

  Serial.print("Pot1: ");
  Serial.print(pot1Percent, 2);  // 2 angka di belakang koma
  Serial.print("% | Pot2: ");
  Serial.print(pot2Percent, 2);
  Serial.println("%");

  delay(500);
}
