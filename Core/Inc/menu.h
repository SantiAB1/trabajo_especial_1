/*
 * menu.h
 *
 *  Created on: 16 abr 2026
 *      Author: Barcala
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#define GPIO1M GPIO_PIN_12
#define GPIO10K GPIO_PIN_13
#define GPIO330R GPIO_PIN_14
#define GPIOR_PORT GPIOB

#include <stdint.h>
#include <stdio.h>
#include "stm32f1xx_hal.h"
#include <string.h>

typedef enum{
	MENU_S,
	PARAMETRO_S,
	MODO_S,
	MEDIR_R,
	MEDIR_C
} estado;

typedef enum{
	BOTON_1,
	BOTON_2,
	PULSADOR,
	TIMER
} evento;

typedef struct{
	estado estado;
	uint32_t P;
	uint32_t M;
	float valor;
} medicion;


void menu_init(medicion *medidor, uint32_t Pinicial, uint32_t Minicial, UART_HandleTypeDef * huart);
void menu_procesarEvento(medicion *medidor, evento, UART_HandleTypeDef * huart, ADC_HandleTypeDef * hadc, TIM_HandleTypeDef * htim);
#endif /* INC_MENU_H_ */
