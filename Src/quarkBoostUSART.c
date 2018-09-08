#include "quarkBoostUSART.h"

/*
 * Data Connector
 * 	Pin 1 - Red - 	 12V
 * 	Pin 2 - Black -  GND
 * 	Pin 3 - Green -  CANH
 * 	Pin 4 - Yellow - CANL
 * Details:
 * USART Baudrate = 9600 kbit/s
 * USART Lenght = 8 bit
 * USART parity = 1 bit, even
 * USART stop bit = 1
 * USART1 = motor
 * USART2 = display
 */


#define FLASH_BACKUP_ADDR       ADDR_FLASH_PAGE_1   /* Address for distance backup and mode */


/* Buffers for display and motor data package*/ 
uint8_t dDat[DISP_SZ];
uint8_t mDat[MOTO_SZ];


uint32_t value = 0;
uint8_t flag = 0, flagB = 0;
uint32_t PAGEError = 0;
quarkBoostDef eBike;

uint8_t walkPressed = 0;
bool settings = false;
bool pressedReleased = true;



extern uint32_t modeResetCount;
extern uint32_t menuCounter;


/**
  * @brief  Initializes CAN1 and CAN2 for receiving in interrupt mode.
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN1 - Motor
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN1 - Display
  * @retval None
  */
void quarkBoostInit(UART_HandleTypeDef* USARThandle1, UART_HandleTypeDef* USARThandle2, quarkBoostDef* dataStruct)
{
  /* Read stored data from flash memory at boot (total distance, mode) */
  //flashRead(dataStruct);
	
	/* Turn on optocoupler*/
	HAL_Delay(50);
	HAL_GPIO_WritePin(TR_GPIO_Port, TR_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	
	/* Link UASAR 1 & 2 handles*/
  dataStruct->displayUSART = USARThandle2;
  dataStruct->motorUSART = USARThandle1;


	/* Set default parameters */
  dataStruct->mode = 1.f;
  dataStruct->totalDist = 32312;
	
	
	/* Start the Reception process and enable reception interrupt */
	HAL_UART_Receive_IT(dataStruct->motorUSART, mDat, MOTO_SZ);
	HAL_UART_Receive_IT(dataStruct->displayUSART, dDat, DISP_SZ);
	
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}


/**
  * @brief  Initializes CAN1 and CAN2 for receiving in interrupt mode.
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN1 - Motor
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN1 - Display
  * @retval None
  */
void quarkBoostMessage(UART_HandleTypeDef* USARThandle, quarkBoostDef* dataStruct)
{
	/* Data package from Display*/
	if(USARThandle->Instance == USART2)
		{
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			
			/* Check for MENU sequence */
			quarkBoostMenu(dDat, dataStruct);
			
			/* Manipulate motor data */
			manipulateData(mDat, dataStruct);
			
			/* Turn off sequence for motor */
			if(dDat[8] == 0xD0)HAL_GPIO_WritePin(TR_GPIO_Port, TR_Pin, GPIO_PIN_RESET);
			
			/* Calculate CRC value for modified motor data package */
			mDat[24] = calcCRC(mDat, MOTO_SZ);
			
			/* Send data package to Display*/
			HAL_UART_Transmit_IT(dataStruct->motorUSART, mDat, MOTO_SZ);
		
			/* Enable again Display USART interrupt*/
			HAL_UART_Receive_IT(dataStruct->displayUSART, dDat, DISP_SZ);
		
		}else
		{
			/* Enable again Motor USART interrupt*/
			HAL_UART_Receive_IT(dataStruct->motorUSART, mDat, MOTO_SZ);
		
		}
}

void quarkBoostScan(UART_HandleTypeDef* USARThandle, quarkBoostDef* dataStruct)
{
	
}

/**
  * @brief  Initializes CAN1 and CAN2 for receiving in interrupt mode.
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN1 - Motor
  * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN1 - Display
  * @retval None
  */
void manipulateData(uint8_t* data, quarkBoostDef* dataStruct)
{
	/* Manipulate Speed */
	if(settings)
	{
		value = (uint16_t)(round(2500.f * dataStruct->mode));
		mDat[9] = (uint8_t)value;
		mDat[8] = (uint8_t)(value >>8) | (mDat[8] & 0xF0);
	}else
	{
		value = (float)(uint16_t)(((0x01 & mDat[8]) << 8) | mDat[9]) * dataStruct->mode;
		mDat[9] = (uint8_t)value;
		mDat[8] = (uint8_t)(value >>8) | (mDat[8] & 0xF0);
	}
	
	/* Manipulate Distance */
	//
	// se pride
	//
	
	
}

void quarkBoostMenu(uint8_t* data, quarkBoostDef* dataStruct)
{
	/* Check if first byte and CRC match - tu se se doda preverjanje CRC-ja */
	if(dDat[1] == 0xFF)
	{
		/* 3s counter to exit settings menu*/
		if(menuCounter > 3000)
		{			
			settings = false;
			walkPressed = 0;
			modeResetCount = 0;
		}

		/* Check if WALK button is pressed */
		if((dDat[8] & 0x10) == 0x10) pressedReleased = false;
		else if(pressedReleased == false)
		{
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

			pressedReleased = true;

			walkPressed++;
			if(walkPressed == 3)
			{
				settings = true;
				walkPressed = 0;
			}

			menuCounter = 0;
		}
				
				
		/* Check if PLUS button is pressed */
		if(settings && ((dDat[7] & 0x01) == 0x01))
		{
			if(pressedReleased)
			{
				pressedReleased = false;
				
				/* if PLUS button pressed change mode value */
				if((uint16_t)(round(250.f * dataStruct->mode)) >= 500)dataStruct->mode += 0.4f;
				else											dataStruct->mode += 0.2f;
				if(dataStruct->mode > 4.f) dataStruct->mode = 4.00000048;
				
				menuCounter = 0;
			}else pressedReleased = true;
			
		/* Check if MINUS button is pressed */	
		}else if(settings && ((dDat[7] & 0x02) == 0x02))
		{
			if(pressedReleased)
			{
				pressedReleased = false;
				
				/* if MINUS button pressed change mode value */
				if((uint16_t)(round(250.f * dataStruct->mode)) > 500)dataStruct->mode -= 0.4f;
				else											dataStruct->mode -= 0.2f;
				if(dataStruct->mode < 0.6f) dataStruct->mode = 0.6f;
				
				menuCounter = 0;
			}else pressedReleased = true;
		}
	
	/*END of start byte & CRC check*/
	}
}


uint8_t calcCRC(uint8_t* b, uint8_t size)
{
	return ((b[0] - b[1] - b[2]- b[3]- b[4]- b[5]- b[6]- b[7]- b[8]- b[9]- b[10]-b[11]-b[12]- b[13]- b[14]- b[15]- b[16]- b[17]- b[18]- b[19]- b[20]- b[21]- b[22]- b[23]))%256+256;
}


/* CRC-32C (iSCSI) polynomial in reversed bit order. */
//#define POLY 0x82f63b78

/* CRC-32 (Ethernet, ZIP, etc.) polynomial in reversed bit order. */
/* #define POLY 0xedb88320 */
/*
uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len)
{
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return ~crc;
}
*/

