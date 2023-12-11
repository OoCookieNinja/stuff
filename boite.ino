#include <AltSoftSerial.h>       // Other Serial
#include <Wire.h>                // I2C
#include "DFRobotDFPlayerMini.h" // MP3 Library
#include <SeeedRFID.h>           // RFID Library
#include "rgb_lcd.h"             // LCD Library

char* tag_list[]    = {"","20000EC5917A"                ,"20000B1693AE","510022542A0D","49005BB88E24"};
char* description[] = {"","Alyx sera un jour magnifique","Boop"        ,"Red"         ,"Blue"};

// Software Serial
// RFID
const int RFID_Rx = 8; // RX
const int RFID_Tx = 9; // TX
// MP3
const int MP3_Rx = 10; // RX
const int MP3_Tx = 11; // TX
// Assignation de liste/variable
// RFID
String rfid_tag = "";
unsigned char buffer[64];
int count = 0;
const char tag_char[15] = {"0123456789ABCDEF"};
char* rfid_buffer_char = "";
const int tag_lenght = sizeof(tag_list) / sizeof(int);
// LCD
const int lcd_width  = 16;
const int lcd_height = 2;
String txt = "";
const int des_lenght = sizeof(description) / sizeof(int);
int idx_des = 0;
int scroll = 0;
unsigned long scroll_time = 0;
int scroll_per_char = 500;
int scroll_wait_long = 1000;
// MP3
const int pin_potentiometre = 3;
const int pin_bouton_play = 4;
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

#define char_length(array) ((sizeof(array)) / (sizeof(array[0])))

DFRobotDFPlayerMini player;
rgb_lcd lcd;

// Basic
void setup() {
  pinMode(pin_bouton_play, INPUT);
  // Truc de Série
  Serial.begin(9600);
  SoftSerial.begin(9600);
  AltSerial.begin(9600);
  player.begin(SoftSerial);

  // Config LCD
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
bool is_acceptable() {
    for(int i = 0; i < 15; i++) {
        if (tag_char[i] == rfid_buffer_char) {
            return true;
        }
    }
    return false;
}

// RFID
void clearBufferArray() {
    for (int i=0; i<count; i++)  {
        buffer[i]= "";
    }
    count = 0;
}
signed int get_rfid_tag() {
  if (AltSerial.available() ) {
    while(AltSerial.available() ) {
      rfid_buffer_char = AltSerial.read();
      if (is_acceptable() and rfid_buffer_char != 0xFC) {
        buffer[count++] = rfid_buffer_char;
        rfid_tag = buffer;
      }
      if (count == 64) break;
    }
  }
  else {
    if (rfid_tag != "") {
      clearBufferArray();
      Serial.print("Code du tag RFID: ");
      Serial.println(rfid_tag);
      for (int idx = 0; idx < tag_lenght; idx++) { // Calcul de la position du TAG
        if (rfid_tag == String(tag_list[idx]) and idx <= des_lenght) {
          player.play(idx+1);
          playing = true;
          rfid_tag = "";
          scroll_time = millis();
          scroll = 0;
          Serial.println(idx);
          return idx;
        }
      }
    }
    rfid_tag = "";
    if (idx_des < 0) {
      Serial.println("Pas dans la Liste");
      return 1;
    }
  }
}

// LCD
void scroll_description() {
  lcd.setCursor(0, 0);
  if (scroll == 0 and millis() - scroll_time >= scroll_wait_long) { // Gestionaire du debut du message
    scroll_time = millis();
    scroll++;
    for (int i = 0; i < lcd_width; i++) {
      lcd.print(txt[i]);
    }
  }
  else if (txt.length()-scroll-lcd_width+1 == 0 and millis() - scroll_time >= scroll_wait_long) { // Gestionaire de la fin du message
    for (int i = 0; i < lcd_width; i++) {
      lcd.print(txt[i]);
    }
    scroll_time = millis();
    scroll = 0;
  }
  else if (txt.length()-scroll-lcd_width+1 > 0 and millis() - scroll_time >= scroll_per_char and scroll > 0) { // Pause pour chaque changement
    for (int i = 0; i < lcd_width; i++) {
      lcd.print(txt[scroll+i]);
    }
    scroll_time = millis();
    scroll++;
  }
}
void print_lcd_screen() { // Fonction pour le LCD
  // Description
  idx_des = get_rfid_tag();
  if (idx_des > -1 and playing) {
    txt = description[idx_des];
    if ( txt.length() > lcd_width ) { // Gestion de l'affichage (si description > ecran)
      scroll_description();
    }
    else { // Afficher sans modif si assez petit
      lcd.setCursor(0, 0);
      lcd.print(txt);
      for (int i=0; i<lcd_width-txt.length(); i++) {
        lcd.print(" ");
      }
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

// Boucle
void loop() {
  // Lecture
  volume_to_map = analogRead(pin_potentiometre);
  button = digitalRead(pin_bouton_play);
  // MP3
  volume_mapped_per = map(volume_to_map,0,1023,0,100);
  ol_volume_mapped_mp3 = volume_mapped_mp3;
  volume_mapped_mp3 = map(volume_to_map,0,1023,0,30);
  if (ol_volume_mapped_mp3 != volume_mapped_mp3) {
    player.volume(volume_mapped_mp3);
  }
  // Boutton Play/Pause
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
  // Autre
  print_lcd_screen(); // Gestion de l'afficheur LCD
}
