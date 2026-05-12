#include "menu.h"

uint16_t valorADC;

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

	mostrar_menu(huart);
}

void menu_procesarEvento(medicion *medidor, evento event, UART_HandleTypeDef * huart, ADC_HandleTypeDef * hadc){
	switch(medidor->estado){
	case MENU_S:
		switch(event){
		case BOTON_1:
			medidor->estado = PARAMETRO_S;
		break;
		case BOTON_2:
			medidor->estado = MODO_S;
		break;
		case PULSADOR:
			medidor->estado = MEDIR_S;
			menu_procesarEvento(medidor, MEDIR, huart, hadc);
		break;
		default: break;
		}
	break;
	case PARAMETRO_S:
		//Código estado PARAMETRO
	break;
	case MODO_S:
		//Código estado MODO
	break;

	case MEDIR_S:
		switch(event){
		case MEDIR:
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

static void mostrar_menu(UART_HandleTypeDef * huart){
	char *msgMenuP = "Opcion 1: Seleccionar parametero\nOpcion 2: Seleccionar modo\r\n\n";
	HAL_UART_Transmit(huart, (uint8_t*) msgMenuP, strlen(msgMenuP), HAL_MAX_DELAY);
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
