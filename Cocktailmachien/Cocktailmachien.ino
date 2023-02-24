#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//voor de GFX
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define LOGO_HEIGHT   64
#define LOGO_WIDTH    16

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
}

void TekenTekst(int getal, int milis) {
  display.clearDisplay();

  display.setCursor(0,0);    
  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE); 
  display.print(F("Drukknop: ")); 
  display.print(getal); 
  display.println(F(" is ingedrukt")); 
  display.setTextSize(2); 
  display.setCursor(0,20);    
  display.println(F("Gin"));
  display.println(F("Tonic"));
  display.drawRect(50, 50, 10, 10, WHITE);

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

  DrawBase();
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

void loop() {
  
  IOMirror(); //simplify IO
  
  if (blnButtonD4PosFlank) {
      DrawFillLevel(20);
      digitalWrite (outputD7, RelayOn);
   }
   else if (blnButtonD5PosFlank) {
      DrawFillLevel(66);
      digitalWrite (outputD8, RelayOn);
   }
   else if (blnButtonD6PosFlank) {
      DrawFillLevel(100);
   }
   else {
      digitalWrite (outputD7, RelayOff);
      digitalWrite (outputD8, RelayOff);
   }


}
