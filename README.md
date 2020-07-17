# MAX30100_for_STM32_HAL
An STM32 HAL library written for the MAX30100 pulse oximeter and heart rate sensor. 

**This library is ***NOT*** intended for clinical use. Proceed at your own risk**.

![Data read from MAX30100](./examples/example_heart_rate.bmp)

## To test
* Minimal setup:
```
                                                         _____________
                                                VDD 1 --|             |
                                               SCL1 2 --|             |
                                               SDA1 3 --|   __        |
                                               #INT 4 --|  |__|       |
                                                N/C 5 --|  |O |       |
                                                N/C 6 --|  |__|       |
                                                GND 7 --|_____________|  
                                   
 ```
 ### STM32CubeMX setup
* In STM32CubeMX, setup I2C1 to "I2C" and USART1 to "Asynchronous"
* Setup an external interrupt pin (say PB0) in GPIO settings, use "external interrupt mode with falling edge trigger detection" and "pull-up" settings.
* Activiate the external interrupt in NVIC settings by checking the corresponding box.
* Connect the #INT pin of your MAX30100 to this external interrupt pin.
* Save and generate code.

### IDE setup
* In your IDE, include [`max30100_for_stm32_hal.h`](./max30100_for_stm32_hal.h) and [`max30100_for_stm32_hal.c`](./max30100_for_stm32_hal.c).

### Main program
* In `main.c` (main program body file), include [`max30100_for_stm32_hal.h`](./max30100_for_stm32_hal.h).
* In `main.c`, before the main loop and after the system inits, add the following lines:
 ```ruby
MAX30100_Init(&hi2c1, &huart1);
MAX30100_SetSpO2SampleRate(MAX30100_SPO2SR_DEFAULT);
MAX30100_SetLEDPulseWidth(MAX30100_LEDPW_DEFAULT);
MAX30100_SetLEDCurrent(MAX30100_LEDCURRENT_DEFAULT, MAX30100_LEDCURRENT_DEFAULT);
MAX30100_SetMode(MAX30100_SPO2_MODE);
 ```
 ### Interrupt service routine
* In `stm32f*xx_it.c` (interrupt service routines file), include [`max30100_for_stm32_hal.h`](./max30100_for_stm32_hal.h).
* In `stm32f*xx_it.c`, add the line between private user functions tags:
```ruby
extern void MAX30100_InterruptCallback(void);
```
* In the corresponding interrupt handler function, add the line:
```ruby
MAX30100_InterruptCallback();
```
* Complie and flash `main.c` to microcontroller.
* Read the results from a UART monitor/plotter.
* Refer to [`./examples`](./examples) and [datasheets](https://www.maximintegrated.com/en/products/sensors/MAX30100.html) for further information.

## Sidenote
* If you're using an MAX30100 module and it cannot be detected by the microcontroller, desoldering the pull-up resistors on the module may resolve the problem. Details [here](https://www.teachmemicro.com/max30100-arduino-heart-rate-sensor/).

## Known issues
* Nil.
