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

#ifndef TIMER_H_
#define TIMER_H_

extern uint32_t HAL_GetTick();

//TODO:stub
inline uint32_t micros(){
	return HAL_GetTick() * 1000;
}



#endif /* TIMER_H_ */
