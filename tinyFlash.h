#ifndef __tiny_Flash__
#define __tiny_Flash__

#include "tinyFlash_port.h"

//tinyFlash提供了如下API供应用程序使用
bool tinyFlash_Init();	
int  tinyFlash_Read(unsigned char KEY, unsigned char * buf, unsigned char * len);
int  tinyFlash_Write(unsigned char KEY, unsigned char * buf, unsigned char len);

#endif