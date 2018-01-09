#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>

const int MESSAGE_SECTIONS = 4;
const String PROTOCOL_VERSION = "1";
const String DEVICE_TYPE = "0"; // Controlador
const String ACTION_WRITE = "POST";
const String ACTION_READ = "GET";
const String MESSAGE_DIVIDER = "|";
const byte CONTROLLER_ID[6] = "00000";
const byte DEVICE_ID[6] = "11111";

const int pinCE = 9;
const int pinCSN = 10;
RF24 radio(pinCE, pinCSN);

int counter = 1;
int limit = 10000;
boolean open = false;
 
void setup(void) {
    Serial.begin(9600);
    radio.begin();

    // Start listening
    //radio.openWritingPipe(DEVICE_ID);
    radio.openReadingPipe(2,CONTROLLER_ID);
    //radio.startListening();

    Serial.println(F("XIPE INICIADO"));
}

void loop() {
    writeToDevice("SUPER", DEVICE_ID);
    //readDevices();
    readSerial();
}

void readSerial() {
    if (Serial.available() > 0) {
        String message = Serial.readString();
  
        if (findText(F("OPEN"), message)) {
            Serial.println(F("STORING OPEN STATE"));
            open = true;
        }
    }
}

void readDevices() {
    if (radio.available()) {
        String parsedMessage[5];
        int size = radio.getPayloadSize();

        char data[size];
        radio.read(data, size); 
        radio.writeAckPayload (DEVICE_ID, "OK", 2);

        Serial.print(F("MENSAJE RECIBIDO: "));
        Serial.print(data);
        Serial.print(F("  TAMANO: "));
        Serial.println(size);

        if (parseResponse(data, parsedMessage)) {
            Serial.println(F("VALID MESSAGE RECEIVED"));

            // Getting the bytes from the source message
            byte source[(parsedMessage[2].length() + 1)];
            parsedMessage[2].getBytes(source, (parsedMessage[2].length() + 1));

            Serial.print("SOURCE: ");
            Serial.println(String((char*) source));

            delay(1000);
            writeToDevice("OK:" + parsedMessage[2], source);
        } else {
            Serial.println(F("** ERROR, INVALID MESSAGE RECEIVED"));
        }
    }
}


void writeToDevice(String message, byte target[]) {
    radio.stopListening ();

    delay(3000);
    radio.openWritingPipe(DEVICE_ID);
    delay(1000);

    char paramArray[(message.length() + 1)];
    message.toCharArray(paramArray, (message.length() + 1));

    Serial.print(F("SENDING: "));
    Serial.print(paramArray);
    Serial.print(F("  LENGTH: "));
    Serial.println(strlen(paramArray));

    if (radio.write(paramArray, strlen(paramArray))) {
        Serial.println(F("***** ***** ***** ***** ***** SENT!"));
    } else {
        Serial.println(F("* * NOT SENT!"));
    }

    radio.startListening();
}


boolean parseResponse(char message[], String parsedMessage[]) {
    // Validate if the message format is valid
    if (!isValidMessage(String(message))) {
        return false;
    }

    // Getting the divider char shaped
    char divider = MESSAGE_DIVIDER.charAt(0);

    for (int sectionPosition = 0, messagePosition = 1; 
            sectionPosition < MESSAGE_SECTIONS && messagePosition < strlen(message); 
            sectionPosition++, messagePosition++) {

        // Creating a string to store each section messagge
        String sectionMessage = "";

        while (messagePosition < strlen(message) && message[messagePosition] != divider) {
            // Append the character at the end of the section messae
            sectionMessage += message[messagePosition];
            messagePosition++;
        }

        // Append the message to the parsed string array
        parsedMessage[sectionPosition] = sectionMessage;
    }

    return true;
}

boolean isValidMessage(String message) {
    int dividersCounter = 0;

    // Validate the message is not null
    if (message && message.length() > 2) {

        // Trim the message and validate the structure is valid
        message.trim();

        // Validate the structure by checing start and end charcter
        if (message.endsWith(MESSAGE_DIVIDER) && 
            message.startsWith(MESSAGE_DIVIDER)) {

              // Validate the amount sections in the message
              for (int i = 0 ; i < message.length() ; i++) {
                  if (message.substring(i, (i+1)).equals(MESSAGE_DIVIDER)) {
                      dividersCounter++;
                  }
              }

              if (dividersCounter == (MESSAGE_SECTIONS + 1)) {
                  return true;
              } else {
                  return false;
              }
        } else {
            return false;
        }
    } else {
        return false;
    }
}



int findText(String subStr, String str) {
  int foundpos = -1;

  if (str.length() < 1 || subStr.length() < 1 || str.length() < subStr.length()) {
    return false;
  }

  int index = str.indexOf(subStr.charAt(0));
  
  if (index < 0) {
    return false;
  }

  for (int i = index; i <= str.length() - subStr.length(); i++) {
    if (str.substring(i,subStr.length()+i) == subStr) {
      foundpos = i;
    }
  }

  return foundpos >= 0;
}
