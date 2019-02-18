// Endi Xu
// CSCE236 
// Project 1

#include <Servo.h>

// Init two servos objects
Servo leftServo;   // create servo object to control the servo on the left
Servo rightServo;  // create servo object to control the servo on the right 

uint16_t readLeft = 0; 
uint16_t readRight = 0;

void setup() {

	// The following is for the LED
	// Set pin5,6,7 as output
	DDRD |= (7 << 5);

	// The following is for the switcher
	// Enable pullup resistor for PC4
	pinMode(A3, INPUT_PULLUP);
  
	// The following is the setting for servos
	leftServo.attach(9);     // attaches the servo on pin 9 to the servo object
	rightServo.attach(10);   // attaches the servo on pin 10 to the servo object

	// The following is the setting for sensor
	pinMode(A1, INPUT);       // Right
	pinMode(A2, INPUT);       // Left
	
	Serial.begin(9600);
	
}

//Provide the value of each color
uint8_t ledR() {return (1<<7);}   // Red refers to turn Left
uint8_t ledG() {return (1<<6);}   // Green refers to go Straight  
uint8_t ledB() {return (1<<5);}   // Blue refers to turn Right  
uint8_t ledOFF() {return 0;}	  // Turn off led 	

// Make the car go straight
void goStraight() {
	Serial.println("Go Straight");
	leftServo.write(88);
	rightServo.write(107);
	PORTD = ledG();
	delay(50);  
	PORTD = ledOFF();
}

// Make it turn right
void turnRight() {
	Serial.println("Turn Right");
	rightServo.write(105);
	leftServo.write(100);
	PORTD = ledB();
	delay(50);  
	PORTD = ledOFF();
}

// Make it turn left
void turnLeft() {
	Serial.println("Turn Left");
	rightServo.write(90);
	leftServo.write(95);
	PORTD = ledR();
	delay(50);  
	PORTD = ledOFF();
}

void Stop() {
  rightServo.write(90);
  leftServo.write(100);
  delay(50);
}

// Last state: stop = 0, go straight = 1, turn right = 2, turn left = 3 
uint8_t lastState = 0;

// Button State
// Init two variables for control
uint8_t buttonState = 1;
uint8_t flag = 0;

void loop() {
	buttonState = digitalRead(A3);
	
	if(flag == 0 && buttonState == LOW){
		flag = 1;
		delay(500);
	} else if (flag == 1 && buttonState == LOW) {
		flag = 0;
		delay(500);
	}
	
	if(flag == 1) {
		// Get the value
		readLeft = analogRead(A1);
		readRight = analogRead(A2);
		Serial.print("Left: ");
		Serial.println(readLeft);
		Serial.print("Right: ");
		Serial.print(readRight);
		
		if (readLeft > 400 && readRight <= 400) {
			goStraight(); 
			lastState = 1; 
		} else if (readLeft > 400 && readRight > 400) {
			if(lastState == 3) {
				turnLeft();
				lastState = 3;
			} else {
				turnRight();
				lastState = 2;  
			}  
		} else if (readLeft <= 400 && readRight <= 400 ) {
			if(lastState == 2) {
				turnRight();
				lastState = 2;  
			} else {
				turnLeft();
				lastState = 3;
			}
		} else {
			turnRight();
			lastState = 2;
		}
	}  else {
		Stop();
	}

}

