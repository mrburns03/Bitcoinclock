#include <Arduino.h>
#include <ezButton.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h> 
#include "DigitLedDisplay.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

ezButton button(2); // Define button on pin 2
int currentTask = 0; // Initialize current task to 0
unsigned long previousMillis = 0; // Initialize timer for current task
const int taskDuration = 10000; // Set duration for each task to 10 seconds
String blocknumber_string;
int blocknumber_old = 0;
int blocknumber_new = 0;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ESP8266WiFiMulti WiFiMulti;

DigitLedDisplay ld = DigitLedDisplay(12, 15, 14);


// Function for displaying the latest Bitcoin Price USD
void getbitcoinprice() {
  Serial.println("Get Bitcoin Price");
  if (WiFi.status() == WL_CONNECTED) {
  button.loop();
  
  // Draw scrolling text "USD/BTC"
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 15);
  display.println(F("USD/BTC"));
  display.display();      // Show text
  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
     client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd")) {  // you change "=usd" to your prefered currency 

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
          int index1 = payload.indexOf("usd");
          //Serial.println(index1);
          int index2 = payload.indexOf("}");
          //Serial.println(index2);
          String BTCprice = payload.substring((index1+5), (index2));
          Serial.println(BTCprice);

          ld.clear();
           /* Prints data to the display */
          ld.printDigit(BTCprice.toInt());
          
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  display.startscrollleft(0x00, 0x0F);
}

// Function for displaying the number of the latest block
void getblockheight() {
  Serial.println("Getting block tip height");

  if (WiFi.status() == WL_CONNECTED) {
    
  // Draw scrolling text "Current Block Height"
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 15);
  display.println(F("Block Height"));
  display.display();      // Show text
  // Scroll to the right then left, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
     client->setInsecure();

    HTTPClient https;

    blocknumber_old = blocknumber_new;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://mempool.space/api/blocks/tip/height")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          blocknumber_string = https.getString();
          blocknumber_new = blocknumber_string.toInt();
            if (blocknumber_old != blocknumber_new && blocknumber_old != 0){
            display.clearDisplay();
            display.setTextSize(1); // Draw 2X-scale text
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 15);
            display.println(F("Block Found"));
            display.display();      // Show text
            blockanimation();
            }
          Serial.println("blockheight:");
          Serial.println(blocknumber_new); 
          ld.clear();
           /* Prints data to the display */
          ld.printDigit(blocknumber_new);         
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  display.startscrollleft(0x00, 0x0F);
}

// Function for getting the number of blocks until the next bitcoin halving
void getblockstohalving() {
  Serial.println("getting the number of blocks to the next halving");
  if (WiFi.status() == WL_CONNECTED) {
    
  // Draw scrolling text "Blocks to next halving"
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 15);
  display.println(F("Blocks to "));
  display.setCursor(0, 24);
  display.println(F("next halving"));
  display.display();      // Show text
  // Scroll to the right then left, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
     client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://mempool.space/api/blocks/tip/height")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String blocknumber = https.getString();
          float blockstohalving = 210000*(ceil(blocknumber.toFloat()/210000)) - blocknumber.toFloat();
          Serial.println("Number of blocks to halving:");
          Serial.println(blockstohalving); 
          ld.clear();
           /* Prints data to the display */
          ld.printDigit(blockstohalving);        
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  display.startscrollleft(0x00, 0x0F);
}

void setup() {
  // Initialize Serial communication
  Serial.begin(9600);
  button.setDebounceTime(50);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("Find and connect to Wifi network:        Bitcoinclock"));
  display.display();      // Show text
  delay(100);
  /* Set the brightness min:1, max:15 */
  ld.setBright(10);
  /* Set the digit count */
  ld.setDigitLimit(8);
  ld.printDigit(21000000);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFiManager wifiManager;
  wifiManager.resetSettings(); // Clear the saved Wi-Fi credentials
  wifiManager.autoConnect("Bitcoinclock-0001");
  Serial.println("connected to wifi");
  
  display.clearDisplay();
  display.setCursor(10, 0);
  display.println(F("Connected"));
  display.display();      // Show text
  
  for (int i = 0; i <= 3; i++) {
    /* Display off */
    ld.off();
    delay(150);

    /* Display on */
    ld.on();
    delay(150);
  }
  ld.clear();

}

void loop() {
  // Check if button is pressed
  button.loop();
  if (button.isReleased()) {
    // Cycle to next task
    currentTask = (currentTask + 1) % 3;
    // Print current task
    Serial.print("Task ");
    Serial.print(currentTask);
    Serial.println(" selected");
    ld.clear();
    switch (currentTask) {
      case 0:
        getbitcoinprice(); // Call function for getting the current price of Bircoin
        break;
      case 1:
        getblockheight(); // Call function for task 2
        break;
      case 2:
        getblockstohalving(); // Call function for task 3
        break;
    }
    // Reset timer
    previousMillis = millis();
  }

  // Check if it's time to perform current task
  if (millis() - previousMillis >= taskDuration) {
    switch (currentTask) {
      case 0:
        getbitcoinprice(); // Call function for getting the current price of Bircoin
        break;
      case 1:
        getblockheight(); // Call function for task 2
        break;
      case 2:
        getblockstohalving(); // Call function for task 3
        break;
    }
    // Reset timer
    previousMillis = millis();
  }
}

void blockanimation(){

  for (int i = 0; i <= 3; i++) {
    /* Select Digit 5 and write B01100011 */
    ld.write(5, B00011101);
    ld.write(6, B00011101);
    ld.write(7, B00011101);
    ld.write(8, B00011101);
    delay(200);

    /* Select Digit 5 and write B00011101 */
    ld.clear();
    ld.write(6, B00011101);
    ld.write(7, B00011101);
    ld.write(8, B00011101);
    delay(200);
  }

  ld.write(6, B00011101);
  ld.write(7, B00011101);
  ld.write(8, B00011101); 
  ld.write(4, B00011101);
  delay(1000);
  ld.write(4, B00000000);
  ld.write(3, B00011101);
  delay(1000);
  ld.write(3, B00000000);
  ld.write(2, B00011101);
  delay(1000);
  ld.write(2, B00000000);
  ld.write(1, B00011101);
  delay(1000);
  ld.clear();

}
