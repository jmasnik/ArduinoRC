#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

#define NO_CELL_VOLTAGE 0.2
#define NO_CELL_STRING "-----"

int n = 1;

char serial_char;
String line;

// celkove napeti
double v_voltage_total = 0;

// napeti na clancich
double v_voltage_c1 = 0;
double v_voltage_c2 = 0;
double v_voltage_c3 = 0;
double v_voltage_c4 = 0;

// cas
long v_time = -1;
long v_prev_time = -1;

// proud
double v_current = 0;

// kapacita
double v_capacity = 0;

char buf[32];

LiquidCrystal_I2C	lcd(I2C_ADDR,20,4);

SoftwareSerial mySerial(10, 11); // RX, TX

void setup(){
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);  
  
  lcd.begin();
  lcd.backlight();
  
  // Switch on the backlight
  //lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  //lcd.setBacklight(HIGH);

  //lcd.setCursor (0,0);
  //char ch[4] = {255,127,126,0};
  //lcd.print(ch);  
  
  lcd.setCursor (13,0);
  lcd.print("X|");
  lcd.setCursor (13,1);
  lcd.print("2|");
  lcd.setCursor (13,2);
  lcd.print("3|");
  lcd.setCursor (13,3);
  lcd.print("4|");
  
  line = "";
}

void loop() {
    
  if(mySerial.available() > 0){
    serial_char = mySerial.read();
    
    if(serial_char == '\n'){
      
      // zpracovani radky
      if(line[0] == '1' && line[1] == ':'){
        
        //Serial.println("L - " + line);

        // hodnoty vyseparovane ze stringu
        if(v_time != -1){
          v_prev_time = v_time;
        }
        v_time = (line.substring(2,7)).toInt() * 1000;
        v_voltage_total = (line.substring(8,13)).toInt() / 1000.0;
        v_current = (line.substring(14,19)).toInt() / 1000.0;        
        v_voltage_c1 = (line.substring(26,31)).toInt() / 1000.0;
        v_voltage_c2 = (line.substring(35,40)).toInt() / 1000.0;
        v_voltage_c3 = (line.substring(44,49)).toInt() / 1000.0;
        v_voltage_c4 = (line.substring(53,58)).toInt() / 1000.0;
        if(v_prev_time != -1 && v_time > v_prev_time){
          v_capacity = v_capacity + (v_current * 1000.0 * ((v_time - v_prev_time) / 3600000.0));
        }

        // cas
        lcd.setCursor (1,0);
        lcd.print(getFormatedTime(v_time));
        lcd.print(" min");
        
        // celkove napeti
        dtostrf(v_voltage_total,7,3,buf);
        lcd.setCursor (0,1);
        lcd.print(buf);
        lcd.print(" V");
      
        
        // cell
        dtostrf(v_voltage_c1,4,3,buf);
        lcd.setCursor (15,0);
        if(v_voltage_c1 < NO_CELL_VOLTAGE){
          lcd.print(NO_CELL_STRING);
        } else {
          lcd.print(buf);
        }

        dtostrf(v_voltage_c2,4,3,buf);
        lcd.setCursor (15,1);
        if(v_voltage_c2 < NO_CELL_VOLTAGE){
          lcd.print(NO_CELL_STRING);
        } else {
          lcd.print(buf);
        }
        
        dtostrf(v_voltage_c3,4,3,buf);
        lcd.setCursor (15,2);
        if(v_voltage_c3 < NO_CELL_VOLTAGE){
          lcd.print(NO_CELL_STRING);
        } else {
          lcd.print(buf);
        }

        dtostrf(v_voltage_c4,4,3,buf);
        lcd.setCursor (15,3);
        if(v_voltage_c4 < NO_CELL_VOLTAGE){
          lcd.print(NO_CELL_STRING);
        } else {
          lcd.print(buf);
        }
        
        // proud
        dtostrf(v_current,7,3,buf);
        lcd.setCursor (0,2);
        lcd.print(buf);
        lcd.print(" A");
        
        // kapacita
        dtostrf(v_capacity,7,0,buf);
        lcd.setCursor (0,3);
        lcd.print(buf);
        lcd.print(" mAh");
      }
      
      // spusteno nabijeni
      if(line.startsWith("(A1) Akku_dran")){
        // vynulujem kapacitu
        v_capacity = 0;
      }
      
      line = "";
    } else {
      line += serial_char;
    }
    
    Serial.write(serial_char);
  }

  //delay(1000);
}

/**
Vraci cas jako " 14:00" - vzdy 6 znaku
*/
String getFormatedTime(long millis){
  String ret = String();
  int min, sec;
  
  min = (millis / 1000) / 60;
  sec = (millis / 1000) - min * 60;
  
  if(min < 10){
    ret += " ";
  }
  if(min < 100){
    ret += " ";
  }
  ret += min;
  ret += ":";
  if(sec < 10){
    ret += "0";
  }
  ret += sec;
  
  return ret;  
}

