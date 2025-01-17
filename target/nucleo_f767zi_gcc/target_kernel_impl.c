/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2016 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 */

/*
 * ターゲット依存モジュール（NUCLEO_F767ZI用）
 */
#include "kernel_impl.h"
#include <sil.h>

#include "tPutLogTarget_tecsgen.h" //出力テスト用

/*
 *  起動直後の初期化(system_stm32f4xx.c)
 */
extern void SystemInit(void);

/*
 *  クロックの初期化(systemclock_config.c)
 */
extern void SystemClock_Config(void);

/*
 *  バナー出力用のUARTの初期化
 */
static void usart_early_init(void);

/*
 *  エラー時の処理
 */
extern void Error_Handler(void);

/*
 *  起動時のハードウェア初期化処理
 */
void
hardware_init_hook(void) {
	SystemInit();

	/*
	 *  -fdata-sectionsを使用するとistkが削除され，
	 *  cfgのパス3のチェックがエラーとなるため，
	 *  削除されないようにする 
	 */
	SystemCoreClock = (uint32_t)istk;
}

/*
 * ターゲット依存部 初期化処理
 */
void
target_initialize(void)
{

	/*
	 * キャッシュの有効化
	 * CPU_CACHE_Enable() : STM32CubeF7/Projects/STM32F767ZI-Nucleo/Templates/Src/main.cより引用
	 */
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();


	/*
	 *  HALによる初期化
	 *  HAL_Init() : stm32f7xx_hal.c の内容から必要な初期化のみ呼び出す．
	 */
//	__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
//	__HAL_FLASH_DATA_CACHE_ENABLE();
//	__HAL_FLASH_PREFETCH_BUFFER_ENABLE();

  __HAL_FLASH_ART_ENABLE();
  __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

	/*
	 *  クロックの初期化
	 */
	SystemClock_Config();

	/*
	 *  コア依存部の初期化
	 */
	core_initialize();

	/*
	 *  使用するペリフェラルにクロックを供給
	 */

	/*
	 *  UserLEDの初期化
	 */
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);

/* LED動作確認
		BSP_LED_On(LED1);
	volatile int loop;
	while(1){
		for(loop = 0; loop < 0x100000; loop++);
		BSP_LED_Toggle(LED1);
		BSP_LED_Toggle(LED2);
		BSP_LED_Toggle(LED3);
	}
*/

	/*
	 *  バナー出力用のシリアル初期化
	 */
	usart_early_init();

//	while(1){
//		ePutLog_putChar('a');
//	}
}

/*
 * ターゲット依存部 終了処理
 */
void
target_exit(void)
{
	/* チップ依存部の終了処理 */
	core_terminate();
	while(1);
}

static UART_HandleTypeDef UartHandle;

void
usart_early_init()
{
	usart_low_init();

	UartHandle.Instance          = USART_NAME; 
	UartHandle.Init.BaudRate     = BPS_SETTING;
//	UartHandle.Init.WordLength   = UART_WORDLENGTH_9B;
	UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits     = UART_STOPBITS_1;
//	UartHandle.Init.Parity       = UART_PARITY_ODD;
	UartHandle.Init.Parity       = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode         = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
  UartHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
	if(HAL_UART_Init(&UartHandle) != HAL_OK) {
		Error_Handler();
	}
	/* UART動作確認
	char msg[16];
	sprintf(msg, "hello!\r\n");
	HAL_UART_Transmit(&UartHandle, msg, strlen(msg), 0xffff);
	*/

};

/*
 * エラー発生時の処理
 */
void
Error_Handler(void){
	volatile int loop;
	BSP_LED_Init(LED2);
	while(1){
		for(loop = 0; loop < 0x100000; loop++);
		BSP_LED_Toggle(LED2);
	}
}

#include "time_event.h"

/*
 *  HAL実行用の関数
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  return HAL_OK;
}

#ifndef USE_ASP3_FOR_STM
uint32_t HAL_GetTick(void)
{
  return current_hrtcnt/1000;
}
#endif
