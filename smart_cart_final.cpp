#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <qrcode.h>

//TFT Pins Definintion
#define TFT_CS 10  
#define TFT_RST 8  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC

//Instance of adafruit st7735 class for controlling the tft display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//Instance of Software serial class for serial communication with EM-18 reader
SoftwareSerial mySerial(2, 3);  // RX, TX

//Buttons for add/remove item and qr code generator
const int button_pin1 = 4;
const int button_pin2 = 5;

// specific id for each items
#define rice_id 1
#define sugar_id 2
#define coffee_id 3

//assigning different rfid uid numbers to each items
String rice_tag = "270012EC2DF4";
String sugar_tag = "5900D4EC6A0B";
String coffee_tag = "5900D4D56830";

//array for item list,quantites and price
String item_array[4] = { "", "Rice", "Sugar", "Coffee" };//element at 0 index is given as empty string so that the  counting starts from 1 intead of 0 which is more natural to people
int quantity_array[4] = { 0, 0, 0, 0 };
int price_array[4] = { 0, 100, 50, 150 };

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

int rxcount = 0; //recieved item's count
int selected_item_id = 0;
int total_price = 0;

void setup() {

  Serial.begin(9600);  // Initialize serial communication
  mySerial.begin(9600);
  pinMode(button_pin1, INPUT_PULLUP); 
  pinMode(button_pin2, INPUT_PULLUP);

  inputString.reserve(200); //reserving 200 bits of space in the memory for the inputString value

  /
  init_display();
  delay(5000);
  display_add_item();
  delay(5000);
  display_item_list_heading();
  delay(5000);
  //update_display_item_list();
  Serial.println("Started..");
  //while (1)
  //  ;
}

void loop() {
  myserialEvent();
  if (stringComplete) {
    Serial.println(inputString);
    if (inputString == rice_tag) {
      selected_item_id = rice_id;
    } else if (inputString == sugar_tag) {
      selected_item_id = sugar_id;
    } else if (inputString == coffee_tag) {
      selected_item_id = coffee_id;
    } else {
      selected_item_id = 0;
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
    rxcount = 0;
  }

  if (selected_item_id > 0) {
    Serial.println(item_array[selected_item_id]);
    if (digitalRead(button_pin1) == 0) {
      if (quantity_array[selected_item_id] > 0) {
        quantity_array[selected_item_id]--;
      }
    } else {
      quantity_array[selected_item_id]++;
    }
    update_display_item_list();
    selected_item_id = 0;
  }
  if (digitalRead(button_pin2) == 0) {
    if (total_price > 0) {
      generate_qrcode();
      while(1);
    }
  }
}

void generate_qrcode() {

  tft.fillScreen(ST7735_BLACK);
  String qrText = "You have been purchased for the amount of  \n";
  qrText += "\n\t\tRs.";
  qrText += String(total_price);
  qrText += ".\n";
  qrText += "\n  Please pay the amount.\n";
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(5)];
  qrcode_initText(&qrcode, qrcodeData, 5, 0, qrText.c_str());
  for (int8_t y = 0; y < qrcode.size; y++) {
    for (int8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        tft.fillRect(x * 3, y * 3, 3, 3, ST7735_WHITE);
      }
    }
  }
}

void myserialEvent() {
  while (mySerial.available()) {
    // get the new byte:
    char inChar = (char)mySerial.read();
    //Serial.print(inChar);
    inputString += inChar;
    rxcount++;
    if (inChar == '\n' || rxcount >= 12) {
      stringComplete = true;
    }
  }
}

void init_display() {
  tft.initR(INITR_BLACKTAB);     // Initialize TFT display
  tft.setRotation(1);            // Rotate the display if needed
  tft.fillScreen(ST7735_BLACK);  // Fill the screen with black color
  // Initialize TFT display with introductory text
  int x = 30, y = 15, width = 105, height = 100, border = 2;
  tft.drawRect(x, y, width, height, ST7735_WHITE);                                              // Draw the outer rectangle
  tft.fillRect(x + border, y + border, width - 2 * border, height - 2 * border, ST7735_BLACK);  // Draw the inner rectangle for the border
  tft.setTextSize(3);
  tft.setCursor(41, 30);
  tft.println("SMART");
  tft.setCursor(36, 60);
  tft.setTextSize(2.5);
  tft.println("SHOPPING");
  tft.setCursor(50, 80);
  tft.setTextSize(3);
  tft.println("CART");
}

void display_add_item() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2.5);
  tft.println("Please add");
  tft.println("item..");
}

void display_item_list_heading() {
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  int x_loc = 20;
  int y_loc = 10;
  tft.setCursor(x_loc, y_loc);
  tft.println("Items");
  tft.setCursor(x_loc * 3, y_loc);
  tft.println("Qty");
  tft.setCursor(x_loc * 5, y_loc);
  tft.println("Price");
  tft.drawFastHLine(0, 22, tft.width(), ST7735_WHITE);
}

void update_display_item_list() {
  //tft.fillScreen(ST7735_BLACK);
  int x_loc = 20;
  int y_loc = 25;
  tft.fillRect(x_loc, y_loc, x_loc * 6, y_loc * 5, ST7735_BLACK);
  tft.setTextSize(1);
  total_price = 0;
  for (int i = 0; i <= 3; i++) {
    if (quantity_array[i] > 0) {
      tft.setCursor(x_loc, y_loc * i);
      tft.println(item_array[i]);
      tft.setCursor(x_loc * 3.5, y_loc * i);
      tft.println(String(quantity_array[i]));
      tft.setCursor(x_loc * 5, y_loc * i);
      tft.println(String(quantity_array[i] * price_array[i]));
      total_price += (quantity_array[i] * price_array[i]);
    }
  }
  tft.setTextSize(1.5);
  tft.setCursor(x_loc, y_loc * 4);
  tft.println("Total Price: ");
  tft.setCursor(x_loc * 5, y_loc * 4);
  tft.println(String(total_price));
}
