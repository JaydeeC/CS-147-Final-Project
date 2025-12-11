/**
 Example uses the following configuration:  mxconfig.double_buff = true;
 to enable double buffering, which means display->flipDMABuffer(); is required.

 Bounce squares around the screen, doing the re-drawing in the background back-buffer.

 Double buffering is not usually required. It is only useful when you have a long (in duration)
 drawing routine that you want to 'flip to' once complete without the drawing being visible to 
 the naked eye when looking at the HUB75 panel.

 Please note that double buffering isn't a silver bullet, and may still result in flickering 
 if you end up 'flipping' the buffer quicker than the physical HUB75 refresh output rate. 

 Refer to the runtime debug output to see, i.e:

[  2103][I][ESP32-HUB75-MatrixPanel-I2S-DMA.cpp:85] setupDMA(): [I2S-DMA] Minimum visual refresh rate (scan rate from panel top to bottom) requested: 60 Hz
[  2116][W][ESP32-HUB75-MatrixPanel-I2S-DMA.cpp:105] setupDMA(): [I2S-DMA] lsbMsbTransitionBit of 0 gives 57 Hz refresh rate.
[  2128][W][ESP32-HUB75-MatrixPanel-I2S-DMA.cpp:105] setupDMA(): [I2S-DMA] lsbMsbTransitionBit of 1 gives 110 Hz refresh rate.
[  2139][W][ESP32-HUB75-MatrixPanel-I2S-DMA.cpp:118] setupDMA(): [I2S-DMA] lsbMsbTransitionBit of 1 used to achieve refresh rate of 60 Hz.

**/

//Photoresistor in IO2


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


 #define WIFI_SSID "GET YOUR OWN"
#define WIFI_PASSWORD "linden2003"
 //SharedAccessSignature sr=cs147group69iothub.azure-devices.net%2Fdevices%2F147esp32group69&sig=VbWA9jEaUjlYbULr2N%2FbT4E1RzJzUu5FEMJlmxHZV7c%3D&se=60001764798811
#define SAS_TOKEN "sp=r&st=2025-12-10T22:46:44Z&se=2025-12-14T07:01:44Z&sv=2024-11-04&sr=c&sig=56%2FJ3nu%2B7YQEreXDvBBb40RB2oxExEbRduh%2FE7B1Uhc%3D"
String iothubName = "cs147group69iothub";
String deviceName = "147esp32group69";
String  url = "GET YOUR OWN"+ iothubName + ".azure-devices.net/devices/" + 
deviceName + "/messages/events?api-version=2021-04-12";

const char* root_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n"
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
"MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n"
"MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n"
"UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n"
"eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n"
"GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n"
"mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n"
"SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n"
"dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n"
"AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n"
"VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n"
"buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n"
"AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n"
"b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n"
"Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n"
"oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n"
"dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n"
"AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n"
"klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n"
"kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n"
"eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n"
"penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n"
"QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n"
"q/xKzj3O9hFh/g==\n"
"-----END CERTIFICATE-----\n";

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

// [{"r": 0, "g": 0, "b": 0}, {"r": 36, "g": 0, "b": 18}, {"r": 72, "g": 0, "b": 36}, {"r": 109, "g": 0, "b": 54}, {"r": 145, "g": 0, "b": 72}, {"r": 182, "g": 0, "b": 91}, {"r": 218, "g": 0, "b": 109}, {"r": 255, "g": 0, "b": 127}, {"r": 0, "g": 36, "b": 18}, {"r": 36, "g": 36, "b": 36}, {"r": 72, "g": 36, "b": 54}, {"r": 109, "g": 36, "b": 72}, {"r": 145, "g": 36, "b": 91}, {"r": 182, "g": 36, "b": 109}, {"r": 218, "g": 36, "b": 127}, {"r": 255, "g": 36, "b": 145}, {"r": 0, "g": 72, "b": 36}, {"r": 36, "g": 72, "b": 54}, {"r": 72, "g": 72, "b": 72}, {"r": 109, "g": 72, "b": 91}, {"r": 145, "g": 72, "b": 109}, {"r": 182, "g": 72, "b": 127}, {"r": 218, "g": 72, "b": 145}, {"r": 255, "g": 72, "b": 163}, {"r": 0, "g": 109, "b": 54}, {"r": 36, "g": 109, "b": 72}, {"r": 72, "g": 109, "b": 91}, {"r": 109, "g": 109, "b": 109}, {"r": 145, "g": 109, "b": 127}, {"r": 182, "g": 109, "b": 145}, {"r": 218, "g": 109, "b": 163}, {"r": 255, "g": 109, "b": 182}, {"r": 0, "g": 145, "b": 72}, {"r": 36, "g": 145, "b": 91}, {"r": 72, "g": 145, "b": 109}, {"r": 109, "g": 145, "b": 127}, {"r": 145, "g": 145, "b": 145}, {"r": 182, "g": 145, "b": 163}, {"r": 218, "g": 145, "b": 182}, {"r": 255, "g": 145, "b": 200}, {"r": 0, "g": 182, "b": 91}, {"r": 36, "g": 182, "b": 109}, {"r": 72, "g": 182, "b": 127}, {"r": 109, "g": 182, "b": 145}, {"r": 145, "g": 182, "b": 163}, {"r": 182, "g": 182, "b": 182}, {"r": 218, "g": 182, "b": 200}, {"r": 255, "g": 182, "b": 218}, {"r": 0, "g": 218, "b": 109}, {"r": 36, "g": 218, "b": 127}, {"r": 72, "g": 218, "b": 145}, {"r": 109, "g": 218, "b": 163}, {"r": 145, "g": 218, "b": 182}, {"r": 182, "g": 218, "b": 200}, {"r": 218, "g": 218, "b": 218}, {"r": 255, "g": 218, "b": 236}, {"r": 0, "g": 255, "b": 127}, {"r": 36, "g": 255, "b": 145}, {"r": 72, "g": 255, "b": 163}, {"r": 109, "g": 255, "b": 182}, {"r": 145, "g": 255, "b": 200}, {"r": 182, "g": 255, "b": 218}, {"r": 218, "g": 255, "b": 236}, {"r": 255, "g": 255, "b": 255}];

class Button{
  public:
    Button(int pin) : pin(pin){};

    void begin(){
      pinMode(pin, INPUT_PULLUP);
    }

    bool pressed(){

      bool reading = digitalRead(pin);

      if(reading != lastReading){
        lastChange = millis();
      }

      if(millis() - lastChange > 30){

        if (reading != state){
          state = reading;

          if(state == LOW){
          return true;
          }
        }
      }

      lastReading = reading;
      return false;
    }

  private:
    int pin;
    bool state = HIGH;
    bool lastReading = HIGH;
    uint32_t lastChange = 0;
};

Button grn_btn(YLW_BTN);
Button ylw_btn(GRN_BTN);

void requestNewImage(){
  WiFiClientSecure client;
  client.setCACert(root_ca);
  HTTPClient http;
  //https://cs147group69iothub.azure-devices.net/devices/147esp32group69/messages/events?api-version=2021-04-12
  http.begin(client, "https://cs147gropu69finalproject.blob.core.windows.net/drawings?sp=r&st=2025-12-10T22:46:44Z&se=2025-12-14T07:01:44Z&sv=2024-11-04&sr=c&sig=56%2FJ3nu%2B7YQEreXDvBBb40RB2oxExEbRduh%2FE7B1Uhc%3D");
  http.addHeader("Content-Type","application/json");
  http.addHeader("Authorization",SAS_TOKEN);
  int httpCode = http.GET();
  Serial.println(httpCode);

  if(httpCode == 200){ //200 is a success
    String payload = http.getString();
    Serial.print(payload);
    ArduinoJson::JsonDocument doc;
    deserializeJson(doc, payload);
    //drawImage();
  }
}

void drawImage(int rgb[2048][3], MatrixPanel_I2S_DMA &display){
  int y = 0;
  int colCount = 0;
  for(int i = 0; i < 2048; i++){
    int RED = rgb[i][0];
    int GRN = rgb[i][1];
    int BLU = rgb[i][2];
    display.drawPixel(colCount, y, display.color565(RED, GRN, BLU));
    colCount++;
    if(colCount == WIDTH){
      y++;
      colCount = 0;
    }
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
void loop()
{
  if(digitalRead(YLW_BTN) == LOW){
    //reset screen to black
    Serial.println("yellow button pressed!");
    requestNewImage();
    delay(1000);
  }
  float sensor = analogRead(RESIST);

  static float sensorSmooth = 0;
  sensorSmooth = sensorSmooth * 0.90 + sensor * 0.10;

  int brightness8 = (int)(sensorSmooth * 255.0 / 2000.0);
  if(brightness8 < 5){
    brightness8 = 5;
  }
  display->setBrightness8(brightness8);

  
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

// #include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// MatrixPanel_I2S_DMA *display = nullptr;

// void setup() {
//   Serial.begin(9600);

//   HUB75_I2S_CFG mxconfig;
//   mxconfig.double_buff = false;   // keep disabled for stable testing
//   display = new MatrixPanel_I2S_DMA(mxconfig);

//   display->begin();

//   delay(500);
// }

// void loop() {

//   // RED
//   display->fillScreen(display->color565(255, 0, 0));
//   delay(2000);

//   // GREEN
//   display->fillScreen(display->color565(0, 255, 0));
//   delay(2000);

//   // BLUE
//   display->fillScreen(display->color565(0, 0, 255));
//   delay(2000);
// }
