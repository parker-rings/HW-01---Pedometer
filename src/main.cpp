#include <arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>
#include <Adafruit_ST7789.h>

Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 135);

void setReports();
#define BNO08X_RESET -1
int pinD0 = 0;
int pinD1 = 1;
int pinD2 = 2;


enum menuState {
  StepsTakenMenu, //0
  DistanceTraveledMenu, //1
  StrideLengthMenu, //2
  AccelerationMenu, //3
  mCount //4
};

menuState menuMode = StepsTakenMenu;
float strideLength = 2.;
float totalDistance = 0.;
long stepsTaken = 17;
float x = 0.;
float y = 0.;
float z = 0.;
float magnitude = 0.;
volatile long prevChangeTime = 0;
volatile long prevChangeTimeTwo = 0;
long debounceTime = 50;
volatile bool changeButtonFlagUp = false;
volatile bool changeButtonFlagDown = false;
volatile bool menuButtonFlag = false;

void IRAM_ATTR buttonToChangeThingsUp() {
  long now = millis();
  if (now > prevChangeTime + debounceTime) {
    changeButtonFlagUp = true;
    prevChangeTime = now;
  }
}

void IRAM_ATTR buttonToChangeThingsDown() {
  long now = millis();
  if (now > prevChangeTime + debounceTime) {
    changeButtonFlagDown = true;
    prevChangeTime = now;
  }
}

void IRAM_ATTR buttonToChangeMenu() {
  long now = millis();
  if (now > prevChangeTimeTwo + debounceTime) {
    menuButtonFlag = true;
    prevChangeTimeTwo = now;
  }
}

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

void setup(void) {
  Serial.begin(115200);
  
  while(!Serial) 
    delay(10);
  
  pinMode(0, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(0), buttonToChangeThingsDown, RISING);

  pinMode(1, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(1), buttonToChangeThingsUp, RISING);

  pinMode(2, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(2), buttonToChangeMenu, RISING);


  
  

  // Wire1.setClock(400000); //Increase I2C data rate to 400kHz

  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
    stepsTaken = stepsTaken - 17;
  }
  Serial.println("BNO08x Found!");

  setReports();
  
  delay(2000);

  display.init(135, 240);
  display.setRotation(1);
  canvas.setTextColor(ST77XX_WHITE);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, 1);
}


void loop() {
  delay(10);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  switch (sensorValue.sensorId) {

    case SH2_ACCELEROMETER:

      x = sensorValue.un.accelerometer.x;
      y = sensorValue.un.accelerometer.y;
      z = sensorValue.un.accelerometer.z;

      //float xEng = x * 3.28084;
      //float yEng = y * 3.28084;
      //float zEng = z * 3.28084;

    
      magnitude = sqrt(x*x + y*y + z*z);
      //float magnitudeEng = sqrt(xEng*xEng + yEng*yEng + zEng*zEng);
      Serial.print("Accelerometer - x: ");
      Serial.print(x);
      Serial.print(" y: ");
      Serial.print(y);
      Serial.print(" z: ");
      Serial.println(z);
    /*
      Serial.print("Accelerometer - x: ");
      Serial.print(xEng);
      Serial.print(" y: ");
      Serial.print(yEng);
      Serial.print(" z: ");
      Serial.println(zEng);
      Serial.print("magnitude: ");
      Serial.print(magnitude);
      Serial.print(" magnitude eng ");
      Serial.println(magnitudeEng);*/
      break;

    case SH2_STEP_COUNTER:
      stepsTaken = sensorValue.un.stepCounter.steps;
      break;
  }
  


  if (menuButtonFlag) {
    menuButtonFlag = false;
    menuMode = (menuState)(((int)menuMode + 1) % (int)menuState::mCount);
    Serial.print("!!!!!!!!!!!!!!!!!!!!!!! Moving to Menu: ");
    Serial.println(menuMode);
  }

  if (changeButtonFlagUp) {
    if (menuMode == StrideLengthMenu) {
      strideLength += 0.1;
      
    }
    if (menuMode == StepsTakenMenu) {
      
    }
    if (menuMode == DistanceTraveledMenu) {
      
    }
    if (menuMode == AccelerationMenu){

    }
  changeButtonFlagUp = false;
  
  }

  if (changeButtonFlagDown) {
    if (menuMode == StrideLengthMenu) {
      strideLength -= 0.1;
      
    }
    if (menuMode == StepsTakenMenu) {
      
    }
    if (menuMode == DistanceTraveledMenu) {
      
    }
    if (menuMode == AccelerationMenu){

    }
  changeButtonFlagDown = false;
  
  }

  canvas.fillScreen(ST77XX_BLACK);
  canvas.setCursor(0,20);
  if (menuMode == menuState::StepsTakenMenu) {
    canvas.print("You have taken ");
    canvas.print(stepsTaken);
    canvas.print(" total steps.");
  }
  if (menuMode == menuState::DistanceTraveledMenu) {
    totalDistance = (stepsTaken)*(strideLength);
    canvas.println("The total distance traveled, based on");
    canvas.print("the stride length of ");
    canvas.print(strideLength);
    canvas.println(" feet,");
    canvas.print("is ");
    canvas.print(totalDistance);
    canvas.println(" total feet.");
  }
  if (menuMode == menuState::StrideLengthMenu) {
    canvas.print("The current stride length is ");
    canvas.print(strideLength);
    canvas.print(" feet.");
  }
  if (menuMode == menuState::AccelerationMenu) {
    canvas.println("The current acceleration in the");
    canvas.print("x direction is ");
    canvas.print(x);
    canvas.println(" m/s^2,");
    canvas.print("in the y direction ");
    canvas.print(y);
    canvas.println(" m/s^2,");
    canvas.print("in the z direction ");
    canvas.print(z);
    canvas.println(" m/s^2,");
    canvas.println("and the magnitude of acceleration is");
    canvas.print(magnitude);
    canvas.println(" m/s^2,");
  }
  
  display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);

  
}

void setReports(void) {
  Serial.println("Setting desired reports");
  
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
  if (!bno08x.enableReport(SH2_STEP_COUNTER)) {
    Serial.println("Could not enable step counter");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
  
  
}

