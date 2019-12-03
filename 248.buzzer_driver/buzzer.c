/*
 * Buzzer Device Driver
 *  Hanback Electronics Co.,ltd
 * File : buzzer.c
 * Date : April,2009
 */

// ����� ������� ����
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define DRIVER_AUTHOR           "hanback"               // ����� ������
#define DRIVER_DESC             "buzzer test program"   // ��⿡ ���� ����

#define BUZZER_MAJOR            248                     // ����̽� �ֹ�ȣ
#define BUZZER_NAME             "BUZZER IO PORT"        // ����̽� �̸�
#define BUZZER_MODULE_VERSION   "BUZZER IO PORT V0.1"   // ����̽� ����
#define BUZZER_ADDRESS          0x88000050              // buzzer�� ���� �ּ�
#define BUZZER_ADDRESS_RANGE    0x1000                  // I/O ������ ũ��

//Global variable
static int buzzer_usage = 0;            // ����̹� ��뿩�θ� Ȯ���ϴ� ��
static unsigned long *buzzer_ioremap;   // IO �ּ� ���� ����

// define functions...
// ���� ���α׷����� ����̽��� ó�� ����ϴ� ��츦 ó���ϴ� �Լ�
int buzzer_open(struct inode *minode, struct file *mfile)
{
        // ����̽��� ���� �ִ��� Ȯ��
        if(buzzer_usage != 0) return -EBUSY;

        // buzzer�� ���� �ּ� ����
        buzzer_ioremap= ioremap(BUZZER_ADDRESS,BUZZER_ADDRESS_RANGE);

        // ����� �� �ִ� I/O �������� Ȯ��
        if(!check_mem_region((unsigned long)buzzer_ioremap,BUZZER_ADDRESS_RANGE)) {
                // I/O �޸� ������ ���
                request_mem_region((unsigned long)buzzer_ioremap,BUZZER_ADDRESS_RANGE,BUZZER_NAME);
        }
        else printk(KERN_WARNING"Can't get IO Region 0x%x\n", (unsigned int)buzzer_ioremap);

        buzzer_usage = 1;
        return 0;
}

// ���� ���α׷����� ����̽��� ���̻� ������� �ʾƼ� �ݱ⸦ �����ϴ� �Լ�
int buzzer_release(struct inode *minode, struct file *mfile)
{
        // ���ε� �����ּҸ� ����
        iounmap(buzzer_ioremap);

        // ��ϵ� I/O �޸� ������ ����
        release_mem_region((unsigned long)buzzer_ioremap,BUZZER_ADDRESS_RANGE);

        buzzer_usage = 0;
        return 0;
}

// ����̽� ����̹��� ���⸦ �����ϴ� �Լ�
ssize_t buzzer_write_byte(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
        unsigned char *addr;
        unsigned char  c;

        // gdata�� ����� ������ �޸𸮿��� c�� �о�´�.
        get_user(c,gdata);

        addr = (unsigned char *)(buzzer_ioremap);
        *addr = c;

        return length;
}

// ���� ���۷��̼� ����ü
// ������ ���� open()�� ����Ѵ�. open()�� �ý��� ���� ȣ���Ͽ� Ŀ�� ���η� ����.
// buzzer_open()���� ���ǵǾ� �����Ƿ� buzzer_open()�� ȣ��ȴ�.
// write�� release�� ���������� �����Ѵ�. ���� ��ϵ��� ���� ���ۿ� ���ؼ��� Ŀ�ο��� ������
// ���� default ������ �ϵ��� �Ǿ� �ִ�.
static struct file_operations buzzer_fops = {
        .owner          = THIS_MODULE,
        .open           = buzzer_open,
        .write          = buzzer_write_byte,
        .release        = buzzer_release,
};

// ����� Ŀ�� ���η� ����
// ��� ���α׷��� �ٽ����� ������ Ŀ�� ���η� ���� ���񽺸� �����޴� ���̹Ƿ�
// Ŀ�� ���η� ���� init()�� ���� �����Ѵ�.
// ���� ���α׷��� �ҽ� ���ο��� ���ǵ��� ���� ���� �Լ��� ����Ѵ�. �װ��� �ܺ�
// ���̺귯���� ������ �������� ��ũ�Ǿ� ���Ǳ� �����̴�. ��� ���α׷��� Ŀ��
// �����ϰ� ��ũ�Ǳ� ������ Ŀ�ο��� �����ϰ� ����ϴ� �Լ����� ����� �� �ִ�.
int buzzer_init(void)
{
        int result;

        // ���� ����̽� ����̹��� ����Ѵ�.
        result = register_chrdev(BUZZER_MAJOR,BUZZER_NAME,&buzzer_fops);

        if(result < 0) {  // ��Ͻ���
                printk(KERN_WARNING"Can't get any major\n");
                return result;
        }

        // major ��ȣ�� ����Ѵ�.
        printk(KERN_WARNING"Init Module, Buzzer Major number : %d\n", BUZZER_MAJOR);
        return 0;
}

// ����� Ŀ�ο��� ����
void buzzer_exit(void)
{
        // ���� ����̽� ����̹��� �����Ѵ�.
        unregister_chrdev(BUZZER_MAJOR,BUZZER_NAME);

        printk(KERN_INFO"driver: %s DRIVER EXIT\n", BUZZER_NAME);
}

module_init(buzzer_init);       // ��� ���� �� ȣ��Ǵ� �Լ�
module_exit(buzzer_exit);       // ��� ���� �� ȣ��Ǵ� �Լ�

MODULE_AUTHOR(DRIVER_AUTHOR);   // ����� ������
MODULE_DESCRIPTION(DRIVER_DESC); //��⿡ ���� ����
MODULE_LICENSE("Dual BSD/GPL");  // ����� ���̼��� ���

