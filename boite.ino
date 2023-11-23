#include <SoftwareSerial.h>      // Serial Stuff
#include <Wire.h>                // I2C
#include "DFRobotDFPlayerMini.h" // MP3 Library
#include <SeeedRFID.h>           // RFID Library
#include "rgb_lcd.h"             // LCD Library

// Software Serial
int MP3_thing  = 11; // MP3
int RFID_Thing = 3;  // RFID
// RFID
String rfid_tag;
unsigned char buffer[64];
int count = 0;
signed int rfid_tag_pos;
char* tag_list[] = {"1","2"};
int tag_lenght = sizeof(tag_list) / sizeof(int);
// LCD
int lcd_width  = 16;
int lcd_height = 2;
char* description[] = {"Alyx sera un jour magnifique","Boop :3"};
int des_lenght = sizeof(description) / sizeof(int);
int idx_des = 0;
int scroll = 0;
unsigned long scroll_time = 0;
// MP3
int pin_potentiometre = 3;
int volume_to_map = 0;
int volume_mapped_per = 0;
int ol_volume_mapped_mp3 = 0;
int volume_mapped_mp3 = 0;

DFRobotDFPlayerMini player;
rgb_lcd lcd;
SoftwareSerial SoftSerial(RFID_Thing, MP3_thing);

// Basic
void setup() {
  Serial.begin(9600);
  SoftSerial.begin(9600);
  lcd.begin(lcd_width, lcd_height);
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
        buffer[i]=NULL;
    }
}
signed int get_rfid_tag() {
  if (SoftSerial.available()) {
    rfid_tag = "";
    while(SoftSerial.available()) {
      buffer[count++] = SoftSerial.read();
      if(count == 64)break;
    }
    rfid_tag = buffer;
    Serial.print("RFID card number: ");
    Serial.println(rfid_tag);
    clearBufferArray();
    count = 0;
    int idx = 0;
    while (idx < tag_lenght) { // Calculates and returns position
      if (rfid_tag == tag_list[idx]) {
        player.play(idx);
        return idx;
      }
      idx++;
    }
    return -1; // Error code
  } else {
    return;
  }
}


// LCD
void scroll_description() {
  String txt = description[idx_des]; // Temp value for printing shifted description
  if (txt.length() - scroll - lcd_width == 0 and millis() - scroll_time >= 740) { // Check if description is being rolled-back / started
    scroll_time = millis();
    scroll = 0;
  }
  else if (txt.length() - scroll - lcd_width != 0 and millis() - scroll_time >= 240) { // Pause for every shift
    for (int i = 0; i < lcd_width; i++) {
      lcd.print(txt[scroll + i]);
    }
    scroll_time = millis();
    scroll++;
  }
}
void print_lcd_screen() { // Function for everything showed on the LCD
  // Description
  for (int i = 0; i < 16; i++) { // Reset Character for description
    lcd.setCursor(i, 0);
    lcd.print(" ");
  }
  lcd.setCursor(0, 0);
  String des_test = description[idx_des];
  if ( des_test.length() > lcd_width) { // If the description is bigger than the screen
    scroll_description();
  }
  else { // prints if small enough
    lcd.print(description[idx_des]);
  }
  // Volume
  lcd.setCursor(lcd_width - 4, lcd_height);
  lcd.print("    ");
  lcd.setCursor(lcd_width - 1 - int_length(volume_mapped_per), lcd_height); // Positioning pointer accordingly to the Percentage
  lcd.print(volume_mapped_per);
  lcd.print("%");
  // Play / Pause
}


// Loop
void loop() {
  // Reading
  volume_to_map = analogRead(pin_potentiometre);
  // MP3
  volume_mapped_per = map(volume_to_map,0,1023,0,100);
  ol_volume_mapped_mp3 = volume_mapped_mp3;
  volume_mapped_mp3 = map(volume_to_map,0,1023,0,30);
  if (ol_volume_mapped_mp3 != volume_mapped_mp3) {
    player.volume(volume_mapped_mp3);
  }
  // Other
  rfid_tag_pos = get_rfid_tag();
  switch (rfid_tag_pos) { // Error codes Managing
    case -1:
      idx_des = 0;
    default:
      idx_des = rfid_tag_pos;
  }
  print_lcd_screen(); // Shows Stuff on LCD
}
