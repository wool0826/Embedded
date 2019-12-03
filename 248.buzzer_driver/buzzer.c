/*
 * Buzzer Device Driver
 *  Hanback Electronics Co.,ltd
 * File : buzzer.c
 * Date : April,2009
 */

// 모듈의 헤더파일 선언
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define DRIVER_AUTHOR           "hanback"               // 모듈의 저작자
#define DRIVER_DESC             "buzzer test program"   // 모듈에 대한 설명

#define BUZZER_MAJOR            248                     // 디바이스 주번호
#define BUZZER_NAME             "BUZZER IO PORT"        // 디바이스 이름
#define BUZZER_MODULE_VERSION   "BUZZER IO PORT V0.1"   // 디바이스 버전
#define BUZZER_ADDRESS          0x88000050              // buzzer의 물리 주소
#define BUZZER_ADDRESS_RANGE    0x1000                  // I/O 영역의 크기

//Global variable
static int buzzer_usage = 0;            // 드라이버 사용여부를 확인하는 값
static unsigned long *buzzer_ioremap;   // IO 주소 공간 저장

// define functions...
// 응용 프로그램에서 디바이스를 처음 사용하는 경우를 처리하는 함수
int buzzer_open(struct inode *minode, struct file *mfile)
{
        // 디바이스가 열려 있는지 확인
        if(buzzer_usage != 0) return -EBUSY;

        // buzzer의 가상 주소 매핑
        buzzer_ioremap= ioremap(BUZZER_ADDRESS,BUZZER_ADDRESS_RANGE);

        // 등록할 수 있는 I/O 영역인지 확인
        if(!check_mem_region((unsigned long)buzzer_ioremap,BUZZER_ADDRESS_RANGE)) {
                // I/O 메모리 영역을 등록
                request_mem_region((unsigned long)buzzer_ioremap,BUZZER_ADDRESS_RANGE,BUZZER_NAME);
        }
        else printk(KERN_WARNING"Can't get IO Region 0x%x\n", (unsigned int)buzzer_ioremap);

        buzzer_usage = 1;
        return 0;
}

// 응용 프로그램에서 디바이스를 더이상 사용하지 않아서 닫기를 구현하는 함수
int buzzer_release(struct inode *minode, struct file *mfile)
{
        // 매핑된 가상주소를 해제
        iounmap(buzzer_ioremap);

        // 등록된 I/O 메모리 영역을 해제
        release_mem_region((unsigned long)buzzer_ioremap,BUZZER_ADDRESS_RANGE);

        buzzer_usage = 0;
        return 0;
}

// 디바이스 드라이버의 쓰기를 구현하는 함수
ssize_t buzzer_write_byte(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
        unsigned char *addr;
        unsigned char  c;

        // gdata의 사용자 공간의 메모리에서 c에 읽어온다.
        get_user(c,gdata);

        addr = (unsigned char *)(buzzer_ioremap);
        *addr = c;

        return length;
}

// 파일 오퍼레이션 구조체
// 파일을 열때 open()을 사용한다. open()는 시스템 콜을 호출하여 커널 내부로 들어간다.
// buzzer_open()으로 정의되어 있으므로 buzzer_open()가 호출된다.
// write와 release도 마찬가지로 동작한다. 만약 등록되지 않은 동작에 대해서는 커널에서 정의해
// 놓은 default 동작을 하도록 되어 있다.
static struct file_operations buzzer_fops = {
        .owner          = THIS_MODULE,
        .open           = buzzer_open,
        .write          = buzzer_write_byte,
        .release        = buzzer_release,
};

// 모듈을 커널 내부로 삽입
// 모듈 프로그램의 핵심적인 목적은 커널 내부로 들어가서 서비스를 제공받는 것이므로
// 커널 내부로 들어가는 init()을 먼저 시작한다.
// 응용 프로그램은 소스 내부에서 정의되지 않은 많은 함수를 사용한다. 그것은 외부
// 라이브러리가 컴파일 과정에서 링크되어 사용되기 때문이다. 모듈 프로그램은 커널
// 내부하고만 링크되기 때문에 커널에서 정의하고 허용하는 함수만을 사용할 수 있다.
int buzzer_init(void)
{
        int result;

        // 문자 디바이스 드라이버를 등록한다.
        result = register_chrdev(BUZZER_MAJOR,BUZZER_NAME,&buzzer_fops);

        if(result < 0) {  // 등록실패
                printk(KERN_WARNING"Can't get any major\n");
                return result;
        }

        // major 번호를 출력한다.
        printk(KERN_WARNING"Init Module, Buzzer Major number : %d\n", BUZZER_MAJOR);
        return 0;
}

// 모듈을 커널에서 제거
void buzzer_exit(void)
{
        // 문자 디바이스 드라이버를 제거한다.
        unregister_chrdev(BUZZER_MAJOR,BUZZER_NAME);

        printk(KERN_INFO"driver: %s DRIVER EXIT\n", BUZZER_NAME);
}

module_init(buzzer_init);       // 모듈 적재 시 호출되는 함수
module_exit(buzzer_exit);       // 모듈 제거 시 호출되는 함수

MODULE_AUTHOR(DRIVER_AUTHOR);   // 모듈의 저작자
MODULE_DESCRIPTION(DRIVER_DESC); //모듈에 대한 설명
MODULE_LICENSE("Dual BSD/GPL");  // 모듈의 라이선스 등록

