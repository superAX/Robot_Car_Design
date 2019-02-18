uint16_t readLeft = 0; 
uint16_t readRight = 0;

void setup() {
  // put your setup code here, to run once:
  // The following is the setting for sensor
  pinMode(A1, INPUT);         // Left
  pinMode(A2, INPUT);       // Right

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //readLeft = analogRead(A1);
  readRight = analogRead(A2);

  Serial.print("Left:");
  Serial.println(readLeft);
  Serial.print("Right:");
  Serial.println(readRight);

  delay(1000);
}
