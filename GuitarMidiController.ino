#define DEBUG

#ifdef DEBUG
#define print(a) Serial.print(a);
#define println(a) Serial.println(a);
#else
#define print(a) 
#define println(a) 
#endif

#include <MIDIUSB.h>

int previousCtrlValues[6];
int ctrlIds[6];

int SCLK = 5;
int RCLK = 4;
int DIO = 3;

unsigned char _LED_0F[29];

int lastChangedCtrl = 0;

void setup() {
	ctrlIds[0] = A0;
	ctrlIds[1] = A1;
	ctrlIds[2] = A2;
	ctrlIds[3] = A3;
	ctrlIds[4] = A4;
	ctrlIds[5] = A5;

	pinMode(SCLK, OUTPUT);
	pinMode(RCLK, OUTPUT);
	pinMode(DIO, OUTPUT);
	
	for ( int i = 0 ; i < 6 ; i++ ) {
		pinMode(ctrlIds[i], INPUT );
		digitalWrite(ctrlIds[i], HIGH);
	}

	_LED_0F[0] = 0xC0; //0
	_LED_0F[1] = 0xF9; //1
	_LED_0F[2] = 0xA4; //2
	_LED_0F[3] = 0xB0; //3
	_LED_0F[4] = 0x99; //4
	_LED_0F[5] = 0x92; //5
	_LED_0F[6] = 0x82; //6
	_LED_0F[7] = 0xF8; //7
	_LED_0F[8] = 0x80; //8
	_LED_0F[9] = 0x90; //9
	_LED_0F[10] = 0x88; //A
	_LED_0F[11] = 0x83; //b
	_LED_0F[12] = 0xC6; //C
	_LED_0F[13] = 0xA1; //d
	_LED_0F[14] = 0x86; //E
	_LED_0F[15] = 0x8E; //F
	_LED_0F[16] = 0xC2; //G
	_LED_0F[17] = 0x89; //H
	_LED_0F[18] = 0xF9; //I
	_LED_0F[19] = 0xF1; //J
	_LED_0F[20] = 0xC3; //L
	_LED_0F[21] = 0xA9; //n
	_LED_0F[22] = 0xC0; //O
	_LED_0F[23] = 0x8C; //P
	_LED_0F[24] = 0x98; //q
	_LED_0F[25] = 0x92; //S
	_LED_0F[26] = 0xC1; //U
	_LED_0F[27] = 0x91; //Y
	_LED_0F[28] = 0xFE; //hight -
	
}

void display(byte controller, byte value ) {
	shiftOut(DIO, SCLK, MSBFIRST, _LED_0F[value%10]);	
	shiftOut(DIO, SCLK, MSBFIRST, (0b0001));
	
	digitalWrite(RCLK, LOW);
	digitalWrite(RCLK, HIGH);	
	
	shiftOut(DIO, SCLK, MSBFIRST, (_LED_0F[(value/10)%10]));
	shiftOut(DIO, SCLK, MSBFIRST, (0b0010));
	
	digitalWrite(RCLK, LOW);
	digitalWrite(RCLK, HIGH);

	shiftOut(DIO, SCLK, MSBFIRST, (_LED_0F[(value/100)%10]));
	shiftOut(DIO, SCLK, MSBFIRST, (0b0100));
	
	digitalWrite(RCLK, LOW);
	digitalWrite(RCLK, HIGH);
	
	shiftOut(DIO, SCLK, MSBFIRST, (_LED_0F[controller]));
	shiftOut(DIO, SCLK, MSBFIRST, (0b1000));
	
	digitalWrite(RCLK, LOW);
	digitalWrite(RCLK, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
	int ctrlValues[6];	
	for (int i = 0; i < 6 ; i++ ) {
		byte count[128] = {0};
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;
		count[analogRead(ctrlIds[i]) >> 3]++;		
		int maxIndex = 0;
		for ( int j = 0 ; j < 128 ; j++ ) {
			if ( count[j] > count[maxIndex] ) {
				maxIndex = j;
			}
		}
		ctrlValues[i] =  maxIndex;
		print("ctrl ");print(i);print(" value "); println(ctrlValues[i]);				
		if ( ctrlValues[i] != previousCtrlValues[i]) {
			previousCtrlValues[i] = ctrlValues[i];
			lastChangedCtrl = i;
			if ( i == 5 ) {
				midiEventPacket_t event = {0x0B, 0xB0 | 1, 74, ctrlValues[i]};
				MidiUSB.sendMIDI(event);
				MidiUSB.flush();
				}
			if ( i == 2 ) {
				midiEventPacket_t event = {0x0B, 0xB0 | 1, 71, ctrlValues[i]};				
				MidiUSB.sendMIDI(event);
				MidiUSB.flush();
			}				
			break;
		}
	}
	display(lastChangedCtrl, previousCtrlValues[lastChangedCtrl] );
	delay(1);
}
