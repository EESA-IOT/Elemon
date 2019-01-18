#include <SerialRAM.h>

// Macros
#define debugSerial SerialUSB
#define loraSerial Serial1
#define esp32Serial Serial
#define loraReset 4
#define RESPONSE_LEN  100
#define MAX_RESPONSES  10

// Variables
static char response[RESPONSE_LEN];
static char responses[MAX_RESPONSES][RESPONSE_LEN];
SerialRAM ram;

/*
 * Vacia el buffer de recepcion desde el modulo ESP32
 */
void esp32ClearReadBuffer()
{
  while(esp32Serial.available())
    esp32Serial.read();
}

/*
 * Vacia el buffer de recepcion desde el modulo LoRa
 */
void loraClearReadBuffer()
{
  while(loraSerial.available())
    loraSerial.read();
}

/*
 * Espera las respuestas del modulo ESP32 y las imprime en el puerto de debug
 */
int esp32WaitForResponses(int timeout)
{
  size_t read = 0;
  int i = 0;

  esp32Serial.setTimeout(timeout);

  while (1) {
    read = esp32Serial.readBytesUntil('\n', responses[i], RESPONSE_LEN);
    if (read > 1) {
      responses[i][read - 1] = '\0'; // set \r to \0
      debugSerial.println(responses[i]);
      if (strcmp(responses[i], "ERROR") == 0) {
        return -i;
      }
      if (strcmp(responses[i], "OK") == 0) {
        return i;
      }
      if (strcmp(responses[i], "SEND OK") == 0) {
        return i;
      }
      if (i < MAX_RESPONSES-1)
        i++;
    }
    else if (read == 0) {
      debugSerial.println("Response timeout");
      return i;
    }
  }
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
 * Envia un comando al modulo ESP32 y espera la respuesta
 */
void esp32SendCommand(char *command, int timeout)
{
  esp32ClearReadBuffer();
  debugSerial.println(command);
  esp32Serial.println(command);
  esp32WaitForResponses(timeout);
}

/*
 * Envia un comando al modulo LoRa y espera la respuesta
 */
void loraSendCommand(char *command, int timeout)
{
  loraClearReadBuffer();
  debugSerial.println(command);
  loraSerial.println(command);
  loraWaitResponse(timeout);

  loraSerial.begin(57600);
  debugSerial.begin(9600);
}

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  pinMode(loraReset, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  digitalWrite(loraReset, LOW);
  
  loraSerial.begin(57600);
  esp32Serial.begin(115200);
  debugSerial.begin(9600);
  ram.begin();

  digitalWrite(PIN_LED, LOW);
  delay(300);
  digitalWrite(PIN_LED, HIGH);
  delay(300);
  digitalWrite(PIN_LED, LOW);
  delay(300);
  digitalWrite(PIN_LED, HIGH);
  delay(300);
  digitalWrite(PIN_LED, LOW);
  delay(300);
  digitalWrite(PIN_LED, HIGH);
  delay(300);

  delay(8000);
  digitalWrite(PIN_LED, LOW);
  
  debugSerial.println("********************");
  debugSerial.println("*   EESA-IOT 5.0   *");
  debugSerial.println("********************");

  digitalWrite(loraReset, HIGH);
  loraWaitResponse(10000);

  loraSendCommand("sys factoryRESET", 10000);

  esp32SendCommand("AT", 1000);
  esp32SendCommand("AT+RST", 1000);
  delay(2000);
  esp32SendCommand("ATE0", 1000);

  if (!ram.getAutoStore()) {
    ram.setAutoStore(true);
    ram.write(0x0100, 0);
    debugSerial.println("Autostore enabled!");
  }
  else
    debugSerial.println("Autostore already enabled!");

  digitalWrite(PIN_LED, HIGH);
  delay(2000);
}

void loop()
{
  uint8_t x = 0;
  char s[30];
  
  digitalWrite(PIN_LED, LOW);
  debugSerial.println("---------------------- EESA-IOT 5.0 WiFi+BLE ESP32 ---------");
  esp32SendCommand("AT+GMR", 1000);
  debugSerial.println("");
  debugSerial.println("---------------------- EESA-IOT 5.0 LoRa RN2903 ------------");
  loraSendCommand("sys get ver", 10000);
  loraSendCommand("sys get hweui", 1000);
  debugSerial.println("");
  debugSerial.println("---------------------- EESA-IOT 5.0 EERAM 47L16 ------------");
  x = ram.read(0x0100);
  sprintf(s, "EERAM read: %d", x);
  debugSerial.println(s);
  ram.write(0x0100, x+1);
  debugSerial.println("");
  
  delay(400);
  digitalWrite(PIN_LED, HIGH);
  delay(500);
}
