
//������: �������������� ���� �������, ���������� �� ���������� ������ 25 �����(W25Q16, ��������� SPI) � ������� DAC 
//STM32F407xx .����� ����� ������� ����������. ������ ������� �� ������� �� 256 ����(2048 ���).������ � DAC ������ 
//� ������� DMA.������ ������ ������ ������� � ������������� ������,������ ������ ����� DMA.DAC ����������� TIM6.

#include "stm32f4xx.h"
#include "delay.h" //��������� ����, ��� �������,������ ����� �� ���� ��������


  unsigned char DAC_Buff[512]; //������������� ����� ��� ������ ������� ��� ���������� � ���

 
 
 
 void init_RCC()//��������� ������������,���������� ��������� CUBE MX
{
RCC->CR|=RCC_CR_HSEON; //��������� ��������� HSE,������� �����
while (!(RCC->CR & RCC_CR_HSERDY)) {}; // ���� ����������
	
FLASH->ACR |= FLASH_ACR_PRFTEN; // Enable Prefetch Buffer.
FLASH->ACR &= ~FLASH_ACR_LATENCY; // �����������.
FLASH->ACR |= FLASH_ACR_LATENCY_2WS; // Two wait states
FLASH->ACR |=FLASH_ACR_ICEN | FLASH_ACR_DCEN;// Instruction cache enable, Data cache enable
	
RCC->CFGR &=~RCC_CFGR_SW; // �������� ���� SW0, SW1
RCC->CFGR |= RCC_CFGR_PPRE1_2;	//100: AHB clock divided by 2 for APB1
RCC->CFGR |= RCC_CFGR_SW_PLL; // ��������� �� PLL SW1=1	---!!!

 //Config PLL
// RCC->PLLCFGR=(cPLLQ<<24) | (cPLLP<<16) | (cPLLN<<6) | cPLLM | RCC_PLLCFGR_PLLSRC_HSE; //page 162	
RCC->PLLCFGR=(4<<24) | (0<<16) | (84<<6) | 4 | RCC_PLLCFGR_PLLSRC_HSE; //   ������������� ��� ��������� ���� 	PLLON  � CR		
RCC->CR |=    RCC_CR_PLLON;
while (!(RCC->CR & RCC_CR_PLLRDY)){}; //wait for PLL ready 	

}//end init_RCC()





int main(void)
{
 





}
	