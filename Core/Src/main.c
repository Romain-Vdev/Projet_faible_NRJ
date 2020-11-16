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

// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

#include "gpio.h"

// systick interrupt handler
/*void SysTick_Handler()
{

}*/
int expe = 1  ;
int bluemode=0;
int counter=0; // compteur utilisé dans le handler Systick pour gérer la LED
void Calibration_MSI_vs_LPE(void);
void SystemClock_Config(void);
void SystemClock_Config_MSI_24Mhz(void);
void RTC_Config(void);



int main(void)
{



// config GPIO
GPIO_init();

// config RTC
RTC_Config();

// lecture de expe dans le backup register

/* Configure the system clock en fonction de expe qui défini Voltage Scaling and Flash Latency et utilisation de la PLL   */
if (expe==1)
	SystemClock_Config(); // fonction à appeler si expe == 1
else
	SystemClock_Config_MSI_24Mhz();


// Fonction qui active la calibration MSI vs LSE en fonction de expe
Calibration_MSI_vs_LPE();


// init systick timer (tick period at 10 ms)
// le handler gère : la LED
// 					repos/pressé du bouton bleu
// 					action GPIO PC10

// init systick timer (tick period at 1 ms)
//LL_Init1msTick( SystemCoreClock );

// init systick timer (tick period at 10 ms)
LL_Init10msTick( SystemCoreClock );

while (1)  {
	if	( BLUE_BUTTON() )
		LED_GREEN(1);
	else {
		LED_GREEN(0);
		LL_mDelay(950);
		LED_GREEN(1);
		LL_mDelay(50);
		}
	}
}


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

/* Désactive le mode PLL */
LL_RCC_PLL_Disable();
LL_RCC_PLL_DisableDomain_SYS();

/*Réglage du Voltage Scaling en fonction de expe si ==2 on laisse le réglage par défaut sinon on met à 2  */
if (expe!=2)
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);


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

	if (LL_RCC_LSE_IsReady()) { // cas du démarrage à chaud
		// le RTC est supposée déjà fonctionner, mais l'interface RTC-MPU n'est pas actif, il faut l'initialiser avant de tenter l'accés aux backup-registers
		LL_APB1_GRP1_EnableClock( LL_APB1_GRP1_PERIPH_PWR );
		LL_PWR_EnableBkUpAccess();

	}else { // cas du démarrage à froid
		// démarre oscillateur LSE
		LL_RCC_LSE_Enable();

		// reset du back-up domain :
		LL_RCC_ForceBackupDomainReset();
		LL_RCC_ReleaseBackupDomainReset();

		// désactive la protection en écriture du registre RTC
		LL_RTC_DisableWriteProtection(RTC);

		// set les 2 prescaler avec 1 valeur à changer ????
		LL_RTC_SetAsynchPrescaler(RTC,1);
		LL_RTC_SetSynchPrescaler(RTC,1);

		// réactive la protection en écriture du registre RTC
		LL_RTC_EnableWriteProtection(RTC);



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
		if ((counter>0) &&  (counter<= 50*expe))
			LED_GREEN(1);

		if ((counter> 50*expe) && (counter<200))
			LED_GREEN(0);

		if (counter>200)
			counter=0;

	  /* USER CODE END SysTick_IRQn 0 */

	  /* USER CODE BEGIN SysTick_IRQn 1 : détection de la transition repos-> pressé du bouton bleu*/
		if (BLUE_BUTTON()) // à regarder si c'est bien la détection repos->pressé
			bluemode=1;

	  /* USER CODE END SysTick_IRQn 1 */

	  /* USER CODE BEGIN SysTick_IRQn 2 : gestion de l'action du GPIO PC10 sortie 50 Hz */

	  /* USER CODE END SysTick_IRQn 2 */


	}





