#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define I2C_ADDR    0x27
/*
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
*/

#define PIN_ROT_A 2
#define PIN_ROT_B 4
#define PIN_BUTT_PUSH 8
#define PIN_BUTT_ROT 7
#define PIN_LED 13

#define NO_CELL_VOLTAGE 0.2
#define NO_CELL_STRING "-----"

#define BUTTON_BLOCK 10000

// znaky (127 druha sipka)
#define LCD_CHAR_BLOCK (char)255
#define LCD_CHAR_SIPKA (char)126

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

char rot_rot = 0;
unsigned int butt_push_block = 0;
unsigned int butt_rot_block = 0;

char buf[32];

// aktualne zvolena funkce
unsigned char actual_function = 0;

// vybrana vec na home
unsigned char home_sel = 1;

LiquidCrystal_I2C	lcd(I2C_ADDR,20,4);

SoftwareSerial mySerial(10, 11); // RX, TX

/*
 * zpracovani preruseni od rotacniho enkoderu
 */
void rotInterupt(){
  delay(3);
  if(digitalRead(PIN_ROT_A) == HIGH && rot_rot == 0){
    if(digitalRead(PIN_ROT_B) == HIGH){
      rot_rot = 1;
    } else {
      rot_rot = 2;
    }
  }
}

/*
 * nastaveni
 */
void setup(){

  // setup pinu
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_ROT_A, INPUT_PULLUP);
  pinMode(PIN_ROT_B, INPUT_PULLUP);
  pinMode(PIN_BUTT_PUSH, INPUT_PULLUP);
  pinMode(PIN_BUTT_ROT, INPUT_PULLUP);

  //digitalWrite(PIN_LED, HIGH);

  attachInterrupt(digitalPinToInterrupt(PIN_ROT_A), rotInterupt, LOW);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {  }

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);  
  
  lcd.begin();
  lcd.backlight();
  
  line = "";

  setActualFunction(0);
}

/**
 * hlavni smycka
 */
void loop() {

  // rotacni enkoder
  if(rot_rot != 0){
    if(rot_rot == 1){
      if(actual_function == 0){
        if(home_sel < 3){
          home_sel++;
          drawHomeDynamic();
        }
      }
    }
    if(rot_rot == 2){
      if(actual_function == 0){
        if(home_sel > 1){
          home_sel--;
          drawHomeDynamic();
        }
      }
    }
    rot_rot = 0;
  }

  // tlacitko mackaci
  if(digitalRead(PIN_BUTT_PUSH) == LOW && butt_push_block == 0){
    if(actual_function == 1){
      setActualFunction(0);
    }
    butt_push_block = BUTTON_BLOCK;
  } else {
    if(butt_push_block > 0) butt_push_block--;
  }

  // tlacitko enkoder
  if(digitalRead(PIN_BUTT_ROT) == LOW && butt_rot_block == 0){
    if(actual_function == 0){
      if(home_sel == 1){
        setActualFunction(1);
      }
    }
    butt_rot_block = BUTTON_BLOCK;
  } else {
    if(butt_rot_block > 0) butt_rot_block--;
  }

  // LipoCard
  if(actual_function == 1){
    // visi nejaka data na seriaku?
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
  }

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

/**
 * Nastaveni aktualni funkce
 */
void setActualFunction(unsigned char fce){
  actual_function = fce;

  if(fce == 0){
    drawHomeStatic();
    drawHomeDynamic();
  }

  if(fce == 1){
    drawLipoCardStaticA();
  }
}

/**
 * Vykresleni statiky pro uvodku
 */
void drawHomeStatic(){
  lcd.setCursor (0,0);
  lcd.print(" LipoCard           ");
  lcd.setCursor (0,1);
  lcd.print(" Servotester        ");
  lcd.setCursor (0,2);
  lcd.print(" Thermometer        ");
  lcd.setCursor (0,3);
  lcd.print("                    ");
}

/**
 * Dynamicka cast uvodky
 */
void drawHomeDynamic(){
  lcd.setCursor (0,0);
  if(home_sel == 1) lcd.print(LCD_CHAR_SIPKA); else lcd.print(" ");
  lcd.setCursor (0,1);
  if(home_sel == 2) lcd.print(LCD_CHAR_SIPKA); else lcd.print(" ");
  lcd.setCursor (0,2);
  if(home_sel == 3) lcd.print(LCD_CHAR_SIPKA); else lcd.print(" ");
}

/**
 * Staticka lipocard A
 */
void drawLipoCardStaticA(){
  lcd.setCursor (0,0);
  lcd.print("             1|     ");
  lcd.setCursor (0,1);
  lcd.print("             2|     ");
  lcd.setCursor (0,2);
  lcd.print("             3|     ");
  lcd.setCursor (0,3);
  lcd.print("             4|     ");
}

