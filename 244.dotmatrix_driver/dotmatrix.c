/*
 * Dotmatrix Device Driver
 *  Hanback Electronics Co.,ltd
 * File : dotmatrix.c
 * Date : April,2009
 */ 

// 모듈의 헤더파일 선언
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

#define DRIVER_AUTHOR		"hanback"	// 모듈의 저작자
#define DRIVER_DESC		"dotmatrix program" // 모듈에 대한 설명

#define DOT_MAJOR		244		// 디바이스 주번호
#define DOT_NAME 		"DOTMATRIX"	// 디바이스 이름
#define DOT_MODULE_VERSION 	"DOTMATRIX V0.1"// 디바이스 버전
#define DOT_PHY_ADDR		0x88000000	// Physical Address
#define DOT_ADDR_RANGE 		0x1000		// I/O 영역의 크기

#define NUMSIZE			4	//font number의 크기	
#define DELAY			2	// delay time
//#define RESULT_LEN		600	// 결과값으로 출력될 배열 크기

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
// 응용 프로그램에서 디바이스를 처음 사용하는 경우를 처리하는 함수
int dot_open(struct inode *minode, struct file *mfile) 
{
	// 디바이스가 열려 있는지 확인.
	if(dot_usage != 0) return -EBUSY;
	
	// dot의 가상 주소 매핑
	dot_ioremap=(unsigned long)ioremap(DOT_PHY_ADDR,DOT_ADDR_RANGE);
	
	// row,col의 주소 설정
	dot_row_addr =(unsigned short *)(dot_ioremap+0x40);
	dot_col_addr =(unsigned short *)(dot_ioremap+0x42);
	*dot_row_addr =0;
	*dot_col_addr =0;
	
	// 등록할 수 있는 I/O 영역인지 확인
	if(!check_mem_region(dot_ioremap, DOT_ADDR_RANGE)) {
		// I/O 메모리 영역을 등록
		request_mem_region(dot_ioremap, DOT_ADDR_RANGE, DOT_NAME);
	}
	else	printk("driver: unable to register this!\n");
	
	dot_usage = 1;
	return 0;
}

// 응용 프로그램에서 디바이스를 더이상 사용하지 않아서 닫기를 구현하는 함수
int dot_release(struct inode *minode, struct file *mfile) 
{
	// 매핑된 가상주소를 해제
	iounmap((unsigned long*)dot_ioremap);

	// 등록된 I/O 메모리 영역을 해제
	release_mem_region(dot_ioremap, DOT_ADDR_RANGE);
	dot_usage = 0;
	return 0;
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

// 디바이스 드라이버의 쓰기를 구현하는 함수
ssize_t dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
	int ret=0, i;
	char buf[20];
	unsigned short result[10] = { 0 };
	unsigned int init=0x001; //Scan value
	unsigned int n1, n2;

	// 사용자 메모리 gdata를 커널 메모리 buf에 length만큼 복사
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

// 파일 오퍼레이션 구조체
// 파일을 열때 open()을 사용한다. open()는 시스템 콜을 호출하여 커널 내부로 들어간다.
// 해당 시스템 콜과 관련된 파일 연산자 구조체 내부의 open에 해당하는 필드가 드라이버 내에서
// dot_open()으로 정의되어 있으므로 dot_open()가 호출된다.
// write와 release도 마찬가지로 동작한다. 만약 등록되지 않은 동작에 대해서는 커널에서 정의해
// 놓은 default 동작을 하도록 되어 있다.
struct file_operations dot_fops = {
	.owner		= THIS_MODULE,
	.write		= dot_write,
	.open		= dot_open,
	.release	= dot_release,
};

// 모듈을 커널 내부로 삽입
// 모듈 프로그램의 핵심적인 목적은 커널 내부로 들어가서 서비스를 제공받는 것이므로
// 커널 내부로 들어가는 init()을 먼저 시작한다.
// 응용 프로그램은 소스 내부에서 정의되지 않은 많은 함수를 사용한다. 그것은 외부
// 라이브러리가 컴파일 과정에서 링크되어 사용되기 때문이다. 모듈 프로그램은 커널
// 내부하고만 링크되기 때문에 커널에서 정의하고 허용하는 함수만을 사용할 수 있다.
int dot_init(void) 
{
	int result;

	// 문자 디바이스 드라이버를 등록한다.
	result = register_chrdev(DOT_MAJOR, DOT_NAME, &dot_fops);
	
	if(result < 0) { // 등록실패
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}
	
	// major 번호를 출력한다.
	printk("Init Module, Dotmatrix Major Number : %d\n", DOT_MAJOR);
	
	return 0;
}

// 모듈을 커널에서 제거
void dot_exit(void) 
{
	// 문자 디바이스 드라이버를 제거한다.
	unregister_chrdev(DOT_MAJOR, DOT_NAME);

	printk("driver: %s DRIVER EXIT\n", DOT_NAME);
}

module_init(dot_init);		// 모듈 적재 시 호출되는 함수
module_exit(dot_exit);		// 모듈 제거 시 호출되는 함수

MODULE_AUTHOR(DRIVER_AUTHOR); 	 // 모듈의 저작자
MODULE_DESCRIPTION(DRIVER_DESC); // 모듈에 대한 설명
MODULE_LICENSE("Dual BSD/GPL");	 // 모듈의 라이선스 등록
