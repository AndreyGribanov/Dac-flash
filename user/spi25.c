#include "stm32f4xx.h"
#include "delay.h"


#define READ  0x03//������� ������ ������. ����� ��� ���� �������� 3 ����� ������ � �������� �� ������ ������. 
                   //����� ������ ����� ����������� �� ��� ��� ���� �� �� ����������� �������� ����������� ������� �� CLK 
									 //����� SPI (����������� ��� ������ ������, � ��� ���������� max ������ ������ ���������� � 
									  //����� ���������� ������ ���� � ��������� �����). 

#define set_CS GPIOB->BSRR |= GPIO_BSRR_BR0//����� ��������� �������(��� 0)
#define reset_CS GPIOB->BSRR |= GPIO_BSRR_BS0//����� ��������� �� �������(��� 1)


extern unsigned char DAC_Buff[512];//�����, � ������� ��������� ���������� ����������
//extern-��������� �� ������� ����������(� main.c ���������� ��� ������� ��� ��� ����� � ������) � �� �������� ������
uint16_t i = 0;//����������,������ ������� � ������� ����� ������


void spi_conf()//��������� spi
{	
  //�������� ������������ SPI1 � GPIOB
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN ;//84M��
	
	  //����������� ������ SPI1:
	//PB0-FCS-����� ���������
	 GPIOB->MODER &=~ GPIO_MODER_MODER0;//�����
	 GPIOB->MODER |= GPIO_MODER_MODER0_0;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->OTYPER &=~ GPIO_OTYPER_OT0;//1-�������� ���������,0-�����������(����� ������)
	 GPIOB->OSPEEDR |=GPIO_OSPEEDER_OSPEEDR0_1;//10-������� �������� 10���,01-2���,11-50���
	 GPIOB->BSRR |= GPIO_BSRR_BS0;//����� ��������� �������(��� 0)
	
  //PB3-SCK: 
   GPIOB->MODER &=~ GPIO_MODER_MODER3;//�����
	 GPIOB->MODER |= GPIO_MODER_MODER3_1;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->AFR[0]  &=~ GPIO_AFRL_AFSEL3 ;//�����
   GPIOB->AFR[0]  |= (GPIO_AFRL_AFSEL3_2 | GPIO_AFRL_AFSEL3_0);//AF=5 -SPI1	 
	 GPIOB->OSPEEDR |=(GPIO_OSPEEDER_OSPEEDR3_1|
	                  GPIO_OSPEEDER_OSPEEDR3_0);//10-������� �������� 10���,01-2���,11-50���
		  
  //PB4-MISO:
	 GPIOB->MODER &=~ GPIO_MODER_MODER4;//�����
	 GPIOB->MODER |= GPIO_MODER_MODER4_1;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->AFR[0]  &=~ GPIO_AFRL_AFSEL4 ;//�����
   GPIOB->AFR[0]  |= (GPIO_AFRL_AFSEL4_2 | GPIO_AFRL_AFSEL4_0);//AF=5 -SPI1	 
	
  
  //PB5-MOSI:
	 GPIOB->MODER &=~ GPIO_MODER_MODER5;//�����
	 GPIOB->MODER |= GPIO_MODER_MODER5_1;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->AFR[0]  &=~ GPIO_AFRL_AFSEL5 ;//�����
   GPIOB->AFR[0]  |= (GPIO_AFRL_AFSEL5_2 | GPIO_AFRL_AFSEL5_0);//AF=5 -SPI1	 
	 GPIOB->OSPEEDR |=(GPIO_OSPEEDER_OSPEEDR5_1|
	                  GPIO_OSPEEDER_OSPEEDR5_0);//10-������� �������� 10���,01-2���,11-50���
		
   SPI1->CR1 &= ~SPI_CR1_DFF;  //������ ����� 8 ��� def=0,def=1-16 ���
   SPI1->CR1 &= ~SPI_CR1_LSBFIRST ;    //MSB ���������� ������,��� =1-LSB ���������� ������(�������)
   SPI1->CR1 |=SPI_CR1_SSM ;         //����������� ���������� ������� ������ NSS ���������� ��������� ���� SSI.
   SPI1->CR1 |=SPI_CR1_SSI ;         //SSi � ������� ���������
   SPI1->CR1 |=(SPI_CR1_BR_2|SPI_CR1_BR_1|SPI_CR1_BR_0) ;//0x04�������� ��������: F_PCLK/4,����� 0 F_PCLK/2
   SPI1->CR1 |=SPI_CR1_MSTR ;        //����� Master (�������)
   SPI1->CR1 &=~(SPI_CR1_CPOL | SPI_CR1_CPHA); //����� ������ SPI: 0
	 SPI1->CR2 |= SPI_CR2_RXNEIE;
	 SPI1->CR1 |= SPI_CR1_SPE; //�������� SPI
}	//end spi_conf
//*******************************************************************************************************************

void SPI_Write(uint8_t data)//
{
  while(!(SPI1->SR & SPI_SR_TXE)){}// ����,����� ����� ����� ��������� ������ � DR
                                    // TXE ���� ����������� ������ ��������
  SPI1->DR = data;//��������� ����� �����������
	while(SPI1->SR & SPI_SR_BSY){}//���� ��������� ��������(�������������� �������)
 
}
//*********************************************************************************************************************
	void SPI1_IRQHandler(void)//���������� ���������� 
{
 if (SPI1->SR & SPI_SR_RXNE)//���� ������ ������(������ ���� RXNE)..,����� � F4 ����� ������ DR
	{
		DAC_Buff[i] = SPI1->DR;
		i++;
		if (i==512) i=0;
	}//������� ���� RXNE ����������� ��� ������ �������� SPI_DR.
	 }
//************************************************************************************************************************

void Read_SPI(uint16_t N)//������ � ����� N ���������� ������,���������� ��������� SPI,� ���������� ������ � ����� 
{ 
	for(uint16_t n=0;n<N;n++)//����� uint8_t =256,�� ��� �������������� ��� 0
	{	SPI_Write(0xff);	}
	
	
}//end Read_SPI
//***********************************************************************************************************************

void Read_En(void)//��������� �������� � ����� ������ ����������
{
  set_CS;
	delay_ms(1);
	SPI_Write(READ);//0x03,������� ������ ����	
	SPI_Write(0);//������� ���� ������ ��������
	SPI_Write(0);//������� ���� ������ ��������	
	SPI_Write(0);//����� ������� �����
  i=0;//������ �������� � ������ ������

}

