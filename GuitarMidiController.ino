#define OPERATIONAL_STATE HIGH
#define CONFIGURATION_STATE LOW

#define DEBUG

#ifdef DEBUG
#define print(a) Serial.print(a);
#define println(a) Serial.println(a);
#else
#define print(a) 
#define println(a) 
#endif

#include <MIDIUSB.h>
#include <EEPROM.h>

byte previousCtrlValues[6];
byte ctrlPins[6];
byte ctrlNumbers[6];
byte state = 0;


int SCLK = 5;
int RCLK = 4;
int DIO = 3;

byte switchPin = 13;

unsigned char _LED_0F[29];

int lastChangedCtrl = 0;

int blinkIntervalMs = 1;
bool displayBlinkOff = false;
byte  previousState = OPERATIONAL_STATE;

void setup() {
	state = digitalRead(switchPin);
	previousState = state;
	
	ctrlPins[0] = A0;
	ctrlPins[1] = A1;
	ctrlPins[2] = A2;
	ctrlPins[3] = A3;
	ctrlPins[4] = A4;
	ctrlPins[5] = A5;

	pinMode(SCLK, OUTPUT);
	pinMode(RCLK, OUTPUT);
	pinMode(DIO, OUTPUT);
	pinMode(switchPin, INPUT);
	
	for ( int i = 0 ; i < 6 ; i++ ) {
		pinMode(ctrlPins[i], INPUT );
		// turn on the pullup resistor.
		digitalWrite(ctrlPins[i], HIGH);
		
		byte ctrlNumber = EEPROM.read(i);
		if ( ctrlNumber > 128 )
			ctrlNumber = 1;
		ctrlNumbers[i] = ctrlNumber;
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

void display(byte controller, byte value, bool blink ) {
	if ( blink ) {
		blinkIntervalMs--;
		if ( blinkIntervalMs == 0 ) {
			println("Blink interval expired")
			displayBlinkOff = !displayBlinkOff;
			blinkIntervalMs = 30;
		}		
		if(displayBlinkOff)
			return;
	}
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
	
	// switch off all digits so to have same intensity for all 4 digits
	 
	shiftOut(DIO, SCLK, MSBFIRST, (0b0000));
	digitalWrite(RCLK, LOW);
	digitalWrite(RCLK, HIGH);	
}

void sendMIDI(byte ctrlNumber, byte ctrlValue) {
	midiEventPacket_t event = {0x0B, 0xB0 | 1, ctrlNumber, ctrlValue};				
	MidiUSB.sendMIDI(event);
	MidiUSB.flush();	
}

void saveTOEEPROM() {
	println("Updating EEPROM");
	for (int i = 0 ; i < 6 ; i++) {
		EEPROM.update(i, ctrlNumbers[i]);
	}
}

void loop() {

	state = digitalRead(switchPin);
	if ( state != previousState) {
		print("State switched to ")
		previousState = state;
		// switched state should hold for at least 10 cycles before doing the actual switch, in order to filter out noise.
		if (state == OPERATIONAL_STATE) {			
			println("Operational state");
			saveTOEEPROM();
		} else {
			println("Configuration state");
		}
	}

	byte ctrlValues[6];
	for (int i = 0; i < 6 ; i++ ) {
		byte count[128] = {0};
		// some magic to get stable reading, basically im sampling the analog input multiple times, and then take the value that
		// had most occurences as read value. I tried without multiple sampling and the reading was not stable...
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;
		count[analogRead(ctrlPins[i]) >> 3]++;		
		int maxIndex = 0;
		for ( int j = 0 ; j < 128 ; j++ ) {
			if ( count[j] > count[maxIndex] ) {
				maxIndex = j;
			}
		}
		ctrlValues[i] =  maxIndex;
		//print("ctrl ");print(i);print(" value "); println(ctrlValues[i]);				
		if ( ctrlValues[i] != previousCtrlValues[i]) {
			previousCtrlValues[i] = ctrlValues[i];
			lastChangedCtrl = i;
			if (state == OPERATIONAL_STATE)
				sendMIDI(ctrlNumbers[i], ctrlValues[i]);
			else
				ctrlNumbers[i] = ctrlValues[i];
			break;
		}
	}
	display(lastChangedCtrl, previousCtrlValues[lastChangedCtrl], state == CONFIGURATION_STATE ? true : false );
	
	delay(1);
}
