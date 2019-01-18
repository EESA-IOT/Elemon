// Macros
#define debugSerial SerialUSB
#define esp32Serial Serial
#define RESPONSE_LEN   100
#define MAX_RESPONSES  10
#define WEB_SERVER "maker.ifttt.com"
#define WEB_PORT 80
#define WEB_URL "/trigger/timestamp/with/key/bUwf9UeUIaipws3HhD3B3qAwv5oNgbQNENcZmJ6mj5v" 

// Variables
static char response[RESPONSE_LEN];
static char responses[MAX_RESPONSES][RESPONSE_LEN];
static const char *REQUEST = 
    "POST " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "Connection: close\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 50\r\n\r\n"
    "{\"value1\": \"12\", \"value2\" : \"34\", \"value3\" : \"56\"}"  ; 

/*
 * Vacia el buffer de recepcion desde el modulo ESP32
 */
void esp32ClearReadBuffer()
{
  while(esp32Serial.available())
    esp32Serial.read();
}

/*
 * Espera respuestas del modulo ESP32 y las imprime en el puerto de debug
 * Sale por timeout o por respuesta terminadora (OK, ERROR, etc.)
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
 * Envia un comando al modulo ESP32 y espera la respuesta
 */
void esp32SendCommand(char *command, int timeout)
{
  esp32ClearReadBuffer();
  debugSerial.println(command);
  esp32Serial.println(command);
  esp32WaitForResponses(timeout);
}

void esp32TcpClientOpen(char *server, int port)
{
  char s[100];
  
  sprintf(s, "AT+CIPSTART=\"TCP\",\"%s\",%d", server, port);
  esp32SendCommand(s, 10000);
}

void esp32TcpSend(const char *data)
{
  size_t read = 0;
  char s[30];

  // Manda el comando con la longitud de los datos
  sprintf(s, "AT+CIPSEND=%d", strlen(data));
  esp32ClearReadBuffer();
  debugSerial.println(s);
  esp32Serial.println(s);

  // Espera el prompt y si lo recibe manda los datos
  esp32Serial.setTimeout(10000);
  while (1) {
    read = esp32Serial.readBytesUntil('>', response, RESPONSE_LEN);
    if (read > 0) {
      esp32Serial.print(data);
      esp32WaitForResponses(10000);
      return;
    }
    else if (read == 0) {
      debugSerial.println("Response timeout");
      return;
    }
  }
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  esp32Serial.begin(115200);
  debugSerial.begin(9600);

  delay(3000);

  esp32SendCommand("AT", 1000);
  delay(1000);
  esp32SendCommand("AT", 1000);
  delay(1000);
  esp32SendCommand("AT+RST", 1000);
  esp32WaitForResponses(5000);
  esp32SendCommand("ATE0", 1000);

  esp32SendCommand("AT+CWMODE=1", 1000);
  esp32SendCommand("AT+CWJAP=\"WiFi-ElemonSA-Invitados\",\"Elemon2016\"", 10000);
  esp32SendCommand("AT+CIFSR", 1000);

  esp32SendCommand("AT+CIPCLOSE", 1000);
  esp32SendCommand("AT+CIPMUX=0", 1000);
  
}

void loop() {
  digitalWrite(PIN_LED, LOW);
  if (!digitalRead(PIN_BUTTON)) {
    esp32SendCommand("AT+CIPCLOSE", 10000);
    esp32TcpClientOpen(WEB_SERVER , WEB_PORT);
    esp32TcpSend(REQUEST);
    delay(20000);
  }
  delay(1000);
  digitalWrite(PIN_LED, HIGH);
  delay(1000);
}
