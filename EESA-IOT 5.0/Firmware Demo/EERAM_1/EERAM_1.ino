#include <SerialRAM.h>
 
#define debugSerial SerialUSB

SerialRAM ram;
 
void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  debugSerial.begin(9600);
  ram.begin();

  delay(3000);

  if (!ram.getAutoStore()) {
    ram.setAutoStore(true);
    debugSerial.println("Autostore enabled!");
  }
  else
    debugSerial.println("Autostore already enabled!");
    
  if (!digitalRead(PIN_BUTTON)) {
    ram.write(0x0100, 0);
    ram.write(0x0101, 'A');
    debugSerial.println("Data initialized!");
  }
}

void loop()
{
  uint8_t x = 0, c;
  char s[30];
  
  x = ram.read(0x0100);
  c = ram.read(0x0101);
  sprintf(s, "read: %c %d", c, x);
  debugSerial.println(s);
  ram.write(0x0100, x+1);
  
  delay(2000);
}
