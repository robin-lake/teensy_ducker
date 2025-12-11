#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "effect_compressor.h"

// GUItool: begin automatically generated code
AudioInputI2S2           i2s2_1;         //xy=180,351
AudioInputI2S            i2s1;           //xy=188,143
AudioAmplifier           amp4;           //xy=330,404
AudioAmplifier           amp3;           //xy=339,320
AudioAmplifier           amp1;           //xy=341,119
AudioAmplifier           amp2;           //xy=345,169
AudioAnalyzeFFT1024      fft1024_1;      //xy=350,225
AudioMixer4              mixer1;         //xy=537,243
AudioOutputI2S           i2s2;           //xy=682,206
AudioEffectCompressor    comp1;
AudioConnection          patchCord1(i2s2_1, 0, amp3, 0);
AudioConnection          patchCord2(i2s2_1, 1, amp4, 0);
AudioConnection          patchCord121(i2s2_1, 1, fft1024_1, 0);
AudioConnection          patchCord3(i2s1, 0, amp1, 0);
AudioConnection          patchCord4(i2s1, 1, amp2, 0);
AudioConnection          patchCord5(i2s1, 1, fft1024_1, 0);
AudioConnection          patchCord6(amp4, 0, mixer1, 3);
AudioConnection          patchCord7(amp3, 0, mixer1, 2);
AudioConnection          patchCord8(amp1, 0, mixer1, 0);
AudioConnection          patchCord9(amp2, 0, mixer1, 1);
AudioConnection          patchCord10(mixer1, 0, comp1, 0);
AudioConnection          patchCord11(comp1, 0, i2s2, 0);
AudioConnection          patchCord12(comp1, 0, i2s2, 1);
// GUItool: end automatically generated code


uint32_t  mytime = 0;
float amplitude = 1.0;
float volume_floor = 0.0;
int hold = 10;
int release = 100;
int a1history=0;


void release_rise() {
  float current_gain = volume_floor;
  float step = (amplitude - volume_floor) / release;
  for (int i = 0; i < release; i++) {
    current_gain += step;
    amp1.gain(current_gain);
    amp2.gain(current_gain);
    delay(1);
  }
}

void duck() {
 amp1.gain(volume_floor);
 amp2.gain(volume_floor);
 delay(hold);
 release_rise();
}

// code for turning knob
void wait(unsigned int milliseconds) {
  elapsedMillis msec=0;
    while (msec <= milliseconds) {
      int a1 = analogRead(A1);
      if (a1 > a1history + 50 || a1 < a1history - 50) {
        Serial.print("Knob (pin A1) = ");
        Serial.println(a1);
        hold = a1/10;
        a1history = a1;
      }
    }
}

void setup() {
    // allocate memory
    AudioMemory(20);
    // configure serial
    Serial.begin(115200);
    // knob
    a1history = analogRead(A1);
    // initialize compressor
  // comp1.set_default_values(-40.0f, 4.0f, 1000.0f, 30.0f);
  comp1.set_default_values(-0.0f, 4.0f, 1000.0f, 30.0f);
}

void loop() {
    // Serial.println(fft1024_1.available());
    // Serial.println(fft1024_1.read(0,5));
      // amp1.gain(amplitude);
  // amp2.gain(amplitude);
  //   delay(4000);
  if (fft1024_1.read(0,5) > 0.08) {
  duck();

  }

    // wait(250);
  // delay(2000);
  // put your main code here, to run repeatedly:

}


