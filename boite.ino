#include <AltSoftSerial.h>       // Other Serial
#include <Wire.h>                // I2C
#include "DFRobotDFPlayerMini.h" // MP3 Library
#include <SeeedRFID.h>           // RFID Library
#include "rgb_lcd.h"             // LCD Library

// Software Serial
// RFID
int RFID_Rx = 8; // RX
int RFID_Tx = 9; // TX
// MP3
int MP3_Rx = 10; // RX
int MP3_Tx = 11; // TX
// RFID
String rfid_tag = "X";
unsigned char buffer[64];
int count = 0;
int getting_data = 0;
signed int rfid_tag_pos;
char* tag_list[] = {"49005BB88E24","510022542A0D"};
int tag_lenght = sizeof(tag_list) / sizeof(int);
// LCD
int lcd_width  = 16;
int lcd_height = 2;
char* description[] = {"Alyx sera un jour magnifique","Boop :3"};
int des_lenght = sizeof(description) / sizeof(int);
int idx_des = 0;
int scroll = 1;
unsigned long scroll_time = 0;
int scroll_per_char = 500;
int scroll_wait_long = 1000;
// MP3
int pin_potentiometre = 3;
int pin_bouton_play = 4;
int ol_button = 1;
int button = 0;
int playing = false;
int volume_to_map = 0;
int volume_mapped_per = 0;
int ol_volume_mapped_mp3 = 0;
int volume_mapped_mp3 = 0;

AltSoftSerial AltSerial;
#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(MP3_Rx, MP3_Tx);
#define FPSerial SoftSerial

DFRobotDFPlayerMini player;
rgb_lcd lcd;

// Basic
void setup() {
  pinMode(pin_bouton_play, INPUT);
  Serial.begin(9600);
  SoftSerial.begin(9600);
  AltSerial.begin(9600);
  player.begin(SoftSerial);
  
  lcd.begin(lcd_width, lcd_height);
  scroll_time = millis();
  scroll_wait_long = (scroll_wait_long-10)*4;
  scroll_per_char = (scroll_per_char-10)*4;
}
int int_length(int l) {
  unsigned int number_of_digits = 0;
  do {
     ++number_of_digits; 
     l /= 10;
  } while (l);
  return number_of_digits;
}


// RFID
void clearBufferArray() {
    for (int i=0; i<count; i++)  {
        buffer[i]= "";
    }
}
signed int get_rfid_tag() {
  if (AltSerial.available() or rfid_tag != "") {
    if (getting_data == 0) {
      getting_data = 1;
      rfid_tag = "";
      Serial.println(" ");
      Serial.print("RFID card number: ");
    }
    while(AltSerial.available()) {
      buffer[count++] = AltSerial.read();
      if(count == 64)break;
    }
    rfid_tag = buffer;
    Serial.print(rfid_tag);
    clearBufferArray();
    count = 0;
    int idx = 0;
    while (idx < tag_lenght) { // Calculates and returns position
      if (rfid_tag == tag_list[idx]) {
        player.play(idx+1);
        return idx;
      }
      idx++;
    }
    getting_data = 0;
  } else {
    getting_data = 0;
    return;
  }
}


// LCD
void scroll_description() {
  String txt = description[idx_des]; // Temp value for printing shifted description
  if (scroll = 0 and millis() - scroll_time >= scroll_wait_long) { // Check if description is being rolled-back / started
    scroll_time = millis();
    scroll++;
  }
  else if (txt.length() - scroll - lcd_width == -1 and millis() - scroll_time >= scroll_wait_long) {
    for (int i = 0; i < lcd_width; i++) {
      lcd.print(txt[i]);
    }
    scroll_time = millis();
    scroll = 0;
  }
  else if (txt.length() - scroll - lcd_width != -1 and millis() - scroll_time >= scroll_per_char and scroll > 0) { // Pause for every shift
    for (int i = 0; i < lcd_width; i++) {
      lcd.print(txt[scroll+i]);
    }
    scroll_time = millis();
    scroll++;
  }
}
void print_lcd_screen() { // Function for everything showed on the LCD
  // Description
  lcd.setCursor(0, 0);
  if (idx_des != -1) {
    String des_test = description[idx_des];
    if ( des_test.length() > lcd_width) { // If the description is bigger than the screen
      scroll_description();
    }
    else { // prints if small enough
      lcd.print(description[idx_des]);
    }
  }
  // Volume
  lcd.setCursor(lcd_width - 4, lcd_height);
  for (int i = 0; i < 3 - int_length(volume_mapped_per); i++) {
    lcd.print(" ");
  }
  lcd.print(volume_mapped_per);
  lcd.print("%");
  // Play / Pause
  lcd.setCursor(0, lcd_height);
  if (playing) {
    lcd.print("Play ");
  }
  else {    
    lcd.print("Pause");
  }
}


// Loop
void loop() {
  // Reading
  volume_to_map = analogRead(pin_potentiometre);
  // MP3
  volume_mapped_per = map(volume_to_map,0,1023,0,101);
  ol_volume_mapped_mp3 = volume_mapped_mp3;
  volume_mapped_mp3 = map(volume_to_map,0,1023,0,30);
  if (ol_volume_mapped_mp3 != volume_mapped_mp3) {
    player.volume(volume_mapped_mp3);
  }
  // Play Button
  button = digitalRead(pin_bouton_play);
  if (button == 1 and ol_button == 0) {
    if (playing) {
      player.pause();
    }
    else if (not playing) {
      player.start();
    }
    playing = not playing;
    ol_button = 1;
  }
  else if (button == 0 and ol_button == 1) {
    ol_button = 0;
  }
  // Other
  idx_des = get_rfid_tag();
  print_lcd_screen(); // Shows Stuff on LCD
}
