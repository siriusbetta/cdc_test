
#include "compiler.h"
#include "preprocessor.h"
#include "board.h"
#include "gpio.h"
#include "sysclk.h"
#include "sleepmgr.h"
#include "conf_usb.h"
#include "udd.h"
#include "udc.h"
#include "udi_cdc.h"

#include "delay.h"
#include "usart.h"
static volatile bool main_b_cdc_enable = false;
static usart_options_t usart_options;


ISR(usart_interrupt, USART_IRQ_GROUP, 3)
{
}

/*! \brief Main function. Execution starts here.
 */
int main(void)
{
	irq_initialize_vectors();
	cpu_irq_enable();

	// Initialize the sleep manager
	sleepmgr_init();

	sysclk_init();
	board_init();
	
	
	delay_init(FOSC0);
	// Start USB stack to authorize VBus monitoring
	udc_start();

	if (!udc_include_vbus_monitoring()) {
		// VBUS monitoring is not available on this product
		// thereby VBUS has to be considered as present
		main_vbus_action(true);
	}
	// The main loop manages only the power mode
	// because the USB management is done by interrupt
	
	while (true) {
	
		int value;
		// Transfer UART RX fifo to CDC TX
		if (udi_cdc_is_rx_ready()) {
			// Transmit next data
		
		}else{
			value = udi_cdc_getc();
			delay_ms(1);
			udi_cdc_putc(value);
		}
			
	}
	
}

void main_vbus_action(bool b_high)
{
	if (b_high) {
		// Attach USB Device
		udc_attach();
	} else {
		// VBUS not present
		udc_detach();
	}
}

bool main_cdc_enable(void)
{
	main_b_cdc_enable = true;
	// Open communication
	uart_open();
	return true;
}

void main_cdc_disable(void)
{
	main_b_cdc_enable = false;
	// Close communication
	uart_close();
}

void uart_rx_notify(void)
{
	// If UART is open
	if (USART->imr & AVR32_USART_IER_RXRDY_MASK) {
	}
}

void uart_config(usb_cdc_line_coding_t * cfg)
{
	uint32_t stopbits, parity;
	uint32_t imr;

	switch (cfg->bCharFormat) {
	case CDC_STOP_BITS_2:
		stopbits = USART_2_STOPBITS;
		break;
	case CDC_STOP_BITS_1_5:
		stopbits = USART_1_5_STOPBITS;
		break;
	case CDC_STOP_BITS_1:
	default:
		// Default stop bit = 1 stop bit
		stopbits = USART_1_STOPBIT;
		break;
	}

	switch (cfg->bParityType) {
	case CDC_PAR_EVEN:
		parity = USART_EVEN_PARITY;
		break;
	case CDC_PAR_ODD:
		parity = USART_ODD_PARITY;
		break;
	case CDC_PAR_MARK:
		parity = USART_MARK_PARITY;
		break;
	case CDC_PAR_SPACE:
		parity = USART_SPACE_PARITY;
		break;
	default:
	case CDC_PAR_NONE:
		parity = USART_NO_PARITY;
		break;
	}
}


void uart_open(void)
{
	// Enable interrupt with priority higher than USB
	irq_register_handler(usart_interrupt, USART_IRQ, 3);
}

void uart_close(void)
{
}

