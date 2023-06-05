/*
 * tcc.c
 *
 *  Created on: 3 de mai de 2023
 *      Author: furst
 */

#include "tcc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "i2c.h"
#include "sx126x.h"

#include "radio.h"
#include "sx126x_board.h"
#include <LoRa.h>

#define LM75_REG_TEMP (0x00) // Temperature Register
#define LM75_REG_CONF (0x01) // Configuration Register

HAL_StatusTypeDef ret;
uint8_t buf[12];
int16_t val;
uint8_t lm75Address = 0x48 << 1;

float temp_c;
float temp_threshold;
float max;
float min;
int sec = 0;

uint32_t startTime;
uint32_t segundos;
uint32_t minutos;
uint32_t horas;

int orgao = 0;
int botaoPressionado = 0;
int started = 0;
int stopped = 0;
int leituras = 0;

void health(){
	printf("Monitoramento iniciando...\n");

	if (HAL_I2C_IsDeviceReady(&hi2c, lm75Address, 10, 100) != HAL_OK)
	{
	  Error_Handler();
	}
	else{
		printf("Termometro esta funcionando\n");
		SX126xCheckDeviceReady();
		printf("RADIO PRONTO\n");
	}
}

void check(){
	if(orgao == 1){
		if(minutos == 0 && segundos == 15)
			ledSet('y');
		else if(minutos == 0 && segundos == 30)
			ledSet('r');
	}
	if(orgao == 2){
		temp_threshold = 23.00;
		if(temp_c/100 > temp_threshold){
			printf("TEMPERATURA EXCEDEU O LIMITE\n");
			ledSet('r');
		}
		if(temp_c/100 == temp_threshold){
			printf("LIMITE DE TEMPERATURA\n");
			ledSet('y');
		}
	}
	if(orgao > 2){
		if(horas == 6)
			ledSet('y');
		else if(horas == 8)
			ledSet('r');

		if(orgao == 3 || orgao == 4 || orgao == 5 || orgao == 7){
			if(temp_c/100 > 8 || temp_c/100 < 2){
				printf("TEMPERATURA EXCEDEU O LIMITE\n");
				ledSet('r');
			}
			if(temp_c/100 == 2.00 || temp_c/100 == 8.00){
				printf("LIMITE DE TEMPERATURA\n");
				ledSet('y');
			}
		}

		if(orgao == 6){
			if(temp_c/100 < 0 || temp_c/100 > 4){
				printf("TEMPERATURA EXCEDEU O LIMITE\n");
				ledSet('r');
			}
			if(temp_c/100 == 0.00 || temp_c/100 == 4.00){
				printf("LIMITE DE TEMPERATURA\n");
				ledSet('y');
			}
		}
	}
}

void temp(){
		if(leituras == 0) max = temp_c;
		if(leituras == 1) min = temp_c;
		leituras = leituras + 1;

	    buf[0] = 0x00;
	    ret = HAL_I2C_Master_Transmit(&hi2c, lm75Address, buf, 1, HAL_MAX_DELAY);
	    if ( ret != HAL_OK ) {
	      printf("Erro TX\n");
	    } else {

	      ret = HAL_I2C_Master_Receive(&hi2c, lm75Address, buf, 2, HAL_MAX_DELAY);
	      if ( ret != HAL_OK ) {
	    	  printf("Erro RX\n");
	      } else {

	        val = ((int16_t)buf[0] << 4) | (buf[1] >> 4);

	        if ( val > 0x7FF ) {
	          val |= 0xF000;
	        }

	        temp_c = val * 0.0625;

	        temp_c *= 100;

	        sprintf((char*)buf,
	              "%u.%u °C",
	              ((unsigned int)temp_c / 100),
	              ((unsigned int)temp_c % 100));

	        printf("[%s]\n", buf);
	      }
	    }
}


void timerStart(){
	startTime = HAL_GetTick();
	segundos = 0;
	minutos = 0;
	horas = 0;
}

void timer(){
	uint32_t elapsedTime = HAL_GetTick() - startTime;
    segundos = elapsedTime / 1000;

    minutos = segundos / 60;
    segundos = segundos % 60;

    horas = minutos / 60;
    minutos = minutos % 60;

    printf("%d horas %d minutos %d segundos\n", horas, minutos, segundos);
}

void led(){
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct_Verde;
	GPIO_InitStruct_Verde.Pin = GPIO_PIN_9;
	GPIO_InitStruct_Verde.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_Verde.Pull = GPIO_NOPULL;
	GPIO_InitStruct_Verde.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Verde);

	GPIO_InitTypeDef GPIO_InitStruct_Amarelo;
	GPIO_InitStruct_Amarelo.Pin = GPIO_PIN_3;
	GPIO_InitStruct_Amarelo.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_Amarelo.Pull = GPIO_NOPULL;
	GPIO_InitStruct_Amarelo.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Amarelo);

	GPIO_InitTypeDef GPIO_InitStruct_Vermelho;
	GPIO_InitStruct_Vermelho.Pin = GPIO_PIN_1;
	GPIO_InitStruct_Vermelho.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_Vermelho.Pull = GPIO_NOPULL;
	GPIO_InitStruct_Vermelho.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Vermelho);
}

void button(){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ledReset(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
}

void ledSet(char cor){
	ledReset();
	if (cor=='r')
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
	else if (cor=='g'){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	}
	else if(cor=='y'){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
	}
}


void startButton(){
	while(1){
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET && !botaoPressionado){
			botaoPressionado = 1;
			orgao = orgao + 1;
			if (orgao > 7 )
				orgao = 1;

			if (orgao == 1)
				printf("Voce selecionou o DEMO TEMPO\n");
			else if (orgao == 2)
				printf("Voce selecionou o DEMO TEMPERATURA\n");
			else if (orgao == 3)
				printf("Voce selecionou o Coracao\n");
			else if (orgao == 4)
				printf("Voce selecionou o Pulmao\n");
			else if (orgao == 5)
				printf("Voce selecionou o Figado\n");
			else if (orgao == 6)
				printf("Voce selecionou o Rim\n");
			else if (orgao == 7)
				printf("Voce selecionou o Pancreas\n");
		}

		else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET  && orgao == 0 && !botaoPressionado){
			botaoPressionado = 1;
			printf("Voce nao selecionou nenhum orgao\n");
		}
		else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET  && orgao > 0){
			botaoPressionado = 1;
			ledSet('g');
			started = 1;
			//sendLoraFrame();
			break;
		}

	    else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET && botaoPressionado)	    {
	    	botaoPressionado = 0;
	    }

		HAL_Delay(10);
	}
}

void sendLoraFrame(){
	if(segundos % 10 == 0){
    sprintf((char*)buf,
          "%u.%u °C",
          ((unsigned int)temp_c / 100),
          ((unsigned int)temp_c % 100));


	SX126xSendPayload(buf, sizeof(buf), 5);
	printf("%d",LoRaMacIsBusy);


	}
}

void reportTemp(){
	if (started == 1) {
		if(temp_c/100 > max/100) max = temp_c;
		if(temp_c/100 < min/100 ) min = temp_c;

		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET  && orgao > 0 && leituras > 1){
			printf("%d leituras realizadas\n", leituras);

	    	sprintf((char*)buf,
	    		"%u.%u °C",
	          	  ((unsigned int)max / 100),
	          	  ((unsigned int)max % 100));
	    	printf("Maior tempratura registrada: [%s]\n", buf);

	    	sprintf((char*)buf,
	    		"%u.%u °C",
	          	  ((unsigned int)min / 100),
	          	  ((unsigned int)min % 100));
	    	printf("Menor tempratura registrada: [%s]\n", buf);

	    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);

	    	stopped = 1;
		}
	}
}

int returnStatus(){
	return stopped;
}

void UART_SendString(char* str) {
	//UART_HandleTypeDef huart1;
	HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}
