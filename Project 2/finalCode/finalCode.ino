// Endi Xu
// CSCE236
// Final Code 

#include <Wire.h>
#include <Servo.h>

#define VCNL4000_ADDRESS 0x13  		// 0x26 write, 0x27 read

// VCNL4000 Register Map
#define COMMAND_0 0x80  			// starts measurments, relays data ready info
#define PRODUCT_ID 0x81  			// product ID/revision ID, should read 0x11
#define IR_CURRENT 0x83  			// sets IR current in steps of 10mA 0-200mA
#define AMBIENT_PARAMETER 0x84  	// Configures ambient light measures
#define AMBIENT_RESULT_MSB 0x85  	// high byte of ambient light measure
#define AMBIENT_RESULT_LSB 0x86  	// low byte of ambient light measure
#define PROXIMITY_RESULT_MSB 0x87  	// High byte of proximity measure
#define PROXIMITY_RESULT_LSB 0x88  	// low byte of proximity measure
#define PROXIMITY_FREQ 0x89  		// Proximity IR test signal freq, 0-3
#define PROXIMITY_MOD 0x8A  		// proximity modulator timing

// Init two servos objects
Servo leftServo;   					// create servo object to control the servo on the left
Servo rightServo; 					// create servo object to control the servo on the right 

// This two is used ti read the light sensors' value
uint16_t readLeft = 0; 
uint16_t readRight = 0;
// This is used to read proximity value
int Pvalue;

// readProximity() returns a 16-bit value from the VCNL4000's proximity data registers
unsigned int readProximity() {
	unsigned int data;
	byte temp;
  
	temp = readByte(COMMAND_0);
	// command the sensor to perform a proximity measure
	writeByte(COMMAND_0, temp | 0x08);  
	
	// Wait for the proximity data ready bit to be set
	while(!(readByte(COMMAND_0)&0x20)); 
	data = readByte(PROXIMITY_RESULT_MSB) << 8;
	data |= readByte(PROXIMITY_RESULT_LSB);
  
	return data;
}


// writeByte(address, data) writes a single byte of data to address
void writeByte(byte address, byte data) {
	Wire.beginTransmission(VCNL4000_ADDRESS);
	Wire.write(address);
	Wire.write(data);
	Wire.endTransmission();
}

// readByte(address) reads a single byte of data from address
byte readByte(byte address) {
	byte data;
  
	Wire.beginTransmission(VCNL4000_ADDRESS);
	Wire.write(address);
	Wire.endTransmission();
	Wire.requestFrom(VCNL4000_ADDRESS, 1);
	while(!Wire.available());
	data = Wire.read();

	return data;
}

void servoAttach() {
	// The following is the setting for servos
	leftServo.attach(9);    			 // attaches the servo on pin 9 to the servo object
	rightServo.attach(10);   			 // attaches the servo on pin 10 to the servo object
}

void servoDettach() {
	// The following is the setting for servos
	leftServo.detach();    			 // attaches the servo on pin 9 to the servo object
	rightServo.detach();   			 // attaches the servo on pin 10 to the servo object
}

void setup() {	
	// Serial's used to debug and print data
	Serial.begin(9600);  			
	// initialize I2C stuff
	Wire.begin();  						
  
	// Test that the device is working correctly 
	byte temp = readByte(PRODUCT_ID);
	// Product ID Should be 0x11
	if (temp != 0x11)  					
	{
		Serial.print("Something's wrong. Not reading correct ID: 0x");
		Serial.println(temp, HEX);
	}
	else {
		Serial.println("VNCL4000 Online...");
	}
	
	//	Now some VNCL400 initialization stuff. Feel free to play with any of these values, but check the datasheet first!
	writeByte(AMBIENT_PARAMETER, 0x0F);  	// Single conversion mode, 128 averages
	writeByte(IR_CURRENT, 20);  			// Set IR current to 200mA
	writeByte(PROXIMITY_FREQ, 2);  			// 781.25 kHz
	writeByte(PROXIMITY_MOD, 0x81); 	    // 129, recommended by Vishay
	
	// The following is for the switcher
	// Enable pullup resistor for PC4
	pinMode(A3, INPUT_PULLUP);
	
	
	// The following is the setting for sensor
	pinMode(A1, INPUT);       // Right
	pinMode(A2, INPUT);       // Left
	
	// The following is for the LED
	// Set pin5,6,7 as output
	DDRD |= (7 << 5);
  
}

//Provide the value of each color
uint8_t ledR() {return (1<<7);}   // Red refers to turn Left
uint8_t ledG() {return (1<<6);}   // Green refers to go Straight  
uint8_t ledB() {return (1<<5);}   // Blue refers to turn Right 
uint8_t ledX() {return (3<<5);}   // X refers to Reverse
uint8_t ledOFF() {return 0;}	  // Turn off led 	

// Make the car go straight
void goStraight() {
	Serial.println("Go Straight");
	leftServo.write(89);
	rightServo.write(107);
	PORTD = ledG();
	delay(100);  
	PORTD = ledOFF();
}

// Make it turn right
void turnRight() {
	Serial.println("Turn Right");
	rightServo.write(106);
	leftServo.write(94);
	PORTD = ledB();
	delay(200);  
	PORTD = ledOFF();
}

// Make it turn left
void turnLeft() {
	Serial.println("Turn Left");
	rightServo.write(102);
	leftServo.write(92);
	PORTD = ledR();
	delay(200);  
	PORTD = ledOFF();
}

// Avoide obstacle from right side
void dodgeRight() {
	uint8_t i;
	for(i=0;i<15;i++) {
		turnRight();
	}
	for(i=0;i<15;i++) {
		turnLeft();
	}
	for(i=0;i<6;i++) {
		goStraight();
	}
}

// Make it turn left
void Reverse() {
	Serial.println("Reverse");
	rightServo.write(81);
	leftServo.write(116);
	PORTD = ledX();
	delay(150);  
	PORTD = ledOFF();
}

// Last state: stop = 0, go straight = 1, turn right = 2, turn left = 3, need dodge = 4 
uint8_t lastState = 0;
// Isblock: No Obstacle = 0, Face Obstacle = 1
uint8_t isBlock = 0;

// Button State
// Init two variables for control
uint8_t buttonState = 1;
uint8_t flag = 0;

void loop() {	
	// Read the buttonState
	buttonState = digitalRead(A3);
	
	// ON/OFF the robot
	if(flag == 0 && buttonState == LOW){
		flag = 1;
		servoAttach();
		delay(500);
	} else if (flag == 1 && buttonState == LOW) {
		flag = 0;
		servoDettach();
		delay(500);
	}
	
	if(flag == 1) {
		// Get the value of the light sensors
		readLeft = analogRead(A1);
		readRight = analogRead(A2);
		Serial.print("Left: ");
		Serial.println(readLeft);
		Serial.print("Right: ");
		Serial.println(readRight);
		// The loop just continuously reads the ambient and proximity values and spits them out over serial. 
		Serial.print("Pvalue =");
		Pvalue = readProximity();
		Serial.println(Pvalue, DEC);
		
		if(Pvalue < 2800 && isBlock == 0) {
			Serial.println("No Obstacle!");
			if (readLeft > 400 && readRight <= 400) {
				goStraight(); 
				lastState = 1; 
			} else if (readLeft > 400 && readRight > 400) {
				if(lastState == 3 || lastState == 4) {
					turnLeft();
					lastState = 3;
				} else {
					turnRight();
					lastState = 2;  
				}  
			} else if (readLeft <= 400 && readRight <= 400 ) {
				if(lastState == 2 || lastState == 4) {
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
		} else if (Pvalue >= 2800 && Pvalue <= 3500 && isBlock == 0) {
			Serial.println("Face Obstacle!");
			uint8_t i;
			for(i = 0; i < 10; i++) {
				Reverse();
			}
			lastState = 4;
			isBlock = 1;
		} else if (Pvalue > 3500 && isBlock == 0) {
			Serial.println("Hit Wall!");
			uint8_t i;
			for(i = 0; i < 10; i++) {
				Reverse();
			}
			lastState = 4;	
		} else if (isBlock == 1) {
			dodgeRight();
			isBlock = 0;
		} 
	}
}

