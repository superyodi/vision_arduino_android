#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

int Tx=7; //전송
int Rx=6; //수신

 
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial btSerial(Tx, Rx);

String readString;
 
void setup() 
{

  Serial.begin(9600);
  btSerial.begin(9600); 
  lcd.begin();
  lcd.backlight();
}
 
void loop(){

  while (btSerial.available()) {  
    delay(3);                  
    char c = btSerial.read();  // 시리얼 읽은 값을 문자로 
    readString += c;         // 문자열로

    }

   if (readString.length() >0){ 
    lcd.clear();
    
    btSerial.println(readString);  
    lcd.print(readString);

 
    readString=""; 

    }
   }



 
 
