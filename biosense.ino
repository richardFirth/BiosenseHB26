// includes
#include <Filters.h>
#include "Buzzer.h"
#include "OledDisplay.h"
#include "Input.h"

// component instantiation
Buzzer buzzer;
OledDisplay oledDisplay;
Input input;

// system variables
int appMode = 0;
int sensorMode = 0;

// sensor polling and graphing variables
float runningHighestAvg = 0.0f;
float runningLowestAvg = 1025.0f;
float runningAvg = 0.0f;
float height = 0.0f;
float scale = 1; 
float mappedVal = 0.0f;
int x = 0; 
int LastTime = 0;
int ThisTime;
bool BPMTiming = false;
bool BeatComplete = false;
int averageBPM = 0;
int BPM = 0;
char outputBuffer[55]; 

void setup()
{
  // initialize serial communications
  Serial.begin(115200);  

  // initialize buzzer system
  buzzer.init();
  
  // initialize display
  oledDisplay.init();

  // initialize input system
  input.init();

  // mute sounds for now
  buzzer.mute();

  // start main menu display
  changeAppMode(0);
}

void loop()
{
  switch(appMode) {

    case 0:
      mainMenuUpdate();
      break;

    case 1:
      pulseUpdate();
      break;

    case 2:
      ECGUpdate();
      break;

    case 3:
      EEGUpdate();
      break;
  }

  oledDisplay.display();
}

void changeAppMode(int mode) {

  switch(mode) {

    case 0: 

      mainMenuStart();
      break;

    case 1:
       
      pulseStart();
      break;

    case 2:
    
      ECGStart();
      break;

    case 3:
      
      EMGStart();
      break;
  }

  appMode = mode;
}

// ################## MAIN MENU ##################

void mainMenuStart() {

  oledDisplay.clear();
  oledDisplay.print(10, 1, "1. EMG");
  oledDisplay.print(10, 20, "2. ECG/EEG");
  oledDisplay.print(10, 40, "3. PULSE");
  
  buzzer.beep(200,600);
  buzzer.beep(300,200);
  buzzer.beep(400,300);
}

void mainMenuUpdate() {

  if (input.getButtonPress(1)) {

    sensorMode = 2;
    changeAppMode(1);
    
  } else if (input.getButtonPress(2)) {

    sensorMode = 1;
    changeAppMode(1);
    
  } else if (input.getButtonPress(3)) {
    
    sensorMode = 0;
    changeAppMode(1);
  }
}

// ################## PULSE ##################

void pulseStart() {

  // reset all sensor / display variables
  LastTime = 0;
  ThisTime;
  BPMTiming = false;
  BeatComplete = false;
  BPM = 0;
  runningHighestAvg = 0.0f;
  runningLowestAvg = 1025.0f;
  runningAvg = 0.0f;

  buzzer.beep(200,600);
  buzzer.beep(300,200);
  buzzer.beep(400,300);

  oledDisplay.print(1, 1, "PULSE");
}

void pulseUpdate() {

  int bioData = input.getBioSensorData(sensorMode);
  //Serial.println(bioData);
  updateDataVisualization(sensorMode, bioData, 16, 2);       
  delay(5);
  
  // if 4th button is held down, exit display
  if (input.getButtonPress(4))
    changeAppMode(0);
}

// ################## ECG ##################

void ECGStart() {

  buzzer.beep(200,600);
  buzzer.beep(300,200);
  buzzer.beep(400,300);

  oledDisplay.print(1, 1, "ECG");
}

void ECGUpdate() {

  // if 4th button is held down, exit display
  if (input.getButtonPress(4))
  changeAppMode(0);
}

// ################## EEG ##################

void EMGStart() {
  
  buzzer.beep(200,600);
  buzzer.beep(300,200);
  buzzer.beep(400,300);

  oledDisplay.print(1, 1, "EMG");
}

void EMGUpdate() {
  
}






void updateDataVisualization(int mode, int value, int topPadding, int bottomPadding) {

  // reset bounds and redraw text info
  if (x > 128) {

    oledDisplay.clear();  
    x = 0;
  }

  runningAvg = (runningAvg + value) / 2;  
  
  if (runningAvg > runningHighestAvg)
      runningHighestAvg = (runningHighestAvg + runningAvg) / 2; 
  
  if (runningAvg < runningLowestAvg)
      runningLowestAvg = (runningLowestAvg + runningAvg) / 2; 
    
  height = runningHighestAvg - runningLowestAvg;

  float newHeight = (64 - topPadding - bottomPadding);

  mappedVal = (int)map(runningAvg, runningLowestAvg, runningHighestAvg, 0, newHeight);
  oledDisplay.drawLine(x, (newHeight / 2) + topPadding, x, topPadding + (mappedVal - (newHeight / 2))+ (newHeight / 2) - 1);
  x ++;

  if (mode == 0)
    calcPulseData(runningAvg);
}

void calcPulseData(int value) {

  float midLine = runningLowestAvg + (height / 2);
  ThisTime = millis();
  
  if (value > midLine)
  {
    if (BeatComplete)
    {
      BPM = ThisTime - LastTime;
      BPM = int(60 / (float(BPM) / 1000));
      BPMTiming = false;
      BeatComplete = false;

      averageBPM = (averageBPM + BPM) / 2;

      if (averageBPM > 30 && averageBPM < 200) {
        
        buzzer.beep(12, 250);
  
        char bpmBuffer[5];
        itoa(averageBPM, bpmBuffer, 10);
        sprintf(outputBuffer, "%s BPM", bpmBuffer);
  
        oledDisplay.eraseRect(60, 0, 128, 10);
        oledDisplay.print(60, 1, outputBuffer);
      }
    }
    
    if (BPMTiming == false)
    {
      LastTime = millis();
      BPMTiming = true;
    }
  }
  
  if ((value < midLine) & BPMTiming)
    BeatComplete = true;

  Serial.print(runningHighestAvg);
  Serial.print(",");
  Serial.print(runningLowestAvg);
  Serial.print(",");
  Serial.print(value);
  Serial.print(",");
  Serial.println(midLine);
}

