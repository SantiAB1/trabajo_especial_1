
#include "menu.h"

void init(medicion *medidor, uint32_t Pinicial, uint32_t Minicial){
	medidor->P = Pinicial;
	medidor->M = Minicial;
	medidor->estado = MENU_S;
	medidor->valor = 0;
}

void procesarEvento(medicion *medidor, evento event){
	switch(medidor->estado){
	case MENU_S:
		//Código estado MENU
	break;
	case PARAMETRO_S:
		//Código estado PARAMETRO
	break;
	case MODO_S:
		//Código estado MODO
	break;
	case MEDIR_S:
		//Código estado MEDIR
	break;
	}
}
