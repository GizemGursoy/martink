#include <arch/soc.h>

static const struct {
	GPIO_TypeDef *gpio; 
} _ports[] = {
	{
		.gpio = GPIOA
	}, 
	{
		.gpio = GPIOB
	}, 
	{
		.gpio = GPIOC
	},
	{
		.gpio = GPIOD
	}
}; 

void gpio_init_default(void){
	RCC_APB2PeriphClockCmd ( 
		RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
		RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE); 
}

void gpio_configure(gpio_pin_t p, uint16_t flags){
	uint32_t pin = p; 
	uint32_t reg = 0; 
	
	// set output 50mhz with push pull or floating input
	reg = (flags & GP_OUTPUT)?3:4; 
	
	// set open drain if it is selected (default push pull)
	reg |= ((flags & GP_OPEN_DRAIN)?1:0) << 2; 
	
	// set alternative function if selected 
	reg |= (((flags & GP_OUTPUT) && (flags & GP_AF))?1:0) << 3; 
	
	// set floating or pullup if input
	if(!(flags & GP_OUTPUT)) reg = (reg & 3) | (((flags & GP_PULLUP)?2:1) << 2); 
	
	// check for analog mode (resets the cnf bits)
	if(flags & GP_ANALOG) reg &= 3; 
	
	if((pin & 0xf) > 7) {
		uint32_t tmp = _ports[pin >> 4].gpio->CRH; 
		uint8_t shift = (((pin & 0xf) - 8) * 4); 
		_ports[pin >> 4].gpio->CRH = (tmp & ~(0xf << shift)) | (reg << shift); 
	} else {
		uint32_t tmp = _ports[pin >> 4].gpio->CRL; 
		uint8_t shift = ((pin & 0xf) * 4); 
		_ports[pin >> 4].gpio->CRL = (tmp & ~(0xf << shift)) | (reg << shift); 
	}
}

void gpio_write_pin(gpio_pin_t p, uint8_t val){
	uint32_t pin = p; 
	if(pin < 0 || pin >= GPIO_COUNT) return; 
	if(val) // the output is inverted for some reason?
		_ports[pin >> 4].gpio->BSRR = (1 << (pin & 0xf)); 
	else
		_ports[pin >> 4].gpio->BRR = (1 << (pin & 0xf)); 
}

uint8_t gpio_read_pin(gpio_pin_t p){
	uint32_t pin = p; 
	if(pin < 0 || pin >= GPIO_COUNT) return 0; 
	return _ports[pin >> 4].gpio->IDR & (1 << (pin & 0xf)); 
}

uint16_t gpio_get_status(gpio_pin_t pin, timestamp_t *t_up, timestamp_t *t_down){
	
}