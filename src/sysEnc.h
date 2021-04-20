/*
 * sysEnc.h
 *
 * Created: 15.04.2021 21:02:40
 *  Author: Pawel Rogoz
 */ 


#ifndef SYSENC_H_
#define SYSENC_H_

#include <avr/io.h>

void Enc_setup(void);
void Enc_handling(void);
void Enc_isrA(void);
void Enc_isrB(void);
void Enc_isrE(void);

#endif /* SYSENC_H_ */