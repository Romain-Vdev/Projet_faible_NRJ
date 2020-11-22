/* Project L476_ats_blink for STM32L476 mounted on Nucleo board:
 * the user LED (mounted on pin PA-5) is flashed every second for 50 ms.
 * The time base is provided by Systick (1000 ticks per second).
 * The clock configuration is the default one (Sysclk = 80 MHz, derived from MSI and PLL).
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_exti.h"

// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

#include "gpio.h"

// systick interrupt handler
/*void SysTick_Handler()
{

}*/
int expe=1;
int bluemode=0;
int counter=0; // compteur utilisé dans le handler Systick pour gérer la LED
void Calibration_MSI_vs_LPE(void);
void SystemClock_Config(void);
void SystemClock_Config_MSI_24Mhz(void);
void RTC_Config(void);
static void RTC_wakeup_init( int delay );
void RTC_wakeup_init_from_standby_or_shutdown( int delay );
void RTC_wakeup_init_from_stop( int delay );
void RTC_WKUP_IRQHandler();
void Set_Low_Power_Mode(uint32_t LowPowerMode);
void Init_Low_Power_Mode(uint32_t LowPowerMode);

int main(void)
{

	  // Reset of all peripherals, Initializes the Flash interface and the Systick. */
	  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	  // config GPIO
	  GPIO_init();

	  // config RTC
	  RTC_Config();

	  // si j'appuie sur le bouton bleu et que je reset => je change d'expérience, expe s'incrémente
	  if (BLUE_BUTTON()){ // à regarder si c'est bien la détection repos->pressé

		  // je récupère la valeur de expe dans le backup register
		  expe=LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0);
		  expe++;

		  if (expe>8)
       		expe=1;

		 // stockage de la nouvelle valeur expe dans le backup register
		LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, expe);

	  }else

		  expe=LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0);


	  /* Configure the system clock en fonction de expe qui défini Voltage Scaling and Flash Latency et utilisation de la PLL   */
	  if (expe==1)
		  SystemClock_Config(); // fonction à appeler si expe == 1
	  else
		  SystemClock_Config_MSI_24Mhz();


	  // Fonction qui active la calibration MSI vs LSE en fonction de expe
	   Calibration_MSI_vs_LPE();



	  // init systick timer (tick period at 10 ms)
	  LL_Init10msTick( SystemCoreClock );

	  while (1)  {

		  if (bluemode==1){

			  //if ((expe==1) || (expe==3))
				  //LL_LPM_EnableSleep();
			  	  // if expe = 1 :  MSI=4MHz | PLL=80Hz | V.Scaling=1 | F.Latency=4 | Calibration=OFF | Sleep=OFF->ON
			 	  // if expe = 3 :  MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration=OFF | Sleep=OFF->ON

			  //if ((expe==2) || (expe==4))
				//  LL_RCC_MSI_EnablePLLMode();
			  	  // if expe = 2 :  MSI=24MHz | PLL=OFF | V.Scaling=1 | F.Latency=1 | Calibration=OFF->ON | Sleep=OFF
			  	  // if expe = 4 :  MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration=OFF->ON | Sleep=OFF





			  switch(expe)
			  {

			  	 case 1:
			  		 LL_LPM_EnableSleep();// MSI=4MHz | PLL=80Hz | V.Scaling=1 | F.Latency=4 | Calibration=OFF | Sleep=OFF->ON
			  		 __WFI();
			  		 // bluemode=0;
			  	 break;

			  	 case 2:
			  		 LL_RCC_MSI_EnablePLLMode(); // MSI=24MHz | PLL=OFF | V.Scaling=1 | F.Latency=1 | Calibration=OFF->ON | Sleep=OFF
			  	break ;

			  	 case 3:
			  		 LL_LPM_EnableSleep(); // MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration=OFF | Sleep=OFF->ON
			  		 __WFI();
			  		//bluemode=0;
			  	break ;
			  	 case 4:
			  		 LL_RCC_MSI_EnablePLLMode(); // MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration=OFF->ON | Sleep=OFF
			  	break ;
			  	case 5: //Set_Low_Power_Mode(LL_PWR_MODE_SHUTDOWN);
			  			Init_Low_Power_Mode(LL_PWR_MODE_STOP0);
			  		    Set_Low_Power_Mode(LL_PWR_MODE_STOP0);
			  			__WFI();
			  			bluemode=0;
			  			break;

			  	case 6: //Set_Low_Power_Mode(LL_PWR_MODE_SHUTDOWN);
			  				  			Init_Low_Power_Mode(LL_PWR_MODE_SHUTDOWN);
			  				  		    Set_Low_Power_Mode(LL_PWR_MODE_SHUTDOWN);
			  				  			__WFI();
			  				  			//bluemode=0;
			  				  			break;

			  	 default:
			  		 break ;

			  	 // case '5': Set_Low_Power_Mode(LL_PWR_MODE_STOP0) // MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration= ON | Sleep= ON | STOP0, wakeup 20s

			  	 // case '6': Set_Low_Power_Mode(LL_PWR_MODE_STOP1) // MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration= ON | Sleep= ON | STOP1, wakeup 20s
			  	 // case '7': Set_Low_Power_Mode(LL_PWR_MODE_STOP2) // MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration= ON | Sleep= ON | STOP2, wakeup 20s
			  	 // case '8': Set_Low_Power_Mode(L_PWR_MODE_SHUTDOWN) // MSI=24MHz | PLL=OFF | V.Scaling=2 | F.Latency=3 | Calibration= ON | Sleep= ON | SHUTDOWN, wakeup 20s


			   }// end switch

		  }// end if

	  }// end while

}// end main


//----------------------------------------------------------------------
	/**
	  * @brief Active le mécanisme dit "calibration" qui utilise une sorte de PLL asservie à l'oscillateur LSE

	  * @param  None
	  * @retval None
	  */
	void Calibration_MSI_vs_LPE(void){
		// calibration active par défaut quand expe vaut 5,6,7 ou 8
		if (expe >= 5)
			LL_RCC_MSI_EnablePLLMode();

	}

//----------------------------------------------------------------------


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config(void) {
/* MSI configuration and activation */
LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
LL_RCC_MSI_Enable();
while	(LL_RCC_MSI_IsReady() != 1)
	{ };
  
/* Main PLL configuration and activation */
LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
LL_RCC_PLL_Enable();
LL_RCC_PLL_EnableDomain_SYS();
while(LL_RCC_PLL_IsReady() != 1)
	{ };
  
/* Sysclk activation on the main PLL */
LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
	{ };
  
/* Set APB1 & APB2 prescaler*/
LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

/* Update the global variable called SystemCoreClock */
SystemCoreClockUpdate();
}
//----------------------------------------------------------------------

/** Fonction appelée quand expe est compris entre 2 et 8

  * @brief  System Clock Configuration when the MSI is set to 24MHz
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 24 000 000
  *            Flash Latency(WS)              = 1 (if expe=2) or 3
  * @param  None
  * @retval None
  */

void SystemClock_Config_MSI_24Mhz(void) {
/* MSI configuration and activation */
// Réglage de la latence d'accès à la mémoire flash en fonction de expe
if (expe==2)
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
else
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);


// activation du MSI
LL_RCC_MSI_Enable();

while	(LL_RCC_MSI_IsReady() != 1)
	{ };

LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);

/* Désactive le mode PLL */
LL_RCC_PLL_Disable();
LL_RCC_PLL_DisableDomain_SYS();

/*Réglage du Voltage Scaling en fonction de expe si ==2 on laisse le réglage par défaut sinon on met à 2  */
if (expe!=2)
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);




//while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
	//{ };

/* Set APB1 & APB2 prescaler*/
LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

// Set MSI clock to 24MHz
LL_RCC_MSI_EnableRangeSelection();
while(LL_RCC_MSI_IsEnabledRangeSelect()!=1)
{};
LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_9);
/* Update the global variable called SystemCoreClock */
SystemCoreClockUpdate();
}


//----------------------------------------------------------------------

/**
  * @brief  RTC Configuration
  *         The RTC is configured as follows :
  *         Au démarrage "à froid"
					- le démarrage de l'oscillateur LSE (Low Speed External = quartz 32768 Hz), alimenté par le backup-domain
					- le reset du backup-domain (ce qui va notamment mettre à zéro les backup-registers).
					- la config des 2 prescalers du RTC (AsynchPrescaler et SynchPrescaler). Attention, les registres de configuration du RTC sont protégés en écriture ! Il est donc nécessaire de désactiver cette protection avant de pouvoir entrer en mode initialisation. Il faudra sortir du mode initialisation et remettre la protection en écriture à l'issue de la confiuration du RTC.
			Aux démarrages "à chaud" le RTC est supposée déjà fonctionner
 	 	 				- initialisation de l'interface RTC-MPU avant de tenter l'accés aux backup-registers
  *
  *
  * @param  None
  * @retval None
  */


void RTC_Config(void){
	if (LL_RCC_LSE_IsReady()== 1){
	//if (((RCC->BDCR)&(RCC_BDCR_LSEON))==RCC_BDCR_LSEON) { // cas du démarrage à chaud
			// le RTC est supposée déjà fonctionner, mais l'interface RTC-MPU n'est pas actif, il faut l'initialiser avant de tenter l'accés aux backup-registers
			LL_APB1_GRP1_EnableClock( LL_APB1_GRP1_PERIPH_PWR );
			LL_PWR_EnableBkUpAccess();

		}else { // cas du démarrage à froid

			//Alimente
			LL_PWR_EnableBkUpAccess();

			// reset du back-up domain :
			LL_RCC_ForceBackupDomainReset();
			LL_RCC_ReleaseBackupDomainReset();

			// ??
			LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);

			// démarre oscillateur LSE
			LL_RCC_LSE_Enable();

			while(LL_RCC_LSE_IsReady() != 1);

			//Set la clock du RCC sur LSE
			LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);

			// Peripheral clock enable
			LL_RCC_EnableRTC();

			// désactive la protection en écriture du registre RTC
			LL_RTC_DisableWriteProtection(RTC);

			//Entrée dans le mode d'initialisation
			if(!LL_RTC_EnterInitMode(RTC));

			// set les 2 prescaler avec 1 valeur à changer ????
			LL_RTC_SetAsynchPrescaler(RTC,128);
			LL_RTC_SetSynchPrescaler(RTC,256);

			//
			LL_RTC_DisableInitMode(RTC);
			while(LL_RTC_IsActiveFlag_INIT(RTC));

			LL_RTC_ClearFlag_RS(RTC);
			while(!LL_RTC_IsActiveFlag_RS(RTC));

			// Démarre la clock RTC
			//LL_RTC_EnableIT_TS(RTC);
			//LL_RTC_TS_Enable(RTC);

			// réactive la protection en écriture du registre RTC
			LL_RTC_EnableWriteProtection(RTC);

			// je stocke la variable expe au premier tour qui vaut 1
			expe=1;
			LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, expe);
		}
}

//----------------------------------------------------------------------
	/**
	  * @brief This function handles System tick timer.
	  */
void SysTick_Handler(void)
	{
	  /* USER CODE BEGIN SysTick_IRQn 0 : gestion du clignotement de la LED 2 sec de période   (0,5 Hz de féquence) et 50ms*expe de durée active   */
		counter ++ ;
		if ((counter>0) &&  (counter<= 5*expe))
			LED_GREEN(1);

		if ((counter> 5*expe) && (counter<200))
			LED_GREEN(0);

		if (counter>200)
			counter=0;

	  /* USER CODE END SysTick_IRQn 0 */

	  /* USER CODE BEGIN SysTick_IRQn 1 : détection de la transition repos-> pressé du bouton bleu*/
		if (BLUE_BUTTON()){ // à regarder si c'est bien la détection repos->pressé
			bluemode=1;

		}


	  /* USER CODE END SysTick_IRQn 1 */

	  /* USER CODE BEGIN SysTick_IRQn 2 : gestion de l'action du GPIO PC10 sortie 50 Hz */
		if (counter%2)
			PWM_50Hz(1);
		else
			PWM_50Hz(0);

	  /* USER CODE END SysTick_IRQn 2 */


	}

//----------------------------------------------------------------------
	// partie commune a toutes les utilisations du wakeup timer
static void RTC_wakeup_init( int delay )
	{
	LL_RTC_DisableWriteProtection( RTC );
	LL_RTC_WAKEUP_Disable( RTC );
	while	( !LL_RTC_IsActiveFlag_WUTW( RTC ) )
		{ }
	// connecter le timer a l'horloge 1Hz de la RTC
	LL_RTC_WAKEUP_SetClock( RTC, LL_RTC_WAKEUPCLOCK_CKSPRE );

	// fixer la duree de temporisation
	LL_RTC_WAKEUP_SetAutoReload( RTC, delay );	// 16 bits
	LL_RTC_ClearFlag_WUT(RTC);
	LL_RTC_EnableIT_WUT(RTC);
	LL_RTC_WAKEUP_Enable(RTC);
	LL_RTC_EnableWriteProtection(RTC);
	}

//----------------------------------------------------------------------
	// Dans le cas des modes STANDBY et SHUTDOWN, le MPU sera reveille par reset
	// causé par 1 wakeup line (interne ou externe) (le NVIC n'est plus alimenté)
	// delay se configure en seconde à condition que l'horloge du wake up timer soit à 1Hz comme la RTC
void RTC_wakeup_init_from_standby_or_shutdown( int delay )
	{
	RTC_wakeup_init( delay );
	// enable the Internal Wake-up line
	LL_PWR_EnableInternWU();	// ceci ne concerne que Standby et Shutdown, pas STOPx
	}

	// Dans le cas des modes STOPx, le MPU sera reveille par interruption
	// le module EXTI et une partie du NVIC sont encore alimentes
	// le contenu de la RAM et des registres étant préservé, le MPU
	// reprend l'execution après l'instruction WFI

//----------------------------------------------------------------------
void RTC_wakeup_init_from_stop( int delay )
	{
	RTC_wakeup_init( delay );
	// valider l'interrupt par la ligne 20 du module EXTI, qui est réservée au wakeup timer
	LL_EXTI_EnableIT_0_31( LL_EXTI_LINE_20 );
	LL_EXTI_EnableRisingTrig_0_31( LL_EXTI_LINE_20 );
	// valider l'interrupt chez NVIC
	NVIC_SetPriority( RTC_WKUP_IRQn, 1 );
	NVIC_EnableIRQ( RTC_WKUP_IRQn );
	}

//----------------------------------------------------------------------
	// wakeup timer interrupt Handler (inutile mais doit etre defini)
void RTC_WKUP_IRQHandler()
	{
	LL_EXTI_ClearFlag_0_31( LL_EXTI_LINE_20 );
	//bluemode=0;
	}

//----------------------------------------------------------------------
/**
  * @brief  Low Power mode configuration avec le wakeup timer réglé à 20 sec
  *
  * @param   @arg @ref LL_PWR_MODE_STOP0
  *         @arg @ref LL_PWR_MODE_STOP1
  *         @arg @ref LL_PWR_MODE_STOP2
  *         @arg @ref LL_PWR_MODE_STANDBY
  *         @arg @ref LL_PWR_MODE_SHUTDOWN
  * @retval None
  */

void Init_Low_Power_Mode(uint32_t LowPowerMode){

	if ((LowPowerMode == LL_PWR_MODE_STOP0) || (LowPowerMode == LL_PWR_MODE_STOP1) || (LowPowerMode == LL_PWR_MODE_STOP2)){
		RTC_wakeup_init_from_stop(10);
	}

	if ((LowPowerMode == LL_PWR_MODE_STANDBY) || (LowPowerMode == LL_PWR_MODE_SHUTDOWN)){
			RTC_wakeup_init_from_standby_or_shutdown(10);
		}



}

void Set_Low_Power_Mode(uint32_t LowPowerMode){
	// Activate the low power mode desired
		// set the SLEEPDEEP bit
		LL_LPM_EnableDeepSleep();
		// Set LPMS in the PWR_CR1 register
		LL_PWR_SetPowerMode(LowPowerMode);


}
