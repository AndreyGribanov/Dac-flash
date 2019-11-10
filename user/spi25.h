#ifndef SPI25_H
#define SPI25_H
#include "stm32f4xx.h"



//��������� ������������ �������� ��� ������ � SPI:
void spi_conf(void);//��������� spi
void SPI_Write(uint8_t data);//�������� ������ �� ����� MOSI,� �.�. CLK ��� ������
void SPI1_IRQHandler(void);//���������� ���������� ,����� ������ � ����� ��� �� ��������� �� ����� MISO
void Read_SPI(uint16_t N);//������ � ����� N ���������� ������,���������� ��������� SPI,� ���������� ������ � ����� 
void Read_En(void);//��������� �������� � ����� ������ ����������
#endif

