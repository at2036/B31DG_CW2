#include <stdio.h>
#include <stdint.h>
#include "B31DGMonitor.h"
#include "Ticker.h"

// Call monitor and ticker instances
B31DGCyclicExecutiveMonitor monitor(1925);
Ticker ticker;

// Pin definitions 
#define output_t1 1
#define output_t2 2
#define input_t3 4
#define input_t4 5
#define led_t6 3
#define led_t7 19
#define button_t7 6

#define frame_duration_ms 2

// Definition of variables for control of frames
uint8_t frameCounter;
const uint8_t frameCounterLimit = 30;

// Definition of debounce variables
const uint8_t debounceDelay = 50;
uint32_t prevDebounceTime;

// Definitions of variables related to tasks
uint32_t freq_t3;
uint32_t freq_t4;
uint32_t freqTotal_t6;
const uint32_t freqTimeout_t3 = 1500;
const uint32_t freqTimeout_t4 = 1200;
const uint32_t freqLimit_t6 = 1500;
const uint32_t us_to_Hz_calc = 1000000;
bool led_state_t7;

void IRAM_ATTR b_t7_ISR() {

  // Task 7 ISR: Toggle LED state and call monitor.doWork() when button is pressed

  unsigned long debounceTime = millis();
  // Delay for debouncing
  if ((debounceTime - prevDebounceTime) > debounceDelay){
    prevDebounceTime = debounceTime;
    led_state_t7 = !led_state_t7;
    digitalWrite(led_t7, led_state_t7);
    monitor.doWork();
  }
}

void setup() {

  Serial.begin(115200);

  // Define LEDs and signal outputs, signals inputs and buttons as inputs with pullups enabled 
  pinMode(output_t1, OUTPUT);
  pinMode(output_t2, OUTPUT);
  pinMode(led_t6, OUTPUT);
  pinMode(led_t7, OUTPUT);
  pinMode(input_t3, INPUT);
  pinMode(input_t4, INPUT);
  pinMode(button_t7, INPUT_PULLUP);

  // Attach interrupt to button
  attachInterrupt(digitalPinToInterrupt(button_t7), b_t7_ISR, RISING);

  // Attach ticker to frame function and call monitor
  ticker.attach_ms(frame_duration_ms, frame);
  monitor.startMonitoring();
}

void signal_t1(){

  /*
  Task 1: Output generated waveform to pin with LED attached for visualization
  Waveform is HIGH for 250us, LOW for 50us, HIGH for 300us
  Task 1 measured to take 606us
  */
  
  monitor.jobStarted(1);

  digitalWrite(output_t1, HIGH);
  delayMicroseconds(250);
  digitalWrite(output_t1, LOW);
  delayMicroseconds(50);
  digitalWrite(output_t1, HIGH);
  delayMicroseconds(300);
  digitalWrite(output_t1, LOW);

  monitor.jobEnded(1);
}

void signal_t2(){
  
  /*
  Task 2: Output generated waveform to pin with LED attached for visualization
  Waveform is HIGH for 100us, LOW for 50us, HIGH for 200us
  Task 2 measured to take 356us
  */

  monitor.jobStarted(2);

  digitalWrite(output_t2, HIGH);
  delayMicroseconds(100);
  digitalWrite(output_t2, LOW);
  delayMicroseconds(50);
  digitalWrite(output_t2, HIGH);
  delayMicroseconds(200);
  digitalWrite(output_t2, LOW);

  monitor.jobEnded(2);
}

void poll_t3(){
  
  /*
  Task 3: Poll frequency of inputted square wave between 666Hz and 1000Hz 
  Function measures length of HIGH or LOW pulse in us, given 50% duty cycle and f = 1/T, frequency = 100000 / double length of measured pulse
  Task 3 measured to take 1501us at 666Hz (worst case execution), 1000us at 1000Hz (best case execution)
  */

  monitor.jobStarted(3);

  // Measure period of half of wave, correct for errors and convert to Frequency
  long tempT3 = (pulseIn(input_t3, !digitalRead(input_t3), freqTimeout_t3));
  if(tempT3 < 760 && tempT3 > 490) freq_t3 = us_to_Hz_calc / (tempT3 * 2);
  
  monitor.jobEnded(3);
}

void poll_t4(){
  
  /*
  Task 4: Poll frequency of inputted square wave between 833Hz and 1500Hz 
  Function measures length of HIGH or LOW pulse in us, given 50% duty cycle and f = 1/T, frequency = 100000 / double length of measured pulse
  Task 4 measured to take 1200us at 833Hz (worst case execution), 666us at 1500Hz (best case execution)
  */

  monitor.jobStarted(4);

  // Measure period of half of wave, correct for errors and convert to Frequency
  long tempT4 = (pulseIn(input_t4, !digitalRead(input_t4), freqTimeout_t4)); 
  if(tempT4 < 610 && tempT4 > 323) freq_t4 = us_to_Hz_calc / (tempT4 * 2);

  monitor.jobEnded(4);
}

void call_t5(){

  // Task 5: Call monitor.doWork(), measured to take 500us
  monitor.jobStarted(5);
  monitor.doWork();
  monitor.jobEnded(5);
}

void sum_t6(){

  // Task 6: Turn on LED when sum of measured frequency exceeds 1500

  freqTotal_t6 = freq_t3 + freq_t4;
  freqTotal_t6 = constrain(freqTotal_t6, 1499, 2500);
  if (freqTotal_t6 > freqLimit_t6) digitalWrite(led_t6, HIGH);
  else digitalWrite(led_t6, LOW);
}

void frame(){

  // Definition of frame schedule
  switch(frameCounter){
    case(0):  signal_t1(); signal_t2(); call_t5();            break;
    case(1):  poll_t3();                                      break;
    case(2):  signal_t1(); signal_t2();                       break;
    case(3):  signal_t2(); poll_t4(); call_t5();              break;
    case(4):  signal_t1();                                    break;
    case(5):  signal_t2(); poll_t3();                         break;
    case(6):  signal_t1(); call_t5();                         break;
    case(7):  signal_t2(); poll_t4();                         break;
    case(8):  signal_t1(); signal_t2(); call_t5();            break;
    case(9):  signal_t2();                                    break;
    case(10): signal_t1(); call_t5();                         break;
    case(11): signal_t2(); poll_t3();                         break;
    case(12): signal_t1(); signal_t2();                       break;
    case(13): poll_t4();   call_t5();                         break;
    case(14): signal_t1(); signal_t2();                       break;
    case(15): signal_t2(); poll_t3();                         break;
    case(16): signal_t1(); call_t5();                         break;
    case(17): signal_t2(); poll_t4();                         break;
    case(18): signal_t1(); call_t5();                         break;
    case(19): signal_t2(); sum_t6();                          break;
    case(20): signal_t1(); signal_t2(); call_t5();            break;
    case(21): signal_t2(); poll_t3();                         break;
    case(22): signal_t1(); poll_t4();                         break;
    case(23): signal_t2(); call_t5();                         break;
    case(24): signal_t1(); signal_t2();                       break;
    case(25): poll_t3();   call_t5();                         break;
    case(26): signal_t1(); signal_t2();                       break;
    case(27): signal_t2(); poll_t4();                         break;
    case(28): signal_t1(); call_t5();                         break;
    case(29): signal_t2();                                    break;
    default: Serial.print("Error: invalid frame reference");
  }
  
  // Iterate frame counter and correct if over limit
  frameCounter++;
  if(frameCounter >= frameCounterLimit) frameCounter = 0;
}

// Main loop unused
void loop() {}


