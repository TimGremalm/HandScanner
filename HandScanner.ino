#include <Servo.h>
#include <math.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define WS2812PIN 9
#define WS2812PIXELS 13
Adafruit_NeoPixel strip = Adafruit_NeoPixel(WS2812PIXELS, WS2812PIN, NEO_GRB + NEO_KHZ800);

const int pinServo = 3;
const int pinAnalogMeterA = 6;
const int pinAnalogMeterB = 5;

Servo servo;
const int servoMin = 0;
const int servoMax = 45;

enum ServoState {
	Up,
	Down
};
int servoPosition = 0;
ServoState servoCurrentState = Up;

unsigned long meterLastChanged = 0;
unsigned long meterChangeIn = 0;

unsigned long ledDataIndicatorLastChanged = 0;
unsigned long ledDataIndicatorChangeIn = 0;
uint32_t ledDataIndicatorColor;

unsigned long ledFluxIndicatorLastChanged = 0;
unsigned long ledFluxIndicatorChangeIn = 0;
uint32_t ledFluxIndicatorColor;

float hueToRgb(float p, float q, float t) {
	if (t < (float)0)
		t += (float)1;
	if (t > (float)1)
		t -= (float)1;
	if (t < (float)1/(float)6)
		return p + (q - p) * (float)6 * t;
	if (t < (float)1/(float)2)
		return q;
	if (t < (float)2/(float)3)
		return p + (q - p) * ((float)2.0/ (float)3.0 - t) * (float)6;
	return p;
}
void hslToRgb(float hue, float saturation, float lightness, int rgb[]) {
	float r, g, b;

	if (saturation == (float)0) {
		r = g = b = lightness; // achromatic
	} else {
		float q = lightness < 0.5 ? lightness * (1 + saturation) : lightness + saturation - lightness * saturation;
		float p = 2 * lightness - q;
		r = hueToRgb(p, q, hue + (float)1/(float)3);
		g = hueToRgb(p, q, hue);
		b = hueToRgb(p, q, hue - (float)1/(float)3);
	}
	rgb[0] = (int) (r * 255);
	rgb[1] = (int) (g * 255);
	rgb[2] = (int) (b * 255);
}
uint32_t HSLtoColor(float H, float S, float L) {
	int nRGB[3] = {0,0,0};
	hslToRgb(H, S, L, nRGB);
	return strip.Color(nRGB[0], nRGB[1], nRGB[2]);
}

void moveServo() {
	switch (servoCurrentState) {
		case Up:
			servoPosition += 1;
			break;
		case Down:
			servoPosition -= 1;
			break;
	}
	if (servoPosition >= servoMax) {
		servoCurrentState = Down;
		servoPosition = servoMax;
	}
	if (servoPosition <= servoMin) {
		servoCurrentState = Up;
		servoPosition = servoMin;
	}
	//Serial.println(servoPosition);
	servo.write(servoPosition);
}

void moveMeterWithServo() {
	int meterOut = servoPosition * 5;
	analogWrite(pinAnalogMeterA, meterOut);
}

void randomizeMeter() {
	unsigned long delta;
	delta = millis() - meterLastChanged;
	if (delta >= meterChangeIn) {
		int out = random(0, 255);
		analogWrite(pinAnalogMeterB, out);
		meterChangeIn = random(500, 4000);
		//Serial.println("Time to change meter");
		//Serial.println(meterChangeIn);
		meterLastChanged = millis();
	}
}

uint32_t getLedDataIndicatorColor() {
	int randomHue = random(0, 359);
	float LedHue = (float)randomHue / 360;
	return (HSLtoColor(LedHue, 1.0, 0.05));
}

void randomizeDataIndicator() {
	unsigned long delta;
	delta = millis() - ledDataIndicatorLastChanged;
	if (delta >= ledDataIndicatorChangeIn) {
		ledDataIndicatorColor = getLedDataIndicatorColor();
		ledDataIndicatorChangeIn = random(100, 500);
		//Serial.println("Time to change data indicator");
		//Serial.println(ledDataIndicatorChangeIn);
		ledDataIndicatorLastChanged = millis();
	}
}

void randomizeFluxIndicator() {
	unsigned long delta;
	delta = millis() - ledFluxIndicatorLastChanged;
	if (delta >= ledFluxIndicatorChangeIn) {
		ledFluxIndicatorColor = getLedDataIndicatorColor();
		ledFluxIndicatorChangeIn = random(800, 1400);
		//Serial.println("Time to change data indicator");
		//Serial.println(ledFluxIndicatorChangeIn);
		ledFluxIndicatorLastChanged = millis();
	}
}

void setWS2812() {
	for(int i = 0; i < WS2812PIXELS; i += 1) {
		//Right LED Flux Indicator
		if (i >= 0 && i <= 4) {
			strip.setPixelColor(i, ledFluxIndicatorColor);
		}
		
		//Left LED Data Indicator
		if (i >= 8 && i <= 13) {
			strip.setPixelColor(i, ledDataIndicatorColor);
		}
	}
	strip.show();
}

void setup() {
	Serial.begin(9600);
	servo.attach(3);
	randomSeed(analogRead(0));
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}

void loop() {
	moveServo();
	moveMeterWithServo();
	randomizeMeter();
	randomizeDataIndicator();
	randomizeFluxIndicator();
	setWS2812();
	delay(15);
}
