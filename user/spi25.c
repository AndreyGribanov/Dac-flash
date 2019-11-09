#include "stm32f4xx.h"
#include "delay.h"


#define READ  0x03//команда чтения памяти. После нее надо передать 3 байта адреса с которого вы хотите читать. 
                   //После память будет считываться до тех пор пока вы не перестанете подавать тактирующие сигналы по CLK 
									 //линии SPI (считываются все адреса подряд, а при достижении max адреса памяти микросхемы — 
									  //адрес становится равным нулю — кольцевой буфер). 

#define set_CS GPIOB->BSRR |= GPIO_BSRR_BR0//выбор кристалла активен(лог 0)
#define reset_CS GPIOB->BSRR |= GPIO_BSRR_BS0//выбор кристалла не активен(лог 1)


extern unsigned char buffer[256];//буфер, в который считывает обработчик прерываний
//extern-указывает на внешнюю переменную(в main.c компилятор уже выделил под нее место в памяти) и не выделяет память
uint8_t i = 0;


void spi_conf()//настройка spi
{	
  //Включаем тактирование SPI1 и GPIOB
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN ;//84MГц
	
	  //Настраиваем выводы SPI1:
	//PB0-FCS-выбор кристалла
	 GPIOB->MODER &=~ GPIO_MODER_MODER0;//сброс
	 GPIOB->MODER |= GPIO_MODER_MODER0_0;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->OTYPER &=~ GPIO_OTYPER_OT0;//1-открытый коллектор,0-двухтактный(после сброса)
	 GPIOB->OSPEEDR |=GPIO_OSPEEDER_OSPEEDR0_1;//10-высокая скорость 10МГц,01-2МГц,11-50Мгц
	 GPIOB->BSRR |= GPIO_BSRR_BS0;//выбор кристалла активен(лог 0)
	
  //PB3-SCK: 
   GPIOB->MODER &=~ GPIO_MODER_MODER3;//сброс
	 GPIOB->MODER |= GPIO_MODER_MODER3_1;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->AFR[0]  &=~ GPIO_AFRL_AFSEL3 ;//сброс
   GPIOB->AFR[0]  |= (GPIO_AFRL_AFSEL3_2 | GPIO_AFRL_AFSEL3_0);//AF=5 -SPI1	 
	// GPIOB->OTYPER |= GPIO_OTYPER_OT3;//1-открытый коллектор,0-двухтактный(после сброса)
	 GPIOB->OSPEEDR |=(GPIO_OSPEEDER_OSPEEDR3_1|
	                  GPIO_OSPEEDER_OSPEEDR3_0);//10-высокая скорость 10МГц,01-2МГц,11-50Мгц
		  
  //PB4-MISO:
	 GPIOB->MODER &=~ GPIO_MODER_MODER4;//сброс
	 GPIOB->MODER |= GPIO_MODER_MODER4_1;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->AFR[0]  &=~ GPIO_AFRL_AFSEL4 ;//сброс
   GPIOB->AFR[0]  |= (GPIO_AFRL_AFSEL4_2 | GPIO_AFRL_AFSEL4_0);//AF=5 -SPI1	 
	// GPIOB->OTYPER |= GPIO_OTYPER_OT4;//1-открытый коллектор,0-двухтактный(после сброса)
	// GPIOB->OSPEEDR |=(GPIO_OSPEEDER_OSPEEDR4_1|
	 //                 GPIO_OSPEEDER_OSPEEDR4_0);//10-высокая скорость 10МГц,01-2МГц,11-50Мгц
  
  //PB5-MOSI:
	 GPIOB->MODER &=~ GPIO_MODER_MODER5;//сброс
	 GPIOB->MODER |= GPIO_MODER_MODER5_1;//01-output,00-input(after reset),10-AF,11-analog.
	 GPIOB->AFR[0]  &=~ GPIO_AFRL_AFSEL5 ;//сброс
   GPIOB->AFR[0]  |= (GPIO_AFRL_AFSEL5_2 | GPIO_AFRL_AFSEL5_0);//AF=5 -SPI1	 
	// GPIOB->OTYPER |= GPIO_OTYPER_OT5;//1-открытый коллектор,0-двухтактный(после сброса)
	 GPIOB->OSPEEDR |=(GPIO_OSPEEDER_OSPEEDR5_1|
	                  GPIO_OSPEEDER_OSPEEDR5_0);//10-высокая скорость 10МГц,01-2МГц,11-50Мгц
		
   SPI1->CR1 &= ~SPI_CR1_DFF;  //Размер кадра 8 бит def=0,def=1-16 бит
  SPI1->CR1 &= ~SPI_CR1_LSBFIRST ;    //MSB передается первым,при =1-LSB передается первым(младший)
   SPI1->CR1 |=SPI_CR1_SSM ;         //Программное управление ведомым сигнал NSS заменяется значением бита SSI.
   SPI1->CR1 |=SPI_CR1_SSI ;         //SSi в высоком состоянии
   SPI1->CR1 |=(SPI_CR1_BR_2|SPI_CR1_BR_1|SPI_CR1_BR_0) ;//0x04Скорость передачи: F_PCLK/4,когда 0 F_PCLK/2
   SPI1->CR1 |=SPI_CR1_MSTR ;        //Режим Master (ведущий)
     SPI1->CR1 &=~(SPI_CR1_CPOL | SPI_CR1_CPHA); //Режим работы SPI: 0
		SPI1->CR2 |= SPI_CR2_RXNEIE;
	SPI1->CR1 |= SPI_CR1_SPE; //Включаем SPI
}	//end spi_conf
//*******************************************************************************************************************

void SPI_Write(uint8_t data)//
{
  while(!(SPI1->SR & SPI_SR_TXE)){}// ждем,когда можно будет загружать данные в DR
                                    // TXE флаг опустошения буфера передачи
  SPI1->DR = data;//заполняем буфер передатчика
	while(SPI1->SR & SPI_SR_BSY){}//ждем окончания передачи(необязательное условие)
 
}
//*********************************************************************************************************************
void clean_buffer()//очистка буфера данных
{
	for(i=0;i<=255;i++)
	{buffer[i] = 0;}
}

//*********************************************************************************************************************

	void SPI1_IRQHandler(void)//обработчик прерываний 
{
 if (SPI1->SR & SPI_SR_RXNE)//если пришли данные(поднят флаг RXNE)..,сброс в F4 после чтения DR
	{
		buffer[i] = SPI1->DR;
		i=i+1;
	}//Очистка бита RXNE выполняется при чтении регистра SPI_DR.
	 }
//************************************************************************************************************************

void Read_SPI(uint8_t N)//чтение в буфер N количества данных,фактически тактирует SPI,а обработчик читает в буфер 
{ i=0;//начинаем с начала буфера
	for(uint16_t n=0;n<=N;n++)//когда uint8_t =256,то оно воспринимается как 0
	{	SPI_Write(0xff);	}
}//end Read_SPI
//***********************************************************************************************************************

void Read_Page(uint16_t addr)//процедура чтения страницы из памяти в буфер
{
   set_CS;
		delay_ms(1);
	SPI_Write(READ);//0x03,команда чтения флеш	
	 SPI_Write((addr&0xff00)>>8);//старший байт номера страницы
	SPI_Write(addr&0xff);//младший байт номера страницы	
	SPI_Write(0);//номер первого байта

	Read_SPI(255);
   reset_CS;
	delay_ms(5);
}