/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __eBikeCAN_H
#define __eBikeCAN_H
#ifdef __cplusplus
 extern "C" {
#endif


 /* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "string.h"
#include "stm32f1xx_hal.h"
#include "stdbool.h"
#include "math.h"
	 
/* Base address of the Flash pages */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#define DISP_SZ 15 // Data lenght of Display package
#define MOTO_SZ 25 // // Data lenght of Motor package
	 

#define MODE     0x03B
#define BUTTON   0x131
#define SPEED    0x0D1
#define DISTANCE 0x0FF //0x202
#define RANGE		 0x202
#define DATE 	   0x210
#define TRIP		 0x221
/* Buttons */
#define PLUS 0x00
#define INFO 0x01
#define MINUS 	 0x02
#define WALK		 0x03


/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

typedef struct
{
	uint32_t functionalTest;		/* Flag indicator if functional test has passed */
	uint32_t totalDist;				/* Total distance saved in the flash memory */
	float mode;					/* Mode for limiting motor help - 1, 2, 3 */
	uint8_t lastMode;
	UART_HandleTypeDef* motorUSART;    /* CAN Handle Type definition for Motor */
	UART_HandleTypeDef* displayUSART;  /* CAN Handle Type definition for Display */
}quarkBoostDef;
/* Private define ------------------------------------------------------------*/


void quarkBoostInit(UART_HandleTypeDef* USARThandle1, UART_HandleTypeDef* USARThandle2, quarkBoostDef* dataStruct);
void quarkBoostMessage(UART_HandleTypeDef* USARThandle, quarkBoostDef* dataStruct);
void quarkBoostScan(UART_HandleTypeDef* USARThandle, quarkBoostDef* dataStruct);
void manipulateData(uint8_t* data, quarkBoostDef* dataStruct);
void quarkBoostMenu(uint8_t* data, quarkBoostDef* dataStruct);
	
HAL_StatusTypeDef flashWrite(quarkBoostDef* dataStruct);
HAL_StatusTypeDef flashRead(quarkBoostDef* dataStruct);
uint8_t calcCRC(uint8_t* b, uint8_t size);

#ifdef __cplusplus
}
#endif
#endif /*__eBikeCAN_H */

 /**
   * @}
   */

 /**
   * @}
   */
