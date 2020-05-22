# tinyFlash

一种轻量级的flash数据存储方案

## 设计原理

本方案采用两个扇区轮流使用的方法存储数据，每个扇区4096字节，扇区状态如下表：

|扇区编号|使用状态|数据分布|
|--------|----|---|
|扇区一 |使用中|0xAA ·······················································|
|扇区二 |未使用|0xFF ·······················································|

使用中的扇区数据分布如下表：

|扇区头部|K1|V1|K2|V2|.....|Kn|Vn|
|--------|----|---|----|----|----|-----|-----|
|32字节  |3字节|n字节|3字节|n字节|.....|3字节|n字节|

每个K-V数据存储形式如下表：

|第一字节|第二字节|第三字节|n个字节|
|--------|----|---|----|
|Key  |~Key|Len|Len个字节数据内容|

第一个字节存放的是数据的Key，取值范围是 0x01 - 0xFE

第二个字节的数据为Key取反，如果第二个字节的数据为0x00，表示该处存储的数据已被废弃

第三个自己表示数据长度，取值范围是0x01 - 0xFF

之后的字节是数据的内容，最长不得超过0xFF

## 使用示例

    #define My_DATA_INDEX = 1

    tinyFlash_Init();
    char * _data = "Hello World";

    tinyFlash_Write(My_DATA_INDEX, _data, sizeof(_data));

    char _buf[128] = { 0 }
    unsigned char _len = 128;

    tinyFlash_Read(My_DATA_INDEX, _buf, &_len);

    printf(_buf);



## API介绍

-------------------------------------------------------------------------------
    bool tinyFlash_Init();

函数功能：初始化tinyFlash

返回值：true 成功

-------------------------------------------------------------------------------
    int tinyFlash_Read(unsigned char KEY, unsigned char * buf, unsigned char * len);

函数功能：从tinyFlash中读取Key中存储的数据内容，数据长度，或者删除数据

参数 KEY：要读取的Key

参数 buf：Key中存储的数据内容的传出指针，如果该指针为NULL，则只传输数据长度

参数 len: 传入的是最大能接受的数据长度，传输的是读取到的实际数据长度，如果为NULL则为删除该KEY

返回值： 0 操作成功，其他值 操作失败

------------------------------------------------------------------------------
    int tinyFlash_Write(unsigned char KEY, unsigned char * buf, unsigned char len);

函数功能： 向tinyFlash中存储一个数据

参数 KEY： 要存储的数据的Key值

参数 buf： 要存储的数据内容

参数 len:  要存储的数据长度

返回值：0 操作成功，其他值 操作失败

------------------------------------------------------------------------------
    void tinyFlash_Format(void);

函数功能：格式化tinyFlash数据区域，擦除全部tinyFlash扇区

--------------------------------------------------------------------------------

## 移植及配置
移植只需要在```tinyFlash_port.c```中实现如下三个函数:

    int _flash_write(unsigned long addr, unsigned long len, unsigned char *buf);
    int _flash_read(unsigned long addr, unsigned long len, unsigned char *buf);
    int _flash_sector_erase(unsigned long addr);

源码中已根据TLSR825X平台实现了相关函数。

### 配置说明
所有的配置项都在```tinyFlash_port.h```中，配置示例如下：

    #define TINY_START_ADDR  0x7A000 //tinyFlash起始地址
    #define TINY_SECTOR_SIZE 4096 //flash扇区大小
    #define TINY_BUFFER_SIZE 256  //tiny缓冲区大小

    #define TINY_SECHAD_SIZE 32   //记录扇区使用情况的扇区头大小

## 接下来的工作
- 多扇区支持
- 数据加密
- 优化查找及存储速度
- 意外掉电防护