#ifndef __MYDRIVERIO_H__
#define __MYDRIVERIO _H__

#include <linux/ioctl.h>

#define MAGIC_NUM 0xE1
#define IOC_GET _IOR(MAGIC_NUM, 0, int)
#define IOC_SET _IO (MAGIC_NUM, 1)

#endif