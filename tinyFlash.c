#include "tinyFlash.h"

static unsigned char _buf[TINY_BUFFER_SIZE] = { 0 };

static unsigned long tinyFlash_Used_Addr = 0; //当前使用的扇区地址
static unsigned long tinyFlash_Swap_Addr = 0; //当前未使用的扇区地址

bool tinyFlash_Init()
{
    memset(_buf, 0 , TINY_BUFFER_SIZE);

    _flash_read(TINY_START_ADDR, TINY_BUFFER_SIZE, _buf);

    if(_buf[0] != 0XFF)  //第一个扇区在使用
    {
        tinyFlash_Used_Addr = TINY_START_ADDR;
        tinyFlash_Swap_Addr = TINY_START_ADDR + TINY_SECTOR_SIZE;
    }
    else //第二个扇区在使用
    {
        tinyFlash_Used_Addr = TINY_START_ADDR + TINY_SECTOR_SIZE;
        tinyFlash_Swap_Addr = TINY_START_ADDR;
    }

    return true;
}
		
int tinyFlash_Read(unsigned char KEY, unsigned char * outbuf, unsigned char * len)
{
    unsigned long _addr_start = tinyFlash_Used_Addr + TINY_SECHAD_SIZE;;
    unsigned long _addr_end   = tinyFlash_Used_Addr + TINY_SECTOR_SIZE;
    
    while(1)
    {
        if(_addr_start > _addr_end -3) //该扇区查找完毕
        {
            break;
        }
        
        _flash_read(_addr_start, TINY_BUFFER_SIZE, _buf);

        if(_buf[0] == KEY) //目标KEY
        {
            if(_buf[1] == (KEY ^ 0xFF)) //该KEY正在使用中
            {
                if(len == NULL)  //删除操作
                {
                    _buf[0] = 0;
                    _flash_write(_addr_start + 1 , 1 , _buf);
                }
                else if(outbuf == NULL) //读取长度
                {
                    *len = _buf[2];
                }
                else //读取数据
                {
                    memcpy(outbuf, _buf + 3, _buf[2]);
                    *len = _buf[2];
                }

                return 0;
            }
            else //该KEY已被删除
            {
                _addr_start += (_buf[2] + 3);
                continue;
            }
        }
        else if((_buf[0] != 0) && (_buf[0] != 0xff)) //其他KEY
        {
            _addr_start += (_buf[2] + 3);
            continue;
        }
        else //读取到扇区尾部未使用的区域
        {
            break;
        }
    }
    
    return -1;
}

int tinyFlash_Write(unsigned char KEY, unsigned char * buf, unsigned char len)
{
    tinyFlash_Read(KEY,  NULL, NULL);

    unsigned long _addr_start = tinyFlash_Used_Addr + TINY_SECHAD_SIZE;;
    unsigned long _addr_end   = tinyFlash_Used_Addr + TINY_SECTOR_SIZE;

    unsigned long dirty_data_len = 0;
        
    while(1)
    {
        if(_addr_start > _addr_end -3 - len) //该扇区已查找完毕，无可用空间
        {
            if(dirty_data_len > 0) //当前扇区中存在脏数据
            {
                tinyFlash_Swap(); //交换新旧扇区，清理脏数据

                _addr_start = tinyFlash_Used_Addr + TINY_SECHAD_SIZE;;
                _addr_end   = tinyFlash_Used_Addr + TINY_SECTOR_SIZE;
            }
            else //无可用空间
            {
                return -1;
            }
        }
        
        _flash_read(_addr_start, TINY_BUFFER_SIZE, _buf);

        if(_buf[0] == 0xFF) //该区域可使用
        {
            //at_print_hexstr(&_addr_start, 2);

            _buf[0] = KEY;
            _buf[1] = (KEY ^ 0xFF);
            _buf[2] = len;
            memcpy(_buf +3, buf, len);

            _flash_write(_addr_start, len + 3, _buf);//写入数据
            
            return 0;
        }
        else if(_buf[0] != 0) //已存储其他KEY
        {
            if(_buf[1] == 0) //脏数据
            {
                dirty_data_len += (_buf[2] + 3);
            }
            else if (_buf[1] == (KEY ^ 0xFF)) //当前要写入的Key
            {
                _buf[0] = 0;
                _flash_write(_addr_start + 1 , 1 , _buf);//删除数据
            }

            _addr_start += (_buf[2] + 3);

            continue;
        }
        else //读取到错误的数据
        {
            break;
        }
    }
    
    return 1;
}

void tinyFlash_Swap() //扇区使用完了，需要清理数据，才能存储别的数据
{
    unsigned long _addr_start = tinyFlash_Used_Addr + TINY_SECHAD_SIZE; //当前使用的扇区的起始地址
    unsigned long _addr_end   = tinyFlash_Used_Addr + TINY_SECTOR_SIZE; //当前使用的扇区的结束地址

    unsigned long _new_addr_start = tinyFlash_Swap_Addr + TINY_SECHAD_SIZE; //将要使用的扇区的起始地址
        
    unsigned long tmp = 0;

    while(1)
    {
        if(_addr_start > _addr_end -3) //该扇区已查找完毕，无可用空间
        {
            break;
        }
        
        _flash_read(_addr_start, TINY_BUFFER_SIZE, _buf);

        if(_buf[0] == 0xFF) //数据转移完毕
        {
            break;
        }
        else if(_buf[0] == 0) //读取到了错误的数据
        {
            break;
        }
        else //读取到正确的Key数据
        {
            _addr_start += (_buf[2] + 3);

            if(_buf[1] == (_buf[0] ^ 0xFF)) //数据仍有效(未删除)
            {
                tmp = _new_addr_start & 0xff;

                _flash_write(_new_addr_start, _buf[2] + 3, _buf);
                _new_addr_start += (_buf[2] + 3);
            }
            continue;
        }
    }

    _buf[0] = 0xaa;
    _flash_write(tinyFlash_Swap_Addr, 1, _buf); //将新扇区标记为在使用
    _flash_sector_erase(tinyFlash_Used_Addr);        //擦除旧扇区

    _new_addr_start = tinyFlash_Swap_Addr;

    tinyFlash_Swap_Addr = tinyFlash_Used_Addr;
    tinyFlash_Used_Addr = _new_addr_start;
}

/*擦除所有扇区*/
void tinyFlash_Format(void)
{
    _flash_sector_erase(tinyFlash_Used_Addr);//擦除旧扇区
    _flash_sector_erase(tinyFlash_Swap_Addr);//擦除旧扇区
} 