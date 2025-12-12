
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include "tasks.h"
#include <stdbool.h>
#include "leds.h"

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

bool SW1_Hit(void) //SW7 and SW8
{ if (HAL_GPIO_ReadPin(TL1_Car_GPIO_Port, TL1_Car_Pin) == 0) return true;
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

void default_state(uint8_t leds[]){
	set(leds, &TL1_Green);
	set(leds, &TL2_Green);
	set(leds, &TL3_Green);
	set(leds, &TL4_Green);

	set(leds, &PL1_Red);
	set(leds, &PL2_Red);
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

/*bool PL1_Green_IsOn(void)
{
    // Check if the bit for PL1 green LED is set in leds[2]
    if (leds[PL1_Green.shift_reg] & (1 << PL1_Green.bits))
    {
        return true;   // LED is ON
    }
    else
    {
        return false;  // LED is OFF
    }
}*/

void initialization(){

	uint8_t leds[3] = {0b0,0b0,0b0};
	bool ped_request1 = false;
	bool ped_request2 = false;

	while(1){

		// Knappar
		if (PL2_Hit()) {
			reset(leds, &PL1_Red);
		}


		else{
		    leds[0] = 0b00100100;
		    leds[1] =  0b01001100;
		    leds[2] = 0b01001100;

			ShiftReg_SendBytes(leds, 3);
		}
	}
}
