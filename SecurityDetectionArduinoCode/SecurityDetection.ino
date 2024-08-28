#include <LiquidCrystal.h>


LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

#define AR_SIZE(x) (sizeof(x)/sizeof(x[0]))

// Define the coefficients for the biquad filter (SOS format)
float b[1][3] = { 
  {1, 2, 1} // b coefficients
};

float a[1][3] = { 
  {1, -1.743260281335717420958530965435784310102, 0.77265997397853158901170900207944214344} // a coefficients
};

float g[1][1] = {
  {0.007349923160703567166784910824617327307  } // Gain
};

const int num_sections = AR_SIZE(g); // 1 biquad filter for this case
const int signal_length = 100;
const int sensorPin = A0; // Analog pin for LDR
const int buzzerPin = 7;  // Digital pin for Buzzer
const int ledPin = 3; // Pin for the LED

int i = 0; // discrete time counter
float y_out[signal_length];
float x[signal_length];
float *y_temp; // temp holder to hold the output of 1 biquad and pass it on to the next

void directForm2(float x[], float b[], float a[], int n, float G, float *y) {
  float w[2] = {0}; // Intermediate storage for w[n] (only need two for second order)
  for (int i = 0; i < n; i++) {
    if (i == 0) {
      w[0] = G * x[i];
      y[i] = b[0] * w[0];
    } else if (i == 1) {
      w[1] = G * x[i] - a[1] * w[0];
      y[i] = b[0] * w[1] + b[1] * w[0];
    } else {
      w[0] = G * x[i] - a[1] * w[1] - a[2] * w[0];
      y[i] = b[0] * w[0] + b[1] * w[1] + b[2] * (i > 2 ? y[i - 2] : 0); 
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Initialize the LCD
  lcd.begin(16, 2);
  
  float t;
  for (int i = 0; i < signal_length; i++) {
    t = micros() / 1.0e6;
    x[i] = analogRead(sensorPin) / 1023.0; // Read from LDR and normalize: Analog to digital Conversion
    delay(4);
  }

  Serial.println("n  X  Y");
  Serial.println("-------");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Light Intensity");
}

void loop() {
  y_temp = x;
  for (int i = 0; i < num_sections; i++) {
    directForm2(y_temp, b[i], a[i], signal_length, *g[i], y_out); // Dereference g[i]
    y_temp = y_out;
  }

  for ( int i = 0; i < signal_length; i++) {
    Serial.print(i);
    Serial.print(" ");
    Serial.print(x[i]);
    Serial.print(" ");
    Serial.print(y_out[i]);
    Serial.println("");

    // Used the filtered output to control the buzzer and led
    if (y_out[i] > 0.5) { // Observed and choosen threshold
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Status: Detected    ");
    } else {
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
      lcd.setCursor(0, 1);
      lcd.print("Status: Normal     ");
    }
  }
}
