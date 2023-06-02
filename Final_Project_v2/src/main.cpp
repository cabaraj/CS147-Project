#include "functionalities.h"
#include "wireless_connections.h"

uint8_t touched_pin;

#define INITIAL_STATE 0
#define FAN_STATE 1
#define SMOKE_STATE 2
#define TEMP_STATE 3
#define ALARM_STATE 4
#define MAIN_STATE 5
#define ALARM_ACTIVE 6

#define FAN_PIN 15

#define one 8
#define two 4
#define three 1

int current_state = 0;

uint8_t pin_touch;

int ble_value;

void setup() {
  Serial.begin(9600);
  
  function_setup();
  //initiate BLE
  BLE_setup();
  //initiate Wifi
  WiFi_setup();
}

int next_fan_state;
int ble_fan_mode;
float t;
int t_state = 1; //1==F and 2==C
bool password_defined = false;
int _password;
unsigned long activation_time;

void loop(){
  WiFi_loop(); //should i pass the temperature to update it into AWS?
  delay(500);
  detect_smoke();
  pin_touch = pin_touched();
  Serial.println(pin_touch);

  update_temp();

  ble_value = get_value();
  ble_fan_mode = power_fan(ble_value);

  switch(current_state)
  {
    case INITIAL_STATE:
      if(pin_touch == one || pin_touch == two || pin_touch == three)
      {
        current_state = MAIN_STATE;
        //delay(500);
        display_main();
      }
      break;
    case FAN_STATE:
      
      power_fan(next_fan_state);
      current_state = MAIN_STATE;
      break;

    case SMOKE_STATE:
      Serial.println("SMOKE STATE");

      break;
    case TEMP_STATE:
      Serial.println("TEMP STATE");
      if(t_state == 1)
      {
        display_temperature_F();
        if(pin_touch == one)
          t_state = 2;
      }
      else if(t_state == 2)
      {
        display_temperature_C();
        if(pin_touch == one)
          t_state = 1;
      }
      if(pin_touch == two)
      {
        current_state = MAIN_STATE;
        display_main();
      }

      break;
    case ALARM_STATE:
      Serial.println("ALARM STATE");
      if(!password_defined)
      {
        _password = display_set_password(pin_touch);
        if (_password != 0)
          password_defined = true;
          //activation_time = millis() + 5000;
      }
      else
      {
        if(millis() < activation_time)
        {
          display_start_alarm();
          if (pin_touch == three)
          {
            current_state = MAIN_STATE;
            display_main();
          }
        }
        else
        {
          display_alarm_active();
        }
      }


      break;
    case MAIN_STATE:
      Serial.println("MAIN STATE");
      switch(pin_touch)
      {
        case one:
          current_state = TEMP_STATE;
          break;
        case two:
          current_state = FAN_STATE;
          next_fan_state = 1 - ble_fan_mode;
          break;
        case three:
          current_state = ALARM_STATE;
          break;
        default:
          break;
      }
      break;
  }

}
