
//������: �������������� ���� �������, ���������� �� ���������� ������ 25 �����(W25Q16, ��������� SPI) � ������� DAC 
//STM32F407xx .����� ����� ������� ����������. ������ ������� �� ������� �� 256 ����(2048 ���).������ � DAC ������ 
//� ������� DMA.������ ������ ������ ������� � ������������� ������,������ ������ ����� DMA.DAC ����������� TIM6.

#include "stm32f4xx.h"
#include "delay.h" //��������� ����, ��� �������,������ ����� �� ���� ��������
#include "spi25.h"//��������� ��� ������ � flash

unsigned char DAC_Buff[512]; //������������� ����� ��� ������ ������� ��� ���������� � ���
unsigned char buffer[256];//�����, � ������� ��������� ���������� ����������
uint16_t Npage=0;//����� �������� ������ flash 25q16
 
 
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
//*****************************************************************************************************************
void init_DAC_TIM6()//������������� ��� � ���6,������������ ���,PA4-���� ������
{
RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
GPIOA->MODER |= GPIO_MODER_MODER4;//01-output,00-input(after reset),10-AF,11-analog.	
RCC->APB1ENR |= RCC_APB1ENR_DACEN;             //������������ ����

DAC->CR   &=(DAC_CR_BOFF1                  //��������� �������� �����
             | DAC_CR_TSEL1);              //000-�������� ������� �������������� TIM6
DAC->CR   |= (DAC_CR_TEN1                  //���������� ����������� �������� �������������� �� ������� �� TIM6
             |DAC_CR_DMAEN1                 //��������� ����� ������ ������ �1 �� ���
             |DAC_CR_EN1); 

RCC->APB1ENR        |=  RCC_APB1ENR_TIM6EN;    //������ ������������ TIM6,�� APB1 ������� �������� � ��������� �������� APB1 �� 84���
TIM6->PSC            =  0;                     //������ ������� ������������� 22050
TIM6->ARR            =  3809; //65536max                  //(��� �������� 84000000)
TIM6->CR2         |=  TIM_CR2_MMS_1;//MMS = 010 : update event is selected as a trigger output (TRGO)
}//end init_DAC_TIM6
//*******************************************************************************************************************
void init_DMA()//������������� ��� ����������� ������ �� ������� � ���
{	
  RCC->AHB1ENR  |= RCC_AHB1ENR_DMA1EN;	//������������ DMA
	//�����7 ����� 5 = dac1//111 -7 �����   
	DMA1_Stream5-> CR  &=~ DMA_SxCR_EN; 
	DMA1_Stream5->CR=0;//����� CR ��������
	
DMA1_Stream5-> PAR  = (uint32_t)&DAC->DHR8R1; //��������� �� ������� ���������???????channel1 8-bit right-aligned data
//DAC_DHR8Rx - ������� ��� ������ 8 ��� ������. ������������� �� ������� ����.
DMA1_Stream5-> M0AR  = (uint32_t)&DAC_Buff; //��������� �� ������ ������ � ������ DAC 
DMA1_Stream5->NDTR =  512;//������ ������,����� ����������
         
                
DMA1_Stream5-> CR  &= DMA_SxCR_PBURST;        //����� ��������� �� ��������������
DMA1_Stream5-> CR  |=  DMA_SxCR_MINC;         //����� ������ ��������������
DMA1_Stream5-> CR  &= ~DMA_SxCR_PSIZE;        //����������� ������ ��������� - 8 ���.
DMA1_Stream5-> CR  &= ~DMA_SxCR_MSIZE;        //����������� ������ ������    - 8 ���
DMA1_Stream5-> CR  |=  DMA_SxCR_CIRC;         //�������� ����������� ����� �������� ������
DMA1_Stream5-> CR  |=  DMA_SxCR_PL_1;         //��������� ����� �������

DMA1_Stream5-> CR  |=  DMA_SxCR_DIR_0; 	       //01-����������� �������� - �� ������ � ���������
DMA1_Stream5-> CR  |=  DMA_SxCR_CHSEL;          //����� ������(111)
	
//DMA1_Stream5-> CR  |=  DMA_SxCR_EN;  	//��������� ������ 5 ������ DMA
}//end init_DMA
//***********************************************************************************************************************
void WriteData(uint16_t N, uint16_t cnt)//����������� �� ������� � ������ ,N-��������� ������ DAC_Buff,cnt-����� ���������
	{
 	Read_Page(Npage);	//������ �������� �� ������ � buffer[]
	Npage++;	
	uint16_t i=0;	
	while (cnt>0) {
        DAC_Buff[N++]= buffer[i++];
       cnt--;
         }
}//END WriteData
//**********************************************************************************************************************

int main(void)
{
	
init_RCC();
spi_conf();	
init_DAC_TIM6(); 	
init_DMA();	
NVIC_EnableIRQ (SPI1_IRQn);// ������� CMSIS ����������� ���������� � NVIC
	__enable_irq ();// ��������� ���������� ����������
	
	
	
	
	
	
WriteData(0,256);//���������� ������ ��������
WriteData(256,256);//���������� ������ ��������

 DMA1_Stream5-> CR  |=  DMA_SxCR_EN;  	//��������� ������ 5 ������ DMA	
 TIM6->CR1 |= TIM_CR1_CEN;//��������� ��������������

	
	
	
	
	
	
	
	
while(1)
  {
      while(!(DMA1->HISR & DMA_HISR_HTIF5)) {}   //���� ������������ ������ ����� ������ 5 ������			
			 
			WriteData(0,256);//�� ������ � ������ �������� ������
			if ((buffer[0]==0xff)&&(buffer[255]==0xff)){break;} 	
      DMA1->HIFCR |= DMA_HISR_HTIF5;             //�������� ����
     
 
       while(!(DMA1->HISR & DMA_HISR_TCIF5)) {}   //���� ������������ ������ ����� ������ 5 ������
			
		 	 WriteData(256,256); // ������ �� ������ ����� ������ 
			 if ((buffer[0]==0xff)&&(buffer[255]==0xff)){break;} 				
        DMA1->HIFCR |= DMA_HISR_TCIF5;             //�������� ����
    
  }
  DAC->CR   &= ~DAC_CR_EN1;
  TIM6->CR1 &= ~TIM_CR1_CEN;


 while(1){}//��������




}
	
