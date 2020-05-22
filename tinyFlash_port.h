/**************包含平台相关系统驱动头文件*********/
#include "tl_common.h"
#include "drivers.h"

/*********如下配置项需根据所在平台参数自行配置****/

#define TINY_START_ADDR  0x7A000 //tinyFlash起始地址
#define TINY_SECTOR_SIZE 4096 //flash扇区大小
#define TINY_BUFFER_SIZE 256  //tiny缓冲区大小

#define TINY_SECHAD_SIZE 32   //记录扇区使用情况的扇区头大小

/*********移植tinyFlash需提供如下函数************/
int _flash_write(unsigned long addr, unsigned long len, unsigned char *buf);
int _flash_read(unsigned long addr, unsigned long len, unsigned char *buf);
int _flash_sector_erase(unsigned long addr);