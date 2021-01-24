/* An STM32 HAL library written for the the MAX30100 pulse oximeter and heart rate sensor. */
/* Libraries by @eepj www.github.com/eepj */
#ifndef MAX30100_FOR_STM32_HAL_H
#define MAX30100_FOR_STM32_HAL_H
#include "main.h"
#include <string.h>
/*----------------------------------------------------------------------------*/
#define MAX30100_DEBUG						1
#define MAX30100_I2C_ADDR					0xae
#define MAX30100_TIMEOUT					HAL_MAX_DELAY
#define MAX30100_FIFO_SIZE					64
#define MAX30100_FIFO_SAMPLE				16
#define MAX30100_SAMPLE_SIZE				4
/*----------------------------------------------------------------------------*/
//Status registers
//Interrupt status
#define MAX30100_INTERRUPT					0x00
#define MAX30100_A_FULL						7
#define MAX30100_TMP_RDY					6
#define MAX30100_HR_RDY						5
#define MAX30100_SPO2_RDY					4
#define MAX30100_PWR_RDY					0
//Interrupt enable
#define MAX30100_INTERRUPT_ENB				0x01
#define MAX30100_ENB_A_FULL					7
#define MAX30100_ENB_TMP_RDY				6
#define MAX30100_ENB_HR_RDY					5
#define MAX30100_ENB_SPO2_RDY				4
//FIFO registers
#define MAX30100_FIFO_WR_PTR				0x02
#define MAX30100_OVF_COUNTER				0x03
#define MAX30100_FIFO_RD_PTR				0x04
#define MAX30100_FIFO_DATA					0x05
//Config registers
//Mode config
#define MAX30100_MODE_CONFIG				0x06
#define MAX30100_SHDN						7
#define MAX30100_RESET						6
#define MAX30100_TMP_EN						3
#define MAX30100_MODE						0
//SpO2 Config
#define MAX30100_SPO2_CONFIG				0x07
#define MAX30100_SPO2_HI_RES_ENB			6
#define MAX30100_SPO2_SR					2
#define MAX30100_LED_PW						0
//LED Config
#define MAX30100_LED_CONFIG					0x09
#define MAX30100_LED_RED_PA					4
#define MAX30100_LED_IR_PA					0
//Temperature registers
#define MAX30100_TMP_INTEGER				0x16
#define MAX30100_TMP_FRACTION				0x17
//Part ID registers
#define MAX30100_REVISION					0xfe
#define MAX30100_PART						0xff
/*----------------------------------------------------------------------------*/
typedef enum MAX30100_SpO2SR {
	//Upto 1600us, 16-bit res
	MAX30100_SPO2SR_50, MAX30100_SPO2SR_100,
	//Upto 800us, 15-bit res
	MAX30100_SPO2SR_167, MAX30100_SPO2SR_200,
	//Upto 400us, 14-bit res
	MAX30100_SPO2SR_400,
	//Cannot exceed 200us, 13-bit res
	MAX30100_SPO2SR_600, MAX30100_SPO2SR_800, MAX30100_SPO2SR_1000,
	MAX30100_SPO2SR_DEFAULT = 1,
} MAX30100_SpO2SR;

typedef enum MAX30100_LEDPulseWidth {
	//Upto 1000 samples/s, 13-bit res
	MAX30100_LEDPW_200,
	//Upto 1000 samples/s, 14-bit res
	MAX30100_LEDPW_400,
	//Upto 200 samples/s, 15-bit res
	MAX30100_LEDPW_800,
	//Cannot exceed 100 samples/s, 16-bit res
	MAX30100_LEDPW_1600,
	MAX30100_LEDPW_DEFAULT = 3,
} MAX30100_LEDPulseWidth;

typedef enum MAX30100_LEDCurrent {
	MAX30100_LEDCURRENT_0_0, MAX30100_LEDCURRENT_4_4, MAX30100_LEDCURRENT_7_6, MAX30100_LEDCURRENT_11_0,
	MAX30100_LEDCURRENT_14_2, MAX30100_LEDCURRENT_17_4, MAX30100_LEDCURRENT_20_8, MAX30100_LEDCURRENT_24_0,
	MAX30100_LEDCURRENT_27_1, MAX30100_LEDCURRENT_30_6, MAX30100_LEDCURRENT_33_8, MAX30100_LEDCURRENT_37_0,
	MAX30100_LEDCURRENT_40_2, MAX30100_LEDCURRENT_43_6, MAX30100_LEDCURRENT_46_8, MAX30100_LEDCURRENT_50_0,
	MAX30100_LEDCURRENT_DEFAULT = 15,
} MAX30100_LEDCurrent;

typedef enum MAX30100_Mode{
	MAX30100_IDLE_MODE = 0x00, MAX30100_HRONLY_MODE = 0x02, MAX30100_SPO2_MODE = 0x03,
}MAX30100_Mode;

extern I2C_HandleTypeDef *_max30100_ui2c;
extern UART_HandleTypeDef *_max30100_uuart;
extern uint8_t _max30100_it_byte;
extern uint8_t _max30100_mode;
extern uint8_t _max30100_mode_prev;
extern uint16_t _max30100_ir_sample[16];
extern uint16_t _max30100_red_sample[16];
extern uint8_t _max30100_ir_current;
extern uint8_t _max30100_red_current;
extern uint8_t _max30100_ir_current_prev;
extern uint8_t _max30100_red_current_prev;
extern float _max30100_temp;

void MAX30100_Init(I2C_HandleTypeDef *ui2c, UART_HandleTypeDef *uuart);

uint8_t MAX30100_ReadReg(uint8_t regAddr);
void MAX30100_WriteReg(uint8_t regAddr, uint8_t byte);

void MAX30100_EnableInterrupt(uint8_t a_full, uint8_t tmp_rdy, uint8_t hr_rdy, uint8_t spo2);
void MAX30100_InterruptHandler(void);

void MAX30100_SetMode(enum MAX30100_Mode mode);
void MAX30100_SetSpO2SampleRate(enum MAX30100_SpO2SR sr);
void MAX30100_SetLEDPulseWidth(enum MAX30100_LEDPulseWidth pw);
void MAX30100_SetLEDCurrent(enum MAX30100_LEDCurrent redpa, enum MAX30100_LEDCurrent irpa);

void MAX30100_ClearFIFO(void);
void MAX30100_ReadFIFO(void);
float MAX30100_ReadTemperature();

void MAX30100_PlotTemperatureToUART(UART_HandleTypeDef *uuart);
void MAX30100_PlotIrToUART(UART_HandleTypeDef *uuart, uint16_t *samples, uint8_t sampleSize);
void MAX30100_PlotBothToUART(UART_HandleTypeDef *uuart, uint16_t *samplesRed, uint16_t *samplesIr, uint8_t sampleSize);

void MAX30100_Stop(void);
void MAX30100_Pause(void);
void MAX30100_Resume(void);

#endif
