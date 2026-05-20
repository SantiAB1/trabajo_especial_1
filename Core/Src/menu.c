#include "menu.h"

char confParametro[15];
char confModo[15];
//--------------------------------FUNCIONES---------------------------------
static void medir_R(medicion *medidor);
static void medir_C(medicion *medidor);
static void mostrar_menu();					//Printear el menu en la terminal
static void setHighZ();	//Poner todas los pines de salida para medición en alta impedancia.
static void enableRange(GPIO_TypeDef *port, uint16_t pin);//Elrjir el rango de resistencia
static uint16_t readADC();							//Tomar muestra del ADC
static uint32_t Descarga();
static uint32_t Carga();

static uint32_t rango;
static uint32_t valorADC;

static UART_HandleTypeDef *huart;
static ADC_HandleTypeDef *hadc;
static TIM_HandleTypeDef *htim;

//Función de inicialización
void menu_init(medicion *medidor, uint32_t Pinicial, uint32_t Minicial,
		UART_HandleTypeDef *huart2, ADC_HandleTypeDef *hadc2,
		TIM_HandleTypeDef *htim2) {
	huart = huart2;
	hadc = hadc2;
	htim = htim2;

	medidor->P = Pinicial;
	medidor->M = Minicial;
	medidor->estado = MENU_S;
	medidor->valor = 0;

	char *msgBienvenida = "Bienvenido al epico tester UART!\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgBienvenida, strlen(msgBienvenida),
	HAL_MAX_DELAY);

	sprintf(confParametro, "RESISTENCIA");
	sprintf(confModo, "UNICA");
	mostrar_menu(huart);
}

void menu_procesarEvento(medicion *medidor, evento event) {

	while(event){

	switch (medidor->estado) {

	case MENU_S:// Estando en el menu apretar: 1 para Parametro, 2 para tipo de medicion
		switch (event) {
		case BOTON_1:

			medidor->estado = PARAMETRO_S;
			char *msgParametro =
					"Opcion 1: medir RESISTENCIA\nOpcion 2: medir CAPACITANCIA\r\n\n";
			HAL_UART_Transmit(huart, (uint8_t*) msgParametro,
					strlen(msgParametro), HAL_MAX_DELAY);
			event = EV_NULL;
			break;

		case BOTON_2:

			medidor->estado = MODO_S;
			char *msgModo =
					"Opcion 1: medicion unica\nOpcion 2: medicion periodica (100 ms)\r\n\n";
			HAL_UART_Transmit(huart, (uint8_t*) msgModo, strlen(msgModo),
			HAL_MAX_DELAY);
			event = EV_NULL;
			break;

		case PULSADOR:   // Y pulsador para empezar a medir
			if (medidor->M == 2) {
				HAL_TIM_Base_Start_IT(htim);
			}
			if (medidor->P == 1) {
				//Cambio a estado MEDIR330
				medidor->estado = RANGO_330;
				//Primera rutina de autorango:
				setHighZ();
				enableRange(GPIOR_PORT, GPIO330R);
				valorADC = readADC();
				if (valorADC <= 3981) {			//95% de valor máximo del ADC
					event = EV_ON_RANGE;
				} else {
					event = EV_OUT_OF_RANGE;
				}
			} else {
				medir_C(medidor);
				medidor->estado = MEDIR_C;
				event = EV_NULL;
			}
			break;

		default: event = EV_NULL; break;
		}
		break;

	case PARAMETRO_S:		// 1 para resisencia, 2 para capacidad.
		switch (event) {

		case BOTON_1:
			medidor->estado = MENU_S;
			medidor->P = 1;
			sprintf(confParametro, "RESISTENCIA");
			mostrar_menu(huart);
			event = EV_NULL;
			break;

		case BOTON_2:
			medidor->estado = MENU_S;
			medidor->P = 2;
			sprintf(confParametro, "CAPACITANCIA");
			mostrar_menu(huart);
			event = EV_NULL;
			break;

		case PULSADOR:	// Pulsador para empezar a medir
			if (medidor->M == 2) {
				HAL_TIM_Base_Start_IT(htim);
			}
			if (medidor->P == 1) {
				//Cambio a estado MEDIR330
				medidor->estado = RANGO_330;
				//Primera rutina de autorango:
				setHighZ();
				enableRange(GPIOR_PORT, GPIO330R);
				valorADC = readADC();
				if (valorADC <= 3981) {			//95% de valor máximo del ADC
					event = EV_ON_RANGE;
				} else {
					event = EV_OUT_OF_RANGE;
				}
			} else {
				medir_C(medidor);
				medidor->estado = MEDIR_C;
				event = EV_NULL;
			}
			break;

		default: event = EV_NULL; break;
		}

		break;

	case MODO_S:// Habiendo elegido el tipo de medicion: 1 para única y 2 para periodica
		switch (event) {
		case BOTON_1:
			medidor->estado = MENU_S;
			medidor->M = 1;
			sprintf(confModo, "UNICA");
			mostrar_menu(huart);
			event = EV_NULL;
			break;

		case BOTON_2:
			medidor->estado = MENU_S;
			medidor->M = 2;
			sprintf(confModo, "PERIODICA");
			mostrar_menu(huart);
			event = EV_NULL;
			break;

		case PULSADOR:
			if (medidor->M == 2) {
				HAL_TIM_Base_Start_IT(htim);
			}
			if (medidor->P == 1) {
				//Cambio a estado MEDIR330
				medidor->estado = RANGO_330;
				//Primera rutina de autorango:
				setHighZ();
				enableRange(GPIOR_PORT, GPIO330R);
				valorADC = readADC();
				if (valorADC <= 3981) {			//95% de valor máximo del ADC
					event = EV_ON_RANGE;
				} else {
					event = EV_OUT_OF_RANGE;
				}
			} else {
				medir_C(medidor);
				medidor->estado = MEDIR_C;
				event = EV_NULL;
			}
			break;

		default: event = EV_NULL; break;
		}
		break;

	case RANGO_330:
		switch (event) {
		case EV_OUT_OF_RANGE:
			medidor->estado = RANGO_10K;
			setHighZ();
			enableRange(GPIOR_PORT, GPIO10K);
			valorADC = readADC();
			if (valorADC <= 3981) {			//95% de valor máximo del ADC
				event = EV_ON_RANGE;
			} else {
				event = EV_OUT_OF_RANGE;
			}
			break;
		case EV_ON_RANGE:
			medidor->estado = MEDIR_R;
			rango = 330;
			medir_R(medidor);
			event = EV_NULL;
			break;
		default: event = EV_NULL; break;
		}
		break;

	case RANGO_10K:
		switch (event) {
		case EV_OUT_OF_RANGE:
			medidor->estado = RANGO_1M;
			setHighZ();
			enableRange(GPIOR_PORT, GPIO1M);
			valorADC = readADC();
			if (valorADC <= 3981) {			//95% de valor máximo del ADC
				event = EV_ON_RANGE;
			} else {
				event = EV_OUT_OF_RANGE;
			}
			break;
		case EV_ON_RANGE:
			medidor->estado = MEDIR_R;
			rango = 10000;
			medir_R(medidor);
			event = EV_NULL;
			break;
		default: event = EV_NULL; break;
		}
		break;

	case RANGO_1M:
		switch (event) {
		case EV_OUT_OF_RANGE:
			medidor->estado = OUT_OF_RANGE;
			char *msgEscala = "FUERA DE ESCALA\r\n\n";
			HAL_UART_Transmit(huart, (uint8_t*) msgEscala, strlen(msgEscala), HAL_MAX_DELAY);
			setHighZ();
			event = EV_NULL;
			break;
		case EV_ON_RANGE:
			medidor->estado = MEDIR_R;
			rango = 1000000;
			medir_R(medidor);
			event = EV_NULL;
			break;
		default: event = EV_NULL; break;
		}
		break;

	case OUT_OF_RANGE:
		switch(event){
		case TIMER:
			//Cambio a estado MEDIR330
			medidor->estado = RANGO_330;
			//Primera rutina de autorango:
			setHighZ();
			enableRange(GPIOR_PORT, GPIO330R);
			valorADC = readADC();
			if (valorADC <= 3981) {			//95% de valor máximo del ADC
				event = EV_ON_RANGE;
			} else {
				event = EV_OUT_OF_RANGE;
			}
			break;
		case PULSADOR:
			medidor->estado = MENU_S;
			if (medidor->M == 2) {
				HAL_TIM_Base_Stop_IT(htim);
			}
			mostrar_menu(huart);
			event = EV_NULL;
			break;
		default: event = EV_NULL; break;
		}
		break;

	case MEDIR_R:
		switch (event) {
		case TIMER:
			medir_R(medidor);
			//Cambio a estado MEDIR330
			medidor->estado = RANGO_330;
			//Primera rutina de autorango:
			setHighZ();
			enableRange(GPIOR_PORT, GPIO330R);
			valorADC = readADC();
			if (valorADC <= 3981) {			//95% de valor máximo del ADC
				event = EV_ON_RANGE;
			} else {
				event = EV_OUT_OF_RANGE;
			}
			break;
		case PULSADOR:
			medidor->estado = MENU_S;
			if (medidor->M == 2) {
				HAL_TIM_Base_Stop_IT(htim);
			}
			mostrar_menu(huart);
			setHighZ();
			event = EV_NULL;
			break;
		default: event = EV_NULL; break;
		}
		break;

	case MEDIR_C:
		switch (event) {
		case TIMER:
			medir_C(medidor);
			break;

		case PULSADOR:
			medidor->estado = MENU_S;
			if (medidor->M == 2) {
				HAL_TIM_Base_Stop_IT(htim);
			}
			mostrar_menu(huart);
		default:
			break;
		}
		break;
	}
	}
}

static void mostrar_menu(UART_HandleTypeDef *huart) {
	char msgConf[200];
	sprintf(msgConf,
			"------------------------------------------------\nConfiguracion actual: medicion %s de %s\n------------------------------------------------\r\n",
			confModo, confParametro);
	char *msgMenuP =
			"Opcion 1: Seleccionar parametero\nOpcion 2: Seleccionar modo\nPulsador: MEDIR\r\n\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgConf, strlen(msgConf),
	HAL_MAX_DELAY);
	HAL_UART_Transmit(huart, (uint8_t*) msgMenuP, strlen(msgMenuP),
	HAL_MAX_DELAY);
}

static void medir_R(medicion *medidor) {
	char *msgMEDIR = "Midiendo...\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgMEDIR, strlen(msgMEDIR),
	HAL_MAX_DELAY);

	uint32_t acumulador = 0;

	for (uint32_t i = 0; i < 32; i++) {
		valorADC = readADC();
		acumulador += valorADC;
	}

	medidor->valor = (float) acumulador;
	medidor->valor /= 32;				//Promediar las 32 lecturas

	//Pasar la medida a valores en ohms:
	medidor->valor *= 3.3 / 4096;
	medidor->valor = (medidor->valor * rango) / (3.3 - medidor->valor);
	//devolver el valor medido
	char tx_buffer[64];
	sprintf(tx_buffer, "Valor medido: %.2f Ohms\n\n", medidor->valor);
	HAL_UART_Transmit(huart, (uint8_t*) tx_buffer, strlen(tx_buffer), HAL_MAX_DELAY);
}

static void medir_C(medicion *medidor) {
	char *msgDescarga = "Descargando capacitor\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgDescarga, strlen(msgDescarga), HAL_MAX_DELAY);
	//
	Descarga();

	medidor->valor = Carga();
	//devolver el valor medido

	if (medidor->valor == 0) {
		char *msgError = "FUERA DE ESCALA\r\n";
		HAL_UART_Transmit(huart, (uint8_t*) msgError, strlen(msgError),
		HAL_MAX_DELAY);
		return;
	}
	char tx_buffer[64];
	sprintf(tx_buffer, "Valor medido: %f nF\n\n", medidor->valor);
	HAL_UART_Transmit(huart, (uint8_t*) tx_buffer, strlen(tx_buffer),
	HAL_MAX_DELAY);

}
static void setHighZ() {//Configurar los GPIO en alta impedancia, con el auto-rango se elige que pin poner como salida en alto
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	GPIO_InitStruct.Pin = GPIO330R;
	HAL_GPIO_Init(GPIOR_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO10K;
	HAL_GPIO_Init(GPIOR_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO1M;
	HAL_GPIO_Init(GPIOR_PORT, &GPIO_InitStruct);
}

static void enableRange(GPIO_TypeDef *port, uint16_t pin)	//Rango
{
	GPIO_InitTypeDef GPIO_InitStruct;

	setHighZ();

	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

static uint16_t readADC() { // medir con el adc
	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
	return HAL_ADC_GetValue(hadc);
}

static uint32_t Descarga() { //Configura el pin que se va a usar para DESCARGAR el cap
	GPIO_InitTypeDef GPIO_InitStruct;
	setHighZ();
	GPIO_InitStruct.Pin = GPIO330R;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOR_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOR_PORT, GPIO330R, GPIO_PIN_RESET);
	uint32_t inicio = HAL_GetTick();
	uint32_t TIMEOUT = 5000;
	while ((HAL_GetTick() - inicio) < TIMEOUT) {
		if (readADC(hadc) <= 82) {
			return 1;
		}
	}
	return 0;	// error hizo timeout
}

static uint32_t Carga() {//Configura el pin que se va a usar para CARGAR el cap
	GPIO_InitTypeDef GPIO_InitStruct;
	setHighZ();
	GPIO_InitStruct.Pin = GPIO1M;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOR_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOR_PORT, GPIO1M, GPIO_PIN_RESET);
	uint32_t TIMEOUT = 5000;
	uint32_t contador = 0;
	HAL_GPIO_WritePin(GPIOR_PORT, GPIO1M, GPIO_PIN_SET);
	uint32_t inicio = HAL_GetTick();
	while ((HAL_GetTick() - inicio) < TIMEOUT) {
		if (readADC(hadc) >= 2580) {  	//0.63*4095
			return HAL_GetTick() - inicio;
		}
		contador++;
		if (contador > 72000000) {
			return 0;				// Exceso de cuentas
		}
	}
	return 0; //ERROR timeout
}
