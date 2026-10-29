#include <Wire.h>

const int TEMP_PIN = 0;
const int LED_PIN = 13;
const int NUM_TEMP_SENSORS = 16;
const float TEMP_MAX = 60.0;

const float GAIN = 2.63;
const float VOLTAGE_OFFSET = -2.419;

const int temps[33] = { -40, -35, -30, -25, -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120 };
const float volts[33] = { 2.44, 2.42, 2.40, 2.38, 2.35, 2.32, 2.27, 2.23, 2.17, 2.11, 2.05, 1.99, 1.92, 1.86, 1.80, 1.74, 1.68, 1.63, 1.59, 1.55, 1.51, 1.48, 1.45, 1.43, 1.40, 1.38, 1.37, 1.35, 1.34, 1.33, 1.32, 1.31, 1.30 };

byte selectArray[16][4]; // Hold 0000 in selectArray[0][0-3], 0001 in [1][0-3], etc.
const int selectLines[4] = { 5, 4, 3, 2 }; // Pins to multiplexer select lines

void selectMuxChannel(int channelNum);
float calculateTemp(float voltDiode);

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
    float highestTemp = 0.0;
    uint8_t highestTempChannel = 0;

    // For each MUX channel
    for (int i = 0; i < NUM_TEMP_SENSORS; i++) {
        selectMuxChannel(i);
        delayMicroseconds(115);
        float tmpTemp = analogRead(TEMP_PIN);
        // Calculate the V_diode (V_out in datasheet of temperature diodes)
        float voltDiode = (tmpTemp - VOLTAGE_OFFSET) / GAIN;
        // Lookup/linear interpolate corresponding temperature in table
        float realTemp = calculateTemp(voltDiode);
        if (realTemp > highestTemp) {
            highestTemp = realTemp;
            highestTempChannel = i;
        }
        // Turn on LED if the temperature is too high
        if (realTemp > TEMP_MAX) {
            digitalWrite(LED_PIN, HIGH);
        }
    }

    // Report highest temperature via I2C
    // First 4 bytes are the highestTemp and the 5th byte is the highestTempChannel
    Wire.beginTransmission(0x67);
    Wire.write((uint8_t *)&highestTemp, sizeof(highestTemp));
    Wire.write(highestTempChannel);
    Wire.endTransmission();
}

void selectMuxChannel(int channelNum) {
    for (int i = 0; i < 4; i++) {
        digitalWrite(selectLines[i], selectArray[channelNum][i]);
    }
}

/**
*  calculateTemp finds the range in which voltDiode is valid in so that
*  linear interpolation can be performed. The equation used is:
*  y = y_1 + [(x - x_1) * \frac{y_2 - y_1}{x_2 - x_1}]
*  y = return val (temperature)
*  x = voltDiode
*  x_1 = volts[i]
*  x_2 = volts[i + 1]
*  y_1 = temps[i]
*  y_2 = temps[i + 1]
*  The function returns the temperature.
*/
float calculateTemp(float voltDiode) {
    if (voltDiode >= volts[0]) {
        return temps[0];
    }
    if (voltDiode <= volts[32]) {
        return temps[32];
    }

    for (int i = 0; i <= 33; i++) {
        if (voltDiode <= volts[i] && voltDiode  >= volts[i + 1]) {
            float fraction = (temps[i + 1] - temps[i]) / (volts[i + 1] - volts[i]);
            float mul = (voltDiode - volts[i]) * fraction;
            return temps[i] + mul;
        }
    }
}