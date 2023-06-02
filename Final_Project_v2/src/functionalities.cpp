#include "functionalities.h"
#include "wireless_connections.h"

//define pins
#define FAN_PIN 15
#define MOTION_PIN 12
#define SMOKE_PIN 2
#define BUZZER_PIN 13

float TEMP_THRESHOLD;

//for pin touch
#define one 8
#define two 4
#define three 1

//display
TFT_eSPI tft = TFT_eSPI(); 

// I2C
#define CAP1188_MISO  21
#define CAP1188_CLK  22
#define CAP1188_SENSITIVITY 0x1F
Adafruit_CAP1188 cap = Adafruit_CAP1188();

//temperature sensor
DHT20 dht;

//global variables
int fan_state = 0;
float temp_F;
float temp_C;

void function_setup()
{
    initiate_display();
    initiate_cap();
    dht.begin();
    //pinMode(MOTION_PIN, INPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(FAN_PIN, HIGH);
    update_temp();

    TEMP_THRESHOLD = temp_C + 1;
}

// For Touch Sensor
void initiate_cap()
{
    if (!cap.begin()) {
    Serial.println("CAP1188 not found");
    while (1);
  }
  Serial.println("CAP1188 found!");

  //Decrease sensitivity a little - default is 0x2F (32x) per datasheet
  //cap.writeRegister(CAP1188_SENSITIVITY, 0x3F);  // 16x sensitivity
  //cap.writeRegister(CAP1188_SENSITIVITY, 0x4F);  // 8x  sensitivity
  //cap.writeRegister(CAP1188_SENSITIVITY, 0x5F);  // 4x  sensitivity
  cap.writeRegister(CAP1188_SENSITIVITY, 0x6F);  // 2x  sensitivity THIS SEEMS TO WORK THE BEST FOR 3.5" plate sensors
  //cap.writeRegister(CAP1188_SENSITIVITY, 0x7F);  // 1x  sensitivity
  //Serial.print("Sensitivity: 0x");
  //Serial.println(cap.readRegister(CAP1188_SENSITIVITY), HEX);
}

uint8_t pin_touched()
{
    /*detect what pin is being touched (1-8) and return it
    return 0 is none is touched*/
    
    uint8_t touched = cap.touched();
    //delay to avoid false touch
    delay(250);

    if (touched == 0) {
        // No touch detected
        return 0;
    }
  
    for (uint8_t i=0; i<8; i++) {
        if (touched & (1 << i)) {
        //Serial.print("C"); Serial.print(i+1); Serial.print("\t");
        return i+1;
        }
    }

    return 0;
}

// *********************
//       Getters
// *********************
float get_temp_F()
{
    return temp_F;
}

// ***********************
//     For Temperature
// ***********************

void update_temp()
{
    int status = dht.read();

    if (status == DHT20_OK) {  // valid read
        temp_C = dht.getTemperature();
        temp_F = dht.getTemperatureF();
    }
}

// *********************
//          Fan
// *********************

bool was_fan_set_off = false;

int power_fan(int &value)
{
    if(value == 1 && fan_state == 0){
        was_fan_set_off = false;
        fan_state = 1;
        display_fan_mode();
        digitalWrite(FAN_PIN, LOW);
        display_main();
    }
    else if(value == 0 && fan_state == 1){
        was_fan_set_off = true;
        fan_state = 0;
        display_fan_mode();
        digitalWrite(FAN_PIN, HIGH);
        display_main();
        
    }
    else if (temp_C > TEMP_THRESHOLD && !was_fan_set_off && fan_state == 0)
    {
        fan_state = 1;
        display_fan_mode();
        digitalWrite(FAN_PIN, LOW);
        display_main();        
    }
    return fan_state;
}

// *********************
//     Motion Sensor
// *********************

int get_motion()
{
    return digitalRead(MOTION_PIN);
}

// *********************
//     Smoke Sensor
// *********************

void detect_smoke()
{
    int sensor_value = digitalRead(SMOKE_PIN);
    Serial.print("smoke value: ");
    Serial.println(sensor_value);
    if (sensor_value == 0)
    {
        tone(BUZZER_PIN, 300);
        delay(250);
        noTone(BUZZER_PIN);
    }
}
// *********************
//      Displays
// *********************

void initiate_display() {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    int y_pos = 0;
    tft.drawString("Ambient",tft.width()/2 - 50,y_pos);
    y_pos += tft.fontHeight();
    tft.drawString("Home   Assistant",25,y_pos);
    y_pos += tft.fontHeight();
    y_pos += tft.drawFloat(2.0,1,tft.width()/2-35, y_pos + 10,6);
    tft.drawString("press any key to continue",40,y_pos-10,2);
    delay(1000);
}

void display_main()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    int y_pos = 0;
    tft.drawString("1) Display Temp.", 0, y_pos);
    y_pos += 2*tft.fontHeight();
    tft.drawString("2) Turn Fan On/Off", 0, y_pos);
    y_pos += 2*tft.fontHeight();
    tft.drawString("3) Set Alarm", 0, y_pos);
    y_pos += 2*tft.fontHeight();
}

void display_fan_mode()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    int y_pos = 1.5*tft.fontHeight();
    if(1 == fan_state)
    {
        tft.drawString("Turning Fan ON", 20, y_pos);
    }
    else
    {
        tft.drawString("Turning Fan OFF", 20, y_pos);

    }
    y_pos += 2*tft.fontHeight();
    tft.drawString(" * ", 50, y_pos);
    delay(750);
    tft.drawString(" * ", 100, y_pos);
    delay(750);
    tft.drawString(" * ", 150, y_pos);
    delay(750);
}

void display_temperature_F()
{
    if (temp_F >= 0)
    {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextFont(4);
        int y_pos = 0;
        int x_pos = 0;
        tft.drawString("Current Temperature", x_pos, y_pos);
        y_pos += 2*tft.fontHeight();
        tft.setTextFont(6);
        x_pos = 50;
        tft.drawFloat(temp_F,1,x_pos, y_pos);
        tft.drawString("o", 160, y_pos,2);
        tft.drawString("F", 175, y_pos, 4);
        y_pos += tft.fontHeight();
        tft.setTextFont(2);
        tft.drawString("[1] C              [2] Exit", 25, y_pos+20);
    }
}

void display_temperature_C()
{
   if (temp_C >= 0)
    {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextFont(4);
        int y_pos = 0;
        int x_pos = 0;
        tft.drawString("Current Temperature", x_pos, y_pos);
        y_pos += 2*tft.fontHeight();
        tft.setTextFont(6);
        x_pos = 50;
        tft.drawFloat(temp_C,1,x_pos, y_pos);
        tft.drawString("o", 160, y_pos,2);
        tft.drawString("C", 175, y_pos, 4);
        y_pos += tft.fontHeight();
        tft.setTextFont(2);
        tft.drawString("[1] F              [2] Exit", 25, y_pos+20);
    }
}

int password;
int len = 0;

int display_set_password(uint8_t pin_touch)
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    int y_pos = 0;
    int x_pos = 0;
    tft.drawString("Create Password:", x_pos, y_pos);
    y_pos += 2*tft.fontHeight();
    tft.setTextFont(6);

    if(pin_touch == one || pin_touch == two || pin_touch == three)
    {
        int i;
        if (pin_touch == one)
            i = 1;
        else if(pin_touch == two)
            i = 2;
        else
            i = 3;
        switch(len)
        {
            case 0:
                password = i*100;
                len +=1;
                tft.drawFloat(i, 0, 50, y_pos);
                break;
            case 1:
                password += i*10;
                len +=1;
                tft.drawFloat(i, 0, 50, y_pos);
                break;
            case 2:
                password += i;
                tft.drawFloat(i, 0, 50, y_pos);
                return password;
                break;
            default:
                break;
        }
    }
    else
    {
        if(len == 1)
            tft.drawString("*", 30, y_pos,4);
        else if(len == 2)
            tft.drawString("* *", 30, y_pos, 4);
    }
    return 0;
}

void display_start_alarm()
{  
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    int y_pos = 30;
    tft.drawString("Activating", 50, y_pos);
    y_pos += 2*tft.fontHeight();
    tft.drawString("Alarm", 70, y_pos);
    y_pos += 2*tft.fontHeight();
    tft.drawString("[3] CANCELL", 50, y_pos,2);
}

void display_alarm_active()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    int y_pos = 60;
    tft.drawString("ACTIVATED!", 50, y_pos);
    y_pos += 2*tft.fontHeight();
    tft.drawString("[1] deactivate", 40, y_pos,2);
}

void display_enter_password()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(4);
    int y_pos = 20;
    tft.drawString("Enter Password:", 70, y_pos);
}