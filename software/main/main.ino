#include <Wire.h>

const int TEMP_PIN = 0;
const int LED_PIN = 13;
const int NUM_TEMP_SENSORS = 16;

// Temperature when the ADC is at 0V in Celsius
const float TEMP_AT_0V = 120.0;
const float TEMP_AT_5V = -40.0;

const float TEMP_MAX = 60.0;

byte selectArray[16][4]; // Hold 0000 in selectArray[0][0-3], 0001 in [1][0-3], etc.
const int selectLines[4] = { 5, 4, 3, 2 }; // Pins to multiplexer select lines

void setup() {
    pinMode(LED_PIN, OUTPUT);
    for (int i = 0; i < 4; i++) {
        pinMode(selectLines[i], OUTPUT);
    }

    pinMode(TEMP_PIN, INPUT);

    for (int num = 0; num < 16; num++) {
        for (int bit = 3; bit >= 0; bit--) {
            selectArray[num][3 - bit] = (num >> bit) & 1; // Bit shift num by bit times and mask the lowest bit
        }
    }

    Wire.begin();
    Serial.begin(9600);
}

void loop() {
    float highestTemp = 0;
    int highestTempChannel = 0;

    // For each MUX channel
    for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
        selectMuxChannel(i);
        delayMicroseconds(20);
        float tmpTemp = analogRead(TEMP_PIN);
        // Linear interpolation will need to take place!!!!
        if (tmpTemp > highestTemp) {
            highestTemp = tmpTemp;
            highestTempChannel = i;
        }
        // Turn on LED if the temperature is too high
        if (tmpTemp > TEMP_MAX) {
            digitalWrite(LED_PIN, HIGH);
        }
    }

    // Report highest temperature via I2C
    Wire.beginTransmission(0x67);
    Wire.write(highestTemp); // Do we also want the channel number?
    Wire.endTransmission();
    delay(500);

    // // Get an average of temperature readings
    // double sum = 0.0;
    // for (int i = 0; i < 10; i++) {
    //     sum += analogRead(TEMP_PIN);
    //     delay(2);
    // }
}

void selectMuxChannel(int channelNum) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(selectLines[i], selectArray[channelNum][i]);
    }
}