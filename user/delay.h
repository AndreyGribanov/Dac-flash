//генерация пауз функциями delay_ms и delay_us
#ifndef DELAY_H//ДЕРЕКТИВЫ,ПОЗВОЛЯЮЩИЕ ПОВТОРНО ВКЛЮЧАТЬ ФАЙЛ ЧЕРЕЗ #INCLUDE
#define DELAY_H//НЕ ВЫЗЫВАЯ ПРИ ЭТОМ ОШИБОК
#include "stm32f4xx.h"
//#include "stm32f10x.h"
//extern uint32_t SystemCoreClock;//возвращает значенин системной частоты

void delay_ms(uint16_t delay);
void delay_us(uint16_t delay);



#endif//

