#ifndef HAL_IRDA_H
#define HAL_IRDA_H

void hal_irda_set_usrcode(uint16_t ir_usercode);
int hal_irda_get_key(void *buffer, uint32_t size, uint32_t timeout);
void hal_irda_init_app(void);

#endif /*HAL_IRDA_H*/
