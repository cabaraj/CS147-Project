#ifndef FUNCTIONALITIES_H
#define FUNCTIONALITIES_H
//include files to share to main and functionalities.cpp
#include <Arduino.h>
#include <Adafruit_CAP1188.h>
#include <Wire.h>
#include <SPI.h>

#include "DHT20.h"
#include <TFT_eSPI.h>



void function_setup();

void initiate_cap();

uint8_t pin_touched();

//getter
float get_temp_F();

//Temperature
void update_temp();

//Fan
int power_fan(int &);

//Motion Sensor
int get_motion();

//smoke sensor
void detect_smoke();

//Displays
void initiate_display();

void display_main();

void display_fan_mode();

void display_temperature_F();

void display_temperature_C();

int display_set_password(uint8_t);

void display_start_alarm();

void display_alarm_active();

void display_enter_password();

#endif


