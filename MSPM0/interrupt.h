#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

extern uint8_t enable_group1_irq;
extern uint8_t imu_ready;

void Interrupt_Init(void);

#endif  /* #ifndef _INTERRUPT_H_ */