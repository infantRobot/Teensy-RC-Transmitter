#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const uint8_t THROTTLE_PIN = A0;
const uint8_t YAW_PIN = A1;
const uint8_t PITCH_PIN = A2;
const uint8_t ROLL_PIN = A3;

//Holds the Min, mid and max value reading from pots
const int t_states[3] = { 0, 511, 1023 };
const int y_states[3] = { 0, 511, 1023 };
const int p_states[3] = { 0, 511, 1023 };
const int r_states[3] = { 0, 511, 1023 };

const uint8_t LEFT_TRIGGER_PIN = 1;
const uint8_t RIGHT_TRIGGER_PIN = 2;

const uint8_t SWITCH_ONE_PIN = A4;
const uint8_t SWITCH_TWO_PIN = A5;
const uint8_t SWITCH_THREE_PIN = A6;
const uint8_t SWITCH_FOUR_PIN = A7;

//1/3 and 2/3 break points for values of tri state switch
const uint16_t LOW_MID_BREAK = 340;
const uint16_t HIGH_MID_BREAK = 680;

const uint64_t pipeOut = 0xE8E8F0F0E1LL;

const uint8_t STATE_ONE = 0;
const uint8_t STATE_TWO = 127;
const uint8_t STATE_THREE = 255;


//Initialize the radio module
RF24 radio(9, 10);

struct MyData
{
	byte throttle;
	byte yaw;
	byte pitch;
	byte roll;
	byte mode;
	byte collective;
};

MyData data;

void setup()
{
	radio.begin();
	radio.setAutoAck(false);
	radio.setDataRate(RF24_250KBPS);

	radio.openWritingPipe(pipeOut);

	resetData();

}


void loop()
{
	// The calibration numbers used here should be measured 
	// for your joysticks using the TestJoysticks sketch.
	data.throttle = mapJoystickValues(analogRead(THROTTLE_PIN), t_states[0], t_states[1], t_states[2], true);
	data.yaw = mapJoystickValues(analogRead(YAW_PIN), y_states[0], y_states[1], y_states[2], true);
	data.pitch = mapJoystickValues(analogRead(PITCH_PIN), p_states[0], p_states[1], p_states[2], true);
	data.roll = mapJoystickValues(analogRead(ROLL_PIN), r_states[0], r_states[1], r_states[2], true);
	data.mode = readTriButton(SWITCH_ONE_PIN);
	data.collective = readTriButton(SWITCH_TWO_PIN);

	radio.write(&data, sizeof(MyData));
}

uint8_t readTriButton(uint8_t button)
{
	uint8_t val = analogRead(button);

	if (val < LOW_MID_BREAK)
	{
		return STATE_ONE;
	}
	else if (val < HIGH_MID_BREAK)
	{
		return STATE_TWO;
	}
	else
	{
		return STATE_THREE;
	}
	
}


void resetData()
{
	data.throttle = 0;
	data.pitch = 127;
	data.roll = 127;
	data.yaw = 127;
	data.mode = 0;
	data.collective = 0;
}

// Returns a corrected value for a joystick position that takes into account
// the values of the outer extents and the middle of the joystick range.
int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse)
{
	val = constrain(val, lower, upper);
	if (val < middle)
		val = map(val, lower, middle, 0, 128);
	else
		val = map(val, middle, upper, 128, 255);
	return (reverse ? 255 - val : val);
}
