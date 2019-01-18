/*
  Sleep RTC Alarm for Arduino Zero

  Demonstrates the use an alarm to wake up an Arduino zero from Standby mode

  This example code is in the public domain

  http://arduino.cc/en/Tutorial/SleepRTCAlarm

  created by Arturo Guadalupi
  17 Nov 2015
  modified 
  01 Mar 2016
  
  NOTE:
  If you use this sketch with a MKR1000 you will see no output on the serial monitor.
  This happens because the USB clock is stopped so it the USB connection is stopped too.
  **To see again the USB port you have to double tap on the reset button!**
*/

#include <RTCZero.h>


/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 00;
const byte hours = 17;

/* Change these values to set the current initial date */
const byte day = 17;
const byte month = 11;
const byte year = 15;

int matchSS;

#define debugSerial SerialUSB
#define loraSerial Serial1
#define loraReset 4
#define RESPONSE_LEN  100

// Variables
static const char hex[] = { "0123456789ABCDEF" };
static char response[RESPONSE_LEN];

/*
 * Vacia el buffer de recepcion desde el modulo LoRa
 */
void loraClearReadBuffer()
{
  while(loraSerial.available())
    loraSerial.read();
}

/*
 * Espera la respuesta del modulo LoRa y la imprime en el puerto de debug
 */
void loraWaitResponse(int timeout)
{
  size_t read = 0;

  loraSerial.setTimeout(timeout);
  
  read = loraSerial.readBytesUntil('\n', response, RESPONSE_LEN);
  if (read > 0) {
    response[read - 1] = '\0'; // set \r to \0
    debugSerial.println(response);
  }
  else
    debugSerial.println("Response timeout");
}

/*
 * Envia un comando al modulo LoRa y espera la respuesta
 */
void loraSendCommand(char *command)
{
  loraClearReadBuffer();
  debugSerial.println(command);
  loraSerial.println(command);
  loraWaitResponse(1000);
}

void setup()
{
  // Configura los pines como salidas o entradas con pull-up
  // o estado definido, porque si quedan flotando aumenta el consumo
  pinMode(0,OUTPUT);
  pinMode(1,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
//  pinMode(13,OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP); 
  pinMode(14,OUTPUT);
  pinMode(15,OUTPUT);
  pinMode(16,OUTPUT);
  pinMode(17,OUTPUT);
  pinMode(18,OUTPUT);
  pinMode(19,OUTPUT);
  pinMode(20,OUTPUT);
  pinMode(21,OUTPUT);
  pinMode(A0,OUTPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(loraReset, OUTPUT);

  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Delay antes de entrar en bajo consumo para que de tiempo
  // a reprogramar desde Arduino
  delay(10000);

  // Saluda y prende el LED
  debugSerial.println("Hellow World!!");
  digitalWrite(PIN_LED, LOW);

  // Arranca el modulo LoRa y lo pone en bajo consumo por 2 minutos
  digitalWrite(loraReset, LOW);
  delay(1000);
  digitalWrite(loraReset, HIGH);
  loraWaitResponse(10000);
  delay(1000);
  loraSendCommand("sys sleep 120000");
  delay(10000);

  // Arranca el RTC
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  // Inicia un match para que interrumpa a los 10"
  matchSS = 10;
  rtc.setAlarmTime(17, 00, matchSS);
  rtc.enableAlarm(rtc.MATCH_SS);
  rtc.attachInterrupt(alarmMatch);
}

void loop()
{
  digitalWrite(PIN_LED, HIGH);  // Apaga el LED
  USBDevice.detach();   // apaga el USB
  rtc.standbyMode();    // Sleep until next alarm match

  // Enciende el LED durante 1"
  digitalWrite(PIN_LED, LOW);
  delay(1000);
}

void alarmMatch()
{
  matchSS = (matchSS + 10) % 60; 
  rtc.setAlarmTime(17, 00, matchSS);
}

