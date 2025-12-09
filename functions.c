
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include <stdbool.h>
#include "leds.h"

#define toggleFreq 2000 //ms

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

/*void initialization(){
	uint8_t leds[3] = {0b00100100, 0b01001100, 0b01001100};
    ShiftReg_SendBytes(leds, 3);
}*/

bool PL1_Hit(void) //SW5 and SW6
{ if (HAL_GPIO_ReadPin(PL1_Switch_GPIO_Port, PL1_Switch_Pin) == 0) return true;
else return false;
}

bool PL2_Hit(void) //SW7 and SW8
{ if (HAL_GPIO_ReadPin(PL2_Switch_GPIO_Port, PL2_Switch_Pin) == 0) return true;
else return false;
}

/*bool SW1_Hit(void) //SW7 and SW8
{ if (HAL_GPIO_ReadPin(TL1_Car_GPIO_Port, TL1_Car_Pin) == 0) return true;
else return false;
}*/

void set(uint8_t leds[], const defLED *led){
	leds[led->shift_reg] = leds[led->shift_reg] | (1 << led->bits);
	//leds[0] = leds[0] | (1 << 0) = leds[0] | (00000001)
	ShiftReg_SendBytes(leds, 3);
}

void initialization(){
	while(1){
		if(PL1_Hit()){ //if SW5 or SW6 are pressed
			uint8_t leds[3] = {0b0,0b0,0b0};
			HAL_Delay(toggleFreq);
			set(leds, &PL1_Green);
		}
		if(PL2_Hit()){ //if SW7 or SW8 are pressed
			uint8_t leds[3] = {0b0,0b0,0b0};
			HAL_Delay(toggleFreq);
			set(leds, &PL2_Green);
		}
		else{
			uint8_t leds[3] = {0b00100100, 0b01001100, 0b01001100};
			ShiftReg_SendBytes(leds, 3);
		}
	}
}
