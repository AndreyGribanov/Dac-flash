//��������� ���� ��������� delay_ms � delay_us
#ifndef DELAY_H//���������,����������� �������� �������� ���� ����� #INCLUDE
#define DELAY_H//�� ������� ��� ���� ������
#include "stm32f4xx.h"
//#include "stm32f10x.h"
//extern uint32_t SystemCoreClock;//���������� �������� ��������� �������

void delay_ms(uint16_t delay);
void delay_us(uint16_t delay);



#endif//

