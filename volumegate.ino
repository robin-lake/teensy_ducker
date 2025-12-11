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
AudioAnalyzeFFT1024      fft1024_1;      // FFT for amps 1&2 (i2s2_1 - sidechain)
AudioAnalyzeFFT1024      fft1024_2;      // FFT for amps 3&4 (i2s1 - main audio)
AudioAnalyzePeak          peak1;          // For debugging audio levels
AudioMixer4              mixer1;         // Main output mixer
AudioMixer4              mixer_amp12;    // Mixer for amp1+amp2 → fft1
AudioMixer4              mixer_amp34;    // Mixer for amp3+amp4 → fft2
AudioOutputI2S           i2s2;           //xy=682,206
AudioEffectCompressor    comp1;
// sidechain input
AudioConnection          patchCord1(i2s2_1, 0, amp1, 0);
AudioConnection          patchCord2(i2s2_1, 1, amp2, 0);
// main audio input
AudioConnection          patchCord3(i2s1, 0, amp3, 0);
AudioConnection          patchCord4(i2s1, 1, amp4, 0);
AudioConnection          patchCord13(i2s1, 0, peak1, 0);  // Debug: check if audio is coming in
// Main output mixer connections
AudioConnection          patchCord6(amp4, 0, mixer1, 3);
AudioConnection          patchCord7(amp3, 0, mixer1, 2);
AudioConnection          patchCord8(amp1, 0, mixer1, 0);
AudioConnection          patchCord9(amp2, 0, mixer1, 1);
// FFT mixer connections - combine amps for analysis
AudioConnection          patchCord14(amp1, 0, mixer_amp12, 0);  // amp1 → mixer_amp12
AudioConnection          patchCord15(amp2, 0, mixer_amp12, 1);  // amp2 → mixer_amp12
AudioConnection          patchCord16(amp3, 0, mixer_amp34, 0);  // amp3 → mixer_amp34
AudioConnection          patchCord17(amp4, 0, mixer_amp34, 1);  // amp4 → mixer_amp34
// FFT connections - analyze each source separately
AudioConnection          patchCord18(mixer_amp12, 0, fft1024_1, 0);  // amps 1&2 → fft1
AudioConnection          patchCord19(mixer_amp34, 0, fft1024_2, 0);  // amps 3&4 → fft2
// Bypass compressor - connect mixer directly to output
// AudioConnection          patchCord10(mixer1, 0, comp1, 0);
AudioConnection          patchCord11(mixer1, 0, i2s2, 0);
AudioConnection          patchCord12(mixer1, 0, i2s2, 1);
// GUItool: end automatically generated code

// Debug flag - set to true to enable Serial.print statements
#define DEBUG false

uint32_t  mytime = 0;
float amplitude = 1.0;
float volume_floor = 0.0;
int hold = 100;
int release = 100;
int a1history=0;

// Non-blocking ducking state machine variables
bool ducking = false;
elapsedMillis duck_timer = 0;
float current_gain = amplitude;


// Non-blocking ducking state machine
void duck() {
  if (!ducking) {
    // Start ducking
    ducking = true;
    duck_timer = 0;
    current_gain = volume_floor;
    amp3.gain(volume_floor);
    amp4.gain(volume_floor);
  }
}

void update_duck() {
  if (ducking) {
    if (duck_timer < (unsigned long)hold) {
      // Still in hold phase - keep volume at floor
      amp3.gain(volume_floor);
      amp4.gain(volume_floor);
    } else if (duck_timer < (unsigned long)(hold + release)) {
      // Release phase - gradually increase gain
      int release_elapsed = duck_timer - (unsigned long)hold;
      float release_ratio = (float)release_elapsed / (float)release;
      current_gain = volume_floor + (amplitude - volume_floor) * release_ratio;
      amp3.gain(current_gain);
      amp4.gain(current_gain);
    } else {
      // Release complete - back to normal
      ducking = false;
      current_gain = amplitude;
      amp3.gain(amplitude);
      amp4.gain(amplitude);
    }
  }
}

// code for turning knob
void adjust_hold_and_release() {
  // Read knob and update hold time (non-blocking)
  int a1 = analogRead(A1);
  if (a1 > a1history + 5 || a1 < a1history - 5) {
    // Knob has changed significantly - update hold time
    hold = a1/2;
    release = a1/4;
    if (hold < 10) hold = 10;      // Minimum hold time (10ms)
    if (hold > 1000) hold = 1000; // Maximum hold time (1000ms = 1 second)
    if (release < 10) release = 10; // Minimum release time
    if (release > 500) release = 500; // Maximum release time
    a1history = a1;
    // Removed Serial.print to avoid blocking audio
  }
}

void setup() {
    // allocate memory (increased for additional FFT and mixers)
    AudioMemory(30);
    // configure serial
    Serial.begin(115200);
    delay(1000);  // Wait for serial to be ready
    if (DEBUG) {
      Serial.println("Volumegate starting...");
    }
    // Initialize amplifier gains - CRITICAL!
    // Sidechain (amps 1&2) stays at full volume
    amp1.gain(1.0);
    amp2.gain(1.0);
    // Main audio (amps 3&4) will be ducked when sidechain is loud
    amp3.gain(amplitude);
    amp4.gain(amplitude);
    // Initialize ducking state
    ducking = false;
    current_gain = amplitude;
    // Initialize FFT mixers (set all channels to 1.0 for summing)
    mixer_amp12.gain(0, 1.0);
    mixer_amp12.gain(1, 1.0);
    mixer_amp12.gain(2, 0.0);
    mixer_amp12.gain(3, 0.0);
    mixer_amp34.gain(0, 1.0);
    mixer_amp34.gain(1, 1.0);
    mixer_amp34.gain(2, 0.0);
    mixer_amp34.gain(3, 0.0);
    // knob
    a1history = analogRead(A1);
    if (DEBUG) {
      Serial.print("Initial A1 reading: ");
      Serial.println(a1history);
    }
    // initialize compressor
  // comp1.set_default_values(-40.0f, 4.0f, 1000.0f, 30.0f);
  comp1.set_default_values(-0.0f, 4.0f, 1000.0f, 30.0f);
  
  // Give FFT time to accumulate samples
  if (DEBUG) {
    Serial.println("Waiting for FFT to initialize...");
  }
  delay(500);
}

void loop() {
  // Debug: Check raw audio peak level and system status (reduced frequency)
  static elapsedMillis debug_timer = 0;
  if (DEBUG && debug_timer > 5000) {  // Every 5 seconds (reduced to avoid blocking)
    Serial.println("=== Audio System Status ===");
    
    // Check audio memory usage
    Serial.print("Audio memory used: ");
    Serial.print(AudioMemoryUsage());
    Serial.print("/");
    Serial.println(AudioMemoryUsageMax());
    
    // Check peak analyzer (i2s1 input)
    if (peak1.available()) {
      float peak_level = peak1.read();
      Serial.print("i2s1 peak level: ");
      Serial.print(peak_level);
      Serial.print(" (");
      Serial.print(peak_level * 100);
      Serial.println("%)");
      peak1.read();  // Clear it
    } else {
      Serial.println("i2s1 peak analyzer: NOT AVAILABLE (no audio on i2s1)");
    }
    
    // Check FFT status for both sources
    Serial.println("FFT Levels:");
    
    // FFT1: amps 1&2 (i2s2_1 - sidechain/trigger source)
    if (fft1024_1.available()) {
      float fft1_sum = fft1024_1.read(0, 5);
      Serial.print("  Source 1 (amps 1&2, i2s2_1): ");
      Serial.print(fft1_sum);
      Serial.print(" [bins: ");
      Serial.print(fft1024_1.read(0));
      Serial.print(" ");
      Serial.print(fft1024_1.read(1));
      Serial.print(" ");
      Serial.print(fft1024_1.read(2));
      Serial.println("]");
    } else {
      Serial.println("  Source 1 (amps 1&2): NOT AVAILABLE");
    }
    
    // FFT2: amps 3&4 (i2s1 - main audio)
    if (fft1024_2.available()) {
      float fft2_sum = fft1024_2.read(0, 5);
      Serial.print("  Source 2 (amps 3&4, i2s1): ");
      Serial.print(fft2_sum);
      Serial.print(" [bins: ");
      Serial.print(fft1024_2.read(0));
      Serial.print(" ");
      Serial.print(fft1024_2.read(1));
      Serial.print(" ");
      Serial.print(fft1024_2.read(2));
      Serial.println("]");
    } else {
      Serial.println("  Source 2 (amps 3&4): NOT AVAILABLE");
    }
    
    // Check A1 reading
    int a1 = analogRead(A1);
    Serial.print("A1 reading: ");
    Serial.print(a1);
    Serial.print(", hold = ");
    Serial.println(hold);
    Serial.println("===========================");
    
    debug_timer = 0;
  }
  
  // // Update ducking state machine (non-blocking)
  // update_duck();
  
  // // adjust hold and release time based on knob
  // adjust_hold_and_release();
  
  // // Check FFT in main loop (more frequently) - trigger on source 1 (sidechain)
  // if (fft1024_1.available()) {
  //   float level_sum = fft1024_1.read(0, 5);
  //   if (level_sum > 0.08) {
  //     duck();
  //   }
  // }
  
  // No delay needed - audio processing is real-time
}


