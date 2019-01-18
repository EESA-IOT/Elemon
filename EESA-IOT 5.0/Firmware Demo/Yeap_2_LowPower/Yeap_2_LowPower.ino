#include <CayenneLPP.h>
#include <RTCZero.h>

// Macros
#define debugSerial SerialUSB
#define loraSerial Serial1
#define loraReset 4
#define RESPONSE_LEN  100

// Variables
static const char hex[] = { "0123456789ABCDEF" };
static char response[RESPONSE_LEN];
CayenneLPP lpp(100);
static enum {
  INIT = 0,
  JOIN, JOINED
} state;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 00;
const byte hours = 17;

/* Change these values to set the current initial date */
const byte day = 17;
const byte month = 11;
const byte year = 15;

int matchSS;

/* Create an rtc object */
RTCZero rtc;

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
  else {
    debugSerial.println("Response timeout");
    response[0] = '\0';
  }
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

/*
 * Transmite un mensaje de datos por LoRa
 */
void loraSendData(int port, uint8_t *data, uint8_t dataSize)
{
  char cmd[200];
  char *p;
  int i;
  uint8_t c;

  sprintf(cmd, "mac tx cnf %d ", port);

  p = cmd + strlen(cmd);
  
  for (i=0; i<dataSize; i++) {
    c = *data++;
    *p++ = hex[c>>4];
    *p++ = hex[c&0x0f];
  }
  *p++ = 0;

  loraSendCommand(cmd);
  if (strstr(response, "ok"))
    loraWaitResponse(20000);
}

void allOutputs()
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
}

/*
 * Funcion de inicializacion
 */
void setup() {
  allOutputs();
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(loraReset, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  
  loraSerial.begin(57600);
  debugSerial.begin(9600);

  delay(10000);
  debugSerial.println("Hellow World!!");

  loraWaitResponse(1000);
  digitalWrite(loraReset, LOW);
  delay(1000);
  digitalWrite(loraReset, HIGH);
  loraWaitResponse(10000);
  digitalWrite(PIN_LED, LOW);
  loraSendCommand("sys reset");
  loraSendCommand("sys get hweui");

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

bool doLowPower()
{
  // pone al modulo LoRa en bajo consumo
  loraSendCommand("sys sleep 120000");
 
  // Entra en bajo consumo hasta que interrumpa el RTC
  digitalWrite(PIN_LED, HIGH);  // Apaga el LED
  USBDevice.detach();           // apaga el USB
  rtc.standbyMode();            // Sleep until next alarm match

  // Al despertar retoma el USB
  USBDevice.init();
  USBDevice.attach();           // recupera el USB
  
  delay(500);
  loraSendCommand("radio get snr");
  delay(1000);
  loraSendCommand("radio get snr");

  if(!response[0]) {
    digitalWrite(loraReset, LOW);
    delay(1000);
    digitalWrite(loraReset, HIGH);
    loraWaitResponse(2000);
    return false;
  }

  return true;
}

void loop() {
  static int i, dr;
  
  switch (state) {
    case INIT: 
 //     loraSendCommand("sys factoryRESET");
 //     loraWaitResponse(10000);
      loraSendCommand("sys reset");
      loraSendCommand("sys get hweui");
      loraSendCommand("mac set appeui 1111111111111111");
      loraSendCommand("mac set appkey 11111111111111111111111111111111");
      loraSendCommand("mac set adr on");
      state = JOIN;
      break;
    case JOIN:
      loraSendCommand("mac join otaa");
      loraWaitResponse(10000);
      if (strstr(response, "accepted")) {
        loraSendCommand("mac get adr");
        i = 0;
        state = JOINED;
      }
      break;
    case JOINED:
      if (i++ < 5) {
        loraSendCommand("mac get dr");
        sscanf(response, "%d", &dr);
        debugSerial.println(dr);
      
        lpp.reset();
        lpp.addTemperature(1, 22.5);
        if (dr >= 1)
          lpp.addBarometricPressure(2, 1073.21);
        if (dr >= 2)
          lpp.addGPS(3, 52.37365, 4.88650, 2);
        if (dr >= 2) {
          lpp.addAnalogInput(4, 12.33);
          lpp.addAnalogInput(5, 12.33);
          lpp.addAnalogInput(6, 12.33);
          lpp.addAnalogInput(7, 12.33);
          lpp.addAnalogInput(8, 12.33);
          lpp.addAnalogInput(9, 12.33);
          lpp.addAnalogInput(10, 12.33);
          lpp.addAnalogInput(11, 12.33);
        }

        loraSendData(1, lpp.getBuffer(), lpp.getSize());
        digitalWrite(PIN_LED, HIGH);
        if (strstr(response, "mac_tx_ok")) {
          delay(200);
          digitalWrite(PIN_LED, LOW);
          delay(200);
          digitalWrite(PIN_LED, HIGH);
          i = 0;
        }

        else if (strstr(response, "invalid_data_len")) {
          delay(1000);
          digitalWrite(PIN_LED, LOW);
          loraSendData(2, lpp.getBuffer(), 1);
          digitalWrite(PIN_LED, HIGH);
        }

        loraSendCommand("radio get snr");
        if (!doLowPower())
          state = INIT;
//        delay(5000);
        digitalWrite(PIN_LED, LOW);
      }
      else
        state = INIT;
      break;
  }
}

void alarmMatch()
{
  // Larga de nuevo la alarma 20" adelante
  matchSS = (matchSS + 20) % 60; 
  rtc.setAlarmTime(17, 00, matchSS);
}

