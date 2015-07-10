#include <TinyGPS.h>
#include <NexStarGPS.h>
#include <SoftwareSerial.h>

#define RX_PIN 3
#define TX_PIN 5
#define RX2_PIN 4
#define TX2_PIN 6
#define RTS_PIN 7
#define CTS_PIN 8

#define SIGNAL_PIN 9
#define LED_PIN 13

SoftwareSerial mountserial(RX_PIN, TX_PIN);
SoftwareSerial sendmountserial(RX2_PIN, TX2_PIN);

TinyGPS gps;

NexstarMessageReceiver msg_receiver;
NexstarMessageSender msg_sender(&gps, RTS_PIN, CTS_PIN)

int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval_nolock = 250;           // interval at which to blink (milliseconds)
long interval_lock = 10;

boolean haveLock = false;

void setup()
{
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);
	pinMode(SIGNAL_PIN, OUTPUT);
	digitalWrite(SIGNAL_PIN, LOW);

	mountserial.begin(19200);

	// Set RTS/CTS to tri-state
	pinModeTri(RTS_PIN);
	pinModeTri(CTS_PIN);
	pinModeTri(RX2_PIN);
	pinModeTri(TX2_PIN);
	msg_receiver.reset();

	// GPS module speed
	Serial.begin(57600);
}

void loop()
{
	unsigned long fix_age;

	if (mountserial.available())
	{
		int c = mountserial.read();
		if (msg_receiver.process(c))
		{
			if (msg_sender.handleMessage(&msg_receiver))
			{
				digitalWrite(LED_PIN, HIGH);
				mountserial.end();
				pinMode(TX2_PIN, OUTPUT);
				digitalWrite(TX2_PIN, HIGH);
				sendmountserial.begin(19200);

				msg_sender.send(&sendmountserial);
				sendmountserial.end();
				pinModeTri(RX2_PIN);
				pinModeTri(TX2_PIN);
				mountserial.begin(19200);
				digitalWrite(LED_PIN, LOW);
			}
		}
	}

	if (Serial.available())
	{
		char c = Serial.read();
		gps.encode(c);

		gps.get_datetime(NULL, NULL, &fix_age);

		if ((fix_age == gps.GPS_INVALID_AGE) || (fix_age > 5000) || (gps.satellites() == gps.GPS_INVALID_SATELLITES) || (gps.satellites() < 4))
		{
			haveLock = false;
		}
		else
		{
			haveLock = true;
		}
	}

	unsigned long currentMillis = millis();

	if (haveLock)
	{
		if (currentMillis - previousMillis > interval_lock)
		{
			previousMillis = currentMillis;
			static int val = 0;
			static int dir = 1;
			analogWrite(SIGNAL_PIN, val);
			if (val == 255)
			{
				dir = dir * (-1);
			}
			val = val + dir;
		}

	}
	else
	{
		if (currentMillis - previousMillis > interval_nolock)
		{
			// save the last time you blinked the LED
			previousMillis = currentMillis;

			// if the LED is off turn it on and vice-versa:
			if (ledState == LOW)
			{
				ledState = HIGH;
			}
			else
			{
				ledState = LOW;
			}

			digitalWrite(SIGNAL_PIN, ledState);
		}
	}
}

inline void pinModeTri(int pin)
{
	digitalWrite(pin, LOW);
	pinMode(pin, OUTPUT);
	pinMode(pin, INPUT);
}