//#include <config.h>
#include <common.h>

#include <nand.h>

#define __REGb(x) (*(volatile unsigned char *)(x))

#define __REGw(x) (*(volatile unsigned short *)(x))

#define __REGi(x) (*(volatile unsigned int *)(x))

#define NF_BASE  0x4e000000



#define NFCONF  __REGi(NF_BASE + 0x0)

#define NFCONT  __REGi(NF_BASE + 0x4)

#define NFCMD  __REGb(NF_BASE + 0x8)

#define NFADDR  __REGb(NF_BASE + 0xc)

#define NFDATA  __REGb(NF_BASE + 0x10)

#define NFDATA16 __REGw(NF_BASE + 0x10)

#define NFSTAT  __REGb(NF_BASE + 0x20)

#define NFSTAT_BUSY (1 << 2)

#define nand_select() (NFCONT &= ~(1 << 1))

#define nand_deselect() (NFCONT |= (1 << 1))

#define nand_clear_RnB() (NFSTAT |= NFSTAT_BUSY)


 

static inline void nand_wait(void)

{

  int i;

 while (!(NFSTAT & NFSTAT_BUSY))

  for (i=0; i<10; i++);

}

/* configuration for 2440 with 2048byte sized flash */

#define NAND_5_ADDR_CYCLE

#define NAND_PAGE_SIZE  2048

#define BAD_BLOCK_OFFSET NAND_PAGE_SIZE

#define NAND_BLOCK_MASK  (NAND_PAGE_SIZE - 1)

#define NAND_BLOCK_SIZE  (NAND_PAGE_SIZE * 64)

/* compile time failure in case of an invalid configuration */

#if defined(CONFIG_S3C2410) && (NAND_PAGE_SIZE != 512)

#error "S3C2410 does not support nand page size != 512"

#endif



static int nand_read_page_ll(unsigned char *buf, unsigned long addr)

{

 unsigned short *ptr16 = (unsigned short *)buf;

 unsigned int i, page_num;

 nand_clear_RnB();

 NFCMD = NAND_CMD_READ0;

#if (NAND_PAGE_SIZE == 512)

 /* Write Address */

 NFADDR = addr & 0xff;

 NFADDR = (addr >> 9) & 0xff;

 NFADDR = (addr >> 17) & 0xff;

 NFADDR = (addr >> 25) & 0xff;

#elif (NAND_PAGE_SIZE == 2048)

 page_num = addr >> 11; /* addr / 2048 */

 /* Write Address */

 NFADDR = 0;

 NFADDR = 0;

 NFADDR = page_num & 0xff;

 NFADDR = (page_num >> 8) & 0xff;

 NFADDR = (page_num >> 16) & 0xff;

 NFCMD = NAND_CMD_READSTART;

#else

#error "unsupported nand page size"

#endif

 nand_wait();

 for (i = 0; i < NAND_PAGE_SIZE; i++) 

 {

  *buf = (NFDATA & 0xff);

  buf++;

 }

 return NAND_PAGE_SIZE;

}

 /* low level nand read function */

int nand_read_ll(unsigned char *buf, unsigned long start_addr, int size)

{

    int i, j;

    

    if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK)) 

    {

     return -1;    /* invalid alignment */

    }

  /* chip Enable */

 nand_select();

 nand_clear_RnB();

 for (i=0; i<10; i++);

 for (i=start_addr; i < (start_addr + size);) 

 {

  j = nand_read_page_ll(buf, i);

  i += j;

  buf += j;

 }

 

  /* chip Disable */

 nand_deselect();

 

  return 0;

}
