#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <asm/fcntl.h>
#include <linux/ioport.h>
#include <linux/delay.h>

#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DRIVER_AUTHOR		"hanback"		//
#define DRIVER_DESC		"7-Segment program"	//

#define	SEGMENT_MAJOR		242			//
#define	SEGMENT_NAME		"SEGMENT"		//
#define SEGMENT_MODULE_VERSION	"SEGMENT PORT V0.1"	//

#define SEGMENT_ADDRESS_GRID	0x88000030	//
#define SEGMENT_ADDRESS_DATA	0x88000032	//
#define SEGMENT_ADDRESS_1	0x88000034	//
#define SEGMENT_ADDRESS_RANGE	0x1000		//
#define MODE_0_IDLE	0x0			//
#define MODE_1_PRINT	0x1

//Global variable
static unsigned int segment_usage = 0;
static unsigned long *segment_data;
static unsigned long *segment_grid;
static int mode_select = 0x0;

// define functions...
int segment_open (struct inode *inode, struct file *filp)
{
	//
	if(segment_usage != 0) return -EBUSY;
	
	//
	segment_grid =  ioremap(SEGMENT_ADDRESS_GRID, SEGMENT_ADDRESS_RANGE);
	segment_data =  ioremap(SEGMENT_ADDRESS_DATA, SEGMENT_ADDRESS_RANGE);
	
	//
	if(!check_mem_region((unsigned long)segment_data,SEGMENT_ADDRESS_RANGE) && !check_mem_region((unsigned long)segment_grid, SEGMENT_ADDRESS_RANGE))
	{
		// I/O
		request_region((unsigned long)segment_grid, SEGMENT_ADDRESS_RANGE, SEGMENT_NAME);
		request_region((unsigned long)segment_data, SEGMENT_ADDRESS_RANGE, SEGMENT_NAME);
	}
	else printk("driver : unable to register this!\n");

	segment_usage = 1;	
	return 0; 
}

//
int segment_release (struct inode *inode, struct file *filp)
{
	//
	iounmap(segment_grid);
	iounmap(segment_data);

	//
	release_region((unsigned long)segment_data, SEGMENT_ADDRESS_RANGE);
	release_region((unsigned long)segment_grid, SEGMENT_ADDRESS_RANGE);

	segment_usage = 0;
	return 0;
}

//
unsigned short Getsegmentcode (short x)
{
	unsigned short code;
	switch (x) {
		case 0x0 : code = 0xfc; break;
		case 0x1 : code = 0x60; break;
		case 0x2 : code = 0xda; break;
		case 0x3 : code = 0xf2; break;
		case 0x4 : code = 0x66; break;
		case 0x5 : code = 0xb6; break;
		case 0x6 : code = 0xbe; break;
		case 0x7 : code = 0xe4; break;
		case 0x8 : code = 0xfe; break;
		case 0x9 : code = 0xf6; break;
		
		case 0xa : code = 0xfa; break;
		case 0xb : code = 0x3e; break;
		case 0xc : code = 0x1a; break;
		case 0xd : code = 0x7a; break;						
		case 0xe : code = 0x9e; break;
		case 0xf : code = 0x8e; break;				
		default : code = 0; break;
	}
	return code;
}						

// 16진수를 10진수로 바꿔주는 함수 just one word
int htoi(const char hexa)
{
	int ch = 0;
	if('0' <= hexa && hexa <= '9')
		ch = hexa - '0';
	if('A' <= hexa && hexa <= 'F')
		ch = hexa - 'A' + 10;
	if('a' <= hexa && hexa <= 'f')
		ch = hexa - 'a' + 10;
	return ch;
}
//
ssize_t segment_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	int ret=0, i;
	char buf[20];
	unsigned char digit[6]={0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	unsigned char result[6] = { 0, 0, 0, 0, 0, 0 };
	int num;

	// 사용자 메모리 gdata를 커널 메모리 buf에 length만큼 복사
	ret = copy_from_user(&buf, gdata, length);
	if(ret<0) return -1;
	
	for (i=0; i<6; i++) {
		num = htoi(buf[2*i]) * 16 + htoi(buf[2*i + 1]);
		result[i] = num;
		*segment_grid = digit[i];
		*segment_data = result[i];
		m_delay(1);
	}

	return length;
}

struct file_operations segment_fops = 
{
	.owner		= THIS_MODULE,
	.open		= segment_open,
	.write		= segment_write,
	.release	= segment_release,
};

int segment_init(void)
{
	int result;

	result = register_chrdev(SEGMENT_MAJOR, SEGMENT_NAME, &segment_fops);
	if (result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	printk(KERN_INFO"Init Module, 7-Segment Major Number : %d\n", SEGMENT_MAJOR);
	return 0;
}

void segment_exit(void)
{
	unregister_chrdev(SEGMENT_MAJOR,SEGMENT_NAME);

	printk("driver: %s DRIVER EXIT\n", SEGMENT_NAME);
}

module_init(segment_init);	//
module_exit(segment_exit);	//

MODULE_AUTHOR(DRIVER_AUTHOR);	//
MODULE_DESCRIPTION(DRIVER_DESC);//
MODULE_LICENSE("Dual BSD/GPL");	//

