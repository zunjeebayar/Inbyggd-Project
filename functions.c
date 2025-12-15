
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include "tasks.h"
#include <stdbool.h>
#include <stdio.h>
#include "leds.h"

extern uint8_t leds[3];

// data[] => an array with bytes that will be sent to the shift register
// length = how many bytes the array has
void ShiftReg_SendBytes(uint8_t data[], uint16_t length)
{
    // Skicka N bytes över SPI
    HAL_SPI_Transmit(&hspi3, data, length, HAL_MAX_DELAY);

    // Latch
    HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(STCP_GPIO_Port, STCP_Pin, GPIO_PIN_RESET);
}

bool PL1_Hit(void) //SW5 and SW6
{ if (HAL_GPIO_ReadPin(PL1_Switch_GPIO_Port, PL1_Switch_Pin) == 0) return true;
else return false;
}

bool PL2_Hit(void) //SW7 and SW8
{ if (HAL_GPIO_ReadPin(PL2_Switch_GPIO_Port, PL2_Switch_Pin) == 0) return true;
else return false;
}

bool SW1_Hit(void) //car for traffic light 1
{ if (HAL_GPIO_ReadPin(TL1_Car_GPIO_Port, TL1_Car_Pin) == 0) return true;
else return false;
}

bool SW2_Hit(void) //car for traffic light 2
{ if (HAL_GPIO_ReadPin(TL2_Car_GPIO_Port, TL2_Car_Pin) == 0) return true;
else return false;
}

bool SW3_Hit(void) //car for traffic light 3
{ if (HAL_GPIO_ReadPin(TL3_Car_GPIO_Port, TL3_Car_Pin) == 0) return true;
else return false;
}

bool SW4_Hit(void) //car for traffic light 4
{ if (HAL_GPIO_ReadPin(TL4_Car_GPIO_Port, TL4_Car_Pin) == 0) return true;
else return false;
}

void set(uint8_t leds[], const defLED *led){
	leds[led->shift_reg] = leds[led->shift_reg] | (1 << led->bits);
	//leds[0] = leds[0] | (1 << 0) = leds[0] | (00000001)
	ShiftReg_SendBytes(leds, 3);
}

void reset(uint8_t leds[], const defLED *led){
	leds[led->shift_reg] = leds[led->shift_reg] & ~(1 << led->bits); //~ = bitwise NOT
	ShiftReg_SendBytes(leds, 3);
}

void toggleLED(uint8_t leds[], const defLED *led){
	static bool state = false; //state for the LED

	if(state){
		//Turn off the LED
		reset(leds, led); //~ = bitwise NOT
		HAL_Delay(100);
	} else {
		//Turn on the LED
		set(leds, led);
		HAL_Delay(100);
	}
	state = !state; //flip the state for the next call
}

bool isLED_On(uint8_t leds[], const defLED *led) {
    // Check if the specific LED bit is set (i.e., ON)
    if (leds[led->shift_reg] & (1 << led->bits)) {
        return true; // LED is ON
    } else {
        return false; // LED is OFF
    }
}

bool carIsRed1(void) {
    // Check if any of the car signals are green
    return isLED_On(leds, &TL1_Red) || isLED_On(leds, &TL3_Red);
}

bool carIsGreen1(void) {
    // Check if any of the car signals are green
    return isLED_On(leds, &TL1_Green) || isLED_On(leds, &TL3_Green);
}

bool carIsOrange1(void) {
    // Check if any of the car signals are orange (yellow in your code)
    return isLED_On(leds, &TL1_Yellow) || isLED_On(leds, &TL3_Yellow);
}

bool carIsActive1(void) {
    // Car is active if it is either green or orange
    return carIsGreen1() || carIsOrange1();
}

bool carIsGreen2(void) {
    // Check if any of the car signals are green
    return isLED_On(leds, &TL2_Green) || isLED_On(leds, &TL4_Green);
}

bool carIsOrange2(void) {
    // Check if any of the car signals are orange (yellow in your code)
    return isLED_On(leds, &TL2_Yellow) || isLED_On(leds, &TL4_Yellow);
}

bool carIsActive2(void) {
    // Car is active if it is either green or orange
    return carIsGreen2() || carIsOrange2();
}

void trafficLogic(void){
	if (isLED_On(leds, &TL1_Red) || isLED_On(leds, &TL1_Yellow)) {
		reset(leds, &TL1_Green); //if either red or orange is on, traffic light cannot be green
		reset(leds, &PL1_Green); //pedestrian light cannot be green
	}

	if (isLED_On(leds, &TL2_Red) || isLED_On(leds, &TL2_Yellow)) { //if either red or orange is on, it cannot be green
		reset(leds, &TL2_Green);
		reset(leds, &PL2_Green);
	}

	if (isLED_On(leds, &TL3_Red) || isLED_On(leds, &TL3_Yellow)) { //if either red or orange is on, it cannot be green
		reset(leds, &TL3_Green);
		reset(leds, &PL1_Green);
	}

	if (isLED_On(leds, &TL4_Red) || isLED_On(leds, &TL4_Yellow)) { //if either red or orange is on, it cannot be green
		reset(leds, &TL4_Green);
		reset(leds, &PL2_Green);
	}

}

void pedestrianLogic(void){
	if (carIsActive1()) {
		// Car signal is active (green or orange), keep pedestrian lights red
		set(leds, &PL1_Red);
		reset(leds, &PL1_Green);
	} else {
		// If car signals are red, we can allow pedestrian signal to be green
		set(leds, &PL1_Green);  // Allow pedestrian signal 1 to be green
	}

	if (carIsActive2()) {
		// Car signal is active (green or orange), keep pedestrian lights red
		set(leds, &PL2_Red);
		reset(leds, &PL2_Green);
	} else {
		// If car signals are red, we can allow pedestrian signal to be green
		set(leds, &PL2_Green);  // Allow pedestrian signal 1 to be green
	}
}

void default_state(uint8_t leds[]){

	set(leds, &TL1_Green);
	set(leds, &TL2_Green);
	set(leds, &TL3_Green);
	set(leds, &TL4_Green);

	set(leds, &PL1_Red);
	set(leds, &PL2_Red);

}

void car1_SetGreen(void){ //function to set trafficlight 1 green
    reset(leds, &TL1_Red);
    reset(leds, &TL1_Yellow);
    set(leds, &TL1_Green);

    reset(leds, &TL3_Red);
    reset(leds, &TL3_Yellow);
    set(leds, &TL3_Green);
}

void car1_SetOrange(void){ //function to set trafficlight 1 orange
	reset(leds, &TL1_Green);
	reset(leds, &TL1_Red);
	set(leds, &TL1_Yellow);

	reset(leds, &TL3_Green);
	reset(leds, &TL3_Red);
	set(leds, &TL3_Yellow);
}

void car1_SetRed(void){ //function to set trafficlight 1 red
    reset(leds, &TL1_Green);
    reset(leds, &TL1_Yellow);
    set(leds, &TL1_Red);

    reset(leds, &TL3_Green);
    reset(leds, &TL3_Yellow);
    set(leds, &TL3_Red);
}

void ped1_SetRed(void){ //function to turn on ped1 red light
    set(leds, &PL1_Red);
    reset(leds, &PL1_Green);
    reset(leds, &PL1_Blue);
}

void ped1_SetGreen(void){ //function to turn of ped1 green light
    reset(leds, &PL1_Red);
    set(leds, &PL1_Green);
    reset(leds, &PL1_Blue);
}

void car2_SetGreen(void) { //function to set trafficlight 2 green
    reset(leds, &TL2_Red);
    reset(leds, &TL2_Yellow);
    set(leds, &TL2_Green);

    reset(leds, &TL4_Red);
    reset(leds, &TL4_Yellow);
    set(leds, &TL4_Green);
}

void car2_SetOrange(void){ //function to set trafficlight 2 orange
	reset(leds, &TL2_Green);
	reset(leds, &TL2_Red);
	set(leds, &TL2_Yellow);

	reset(leds, &TL4_Green);
	reset(leds, &TL4_Red);
	set(leds, &TL4_Yellow);
}

void car2_SetRed(void){ //function to set trafficlight 2 red
    reset(leds, &TL2_Green);
    reset(leds, &TL2_Yellow);
    set(leds, &TL2_Red);

    reset(leds, &TL4_Green);
    reset(leds, &TL4_Yellow);
    set(leds, &TL4_Red);
}

void ped2_SetRed(void){ //function to turn on ped2 red light
    set(leds, &PL2_Red);
    reset(leds, &PL2_Green);
    reset(leds, &PL2_Blue);
}

void ped2_SetGreen(void){ //function to turn of ped2 green light
    reset(leds, &PL2_Red);
    set(leds, &PL2_Green);
    reset(leds, &PL2_Blue);
}

void initialization(){

	//uint8_t leds[3] = {0b0,0b0,0b0};

	while(1){

		// Knappar
	    if(SW1_Hit()){
	    	car1_SetRed();
	    	car2_SetOrange();
	    }

		else{
		    leds[0] = 0b00100100;
		    leds[1] =  0b01001100;
		    leds[2] = 0b01001100;
		}

	}
}



