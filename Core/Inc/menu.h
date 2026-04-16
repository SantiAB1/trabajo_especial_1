/*
 * menu.h
 *
 *  Created on: 16 abr 2026
 *      Author: Barcala
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_


typedef enum{
	MENU_S,
	PARAMETRO_S,
	MODO_S,
	MEDIR_S
} estado;

typedef enum{
	BOTON_1,
	BOTON_2,
	PULSADOR
} evento;

typedef struct{
	estado estado;
	uint32_t P;
	uint32_t M;
	uint32_t valor;
} medicion;


void init(medicion *medidor, uint32_t Pinicial, uint32_t Minicial);

void procesarEvento(medicion *medidor, evento);

#endif /* INC_MENU_H_ */
