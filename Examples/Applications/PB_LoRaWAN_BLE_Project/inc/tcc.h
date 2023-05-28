/*
 * tcc.h
 *
 *  Created on: 3 de mai de 2023
 *      Author: furst
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef INC_TCC_H_
#define INC_TCC_H_

void health(void);
void temp(void);
void timer(void);
void timerStart(void);
void led(void);
void ledSet(char c);
void ledReset(void);
void button(void);
void startButton(void);
void check(void);
void sendLoraFrame (void);
void UART_SendString (char* str);
void reportTemp(void);
int returnStatus(void);
#endif /* INC_TCC_H_ */
