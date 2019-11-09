
//Задача: Вооспроизвести файл сэмплов, хранящихся на микросхеме памяти 25 серии(W25Q16, интерфейс SPI) с помощью DAC 
//STM32F407xx .Длина файла заранее неизвестна. Память состоит из страниц по 256 Байт(2048 бит).Данные в DAC грузим 
//с помощью DMA.Майном читаем память сначала в промежуточный массив,откуда данные берет DMA.DAC тактируется TIM6.

#include "stm32f4xx.h"
#include "delay.h" //Генерация пауз, как правило,всегда нужна во всех проектах
#include "spi25.h"//процедуры для работы с flash

unsigned char DAC_Buff[512]; //промежуточный буфер для данных которые ДМА перекидает в ЦАП
unsigned char buffer[256];//буфер, в который считывает обработчик прерываний
 uint16_t Npage=0;//номер страницы памяти flash 25q16
 
 
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
//*****************************************************************************************************************
void init_DAC_TIM6()//инициализация ЦАП И ТИМ6,тактирующего ЦАП,PA4-порт выхода
{
RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
GPIOA->MODER |= GPIO_MODER_MODER4;//01-output,00-input(after reset),10-AF,11-analog.	
RCC->APB1ENR |= RCC_APB1ENR_DACEN;             //тактирование ЦАПа

DAC->CR   &=(DAC_CR_BOFF1                  //ОТключить выходной буфер
             | DAC_CR_TSEL1);              //000-источник запуска преобразования TIM6
DAC->CR   |= (DAC_CR_TEN1                  //обновление содержимого регистра преобразования по событию от TIM6
             |DAC_CR_DMAEN1                 //разрешить прием данных канала №1 от ДМА
             |DAC_CR_EN1); 

RCC->APB1ENR        |=  RCC_APB1ENR_TIM6EN;    //подаем тактирование TIM6,на APB1 таймеры работают с удвоенной частотой APB1 те 84МГц
TIM6->PSC            =  0;                     //Задаем частоту дискретизации 22050
TIM6->ARR            =  3809; //65536max                  //(при тактовой 84000000)
TIM6->CR2         |=  TIM_CR2_MMS_1;//MMS = 010 : update event is selected as a trigger output (TRGO)
}//end init_DAC_TIM6
//*******************************************************************************************************************
void init_DMA()//инициализация ДМА передающего данные из массива в ЦАП
{	
  RCC->AHB1ENR  |= RCC_AHB1ENR_DMA1EN;	//тактирование DMA
	//канал7 поток 5 = dac1//111 -7 канал   
	DMA1_Stream5-> CR  &=~ DMA_SxCR_EN; 
	DMA1_Stream5->CR=0;//сброс CR регистра
	
DMA1_Stream5-> PAR  = (uint32_t)&DAC->DHR8R1; //указатель на регистр периферии???????channel1 8-bit right-aligned data
//DAC_DHR8Rx - регистр для записи 8 бит данных. Выравниваение по правому краю.
DMA1_Stream5-> M0AR  = (uint32_t)&DAC_Buff; //указатель на начало буфера в памяти DAC 
DMA1_Stream5->NDTR =  512;//размер буфера,число транзакций
         
                
DMA1_Stream5-> CR  &= DMA_SxCR_PBURST;        //адрес периферии не инкрементируем
DMA1_Stream5-> CR  |=  DMA_SxCR_MINC;         //адрес памяти инкрементируем
DMA1_Stream5-> CR  &= ~DMA_SxCR_PSIZE;        //размерность данных периферии - 8 бит.
DMA1_Stream5-> CR  &= ~DMA_SxCR_MSIZE;        //размерность данных памяти    - 8 бит
DMA1_Stream5-> CR  |=  DMA_SxCR_CIRC;         //включить циклический режим передачи данных
DMA1_Stream5-> CR  |=  DMA_SxCR_PL_1;         //приоритет очень высокий

DMA1_Stream5-> CR  |=  DMA_SxCR_DIR_0; 	       //01-направление передачи - из памяти в периферию
DMA1_Stream5-> CR  |=  DMA_SxCR_CHSEL;          //Выбор канала(111)
	
//DMA1_Stream5-> CR  |=  DMA_SxCR_EN;  	//разрешаем работу 5 потока DMA
}//end init_DMA
//***********************************************************************************************************************
void WriteData(uint16_t N, uint16_t cnt)//копирование из массива в массив ,N-начальный индекс DAC_Buff,cnt-число элементов
	{
 	Read_Page(Npage);	//читаем страницу из памяти в buffer[]
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
init_DAC_TIM6(); 	
init_DMA();	

WriteData(0,256);//заполнение первой половины
WriteData(256,256);//заполнение второй половины

 DMA1_Stream5-> CR  |=  DMA_SxCR_EN;  	//разрешаем работу 5 потока DMA	
 TIM6->CR1 |= TIM_CR1_CEN;//запустить преобразование

	
	
	
	
	
	
	
	
while(1)
  {
      while(!(DMA1->HISR & DMA_HISR_HTIF5)) {}   //ждем освобождение первой части буфера 5 потока			
			 
			WriteData(0,256);//то читаем в первую половину буфера
			if ((buffer[0]==0xff)&&(buffer[255]==0xff)){break;} 	
      DMA1->HIFCR |= DMA_HISR_HTIF5;             //сбросить флаг
     
 
       while(!(DMA1->HISR & DMA_HISR_TCIF5)) {}   //ждем освобождение второй части буфера 5 потока
			
		 	 WriteData(256,256); // читаем во вторую часть буфера 
			 if ((buffer[0]==0xff)&&(buffer[255]==0xff)){break;} 				
        DMA1->HIFCR |= DMA_HISR_TCIF5;             //сбросить флаг
    
  }
  DAC->CR   &= ~DAC_CR_EN1;
  TIM6->CR1 &= ~TIM_CR1_CEN;


 while(1){}//зависаем




}
	