#include "aos/aos.h"
#include "hal/soc/soc.h"

#include "intc_pub.h"
#include "icu_pub.h"
#include "irda_pub.h"
#include "gpio_pub.h"
#include "drv_model_pub.h"

void hal_irda_set_usrcode(uint16_t ir_usercode)
{
	set_irda_usrcode(ir_usercode);
}

int hal_irda_get_key(void *buffer, uint32_t size, uint32_t timeout)
{
	return IR_get_key(buffer, size, timeout);
}

void hal_irda_init_app(void)
{
	Irda_init_app();
}

