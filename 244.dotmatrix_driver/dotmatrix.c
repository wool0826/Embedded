/*
 * Dotmatrix Device Driver
 *  Hanback Electronics Co.,ltd
 * File : dotmatrix.c
 * Date : April,2009
 */ 

// ����� ������� ����
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#define DRIVER_AUTHOR		"hanback"	// ����� ������
#define DRIVER_DESC		"dotmatrix program" // ��⿡ ���� ����

#define DOT_MAJOR		244		// ����̽� �ֹ�ȣ
#define DOT_NAME 		"DOTMATRIX"	// ����̽� �̸�
#define DOT_MODULE_VERSION 	"DOTMATRIX V0.1"// ����̽� ����
#define DOT_PHY_ADDR		0x88000000	// Physical Address
#define DOT_ADDR_RANGE 		0x1000		// I/O ������ ũ��

#define NUMSIZE			4	//font number�� ũ��	
#define DELAY			2	// delay time
//#define RESULT_LEN		600	// ��������� ��µ� �迭 ũ��

//Global variable
static int dot_usage = 0;
static unsigned long  dot_ioremap;
static unsigned short *dot_row_addr,*dot_col_addr;

// Delay func.
void m_delay(int num) 
{
	volatile int i,j;
	for(i=0;i<num;i++)
		for(j=0;j<16384;j++);
}

// define functions...
// ���� ���α׷����� ����̽��� ó�� ����ϴ� ��츦 ó���ϴ� �Լ�
int dot_open(struct inode *minode, struct file *mfile) 
{
	// ����̽��� ���� �ִ��� Ȯ��.
	if(dot_usage != 0) return -EBUSY;
	
	// dot�� ���� �ּ� ����
	dot_ioremap=(unsigned long)ioremap(DOT_PHY_ADDR,DOT_ADDR_RANGE);
	
	// row,col�� �ּ� ����
	dot_row_addr =(unsigned short *)(dot_ioremap+0x40);
	dot_col_addr =(unsigned short *)(dot_ioremap+0x42);
	*dot_row_addr =0;
	*dot_col_addr =0;
	
	// ����� �� �ִ� I/O �������� Ȯ��
	if(!check_mem_region(dot_ioremap, DOT_ADDR_RANGE)) {
		// I/O �޸� ������ ���
		request_mem_region(dot_ioremap, DOT_ADDR_RANGE, DOT_NAME);
	}
	else	printk("driver: unable to register this!\n");
	
	dot_usage = 1;
	return 0;
}

// ���� ���α׷����� ����̽��� ���̻� ������� �ʾƼ� �ݱ⸦ �����ϴ� �Լ�
int dot_release(struct inode *minode, struct file *mfile) 
{
	// ���ε� �����ּҸ� ����
	iounmap((unsigned long*)dot_ioremap);

	// ��ϵ� I/O �޸� ������ ����
	release_mem_region(dot_ioremap, DOT_ADDR_RANGE);
	dot_usage = 0;
	return 0;
}

// 16������ 10������ �ٲ��ִ� �Լ� just one word
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

// ����̽� ����̹��� ���⸦ �����ϴ� �Լ�
ssize_t dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	int ret=0, i;
	char buf[20];
	unsigned short result[10] = { 0 };
	unsigned int init=0x001; //Scan value
	unsigned int n1, n2;

	// ����� �޸� gdata�� Ŀ�� �޸� buf�� length��ŭ ����
	ret = copy_from_user(&buf, gdata, length);
	if(ret<0) return -1;
	
	for (i=0; i < 10; i++) {
		n1 = htoi( buf[2*i] );
		n2 = htoi( buf[2*i+1] );
		result[i] = n1*16+n2;
		*dot_row_addr = init << i;
		*dot_col_addr = 0x8000 | result[ i ];
		m_delay(3);
	}
	return length;
}

// ���� ���۷��̼� ����ü
// ������ ���� open()�� ����Ѵ�. open()�� �ý��� ���� ȣ���Ͽ� Ŀ�� ���η� ����.
// �ش� �ý��� �ݰ� ���õ� ���� ������ ����ü ������ open�� �ش��ϴ� �ʵ尡 ����̹� ������
// dot_open()���� ���ǵǾ� �����Ƿ� dot_open()�� ȣ��ȴ�.
// write�� release�� ���������� �����Ѵ�. ���� ��ϵ��� ���� ���ۿ� ���ؼ��� Ŀ�ο��� ������
// ���� default ������ �ϵ��� �Ǿ� �ִ�.
struct file_operations dot_fops = {
	.owner		= THIS_MODULE,
	.write		= dot_write,
	.open		= dot_open,
	.release	= dot_release,
};

// ����� Ŀ�� ���η� ����
// ��� ���α׷��� �ٽ����� ������ Ŀ�� ���η� ���� ���񽺸� �����޴� ���̹Ƿ�
// Ŀ�� ���η� ���� init()�� ���� �����Ѵ�.
// ���� ���α׷��� �ҽ� ���ο��� ���ǵ��� ���� ���� �Լ��� ����Ѵ�. �װ��� �ܺ�
// ���̺귯���� ������ �������� ��ũ�Ǿ� ���Ǳ� �����̴�. ��� ���α׷��� Ŀ��
// �����ϰ� ��ũ�Ǳ� ������ Ŀ�ο��� �����ϰ� ����ϴ� �Լ����� ����� �� �ִ�.
int dot_init(void) 
{
	int result;

	// ���� ����̽� ����̹��� ����Ѵ�.
	result = register_chrdev(DOT_MAJOR, DOT_NAME, &dot_fops);
	
	if(result < 0) { // ��Ͻ���
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}
	
	// major ��ȣ�� ����Ѵ�.
	printk("Init Module, Dotmatrix Major Number : %d\n", DOT_MAJOR);
	
	return 0;
}

// ����� Ŀ�ο��� ����
void dot_exit(void) 
{
	// ���� ����̽� ����̹��� �����Ѵ�.
	unregister_chrdev(DOT_MAJOR, DOT_NAME);

	printk("driver: %s DRIVER EXIT\n", DOT_NAME);
}

module_init(dot_init);		// ��� ���� �� ȣ��Ǵ� �Լ�
module_exit(dot_exit);		// ��� ���� �� ȣ��Ǵ� �Լ�

MODULE_AUTHOR(DRIVER_AUTHOR); 	 // ����� ������
MODULE_DESCRIPTION(DRIVER_DESC); // ��⿡ ���� ����
MODULE_LICENSE("Dual BSD/GPL");	 // ����� ���̼��� ���
