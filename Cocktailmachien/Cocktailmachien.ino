#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//For the GFX
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define LOGO_HEIGHT   64
#define LOGO_WIDTH    16

//IO
int buttonD4 = 2;                      
int buttonD5 = 14;
int buttonD6 = 12;
int outputD7 = 13;
int outputD8 = 15;
#define RelayOn   0
#define RelayOff  1
bool blnButtonD4Last;//check flank
bool blnButtonD5Last;//check flank
bool blnButtonD6Last;//check flank
bool blnButtonD4PosFlank;
bool blnButtonD5PosFlank;
bool blnButtonD6PosFlank;

//Timers
unsigned long StartMillisD4;
unsigned long StartMillisD5;
unsigned long CurrentMillis;
unsigned long StartMillis2HzPulse;
bool TwoHzPulse;

//Dosing memory
bool DosingStartedD4;
bool DosingStartedD5;

void setup() {
  Serial.begin(115200);
  Serial.println("Opstart cocktailmachien");

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(buttonD4, INPUT);    // sets pin as input
  pinMode(buttonD5, INPUT);    // sets pin as input
  pinMode(buttonD6, INPUT);    // sets pin as input
  pinMode(outputD7, OUTPUT);
  pinMode(outputD8, OUTPUT);

  DrawBase();
  
  StartMillis2HzPulse = millis();
  CurrentMillis = millis();
}

void DrawText() {
  display.setCursor(0,0);    
  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE); 
  display.println(F("GIN")); 
  display.print(getal); 
  display.println(F("TONIC")); 
  display.display();
}

void DrawBase() {
  display.clearDisplay();
  display.drawTriangle(85, 10, 80, 0, 90, 0, WHITE);
  display.drawTriangle(105, 10, 100, 0, 110, 0, WHITE);
  display.drawLine(80, 20, 80, 60, WHITE); 
  display.drawLine(110, 20, 110, 60, WHITE); 
  display.drawLine(80, 60, 110, 60, WHITE); 
  display.display();
  Serial.println("Base getekend");
}

void DrawDosingActiveD4() {
  display.fillTriangle(85, 10, 80, 0, 90, 0, WHITE);  
  display.drawLine(85, 10, 85, 60, WHITE); 
  display.display();
  Serial.println("Dosing1 getekend");
}

void DrawDosingStoppedD4() {
  display.fillTriangle(85, 10, 80, 0, 90, 0, BLACK);
  display.drawTriangle(85, 10, 80, 0, 90, 0, WHITE);  
  display.display();
  Serial.println("Dosing1 leeg");
}


void DrawFillLevel(int level){
  //in:0..100%
  //0%    :y=60
  //100%  :y=20
  int y;
  int height;

  if (level<0) {
    level=0;
  }
  if (level>100) {
    level=100;
  }
  
  y=60-(level*40/100);
  height=level*40/100;

 // DrawBase();
  display.fillRect(80,y,30,height, WHITE);
  display.display();
  Serial.print("Fill level: ");
  Serial.println(level);
  Serial.print("y: ");
  Serial.println(y);
  Serial.print("height: ");
  Serial.println(height);
}

void IOMirror(){
  if ((digitalRead(buttonD4) == HIGH) && (!blnButtonD4Last)) {
    blnButtonD4PosFlank = true;
  }
  else {
    blnButtonD4PosFlank = false;
  }

  if ((digitalRead(buttonD5) == HIGH) && (!blnButtonD5Last)) {
    blnButtonD5PosFlank = true;
  }
  else {
    blnButtonD5PosFlank = false;
  }

  if ((digitalRead(buttonD6) == HIGH) && (!blnButtonD6Last)) {
    blnButtonD6PosFlank = true;
  }
  else {
    blnButtonD6PosFlank = false;
  }

  blnButtonD4Last = (digitalRead(buttonD4) == HIGH);
  blnButtonD5Last = (digitalRead(buttonD5) == HIGH);
  blnButtonD6Last = (digitalRead(buttonD6) == HIGH);
}

int DosingActiveD4(unsigned long PresetTime){
  unsigned long ElapsedTime = (CurrentMillis - StartMillisD4);
  
  Serial.print("StartMillis: ");
  Serial.println(StartMillisD4);
  Serial.print("CurrentMillis: ");
  Serial.println(CurrentMillis);
  Serial.print("CurrentMillis: ");
  Serial.println(PresetTime);
  Serial.print("ElapsedTime: ");
  Serial.println(ElapsedTime);
  
  if (ElapsedTime <= PresetTime){
    Serial.print("ElapsedTime in de if: ");
    Serial.println((ElapsedTime*100)/PresetTime);
    return ((ElapsedTime*100)/PresetTime);
  }
  else {
    DosingStartedD4 = false;
    return (100);
  }
}

void System(){
  
  if (CurrentMillis - StartMillis2HzPulse >= 500){
    StartMillis2HzPulse = millis();
    TwoHzPulse = true;
  }
  else {
    TwoHzPulse = false;
  }
}

void loop() {

  CurrentMillis = millis();
  IOMirror(); //simplify IO
  System ();
  
  if (blnButtonD4PosFlank) {
//      DrawFillLevel(20);
//      digitalWrite (outputD7, RelayOn);
  DosingStartedD4 = true;
  StartMillisD4 = millis();
   }
   if (blnButtonD5PosFlank) {
      DrawBase();
      digitalWrite (outputD8, RelayOn);
   }
   if (blnButtonD6PosFlank) {
      DrawBase();
   }
   else {
      digitalWrite (outputD8, RelayOff);
   }

  if (DosingStartedD4){// && (TwoHzPulse)){
    Serial.print("DosingStarted: ");
    Serial.println("Yup");
    DrawFillLevel(DosingActiveD4(3000));
    DrawDosingActiveD4();
  }
  else {
    DrawDosingStoppedD4();
    digitalWrite (outputD7, RelayOff);
  } 
}
