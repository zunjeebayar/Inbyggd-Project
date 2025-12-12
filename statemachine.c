/*
 * statemachine.c
 *
 *  Created on: 9 Dec 2025
 *      Author: kaur_
 */
/*
 * statemachine.c
 *
 *  Created on: 9 Dec 2025
 *      Author: kaur_
 */
#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "gpio.h"
#include "tasks.h"
#include <stdbool.h>
#include "leds.h"
#include "functions.h"

#define toggleFreq 4 //Hz
#define blink_duration 8000 //ms
#define walkingDelay 5000 //ms, how long ped signal is green
#define pedastrianDelay 5000 //ms, how long after the button is pressed that car signal turns red

uint32_t togglePeriod = 1000 / toggleFreq;

extern uint8_t leds[3];

typedef enum {
	Default,	//default state where ped light is red and traffic light is green
	PedBlink,	//pedastrian button pressed, indicator blinks
	PedGreen	//pedastrian green light on
} states;

void PedestrianCrossing1(void)
{
	//uint8_t leds1[3] = {0,0,0};

    static states State1 = Default;
    static states NextState1 = Default;

	static uint32_t lastToggleTime1 = 0;  // time of last indicator toggle
	static uint32_t startBlinkTime1 = 0;
	static uint32_t startPedGreenTime1 = 0;
	static uint32_t startCarRedTime1 = 0;
	static bool indicatorOn1 = false;

	bool carIsRed = isLED_On(leds, &TL1_Red) && isLED_On(leds, &TL3_Red);
	bool carIsOrange = isLED_On(leds, &TL1_Yellow) && isLED_On(leds, &TL3_Yellow);

		State1 = NextState1;

		switch (State1) // State machine
			{
				case Default:
			    default_state(leds);

				if (PL1_Hit()) { //if SW5 or Sw6 is pressed
					NextState1 = PedBlink;
					startBlinkTime1 = HAL_GetTick();
					lastToggleTime1 = HAL_GetTick(); // start blinking
					startCarRedTime1 = HAL_GetTick();
					indicatorOn1 = false;
				}
				else
					NextState1 = Default; // Stay in Start state
				break;

				case PedBlink:

					if(!carIsRed || !carIsOrange) {
						//car is active - pedestrian must remain red
						reset(leds, &PL1_Green);
						set(leds, &PL1_Red);
					}
					if (HAL_GetTick() - startCarRedTime1 >= pedastrianDelay) {
						//Car signal of active lane
						set(leds, &TL1_Red);
						set(leds, &TL3_Red);
						reset(leds, &TL1_Green);
						reset(leds, &TL3_Green);
					}
					// toggle indicator based on toggleFreq
					if (HAL_GetTick() - lastToggleTime1 >= togglePeriod) {
						indicatorOn1 = !indicatorOn1;

						if (indicatorOn1)
						   set(leds, &PL1_Blue);
						else
						   reset(leds, &PL1_Blue);

						lastToggleTime1 = HAL_GetTick();
					}

		            // After first toggle interval, switch pedestrian light green

					if (HAL_GetTick() - startBlinkTime1 >= blink_duration) {
						reset(leds, &PL1_Blue); // turn off indicator
						set(leds, &PL1_Green);  // pedestrian green

						startPedGreenTime1 = HAL_GetTick();
						NextState1 = PedGreen;
					}

					break;

				case PedGreen:
					set(leds, &PL1_Green);
					reset(leds, &PL1_Blue);
					reset(leds, &PL1_Red);
					set(leds, &TL1_Red);
					set(leds, &TL3_Red);
					reset(leds, &TL1_Green);
					reset(leds, &TL3_Green);

					if (HAL_GetTick() - startPedGreenTime1 >= walkingDelay) {
						reset(leds, &PL1_Green);
						set(leds, &PL1_Red);
						NextState1 = Default;
					}
					break;

				default:
				break;
    }
  }


void PedestrianCrossing2(void)
{
	//uint8_t leds2[3] = {0,0,0};

	 static states State2 = Default;
	 static states NextState2 = Default;

	 static uint32_t lastToggleTime2 = 0;  // time of last indicator toggle
	 static uint32_t startBlinkTime2 = 0;
	 static uint32_t startPedGreenTime2 = 0;
	 static bool indicatorOn2 = false;

		State2 = NextState2;

		switch (State2) // State machine
		{
		   case Default:
			   default_state(leds);

				if (PL2_Hit()) { //if SW7 or Sw8 is pressed
					NextState2 = PedBlink;
					startBlinkTime2 = HAL_GetTick();
					lastToggleTime2 = HAL_GetTick(); // start blinking
					indicatorOn2 = false;
				}
				else
					NextState2 = Default; // Stay in Start state
			break;

		  case PedBlink:
				// toggle indicator based on toggleFreq
				if (HAL_GetTick() - lastToggleTime2 >= togglePeriod) {
				    indicatorOn2 = !indicatorOn2;

				    if (indicatorOn2)
				       set(leds, &PL2_Blue);
				    else
				       reset(leds, &PL2_Blue);

				    lastToggleTime2 = HAL_GetTick();
				}

                // After first toggle interval, switch pedestrian light green
				if (HAL_GetTick() - startBlinkTime2 >= blink_duration) {
				    reset(leds, &PL2_Blue); // turn off indicator
				    set(leds, &PL2_Green);  // pedestrian green

					startPedGreenTime2 = HAL_GetTick();
				    NextState2 = PedGreen;
				}
			break;

		  case PedGreen:

				set(leds, &PL2_Green);
				reset(leds, &PL2_Blue);
				reset(leds, &PL2_Red);

				if (HAL_GetTick() - startPedGreenTime2 >= walkingDelay) {
					reset(leds, &PL2_Green);
					set(leds, &PL2_Red);
					NextState2 = Default;
				}
				break;

		default:
		break;

	}
 }


