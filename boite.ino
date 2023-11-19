#include <SoftwareSerial.h> // Serial Stuff
#include <Wire.h>           // I2C
#include <DFMiniMp3.h>      // MP3 Library
#include <SeeedRFID.h>      // RFID Library
#include "rgb_lcd.h"        // LCD Library

// RFID
int RFID_pin_in  = 5;
int RFID_pin_out = 6;
int rfid_tag;
signed int rfid_tag_pos;
int tag_list[] = [1];
int tag_lenght = sizeof(tag_list) / sizeof(int);
// LCD
int lcd_width  = 16;
int lcd_height = 2;
char description[] = ["Alyx sera un jour magnifique","Boop :3"];
int des_lenght = sizeof(description) / sizeof(int);
int idx_des = 0;
int scroll = 0;
unsigned long scroll_time = 0;
// MP3
int pin_potentiometre = 0;
int volume_to_map = 0;
int volume_mapped_per = 0;
int volume_mapped_mp3 = 0;

rgb_lcd lcd;
SeeedRFID RFID(RFID_pin_out, RFID_pin_in);



void setup() {
  Serial.begin(57600);
  lcd.begin(lcd_width, lcd_height);
}
// RFID
signed int get_rfid_tag() {
  RFID.listen() // Opens serial for RFID

  if (RFID.isAvailable()) { // Gets tag ID
    rfid_tag = RFID.cardNumber()
    Serial.print("RFID card number: ");
    Serial.println(rfid_tag);
    int idx = 0
    while (idx < tag_lenght) { // Calculates and returns position
      if (rfid_tag == tag_list[idx]) {
        return idx;
      }
      idx++
    }
    return -1; // Error code
  }
  else {
    return -2; // Error code
  }
}
// LCD
void scroll_description() {
  char txt = description[idx_des] // Temp value for printing shifted description
  if (lenght(txt) - scroll - lcd_width == 0 and millis() - scroll_time >= 740) { // Check if description is being rolled-back / started
    scroll_time = millis();
    scroll = 0;
  }
  else if (lenght(txt) - scroll - lcd_width != 0 and millis() - scroll_time >= 240) { // Pause for every shift
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
  if ( size(description[idx_des]) > lcd_width) { // If the description is bigger than the screen
    scroll_description();
  }
  else { // prints if small enough
    lcd.print(description[idx_des]);
  }
  // Volume
  lcd.setCursor(lcd_width-1-length(volume_mapped_per), lcd_height); // Positioning pointer accordingly to the Percentage
  lcd.print(string(volume_mapped_per));
  lcd.print("%");
  // Play / Pause
}
// Loop
void loop() {
  // Reading
  volume_to_map = analogRead(pin_potentiometre);
  // Assign
  volume_mapped_per = map(volume_to_map,0,1023,0,100);
  volume_mapped_mp3 = map(volume_to_map,0,1023,0,30);
  // Other
  rfid_tag_pos = get_rfid_tag();
  switch (rfid_tag_pos) { // Error codes Managing
    case -1:
      Serial.print("Error Code -1: Tag is not in list");
    case -2:
      Serial.print("Error Code -2: RFID is not available");
    default:
      idx_des = rfid_tag_pos;
  }
  print_lcd_screen() // Shows Stuff on LCD
}