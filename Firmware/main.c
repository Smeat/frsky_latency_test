/*
This file is part of rx_latency_test.

ppm2usb is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ppm2usb is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ppm2usb.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "STM32F1/Inc/stm32f1xx_hal_conf.h"

#include "timer.h"

#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#include "xprintf.h"
#endif

//#define TEST_SBUS
#define TEST_PPM

#ifdef TEST_SBUS
#include "sbus.h"
uint8_t uart_data;
extern UART_HandleTypeDef huart3;
#endif

#ifdef TEST_PPM
extern TIM_HandleTypeDef htim1;

#define TX_CHANNELS 8

uint32_t ppm_data[TX_CHANNELS];
uint32_t last_ppm_time = 0;

uint8_t curr_channel = 0;

uint64_t ppm_frame_count = 0;

#endif

extern void HAL_Delay(uint32_t time);



#ifdef DEBUG_PRINT
void send_command(int command, const void *message)
{
   asm("mov r0, %[cmd];"
       "mov r1, %[msg];"
       "bkpt #0xAB"
         :
         : [cmd] "r" (command), [msg] "r" (message)
         : "r0", "r1", "memory");
}

char buf[512];
uint16_t buffPos = 0;
// dirty hack but its faster and works
void serial_putc( uint8_t c ){

	buf[buffPos++] = c;

	if(c == '\n'){
		buf[buffPos++] = 0;
		send_command(0x04, buf);
		buffPos = 0;
	}

}
#endif

#ifdef TEST_SBUS

void read_sbus_frame(){
	while(sbusFrameStatus() != RX_FRAME_COMPLETE){
		HAL_UART_Receive(&huart3, &uart_data, 1, 1000);
		sbusDataReceive(uart_data);
	}
}
#endif

#ifdef TEST_PPM

int isSyncFrame(uint32_t time){
	return time > 5000;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	uint32_t now = __HAL_TIM_GET_COUNTER(htim);
	uint32_t ppm = 0;
	if(now < last_ppm_time){ //timer overflowed
		ppm = now - last_ppm_time + 65536;
	}
	else {
		ppm = now - last_ppm_time;
	}

	last_ppm_time = now;

	if(isSyncFrame(ppm)){
		curr_channel = 0;
		++ppm_frame_count;
	}
	else {
		ppm_data[curr_channel] = ppm;
		++curr_channel;
		if(curr_channel == TX_CHANNELS){
			curr_channel = 0;
		}
	}
}
#endif

int main(){

	init_stm32cube();

#ifdef TEST_PPM
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
#endif


#ifdef DEBUG_PRINT
	xdev_out(serial_putc);
#endif
	while(42){
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_SET); //wait for pin to get low
		uint16_t data = 0;
		uint16_t frame_count = 0;
		uint32_t now = micros();

#ifdef TEST_PPM
		uint64_t start_frame_count = ppm_frame_count;
#endif
		while(data < 1700){
			//read next frame
#ifdef TEST_SBUS
			read_sbus_frame();
			data = sbusReadRawRC(7);
			++frame_count;
#endif
#ifdef TEST_PPM
			data = ppm_data[7];
#endif
		}
		uint32_t time = micros() - now;

#ifdef TEST_PPM
			frame_count = ppm_frame_count - start_frame_count + 1;
#endif
#ifdef DEBUG_PRINT
		if(frame_count > 1){
			xprintf("Time is %dus chan 8 %d. Delay is %d frames.\n", time, data, frame_count-1);
		}
#endif
	}
}
