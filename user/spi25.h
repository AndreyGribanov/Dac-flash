#ifndef SPI25_H
#define SPI25_H
#include "stm32f4xx.h"



//прототипы используемых процедур при работе с SPI:
void spi_conf(void);//настройка spi
void SPI_Write(uint8_t data);//отправка данных по линии MOSI,в т.ч. CLK при чтении
void SPI1_IRQHandler(void);//обработчик прерываний ,пишет данные в буфер при их появлении на линии MISO
void Read_SPI(uint16_t N);//чтение в буфер N количества данных,фактически тактирует SPI,а обработчик читает в буфер 
void Read_En(void);//процедура перевода в режим чтения микросхемы
#endif

