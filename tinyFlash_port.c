#include "tinyFlash_port.h"

//Flash写入函数
int _flash_write(unsigned long addr, unsigned long len, unsigned char *buf)
{
    unsigned long tmp = addr & 0xff;

    if(tmp + len > 0x100) //跨扇区写入
    {
        flash_write_page(addr, 0x100 - tmp, buf);

        tmp = (0x100 - tmp);

        len -= tmp;

        buf += tmp;

        addr &= 0xffffff00;

        addr += 0x100;
    }

    flash_write_page(addr, len, buf);

    return 0;
}

int _flash_read(unsigned long addr, unsigned long len, unsigned char *buf)
{
    flash_read_page(addr, len, buf);
    return 0;
}

int _flash_sector_erase(unsigned long addr)
{
    flash_erase_sector(addr);
    return 0;
}