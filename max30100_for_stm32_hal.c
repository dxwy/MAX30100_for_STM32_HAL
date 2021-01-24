/* An STM32 HAL library written for the the MAX30100 pulse oximeter and heart rate sensor. */
/* Libraries by @eepj www.github.com/eepj */
#include "max30100_for_stm32_hal.h"
#include "main.h"
#ifdef __cplusplus
extern "C"{
#endif

I2C_HandleTypeDef *_max30100_ui2c;
UART_HandleTypeDef *_max30100_uuart;
uint8_t _max30100_it_byte = 0x00;
uint8_t _max30100_mode;
uint8_t _max30100_mode_prev;
uint16_t _max30100_ir_sample[16];
uint16_t _max30100_red_sample[16];
uint8_t _max30100_ir_current;
uint8_t _max30100_red_current;
uint8_t _max30100_ir_current_prev;
uint8_t _max30100_red_current_prev;
float _max30100_temp;

void MAX30100_Init(I2C_HandleTypeDef *ui2c, UART_HandleTypeDef *uuart){
	_max30100_ui2c = ui2c;
	_max30100_uuart = uuart;
	MAX30100_Stop();
	MAX30100_ClearFIFO();
}

uint8_t MAX30100_ReadReg(uint8_t regAddr){
	uint8_t reg = regAddr, result;
	HAL_I2C_Master_Transmit(_max30100_ui2c, MAX30100_I2C_ADDR, &reg, 1, MAX30100_TIMEOUT);
	HAL_I2C_Master_Receive(_max30100_ui2c, MAX30100_I2C_ADDR, &result, 1, MAX30100_TIMEOUT);
	return result;
}

void MAX30100_WriteReg(uint8_t regAddr, uint8_t byte){
	uint8_t reg[2] = { regAddr, byte };
	HAL_I2C_Master_Transmit(_max30100_ui2c, MAX30100_I2C_ADDR, reg, 2, MAX30100_TIMEOUT);
}

void MAX30100_EnableInterrupt(uint8_t a_full, uint8_t tmp_rdy, uint8_t hr_rdy, uint8_t spo2){
	uint8_t itReg = ((a_full & 0x01) << MAX30100_ENB_A_FULL) | ((tmp_rdy & 0x01) << MAX30100_ENB_TMP_RDY) | ((hr_rdy & 0x01) << MAX30100_ENB_HR_RDY) | ((spo2 & 0x01) << MAX30100_ENB_SPO2_RDY);
	MAX30100_WriteReg(MAX30100_INTERRUPT_ENB, itReg);
}

void MAX30100_InterruptHandler(void){
	uint8_t itReg = MAX30100_ReadReg(MAX30100_INTERRUPT);
	if((itReg >> MAX30100_A_FULL) & 0x01){
		MAX30100_ReadFIFO();
		if(_max30100_mode == MAX30100_HRONLY_MODE)
			MAX30100_PlotIrToUART(_max30100_uuart, _max30100_ir_sample, 16);
		else if(_max30100_mode == MAX30100_SPO2_MODE)
			MAX30100_PlotBothToUART(_max30100_uuart, _max30100_red_sample, _max30100_ir_sample, 16);
		MAX30100_SetMode(_max30100_mode);
	}else if((itReg >> MAX30100_TMP_RDY) & 0x01){
		_max30100_temp = MAX30100_ReadTemperature();
		MAX30100_EnableInterrupt(1, 0, 0, 0);
	}else if((itReg >> MAX30100_HR_RDY) & 0x01){

	}else if((itReg >> MAX30100_SPO2_RDY) & 0x01){

	}
}

void MAX30100_SetMode(enum MAX30100_Mode mode){
	_max30100_mode = mode;
	uint8_t modeReg = (MAX30100_ReadReg(MAX30100_MODE_CONFIG) & ~(0x07)) | ((mode << MAX30100_MODE) & 0x07);
	if(mode == MAX30100_SPO2_MODE)
		modeReg |= 0x08;
	else
		modeReg &= ~0x08;
	MAX30100_WriteReg(MAX30100_MODE_CONFIG, modeReg);
	if(_max30100_mode == MAX30100_SPO2_MODE)
		MAX30100_EnableInterrupt(0, 1, 0, 0);
	else if(_max30100_mode == MAX30100_HRONLY_MODE)
		MAX30100_EnableInterrupt(1, 0, 0, 0);
	else
		MAX30100_EnableInterrupt(0, 0, 0, 0);
}
//ok
void MAX30100_SetSpO2SampleRate(enum MAX30100_SpO2SR sr){
	uint8_t spo2Reg = MAX30100_ReadReg(MAX30100_SPO2_CONFIG);
	spo2Reg = ((sr << MAX30100_SPO2_SR) & 0x1c) | (spo2Reg & ~0x1c);
	MAX30100_WriteReg(MAX30100_SPO2_CONFIG, spo2Reg);
}
//ok
void MAX30100_SetLEDPulseWidth(enum MAX30100_LEDPulseWidth pw){
	uint8_t spo2Reg = MAX30100_ReadReg(MAX30100_SPO2_CONFIG);
	spo2Reg = ((pw << MAX30100_LED_PW) & 0x03) | (spo2Reg & ~0x03);
	MAX30100_WriteReg(MAX30100_SPO2_CONFIG, spo2Reg);
}
void MAX30100_SetLEDCurrent(enum MAX30100_LEDCurrent redpa, enum MAX30100_LEDCurrent irpa){
	_max30100_red_current = redpa;
	_max30100_ir_current = irpa;
	MAX30100_WriteReg(MAX30100_LED_CONFIG, (redpa << MAX30100_LED_RED_PA) | irpa);
}

void MAX30100_ClearFIFO(void){
	MAX30100_WriteReg(MAX30100_FIFO_WR_PTR, 0x00);
	MAX30100_WriteReg(MAX30100_FIFO_RD_PTR, 0x00);
	MAX30100_WriteReg(MAX30100_OVF_COUNTER, 0x00);
}

void MAX30100_ReadFIFO(void){
	//uint8_t fifo_wr_ptr = MAX30100_ReadReg(MAX30100_FIFO_WR_PTR);
	//uint8_t fifo_rd_ptr = MAX30100_ReadReg(MAX30100_FIFO_RD_PTR);
	uint8_t num_sample = 64;//(fifo_wr_ptr - fifo_rd_ptr) * 4;
	uint8_t fifo_data[64] = { 0 };
	uint8_t reg = MAX30100_FIFO_DATA;
	HAL_I2C_Master_Transmit(_max30100_ui2c, MAX30100_I2C_ADDR, &reg, 1, MAX30100_TIMEOUT);
	HAL_I2C_Master_Receive(_max30100_ui2c, MAX30100_I2C_ADDR, fifo_data, num_sample, MAX30100_TIMEOUT);
	for(uint8_t i = 0; i < num_sample; i += 4){
		_max30100_ir_sample[i / 4] = (fifo_data[i] << 8) | fifo_data[i + 1];
		_max30100_red_sample[i / 4] = (fifo_data[i + 2] << 8) | fifo_data[i + 3];
	}

}

float MAX30100_ReadTemperature(){
	int8_t tempInt = (int8_t) MAX30100_ReadReg(MAX30100_TMP_INTEGER);
	uint8_t tempFrac = MAX30100_ReadReg(MAX30100_TMP_FRACTION);
	return (tempInt + tempFrac / 16.0);
}

void MAX30100_PlotTemperatureToUART(UART_HandleTypeDef *uuart){
	uint8_t tempInt = _max30100_temp / 1;
	uint8_t tempFrac = (_max30100_temp - tempInt) * 10;
	char data[15];
	sprintf(data, "temp:%d.%d\n", tempInt, tempFrac);
	HAL_UART_Transmit(uuart, data, strlen(data), MAX30100_TIMEOUT);
}

void MAX30100_PlotIrToUART(UART_HandleTypeDef *uuart, uint16_t *samples, uint8_t sampleSize){
	char data[10];
	for(uint8_t i = 0; i< sampleSize; i++){
		sprintf(data, "s:%d\n", samples[i]);
		HAL_UART_Transmit(uuart, data, strlen(data), MAX30100_TIMEOUT);
	}
}

void MAX30100_PlotBothToUART(UART_HandleTypeDef *uuart, uint16_t *samplesRed, uint16_t *samplesIr, uint8_t sampleSize){
	char data[20];
	for(uint8_t i = 0; i< sampleSize; i++){
		sprintf(data, "red:%d\tir:%d\n", samplesRed[i], samplesIr[i]);
		HAL_UART_Transmit(uuart, data, strlen(data), MAX30100_TIMEOUT);
	}
}

void MAX30100_Stop(void){
	_max30100_mode = MAX30100_IDLE_MODE;
	MAX30100_SetLEDCurrent(MAX30100_LEDCURRENT_0_0, MAX30100_LEDCURRENT_0_0);
	MAX30100_WriteReg(MAX30100_INTERRUPT_ENB, 0x00);
}

void MAX30100_Pause(void){
	_max30100_mode_prev = _max30100_mode;
	_max30100_red_current_prev = _max30100_red_current;
	_max30100_ir_current_prev = _max30100_ir_current;
	MAX30100_SetLEDCurrent(MAX30100_LEDCURRENT_0_0, MAX30100_LEDCURRENT_0_0);
	MAX30100_SetMode(MAX30100_IDLE_MODE);
}

void MAX30100_Resume(void){
	MAX30100_SetLEDCurrent(_max30100_red_current_prev, _max30100_ir_current_prev);
	MAX30100_SetMode(_max30100_mode_prev);
}

#ifdef __cplusplus
}
#endif
