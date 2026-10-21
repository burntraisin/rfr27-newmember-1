const int TEMP_PIN = A0;
const int LED_PIN = 13;
const int MUX_S0 = 2;
const int MUX_S1 = 3;
const int MUX_S2 = 4;
const int MUX_S3 = 5;

// Temperature when the ADC is at 0V in Celsius
const double TEMP_AT_0V = 120.0;
const double TEMP_AT_5V = -40.0;

const float TEMP_MAX = 60.0;

byte selectArray[16][4]; // Hold 0000 in selectArray[0][0-3], 0001 in [1][0-3], etc.

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);
    pinMode(MUX_S2, OUTPUT);
    pinMode(MUX_S3, OUTPUT);

    pinMode(TEMP_PIN, INPUT);

    // Double check logic for this
    for (int num = 0; num < 16; num++) {
        for (int bit = 3; bit >= 0; bit--) {
            selectArray[num][3 - bit] = (num >> bit) & 1; // bit shift num by bit times and mask the lowest bit
        }
    }

    Serial.begin(9600);
}

void loop() {
    // Send select info to MUX, for each do the following

    // Get an average of temperature readings
    double sum = 0.0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(TEMP_PIN);
        delay(2);
    }
    double rawAnalog = sum / 10.0;
    // Linear interpolation, as suggested by the datasheet

    if (tempC > TEMP_MAX) {
        digitalWrite(LED_PIN, HIGH);
    }

    delay(200);
}
