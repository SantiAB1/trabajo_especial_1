#include "menu.h"

char confParametro[15];
char confModo[15];
//--------------------------------FUNCIONES---------------------------------
static void medir_R(medicion *medidor);
static void mostrar_menu(UART_HandleTypeDef *huart);					//Printear el menu en la terminal
static void setHighZ();	//Poner todas los pines de salida para medición en alta impedancia.
static void enableRange(GPIO_TypeDef *port, uint16_t pin);//Elrjir el rango de resistencia
static void Descarga();
static void Carga();
static uint16_t readADC();							//Tomar muestra del ADC

//variables globales
static uint32_t rango;
static uint32_t valorADC;
//contadores
static uint32_t t;
static uint32_t u;
//flags
static uint32_t on_range;
static uint32_t desc_fin;
static uint32_t charge_fin;
//periféricos
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

	switch (medidor->estado) {

	case MENU_S:// Estando en el menu apretar: 1 para Parametro, 2 para tipo de medicion
		switch (event) {
		case BOTON_1:
			medidor->estado = PARAMETRO_S;
			char *msgParametro =
					"Opcion 1: medir RESISTENCIA\nOpcion 2: medir CAPACITANCIA\r\n\n";
			HAL_UART_Transmit(huart, (uint8_t*) msgParametro,
					strlen(msgParametro), HAL_MAX_DELAY);
			break;

		case BOTON_2:

			medidor->estado = MODO_S;
			char *msgModo =
					"Opcion 1: medicion unica\nOpcion 2: medicion periodica (100 ms)\r\n\n";
			HAL_UART_Transmit(huart, (uint8_t*) msgModo, strlen(msgModo),
			HAL_MAX_DELAY);
			break;

		case PULSADOR:   // Y pulsador para empezar a medir
			if (medidor->P == 1) {
				//Cambio a estado MEDIR330
				medidor->estado = RANGO_330;
				t = 0;
				//Primera rutina de autorango:
				setHighZ();
				enableRange(GPIOR_PORT, GPIO330R);
				valorADC = readADC();
				if (valorADC <= 3900) {			//95% de valor máximo del ADC
					on_range = 1;
				} else {
					on_range = 0;
				}
			} else {
				medidor->estado = DESCARGA;
				t = 0;				// Realizar la descarga y comprobar la flag
				desc_fin = 0;
				Descarga();
			}
			break;

		default: break;
		}
		break;

	case PARAMETRO_S:		// 1 para resisencia, 2 para capacidad.
		switch (event) {

		case BOTON_1:
			medidor->estado = MENU_S;
			medidor->P = 1;
			sprintf(confParametro, "RESISTENCIA");
			mostrar_menu(huart);
			break;

		case BOTON_2:
			medidor->estado = MENU_S;
			medidor->P = 2;
			sprintf(confParametro, "CAPACITANCIA");
			mostrar_menu(huart);
			break;

		case PULSADOR:	// Pulsador para empezar a medir
			if (medidor->P == 1) {
				//Cambio a estado MEDIR330
				medidor->estado = RANGO_330;
				t = 0;
				//Primera rutina de autorango:
				setHighZ();
				enableRange(GPIOR_PORT, GPIO330R);
				valorADC = readADC();
				if (valorADC <= 3900) {			//95% de valor máximo del ADC
					on_range = 1;
				} else {
					on_range = 0;
				}
			} else {
				medidor->estado = DESCARGA;
				t = 0;
				desc_fin = 0;
				Descarga();
			}
			break;

		default: break;
		}

		break;

	case MODO_S:// Habiendo elegido el tipo de medicion: 1 para única y 2 para periodica
		switch (event) {
		case BOTON_1:
			medidor->estado = MENU_S;
			medidor->M = 1;
			sprintf(confModo, "UNICA");
			mostrar_menu(huart);
			break;

		case BOTON_2:
			medidor->estado = MENU_S;
			medidor->M = 2;
			sprintf(confModo, "PERIODICA");
			mostrar_menu(huart);
			break;

		case PULSADOR:
			if (medidor->P == 1) {
				//Cambio a estado MEDIR330
				medidor->estado = RANGO_330;
				t = 0;
				//Primera rutina de autorango:
				setHighZ();
				enableRange(GPIOR_PORT, GPIO330R);
				valorADC = readADC();
				if (valorADC <= 3900) {			//95% de valor máximo del ADC
					on_range = 1;
				} else {
					on_range = 0;
				}
			} else {
				medidor->estado = DESCARGA;
				t = 0;
				desc_fin = 0;
				Descarga();
			}
			break;

		default: break;
		}
		break;

	case RANGO_330:
		switch (event) {
		case TICK:
			if(on_range){
				medidor->estado = MEDIR_R;
				rango = 330;
				medir_R(medidor);
			} else{
				medidor->estado = RANGO_10K;
				setHighZ();
				enableRange(GPIOR_PORT, GPIO10K);
				valorADC = readADC();
				if (valorADC <= 3900) {			//95% de valor máximo del ADC
					on_range = 1;
				} else {
					on_range = 0;
				}
			}
			break;
		default: break;
		}
		break;

	case RANGO_10K:
		switch (event) {
		case TICK:
			if(on_range){
				medidor->estado = MEDIR_R;
				rango = 10000;
				medir_R(medidor);
			} else{
				medidor->estado = RANGO_1M;
				setHighZ();
				enableRange(GPIOR_PORT, GPIO1M);
				valorADC = readADC();
				if (valorADC <= 3900) {			//95% de valor máximo del ADC
					on_range = 1;
				} else {
					on_range = 0;
				}
			}
			break;
		default: break;
		}
		break;

	case RANGO_1M:
		switch (event) {
		case TICK:
			if(on_range){
				medidor->estado = MEDIR_R;
				rango = 1000000;
				medir_R(medidor);
			} else{
				medidor->estado = OUT_OF_RANGE;
				char *msgError = "FUERA DE ESCALA\r\n\n";
				HAL_UART_Transmit(huart, (uint8_t*) msgError, strlen(msgError),	HAL_MAX_DELAY);
				setHighZ();
			}
			break;
		default: break;
		}
		break;

	case OUT_OF_RANGE:
		switch(event){
		case TICK:
			if(t < 1000){
				t++;
			} else if(medidor->M == 2){
				t = 0;
				if(medidor->P == 1){
					//Cambio a estado MEDIR330
					medidor->estado = RANGO_330;
					//Primera rutina de autorango:
					setHighZ();
					enableRange(GPIOR_PORT, GPIO330R);
					valorADC = readADC();
					if (valorADC <= 3900) {			//95% de valor máximo del ADC
						on_range = 1;
					} else {
						on_range = 0;
					}
				} else{
					medidor->estado = DESCARGA;
					Descarga();
					desc_fin = 0;
				}
			}
			break;
		case PULSADOR:
			medidor->estado = MENU_S;
			setHighZ();
			mostrar_menu(huart);
			break;
		default: break;
		}
		break;

	case DESCARGA:
		switch(event){
		case TICK:
			if (readADC() <= 82)
				desc_fin = 1;

			if(desc_fin){
				medidor->estado = CARGA;
				u = 0;
				charge_fin = 0;
				Carga();
			} else if (t < 5000){
				t++;
			} else{
				medidor->estado = TIMEOUT;
				setHighZ();
				char *msgError = "ERROR: Timeout en la descarga. Presiona el PULSADOR para continuar.\r\n\n";
				HAL_UART_Transmit(huart, (uint8_t *) msgError, strlen(msgError), HAL_MAX_DELAY);
			}
			break;

		case PULSADOR:
			medidor->estado = MENU_S;
			char *msgCancel = "Medida cancelada.\r\n\n";
			setHighZ();
			HAL_UART_Transmit(huart, (uint8_t *) msgCancel, strlen(msgCancel), HAL_MAX_DELAY);
			mostrar_menu(huart);
			break;

		default: break;
		}
		break;

	case CARGA:
		switch(event){
		case TICK:
			if (readADC(hadc) >= 2580) {  	//0.63*4095
				medidor->estado = MEDIR_C;
				medidor->valor = u;
				char tx_buffer[64];
				sprintf(tx_buffer, "Valor medido: %lu nF\n\n", u);
				HAL_UART_Transmit(huart, (uint8_t*) tx_buffer, strlen(tx_buffer), HAL_MAX_DELAY);
			} else if (u < 1000){
				t++;
				u++;
			} else{
				medidor->estado = OUT_OF_RANGE;
				char *msgError = "FUERA DE ESCALA\r\n\n";
				HAL_UART_Transmit(huart, (uint8_t*) msgError, strlen(msgError),	HAL_MAX_DELAY);
				setHighZ();
			}
			break;

		case PULSADOR:
			medidor->estado = MENU_S;
			char *msgCancel = "Medida cancelada.\r\n\n";
			setHighZ();
			HAL_UART_Transmit(huart, (uint8_t *) msgCancel, strlen(msgCancel), HAL_MAX_DELAY);
			mostrar_menu(huart);
			break;
		default: break;
		}
		break;

	case TIMEOUT:
		if(event == PULSADOR){
			medidor->estado = MENU_S;
			mostrar_menu(huart);
		}
		break;

	case MEDIR_R:
		switch (event) {
		case TICK:
			if(t < 1000){
				t++;
			} else if(medidor->M == 2){
				medidor->estado = RANGO_330;
				t = 0;
				//Primera rutina de autorango:
				setHighZ();
				enableRange(GPIOR_PORT, GPIO330R);
				valorADC = readADC();
				if (valorADC <= 3900) {			//95% de valor máximo del ADC
					on_range = 1;
				} else {
					on_range = 0;
				}
			}
			break;
		case PULSADOR:
			medidor->estado = MENU_S;
			mostrar_menu(huart);
			break;
		default: break;
		}
		break;

	case MEDIR_C:
		switch (event){
		case TICK:
			if(t < 1000){
				t++;
			} else if(medidor->M == 2){
				medidor->estado = DESCARGA;
				t = 0;				// Realizar la descarga y comprobar la flag
				desc_fin = 0;
				Descarga();
			}
			break;
		case PULSADOR:
			medidor->estado = MENU_S;
			mostrar_menu(huart);
			break;
		default: break;
		}
		break;
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

	char *msgMidiendo = "Midiendo...\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgMidiendo, strlen(msgMidiendo), HAL_MAX_DELAY);

	uint32_t acumulador = 0;

	for (uint32_t i = 0; i < 32; i++) {
		valorADC = readADC();
		acumulador += valorADC;
	}

	medidor->valor = (float) acumulador;
	medidor->valor /= 32;				//Promediar las 32 lecturas

	//Pasar la medida a valores en ohms:
	medidor->valor *= 3.3 / 4020;
	medidor->valor = (medidor->valor * rango) / (3.3 - medidor->valor);
	//unidad
	char tx_buffer[64];
	if(medidor->valor < 1000){
	sprintf(tx_buffer, "Valor medido: %.2f Ohms\n\n", medidor->valor);
	HAL_UART_Transmit(huart, (uint8_t*) tx_buffer, strlen(tx_buffer), HAL_MAX_DELAY);
	} else if(medidor->valor < 1000000){
		medidor->valor/=1000;
		sprintf(tx_buffer, "Valor medido: %.2f KOhms\n\n", medidor->valor);
		HAL_UART_Transmit(huart, (uint8_t*) tx_buffer, strlen(tx_buffer), HAL_MAX_DELAY);
	} else{
		medidor->valor /= 1000000;
		sprintf(tx_buffer, "Valor medido: %.2f MOhms\n\n", medidor->valor);
		HAL_UART_Transmit(huart, (uint8_t*) tx_buffer, strlen(tx_buffer), HAL_MAX_DELAY);
	}
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

static void Descarga() { //Configura el pin que se va a usar para DESCARGAR el cap

	char *msgDesc = "Descargando capacitor...\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgDesc, strlen(msgDesc), HAL_MAX_DELAY);

	GPIO_InitTypeDef GPIO_InitStruct;
	setHighZ();
	GPIO_InitStruct.Pin = GPIO330R;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOR_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOR_PORT, GPIO330R, GPIO_PIN_RESET);
}

static void Carga() {//Configura el pin que se va a usar para CARGAR el cap
	char *msgMidiendo = "Midiendo...\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgMidiendo, strlen(msgMidiendo), HAL_MAX_DELAY);

	GPIO_InitTypeDef GPIO_InitStruct;
	setHighZ();
	GPIO_InitStruct.Pin = GPIO1M;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOR_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOR_PORT, GPIO1M, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOR_PORT, GPIO1M, GPIO_PIN_SET);
}
