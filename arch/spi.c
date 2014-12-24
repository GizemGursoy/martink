/**
	Immediate mode (no interrupts) SPI driver

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#include <arch/soc.h>

#include "gpio.h"
#include "spi.h"
#include "interface.h"

struct spi_dev {
	uint8_t id;
	struct serial_if *serial;
};

static struct spi_dev _spi[4] = {
	{.id = 0, .serial = 0},
	{.id = 1, .serial = 0},
	{.id = 2, .serial = 0},
	{.id = 3, .serial = 0}
};

DECLARE_STATIC_CBUF(spi0_rx_buf, uint8_t, 4); 

#define GET_SPI_DEV(s, dev) \
	struct spi_dev *dev = container_of(s, struct spi_dev, serial)
	
uint16_t _spi_putc(serial_dev_t self, uint8_t ch){
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: {
			uint8_t data = hwspi0_transfer(ch); 
			cbuf_put(&spi0_rx_buf, data);
			break;
		}
	}
	return 0; 
}

uint16_t _spi_getc(serial_dev_t self) {
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: 
			if(!cbuf_get_data_count(&spi0_rx_buf)) return SERIAL_NO_DATA; 
			return cbuf_get(&spi0_rx_buf);
			break;
	}
	return -1; 
}

size_t _spi_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: 
			while(sz--){
				hwspi0_transfer(*data++); 
			}
			break;
	}
	return size; 
}

size_t _spi_getn(serial_dev_t self, uint8_t *data, size_t sz){
	size_t count = 0;
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: 
			while(sz--){
				*data = hwspi0_transfer(0); 
				data++; 
				count++; 
			}
			break;
	}
	return count; 
}

size_t _spi_waiting(serial_dev_t self){
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: 
			return cbuf_get_data_count(&spi0_rx_buf); 
			break;
	}
	return 0; 
}

void _spi_flush(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
}

serial_dev_t spi_get_serial_interface(uint8_t dev){
	dev &= 0x03;
	static struct serial_if _if;
	_if = (struct serial_if) {
		.put = _spi_putc,
		.get = _spi_getc,
		.putn = _spi_putn,
		.getn = _spi_getn,
		.flush = _spi_flush,
		.waiting = _spi_waiting
	}; 
	_spi[dev].serial = &_if;
	return &_spi[dev].serial; 
}

static void __init _spi_init(void){
	kdebug("SPI: starting interface: ");
#ifdef CONFIG_HAVE_SPI0
	hwspi0_init_default(); kdebug("0 "); 
#endif
#ifdef CONFIG_HAVE_SPI1
	hwspi1_init_default(); kdebug("1 "); 
#endif
#ifdef CONFIG_HAVE_SPI2
	hwspi2_init_default(); kdebug("2 "); 
#endif
	kdebug("\n");
}
