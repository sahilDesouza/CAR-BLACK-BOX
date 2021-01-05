/* 
 * File:   adc.h
 */

#ifndef ADC_H
#define	ADC_H

#define CHANNEL0        0x00
#define CHANNEL1        0x01
#define CHANNEL2        0x02

void init_adc(void);
unsigned short read_adc(void);

#endif	/* ADC_H */

