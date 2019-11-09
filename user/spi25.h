#ifndef SPI25_H
#define SPI25_H
#include "stm32f4xx.h"



//прототипы используемых процедур при работе с SPI:
void spi_conf();//настройка spi
void SPI_Write(uint8_t data);//отправка данных по линии MOSI,в т.ч. CLK при чтении
void SPI1_IRQHandler(void);//обработчик прерываний ,пишет данные в буфер при их появлении на линии MISO
void Read_SPI(uint8_t N);//чтение в буфер N количества данных,фактически тактирует SPI,а обработчик читает в буфер 
void Read_Page(uint16_t addr);//процедура чтения страницы из памяти в буфер
void clean_buffer();//очистка буфера данных????скорее всего не понадобится
#endif
