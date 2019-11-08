
//Задача: Вооспроизвести файл сэмплов, хранящихся на микросхеме памяти 25 серии(W25Q16, интерфейс SPI) с помощью DAC 
//STM32F407xx .Длина файла заранее неизвестна. Память состоит из страниц по 256 Байт(2048 бит).Данные в DAC грузим 
//с помощью DMA.Майном читаем память сначала в промежуточный массив,откуда данные берет DMA.DAC тактируется TIM6.

#include "stm32f4xx.h"
#include "delay.h" //Генерация пауз, как правило,всегда нужна во всех проектах


  unsigned char DAC_Buff[512]; //промежуточный буфер для данных которые ДМА перекидает в ЦАП

 
 
 
 void init_RCC()//настройка тактирования,пользуемся картинкой CUBE MX
{
RCC->CR|=RCC_CR_HSEON; //Запускаем генератор HSE,внешний кварц
while (!(RCC->CR & RCC_CR_HSERDY)) {}; // Ждем готовности
	
FLASH->ACR |= FLASH_ACR_PRFTEN; // Enable Prefetch Buffer.
FLASH->ACR &= ~FLASH_ACR_LATENCY; // Предочистка.
FLASH->ACR |= FLASH_ACR_LATENCY_2WS; // Two wait states
FLASH->ACR |=FLASH_ACR_ICEN | FLASH_ACR_DCEN;// Instruction cache enable, Data cache enable
	
RCC->CFGR &=~RCC_CFGR_SW; // Очистить биты SW0, SW1
RCC->CFGR |= RCC_CFGR_PPRE1_2;	//100: AHB clock divided by 2 for APB1
RCC->CFGR |= RCC_CFGR_SW_PLL; // Переходим на PLL SW1=1	---!!!

 //Config PLL
// RCC->PLLCFGR=(cPLLQ<<24) | (cPLLP<<16) | (cPLLN<<6) | cPLLM | RCC_PLLCFGR_PLLSRC_HSE; //page 162	
RCC->PLLCFGR=(4<<24) | (0<<16) | (84<<6) | 4 | RCC_PLLCFGR_PLLSRC_HSE; //   настраевается при сброшеном бите 	PLLON  в CR		
RCC->CR |=    RCC_CR_PLLON;
while (!(RCC->CR & RCC_CR_PLLRDY)){}; //wait for PLL ready 	

}//end init_RCC()





int main(void)
{
 





}
	