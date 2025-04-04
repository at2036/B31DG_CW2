#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "B31DGMonitor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Enable vTaskDelayUntil function
#define INCLUDE__vTaskDelayUntil 1

// Call monitor instance
B31DGCyclicExecutiveMonitor monitor(2800);

// Pin definition s
#define output_t1 1
#define output_t2 2
#define input_t3 4
#define input_t4 5
#define led_t6 3
#define led_t7 19
#define button_t7 6

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

// Definitions of mutexes and task handles
SemaphoreHandle_t freqMutex_t3, freqMutex_t4, ledMutex;
TaskHandle_t Task1_handle, Task2_handle, Task3_handle, Task4_handle, Task5_handle, Task6_handle, Task7_handle;

// Definitions of timing periods of each task
const TickType_t xFrequency_t1 = pdMS_TO_TICKS(4);
const TickType_t xFrequency_t2 = pdMS_TO_TICKS(3);
const TickType_t xFrequency_t3 = pdMS_TO_TICKS(10);
const TickType_t xFrequency_t4 = pdMS_TO_TICKS(10);
const TickType_t xFrequency_t5 = pdMS_TO_TICKS(5);
const TickType_t xFrequency_t6 = pdMS_TO_TICKS(100);
const TickType_t xFrequency_t7 = pdMS_TO_TICKS(50);


void IRAM_ATTR b_t7_ISR() {

  // Task 7 ISR: Toggle LED state and call monitor.doWork() when button is pressed. ISR gives semaphore to task.

  unsigned long debounceTime = millis();

  // Delay for debouncing
  
  if ((debounceTime - prevDebounceTime) > debounceDelay){
    prevDebounceTime = debounceTime;
    xSemaphoreGiveFromISR(ledMutex, NULL);
  }
}

void signal_t1(void*){

  /*
  Task 1: Output generated waveform to pin with LED attached for visualization
  Waveform is HIGH for 250us, LOW for 50us, HIGH for 300us
  Task 1 measured to take 606us
  */

  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  while(1){

    // Delay task from being called until next deadline 
    vTaskDelayUntil(&xLastWakeTime, xFrequency_t1);

    monitor.jobStarted(1);
    
    digitalWrite(output_t1, HIGH);
    vTaskDelay(pdMS_TO_TICKS(0.25));
    digitalWrite(output_t1, LOW);
    vTaskDelay(pdMS_TO_TICKS(0.05));
    digitalWrite(output_t1, HIGH);
    vTaskDelay(pdMS_TO_TICKS(0.3));
    digitalWrite(output_t1, LOW);
    
    monitor.jobEnded(1);
    
  }
}

void signal_t2(void*){
  
  /*
  Task 2: Output generated waveform to pin with LED attached for visualization
  Waveform is HIGH for 100us, LOW for 50us, HIGH for 200us
  Task 2 measured to take 356us
  */

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){

    // Delay task from being called until next deadline
    vTaskDelayUntil(&xLastWakeTime, xFrequency_t2);
    
    monitor.jobStarted(2);
    
    digitalWrite(output_t2, HIGH);
    vTaskDelay(pdMS_TO_TICKS(0.1));
    digitalWrite(output_t2, LOW);
    vTaskDelay(pdMS_TO_TICKS(0.05));
    digitalWrite(output_t2, HIGH);
    vTaskDelay(pdMS_TO_TICKS(0.2));
    digitalWrite(output_t2, LOW);
    
    monitor.jobEnded(2);
    
  }
}

void poll_t3(void*){

  /*
  Task 3: Poll frequency of inputted square wave between 666Hz and 1000Hz 
  Function measures length of HIGH pulse in us, given 50% duty cycle and f = 1/T, frequency = 50000 / length of HIGH pulse
  Task 3 measured to take 1501us at 666Hz (worst case execution), 1000us at 1000Hz (best case execution)
  */

  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  while(1){

    // Delay task from being called until next deadline
    vTaskDelayUntil(&xLastWakeTime, xFrequency_t3);

    monitor.jobStarted(3);
    
    xSemaphoreTake(freqMutex_t3, portMAX_DELAY);
    
    // Measure period of half of wave, correct for errors and convert to Frequency
    long tempT3 = (pulseIn(input_t3, !digitalRead(input_t3), freqTimeout_t3));
    if(tempT3 < 760 && tempT3 > 490) freq_t3 = us_to_Hz_calc / (tempT3 * 2);
    
    xSemaphoreGive(freqMutex_t3);
    
    monitor.jobEnded(3);
  }
}

void poll_t4(void*){

  /*
  Task 4: Poll frequency of inputted square wave between 833Hz and 1500Hz 
  Function measures length of HIGH pulse in us, given 50% duty cycle and f = 1/T, frequency = 50000 / length of HIGH pulse
  Task 4 measured to take 1200us at 833Hz (worst case execution), 666us at 1500Hz (best case execution)
  */

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){

    // Delay task from being called until next deadline
    vTaskDelayUntil(&xLastWakeTime, xFrequency_t4);

    monitor.jobStarted(4);
    
    xSemaphoreTake(freqMutex_t4, portMAX_DELAY);
    
    // Measure period of half of wave, correct for errors and convert to Frequency
    long tempT4 = (pulseIn(input_t4, !digitalRead(input_t4), freqTimeout_t4)); 
    if(tempT4 < 610 && tempT4 > 323) freq_t4 = us_to_Hz_calc / (tempT4 * 2);
    
    xSemaphoreGive(freqMutex_t4);
    
    monitor.jobEnded(4);
  }
}

void call_t5(void*){

  // Task 5: Call monitor.doWork(), measured to take 500us

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){

    // Delay task from being called until next deadline
    vTaskDelayUntil(&xLastWakeTime, xFrequency_t5);

    monitor.jobStarted(5);
    
    monitor.doWork();
    
    monitor.jobEnded(5);
    
  }
}

void sum_t6(void*){

  // Task 6: Turn on LED, near instantaneous

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){

    // Delay task from being called with appropriate delay
    vTaskDelayUntil(&xLastWakeTime, xFrequency_t6);

    xSemaphoreTake(freqMutex_t3, portMAX_DELAY);
    xSemaphoreTake(freqMutex_t4, portMAX_DELAY);

    // Task 6: Turn on LED when sum of measured frequency exceeds 1500
    freqTotal_t6 = freq_t3 + freq_t4;
    freqTotal_t6 = constrain(freqTotal_t6, 1499, 2500);
    if (freqTotal_t6 > freqLimit_t6) digitalWrite(led_t6, HIGH);
    else digitalWrite(led_t6, LOW);

    xSemaphoreGive(freqMutex_t3);
    xSemaphoreGive(freqMutex_t4);
  }
}

void toggleLed_t7(void*){

  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){
    
    // Delay task from being called with appropriate delay
    vTaskDelayUntil(&xLastWakeTime, xFrequency_t7);

    // Toggles LED and calls monitor.doWork() if semaphor received from ISR
    if(xSemaphoreTake(ledMutex, portMAX_DELAY) == pdTRUE){

      led_state_t7 = !led_state_t7;
      digitalWrite(led_t7, led_state_t7);
      monitor.doWork();

    }
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

  // Create semaphores with respective mutexes attached
  freqMutex_t3 = xSemaphoreCreateMutex();
  freqMutex_t4 = xSemaphoreCreateMutex();
  ledMutex = xSemaphoreCreateBinary();

  // Ensure Task 7 LED Semaphore starts in correct state
  xSemaphoreGive(ledMutex);

  // Attach interrupt to button
  attachInterrupt(digitalPinToInterrupt(button_t7), b_t7_ISR, RISING);

  // Call monitor
  monitor.startMonitoring();

  // Create Tasks, giving each an appropriate name, stack memory and priority, attaching the respective function and handle to each
  xTaskCreate(signal_t1, "Task 1", 4096, NULL, 3, &Task1_handle);
  xTaskCreate(signal_t2, "Task 2", 4096, NULL, 3, &Task2_handle);
  xTaskCreate(poll_t3, "Task 3", 8192, NULL, 2, &Task3_handle);
  xTaskCreate(poll_t4, "Task 4", 8192, NULL, 2, &Task4_handle);
  xTaskCreate(call_t5, "Task 5", 4096, NULL, 3, &Task5_handle);
  xTaskCreate(sum_t6, "Task 6", 4096, NULL, 1, &Task6_handle);
  xTaskCreate(toggleLed_t7, "Task 7 LED", 2048, NULL, 1, &Task7_handle);
}

// Main loop unused
void loop() {}


