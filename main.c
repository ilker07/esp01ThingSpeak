
#include "stm32f4xx.h" 
#include "rcc.h"
#include "gpio.h"
#include "usart.h"
#include "timer.h"
#include "veri.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


volatile uint16_t msn=0;
volatile uint16_t birsn=0;
void bekle(uint32_t beklenen)
{

  msn=0;
	while(msn<beklenen);
}

volatile bool sureDoldu=0;

volatile char gelenCevap[100]="";
char karakterDizisi[100]="";
volatile int sayac=0;
uint8_t mod=0;
uint8_t saat=0;
uint8_t dakika=0;
uint8_t saniye=0;



void uygula(const char * gonderilecek,const char * beklenen)
{

      if(strstr((char *)gelenCevap,beklenen))
			{
				
				memset((char *)gelenCevap,0,sizeof(gelenCevap));//Array temizlendi.
				sayac=0;
				mod++;
				veri_yollaESP8266(gonderilecek);
				
			}


}






void USART2_IRQHandler()   //Espden gelen veriler.
{
	
	NVIC->ICER[1] =1<<6;//Interrupt pasif

 
	if(USART2->SR & 1<<5)  //RX bayrak kontrolu.
	{
		
	 
		gelenCevap[sayac]=USART2->DR;//Veri alindi.
		sayac++;
		if(sayac>100)
			sayac=0;
		
		
	}
	
	if(USART2->SR & 1<<6) //TX bayrak kontrolu.
	{
		 
		 USART2->SR &=~(1<<6);//Bayrak temizlemek icin.
	}
	
	
	NVIC->ISER[1] =1<<6;//Interrupt aktif.
	
}



void TIM2_IRQHandler(void)
{
   	if (TIM2->SR & (1<<0))  //UIF Bayragi mi
  {
			
			msn++;
		  birsn++;
		
		if(birsn>=1000)
		{
		  birsn=0;
			sureDoldu=1;
		}
		 
			if(msn>=65000)  
			{
				 msn=0;
			}
      TIM2->SR &=~(1<<0); //UIF temizlendi.
  }


}


int main()
	
{
  RCC_Config();//PLL ile sistem 168 Mhz.
	GPIO_Config();
	USART_Config();
	timer_Ayar();
	veri_yollaESP8266(" AT\r\n");//Gelen cevap:AT\r\r\n\r\nOK\r\n
	NVIC->ISER[1] =1<<6;//USART2 Kesme Aktif.
	
	
	
	
while(1)
{
  
	
   
	
  if(sureDoldu==1)
	{
	//sprintf(karakterDizisi,"%s \r\n Gelen Veri Sayisi:%d MOD:%d\r\n",gelenCevap,sayac,mod);
	//veri_yollaUSBTTL(karakterDizisi);
	 veri_yollaUSBTTL((char *)gelenCevap);
	sureDoldu=0;
	}
	
	
	if(mod==0) 
		uygula("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n","OK");//Thinspeak adresi
	
	if(mod==1)
	{
		uygula("AT+CIPSEND=65\r\n","CONNECT");//65 bayt veri yollayacagiz ve CONNECT cevabi beklenir.
		
	}
	if(mod==2) //AT+CIPSEND gonderildikten sonra 1sn oldu ise.&& msn>=1000
	{
		uygula("GET /apps/thinghttp/send_request?api_key=EVZYLR57FLRCDA9D\r\nHTTP/1.1\r\n",">");//Saat bilgisi icin gerekli adres.
	  msn=0;
	}
	
	if(mod==3 && msn>=1000) //
	{
		 msn=0;
		 char *ptr=NULL;
		 ptr=strstr((char *)gelenCevap,"+IPD");
		 if(ptr)
			{
		    //+IPD,8:15:25:30CLOSED  Aldigimiz veri bu.8 verisi 15:25:30 bunun uzunlugu.
        				
				saat =(((int)ptr[7]-48)*10) +((int)ptr[8]-48);
				dakika=(((int)ptr[10]-48)*10) +((int)ptr[11]-48);
				saniye=(((int)ptr[13]-48)*10) +((int)ptr[14]-48);
			  mod=0;
	
			}
	 
	}
	
	
    if(strstr((char *)gelenCevap,"ERROR") || strstr((char *)gelenCevap,"FAIL") || strstr((char *)gelenCevap,"busy p") ) //Herhangi bir anda ERROR geldiyse.
		{
			veri_yollaESP8266("AT\r\n");//Yeniden AT yolla.
			mod=0;
			
	  }
	


}

}

/*

  USART3 USB-TTL STM32 arasi (Sadece veri yolla)
  USART2 ESP8266 STM32 arasi (Veri al-yolla.)
*/



