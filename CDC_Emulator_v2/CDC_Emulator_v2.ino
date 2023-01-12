////////////////////////////////////////////////////
// — CD Changer Emulator for Audi Delta CC v2.0 — //
////////////////////////////////////////////////////

#include <Wire.h>
#include <SoftwareSerial.h>

#define HU_I2C_ADDRESS    (uint8_t)0x40

#define HU_START          (uint16_t)0xA121
#define HU_STOP           (uint16_t)0x21A1
#define HU_LEFT           (uint16_t)0x0181
#define HU_RIGHT          (uint16_t)0x0282
#define HU_DOWN           (uint16_t)0x0383
#define HU_UP             (uint16_t)0x0484
#define HU_SCAN           (uint16_t)0x8707
#define HU_RND            (uint16_t)0xA323
#define HU_1              (uint16_t)0x9901
#define HU_2              (uint16_t)0x9902
#define HU_3              (uint16_t)0x9903
#define HU_4              (uint16_t)0x9904
#define HU_5              (uint16_t)0x9905
#define HU_6              (uint16_t)0x9906

uint32_t timer = millis();
uint8_t cd = 0;
uint8_t tr = 0;
uint8_t cdc = 0;
uint8_t stp = 0;
bool isScan = false;
bool isRandom = false;

SoftwareSerial BT201(16, 17);

void setup() {
  wire();
  BT201.begin(9600);
  pp(450);
  BT201.println("AT+CM08\r\n");
  BT201.flush();
}

void loop() {
  if (cdc != 0) {
   if (cdc == 1) {  //HU_START
      cdc = 0;
      BT201.println("AT+CM01\r\n");  //Switch to BT Mode
      BT201.flush();
      delay(500);
      BT201.println("AT+CB\r\n");  //Start playback
      BT201.flush();
    }
    
    if (cdc == 2) {  //HU_STOP
      cdc = 0;
      BT201.println("AT+CM08\r\n");  //Stop playback & switch to idle mode
      BT201.flush();
    }

    if (cdc == 3) {  //HU_LEFT
      cdc = 0;
      BT201.println("AT+CD\r\n");  //Previous Song
      BT201.flush();
    }

    if (cdc == 4) {  //HU_RIGHT
      cdc = 0;
      BT201.println("AT+CC\r\n");  //Next Song
      BT201.flush();
    }  
    
    if (cdc == 5) {  //HU_DOWN
      cdc = 0;
      BT201.println("AT+BA03\r\n");  //Hang Up the call
      BT201.flush();
    }
    
    if (cdc == 6) {  //HU_UP
      cdc = 0;
      BT201.println("AT+BA04\r\n");  //Answer the call
      BT201.flush();
    }
     
    if (cdc == 7) {  //HU_SCAN
      cdc = 0;
      if (isScan) {
        BT201.println("AT+CU00\r\n");  //Unmute
        BT201.flush();
      }
      else if (!isScan) {
        BT201.println("AT+CU01\r\n");  //Mute
        BT201.flush();
      }
      isScan = !isScan;
      stat();
    }
    
    if (cdc == 8) {  //HU_RND
      cdc = 0;
      BT201.println("AT+CB\r\n");  //Play-Pause
      BT201.flush();
      isRandom = !isRandom;
      stat();
    }    
        
    if (cdc == 9) {  //HU_1
      cdc = 0;
      BT201.println("AT+CE\r\n");  //BT201 volume up
      BT201.flush();
    }

    if (cdc == 10) {  //HU_2
      cdc = 0;
      BT201.println("AT+CF\r\n");  //BT201 volume down
      BT201.flush();
    }

    if (cdc == 11) {  //HU_3
      cdc = 0;
      BT201.println("AT+CM00\r\n");  //Switch mode to next one
      BT201.flush();
    }
    
    //Output test
    if (cdc == 12) {  //HU_4
      cdc = 0;
      cd++;
      report();
    }

    if (cdc == 13) {  //HU_5
      cdc = 0;
      tr++;
      report();
    }

    if (cdc == 14) {  //HU_6
      cdc = 0;
      cd = 0;
      tr = 0;
      report();
    }
  }
}

void wire() {
  Wire.begin(HU_I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
}

void stat() {
  stp = 0;
  if (isScan) stp = 1;
  if (isRandom) stp = 2;
  if (isScan && isRandom) stp = 3;
  uint8_t st[27]; 
  uint8_t check = stp + 3;
  pp(40);
  for (uint8_t a = 0; a < 8; a++) {
    st[7-a] = bitRead(0x03, a);
    st[16-a] = bitRead(stp, a);
    st[25-a] = bitRead(check, a);
  }
  st[8] = 1;
  st[17] = 1;
  st[26] = 1;
  ticks(st, 27);
}

void report() {
  uint8_t st[36]; 
  uint16_t check = cd + tr + 1;
  if ((check & 0x1F0) && ((check - (cd & 0x0f)) <= (check & 0x1F0))) {
    check -= 0x10;
  }
  pp(30);
  for (uint8_t a = 0; a < 8; a++) {
    st[7-a] = bitRead(0x01, a);
    st[16-a] = bitRead(cd, a);
    st[25-a] = bitRead(tr, a);
    st[34-a] = bitRead(check, a);
  }
  st[8] = 1;
  st[17] = 1;
  st[26] = 1;
  st[35] = 1;
  ticks(st, 36);
}

void pp(uint16_t pau) {
  uint32_t timer3 = millis();
  while (millis() - timer3 < pau);
}

void ticks(uint8_t *arrSt, uint8_t tick) {
  Wire.end();
  DDRC = 0b110000;
  PORTC &= ~(1 << 4); delayMicroseconds(100);
  PORTC &= ~(1 << 5); delayMicroseconds(600);
  uint8_t b = 0;
  uint32_t timer2 = micros();
  uint8_t t = 0;
  while (b < tick) {
    if (micros() - timer2 >= 300) {
      timer2 = micros();
      if (t == 0 && arrSt[b] == 1) PORTC |= (1 << 4);
      if (t == 0 && arrSt[b] == 0) PORTC &= ~(1 << 4);
      if (t == 1) PORTC |= (1 << 5);
      if (t == 3) PORTC &= ~(1 << 5);
      t++;
      if (t == 4) {t = 0; b++;}
    }
  }
  PORTC &= ~(1 << 4); delayMicroseconds(600);
  PORTC |= (1 << 5); delayMicroseconds(100);
  PORTC |= (1 << 4);
  pp(20);
  wire();
}

void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    uint8_t x0 = Wire.read();
    if (0 < Wire.available()) {
      uint8_t x1 = Wire.read();
      uint16_t x = ((x0 << 8) + x1);
      switch (x) {
        case HU_START:
          cdc = 1;
          break;
        case HU_STOP:
          cdc = 2;
          break;
        case HU_LEFT:
          cdc = 3;
          break;
        case HU_RIGHT:
          cdc = 4;
          break;
        case HU_DOWN:
          cdc = 5;
          break;
        case HU_UP:
          cdc = 6;
          break;
        case HU_SCAN:
          cdc = 7;
          break;
        case HU_RND:
          cdc = 8;
          break;
        case HU_1:
          cdc = 9;
          break;
        case HU_2:
          cdc = 10;
          break;
        case HU_3:
          cdc = 11;
          break;
        case HU_4:
          cdc = 12;
          break;
        case HU_5:
          cdc = 13;
          break;
        case HU_6:
          cdc = 14;
          break;
      }
    }
  }
}
