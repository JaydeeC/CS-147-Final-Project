

#include "secrets.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <array>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "Wire.h"
#include <ArduinoJson.h>
#include <string>
#include <map>



 //SharedAccessSignature sr=cs147group69iothub.azure-devices.net%2Fdevices%2F147esp32group69&sig=VbWA9jEaUjlYbULr2N%2FbT4E1RzJzUu5FEMJlmxHZV7c%3D&se=60001764798811
#define SAS_TOKEN "sp=r&st=2025-12-10T22:46:44Z&se=2025-12-14T07:01:44Z&sv=2024-11-04&sr=c&sig=56%2FJ3nu%2B7YQEreXDvBBb40RB2oxExEbRduh%2FE7B1Uhc%3D"
String iothubName = "cs147group69iothub";
String deviceName = "147esp32group69";
String url = "https://" + iothubName + ".azure-devices.net/devices/" + 
deviceName + "/messages/events?api-version=2021-04-12";

struct Pixel{
  int r;
  int g;
  int b;
};

#define RESIST 33
#define YLW_BTN 32
#define GRN_BTN 35

#define WIDTH 64
#define HEIGHT 32

MatrixPanel_I2S_DMA *display = nullptr;

constexpr std::size_t color_num = 5;
using colour_arr_t = std::array<uint16_t, color_num>;

uint16_t myDARK, myWHITE, myRED, myGREEN, myBLUE;
colour_arr_t colours;

struct Square
{
  float xpos, ypos;
  float velocityx;
  float velocityy;
  boolean xdir, ydir;
  uint16_t square_size;
  uint16_t colour;
};

const int numSquares = 25;
Square Squares[numSquares];

void drawImage(JsonArray pixels, MatrixPanel_I2S_DMA *display){
  int y = 0;
  int colCount = 0;
  for(int i = 0; i < 2048; i++){
    Serial.print("Drawing... ");
    Serial.println(i);
    JsonArray color = pixels[i];
    int RED = color[0];
    int GRN = color[1];
    int BLU = color[2];
    display->drawPixel(colCount, y, display->color565(RED, GRN, BLU));
    colCount++;
    if(colCount == WIDTH){
      y++;
      colCount = 0;
    }
  }
}

void requestNewImage(MatrixPanel_I2S_DMA* display){
  WiFiClientSecure client;
  client.setCACert(root_ca);
  HTTPClient http;
  http.begin(client, AZURE_URL);
  http.addHeader("Content-Type","application/json");                                
  int httpCode = http.GET();

  if(httpCode == 200){ //200 is a success
    String payload = http.getString();
    ArduinoJson::JsonDocument doc;
    deserializeJson(doc, payload);
    drawImage(doc["pixels"].as<JsonArray>(), display);
  }
}



void setup()
{
  Serial.begin(9600);
  // put your setup code here, to run once:
  Serial.println("Beginning setup...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());


  pinMode(RESIST, INPUT);
  delay(1000);
  delay(200);
  pinMode(YLW_BTN, INPUT_PULLUP);
  pinMode(GRN_BTN, INPUT_PULLUP);

  HUB75_I2S_CFG mxconfig;
  mxconfig.double_buff = true; // <------------- Turn on double buffer
  //mxconfig.clkphase = false; // <------------- Turn off double buffer and it'll look flickery

  // OK, now we can create our matrix object
  display = new MatrixPanel_I2S_DMA(mxconfig);
  display->begin();  // setup display with pins as pre-defined in the library

  myDARK = display->color565(64, 64, 64);
  myWHITE = display->color565(192, 192, 192);
  myRED = display->color565(255, 0, 0);
  myGREEN = display->color565(0, 255, 0);
  myBLUE = display->color565(0, 0, 255);

  colours = {{ myDARK, myWHITE, myRED, myGREEN, myBLUE }};

  // Create some random squares
  for (int i = 0; i < numSquares; i++)
  {
    Squares[i].square_size = random(2,10);
    Squares[i].xpos = random(0, display->width() - Squares[i].square_size);
    Squares[i].ypos = random(0, display->height() - Squares[i].square_size);
    Squares[i].velocityx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    Squares[i].velocityy = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    int random_num = random(6);
    Squares[i].colour = colours[random_num];
  }

  Serial.println("Finished Setup!");
}
int bright = 10;
int count = 0;
int brightness = 0;
bool buttPressed = false;
void loop()
{
  if(digitalRead(YLW_BTN) == LOW){
    //reset screen to black
    Serial.println("yellow button pressed!");
    buttPressed = true;
    display->flipDMABuffer();
    display->clearScreen();
    requestNewImage(display);
    display->flipDMABuffer();
    delay(1000);
    Serial.println("Back in loop()");

    return;
  }
  float sensor = analogRead(RESIST);

  static float sensorSmooth = 0;
  sensorSmooth = sensorSmooth * 0.90 + sensor * 0.10;

  int brightness8 = (int)(sensorSmooth * 255.0 / 2000.0);
  if(brightness8 < 5){
    brightness8 = 5;
  }
  display->setBrightness8(brightness8);

  if(!buttPressed){
    // Flip all future drawPixel calls to write to the back buffer which is NOT being displayed.
    display->flipDMABuffer(); 

    // SUPER IMPORTANT: Wait at least long enough to ensure that a "frame" has been displayed on the LED Matrix Panel before the next flip!
    delay(1000/display->calculated_refresh_rate);  

    // Now clear the back-buffer we are drawing to.
    display->clearScreen();   

    // This is here to demonstrate flicker if double buffering is disabled. Emulates a long draw routine that would typically occur after a 'clearscreen'.
    delay(25);
  

    for (int i = 0; i < numSquares; i++)
    {
      // Draw rect and then calculate
      display->fillRect(Squares[i].xpos, Squares[i].ypos, Squares[i].square_size, Squares[i].square_size, Squares[i].colour);

      if (Squares[i].square_size + Squares[i].xpos >= display->width()) {
        Squares[i].velocityx *= -1;
      } else if (Squares[i].xpos <= 0) {
        Squares[i].velocityx = abs (Squares[i].velocityx);
      }

      if (Squares[i].square_size + Squares[i].ypos >= display->height()) {
        Squares[i].velocityy *= -1;
      } else if (Squares[i].ypos <= 0) {
        Squares[i].velocityy = abs (Squares[i].velocityy);
      }

      Squares[i].xpos += Squares[i].velocityx;
      Squares[i].ypos += Squares[i].velocityy;
    }
  }
}

