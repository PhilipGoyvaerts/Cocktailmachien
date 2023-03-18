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
int outputD3 = 0;
#define RelayOn   0
#define RelayOff  1
bool blnButtonD4Status;//check status
bool blnButtonD5Status;//check status
bool blnButtonD6Status;//check status
bool blnButtonD4Last;//check flank
bool blnButtonD5Last;//check flank
bool blnButtonD6Last;//check flank
bool blnButtonD4PosFlank;
bool blnButtonD5PosFlank;
bool blnButtonD6PosFlank;

//Timers
unsigned long StartMillisGin;
unsigned long StartMillisTonic;
unsigned long CurrentMillis;

//Dosing memory
bool DosingStartedGin;
bool DosingStartedTonic;
int DosingStep = 0;
bool TransStep0;
bool TransStep1;
bool TransStep2;
bool TransStep3;
bool ResetStepper;

//Recipe
int RatioGin = 20;
int LevelGin = 0;
int BaseTimeGin = 33750; //how many ms to dose for a 50/50 mix?
int RatioTonic = 80;
int LevelTonic = 0;
int BaseTimeTonic = 9400; //how many ms to dose for a 50/50 mix?

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
  pinMode(outputD3, OUTPUT);
  digitalWrite (outputD7, RelayOff); 
  digitalWrite (outputD3, RelayOff); 
  DrawBase();
  CurrentMillis = millis();
}

void DrawText() {
  display.setCursor(0,0);    
  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE); 
  display.println(F("GIN TONIC")); 
  display.println(F(" MACHIEN")); 
  display.setTextSize(0);      
  display.setCursor(0,30);
  display.println(F("Sterkte")); 
  display.println(F("(Gin/Tonic):")); 
  display.setTextColor(WHITE, BLACK);
  display.print(RatioGin); 
  display.print(F("% / ")); 
  display.setTextColor(WHITE, BLACK);
  display.print(RatioTonic); 
  display.println(F("%")); 
  display.print(F("Step: ")); 
  display.setTextColor(WHITE, BLACK);
  display.print(DosingStep); 
  display.display();
}

void DrawBase() {
  display.clearDisplay();
  display.drawLine(80, 20, 80, 60, WHITE); 
  display.drawLine(110, 20, 110, 60, WHITE); 
  display.drawLine(80, 60, 110, 60, WHITE); 
  DrawText();
  display.display();
  Serial.println("Base getekend");
}

void DrawDosingPumps(int x, int state){
//x=startposition x coord
//state=active/idle
  if (state==0){
    display.fillTriangle(x, 10, x-5, 0, x+5, 0, BLACK);
    display.drawTriangle(x, 10, x-5, 0, x+5, 0, WHITE);
    display.drawLine(x, 11, x, 19, BLACK); 
    display.display();
  }
  else if (state==1){
    display.fillTriangle(x, 10, x-5, 0, x+5, 0, WHITE);
    display.drawLine(x, 11, x, 19, WHITE); 
    display.display();
  }
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

  display.fillRect(80,y,30,height, WHITE);
  display.display();
}

void IOMirror(){
blnButtonD4Status = (digitalRead(buttonD4) == HIGH);
blnButtonD5Status = (digitalRead(buttonD5) == HIGH);
blnButtonD6Status = (digitalRead(buttonD6) == HIGH);
  
  if ((blnButtonD4Status) && (!blnButtonD4Last)) {
    blnButtonD4PosFlank = true;
  }
  else {
    blnButtonD4PosFlank = false;
  }

  if ((blnButtonD5Status) && (!blnButtonD5Last)) {
    blnButtonD5PosFlank = true;
  }
  else {
    blnButtonD5PosFlank = false;
  }

  if ((blnButtonD6Status) && (!blnButtonD6Last)) {
    blnButtonD6PosFlank = true;
  }
  else {
    blnButtonD6PosFlank = false;
  }

  blnButtonD4Last = (digitalRead(buttonD4) == HIGH);
  blnButtonD5Last = (digitalRead(buttonD5) == HIGH);
  blnButtonD6Last = (digitalRead(buttonD6) == HIGH);
}

int DosingTimerGin(unsigned long PresetTime){
  unsigned long ElapsedTime = (CurrentMillis - StartMillisGin);
  
  Serial.print("StartMillis: ");
  Serial.println(StartMillisGin);
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
    DosingStartedGin = false;
    return (100);
  }
}

int DosingTimerTonic(unsigned long PresetTime){
  unsigned long ElapsedTime = (CurrentMillis - StartMillisTonic);
  
  Serial.print("StartMillis: ");
  Serial.println(StartMillisTonic);
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
    DosingStartedTonic = false;
    return (100);
  }
}

void DosingStepper(){
  if (ResetStepper){
    DosingStep = 0;
  }
  if ((DosingStep==0) && (TransStep0)){
    DosingStep = 1;
    DosingStartedGin = true;
    DosingStartedTonic = true;
    StartMillisGin = millis();
    StartMillisTonic = millis();
  } 
  if ((DosingStep==1) && (TransStep1)){
    DosingStep = 2;
  }
  if ((DosingStep==2) && (TransStep2)){
    DosingStep = 3;
  }

  TransStep0 = false;
  TransStep1 = false;
  TransStep2 = false;
  ResetStepper = false;
}

void ChangeRecipeRatio(bool up, bool down){
  if (up){
    RatioGin = RatioGin + 10;
    Serial.println("changeRecipeRatio cond1");
  }
  if (down){
    RatioGin = RatioGin - 10;
    Serial.println("changeRecipeRatio cond2");
  }
  if (RatioGin >= 100){
    RatioGin = 100;
    Serial.println("changeRecipeRatio cond3");
  }
  if (RatioGin <= 0){
      RatioGin = 0;
    Serial.println("changeRecipeRatio cond4");
  }    
  RatioTonic = 100 - RatioGin;
}


int RecalcTimerVal(int BaseTime, int Ratio){
  //Time value for 50% dosing ratio is known.
    Serial.print("RecalcTimerVal - BaseTime = ");
    Serial.println(BaseTime);
    Serial.print("RecalcTimerVal - Ratio = ");
    Serial.println(Ratio);
    float fltRatio = Ratio;
    float TimerRatio = fltRatio/50;
    Serial.print("RecalcTimerVal - TimerRatio = ");
    Serial.println(TimerRatio);
    Serial.print("RecalcTimerVal - Return = ");
    Serial.println(BaseTime*TimerRatio);
  return BaseTime*TimerRatio;
}

void loop() {

  CurrentMillis = millis();
  IOMirror(); //simplify IO
  
   if (blnButtonD4PosFlank) {
      TransStep0 = true;  
      if (DosingStep = 2) {
        ResetStepper = true;
        DrawBase();
      }
   }
   if (blnButtonD5PosFlank) {
      ChangeRecipeRatio(true,false);
   }
   if (blnButtonD6PosFlank) {
      ChangeRecipeRatio(false,true);
   }
   
   if ((blnButtonD5Status) && (blnButtonD6Status)) {
      ResetStepper = true;
      DrawBase();
      RatioGin = 20;
      RatioTonic = 80;
   }
  if (DosingStartedGin){
    LevelGin = DosingTimerGin(RecalcTimerVal(BaseTimeGin,RatioGin));
    digitalWrite (outputD7, RelayOn);
  }
  else {
    digitalWrite (outputD7, RelayOff);
  } 
  if (DosingStartedTonic){;
    LevelTonic = DosingTimerTonic(RecalcTimerVal(BaseTimeTonic,RatioTonic));
    digitalWrite (outputD3, RelayOn);
  }
  else {
    digitalWrite (outputD3, RelayOff);
  } 
  if ((DosingStartedGin) || (DosingStartedTonic)) { //OR
    DrawFillLevel((LevelGin + LevelTonic)/2);
  }
  DrawDosingPumps(105,DosingStartedGin);
  DrawDosingPumps(85,DosingStartedTonic);
  DrawText();
  DosingStepper();

  TransStep1 = ((!DosingStartedGin) && (!DosingStartedGin));
}
