/*
 * car_statemachine.c
 *
 *  Created on: 14 Dec 2025
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

#define orangeDelay 2000 	//ms, how long traffic light stays orange
#define greenDelay 5000 	//ms, how long traffic light stays green
#define redDelayMax 6000 	//ms, how long to wait for car signal to turn green

extern uint8_t leds[3];

typedef enum {
    VerticalGreen,    // SW1 & SW3 GREEN, SW2 & SW4 RED. Vertical lane green, horizontal red
    VerticalOrange,   // SW1 & SW3 ORANGE, SW2 & SW4 RED. Vertical lane orange, then red
    HorizontalOrange, // SW1 & SW3 RED, SW2 & SW4 ORANGE. Horizontal lane orange, then red
    HorizontalGreen   // SW1 & SW3 RED, SW2 & SW4 GREEN. Horizontal lane green, vertical red
} states;

void RoadCrossing(void)
{

    static states State = VerticalGreen;
    static states NextState = VerticalGreen;

    /* Time stamps used for non-blocking timing control */
	static uint32_t startCarGreenTime = 0;	// time when the traffic signal turns green
	static uint32_t startCarRedTime = 0;	// time when the car signal turns red
	static uint32_t startCarOrangeTime = 0;	// time when the car signal turns orange

	/* Car States */
    bool carVertical   = SW1_Hit() || SW3_Hit();
    bool carHorizontal = SW2_Hit() || SW4_Hit();

		State = NextState;

		switch (State) // State machine
			{
				case VerticalGreen:
				car1_SetGreen();	//SW1 & SW3 car signals shall be green
				startCarGreenTime = HAL_GetTick();
				car2_SetRed();		//SW2 & SW4 car signals shall be red
				startCarRedTime = HAL_GetTick();
				//ped1_SetRed();	//all pedestrian signals shall be red
				//ped2_SetRed();

				if(!carHorizontal && !carVertical && (HAL_GetTick() - startCarGreenTime >= greenDelay)) { //R2.4
					/* If there are no active cars in any direction and greenDelay deadline has passed */
					NextState = VerticalOrange;

					/* Initialize timing variables for car transition */
					startCarOrangeTime = HAL_GetTick();
				}

				if(carHorizontal && carVertical && (HAL_GetTick() - startCarRedTime >= redDelayMax)) { //R2.6
					/* If there are active cars in horizontal and vertical lane, it waits redDelayMax until vertical lane changes to green */
					NextState = VerticalOrange;

					/* Initialize timing variables for car transition */
					startCarOrangeTime = HAL_GetTick();
				}

				if (carHorizontal && !carVertical) { //R2.7
					/* If there are active cars in horizontal lane but no active cars in vertical lane */
					NextState = VerticalOrange;

					/* Initialize timing variables for car transition */
					startCarOrangeTime = HAL_GetTick();
				}

				else
					NextState = VerticalGreen; // Stay in Default state
				break;

				case VerticalOrange:

					/* After orangeDelay, switch car signals from green to orange */
					if (HAL_GetTick() - startCarOrangeTime >= orangeDelay) { //turn on orange light
						set(leds, &TL1_Yellow);
						set(leds, &TL3_Yellow);
						reset(leds, &TL1_Green);
						reset(leds, &TL3_Green);

						startCarOrangeTime = HAL_GetTick();
						NextState = HorizontalOrange;
					}

					break;

				case HorizontalOrange:

					if (HAL_GetTick() - startCarOrangeTime >= orangeDelay) { //turn on orange light
						set(leds, &TL2_Yellow);
						set(leds, &TL4_Yellow);
						reset(leds, &TL2_Red);
						reset(leds, &TL4_Red);
					}
					/* Ensure cars remain red after pedestrian request
					if (HAL_GetTick() - startCarRedTime1 >= pedestrianDelay) {
						set(leds, &TL1_Red);
						set(leds, &TL3_Red);
						reset(leds, &TL1_Green);
						reset(leds, &TL3_Green);

						startCarOrangeTime1 = HAL_GetTick();
						NextState1 = CarOrangeToGreen;
					}*/
					break;

				case HorizontalGreen:

					car1_SetRed();

				/*
				// Car stays orange
				reset(leds, &TL1_Red);
				reset(leds, &TL3_Red);
				set(leds, &TL1_Yellow);
				set(leds, &TL3_Yellow);
				reset(leds, &TL1_Green);
				reset(leds, &TL3_Green);

				// After orangeDelay, switch to green
				if (HAL_GetTick() - startCarOrangeTime1 >= orangeDelay) {
					reset(leds, &TL1_Yellow);
					reset(leds, &TL3_Yellow);
					set(leds, &TL1_Green);
					set(leds, &TL3_Green);
					NextState1 = Default;
				}*/
				break;

				default:
					break;
    }
  }




