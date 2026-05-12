#include "menu.h"

uint16_t valorADC;
char confParametro[15];
char confModo[15];

static void medir_R(medicion *medidor, UART_HandleTypeDef *huart, ADC_HandleTypeDef *hadc);
static void mostrar_menu(UART_HandleTypeDef * huart);
static void setHighZ();		//Pone todas los pines de salida para medición en alta impedancia.
static void enableRange(GPIO_TypeDef* port, uint16_t pin);
static uint16_t readADC(ADC_HandleTypeDef * hadc);

void menu_init(medicion *medidor, uint32_t Pinicial, uint32_t Minicial, UART_HandleTypeDef * huart){
	medidor->P = Pinicial;
	medidor->M = Minicial;
	medidor->estado = MENU_S;
	medidor->valor = 0;

	char *msgBienvenida = "Bienvenido al epico tester UART!\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgBienvenida, strlen(msgBienvenida), HAL_MAX_DELAY);

	sprintf(confParametro,"RESISTENCIA");
	sprintf(confModo,"UNICA");
	mostrar_menu(huart);
}

void menu_procesarEvento(medicion *medidor, evento event, UART_HandleTypeDef * huart, ADC_HandleTypeDef * hadc){
	switch(medidor->estado){
	case MENU_S:
		switch(event){
		case BOTON_1:
			medidor->estado = PARAMETRO_S;
			char *msgParametro = "Opcion 1: medir RESISTENCIA\nOpcion 2: medir CAPACITANCIA\r\n\n";
			HAL_UART_Transmit(huart, (uint8_t*) msgParametro, strlen(msgParametro), HAL_MAX_DELAY);
		break;
		case BOTON_2:
			medidor->estado = MODO_S;
			char *msgModo = "Opcion 1: medicion unica\nOpcion 2: medicion periodica (100 ms)\r\n\n";
			HAL_UART_Transmit(huart, (uint8_t*) msgModo, strlen(msgModo), HAL_MAX_DELAY);
		break;
		case PULSADOR:
			medidor->estado = MEDIR_S;
			medir_R(medidor, huart, hadc);
		break;
		default: break;
		}
	break;
	case PARAMETRO_S:
		switch(event){
		case BOTON_1:
			medidor->estado = MENU_S;
			sprintf(confParametro,"RESISTENCIA");
			mostrar_menu(huart);
		break;
		case BOTON_2:
			medidor->estado = MENU_S;
			sprintf(confParametro,"CAPACITANCIA");
			mostrar_menu(huart);
		break;
		case PULSADOR:
			medidor->estado = MEDIR_S;
			medir_R(medidor, huart, hadc);
		break;
		default: break;
		}
	break;
	case MODO_S:
		switch(event){
		case BOTON_1:
			medidor->estado = MENU_S;
			sprintf(confModo,"UNICA");
			mostrar_menu(huart);
		break;
		case BOTON_2:
			medidor->estado = MENU_S;
			sprintf(confModo,"PERIODICA");
			mostrar_menu(huart);
		break;
		case PULSADOR:
			medidor->estado = MEDIR_S;
			medir_R(medidor, huart, hadc);
		break;
		default: break;
		}
	break;

	case MEDIR_S:
		switch(event){
		case PULSADOR:
			medidor->estado = MENU_S;
			mostrar_menu(huart);
		break;
		default: break;
		}
	break;
	}
}

static void mostrar_menu(UART_HandleTypeDef * huart){
	char msgConf[200];
	sprintf(msgConf,"------------------------------------------------\nConfiguracion actual: medicion %s de %s\n------------------------------------------------\r\n",confModo,confParametro);
	char *msgMenuP = "Opcion 1: Seleccionar parametero\nOpcion 2: Seleccionar modo\nPulsador: MEDIR\r\n\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgConf, strlen(msgConf), HAL_MAX_DELAY);
	HAL_UART_Transmit(huart, (uint8_t*) msgMenuP, strlen(msgMenuP), HAL_MAX_DELAY);
}

static void medir_R(medicion *medidor, UART_HandleTypeDef *huart, ADC_HandleTypeDef *hadc){
	char *msgMEDIR = "Midiendo...\r\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgMEDIR, strlen(msgMEDIR), HAL_MAX_DELAY);

	enableRange(GPIOR_PORT, GPIO10K);
	valorADC = readADC(hadc);

	medidor->valor = (float) valorADC;
	medidor->valor *= 3.3 / 4096;
	medidor->valor = (medidor->valor * 1000)/(3.3 - medidor->valor);

	char tx_buffer[64];
	sprintf(tx_buffer, "Valor medido: %.2f\n\n", medidor->valor);
	HAL_UART_Transmit(huart, (uint8_t*) tx_buffer , strlen(tx_buffer), HAL_MAX_DELAY);

}

static void setHighZ(){
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

static void enableRange(GPIO_TypeDef* port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    setHighZ();

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

static uint16_t readADC(ADC_HandleTypeDef * hadc){
	HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    return HAL_ADC_GetValue(hadc);
}
