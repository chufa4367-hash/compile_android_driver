#include <linux/module.h>
#include <linux/tty.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/time.h>
#include "comm.h"
#include "memory.h"
#include "process.h"

/* 全局变量用于存储随机生成的设备名称 */
static char random_device_name[64] = {0};

/* 生成随机后缀字符 */
static char get_random_char(void)
{
    /* 使用当前时间作为随机种子 */
    struct timespec ts;
    static unsigned int seed = 0;
    
    getnstimeofday(&ts);
    seed = (seed + (unsigned int)ts.tv_nsec) * 1103515245 + 12345;
    
    /* 字符集: 小写字母和数字 */
    const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789ZXCVBNMASDFGHJKLQWERTYUIOP";
    int index = (seed >> 16) % (sizeof(charset) - 1);
    
    return charset[index];
}

/* 生成随机设备名称: chu + 6位随机字符 */
static void generate_random_device_name(void)
{
    int i;
    
    /* 固定前缀 "chu" */
    strcpy(random_device_name, "chu");
    
    /* 添加6位随机字符作为后缀 */
    for (i = 0; i < 6; i++) {
        random_device_name[3 + i] = get_random_char();
    }
    random_device_name[9] = '\0';  /* 确保以NULL结尾 */
    
    //printk(KERN_INFO "[TearGame] Generated random device name: %s\n", random_device_name);
}

int dispatch_open(struct inode *node, struct file *file)
{
    return 0;
}

int dispatch_close(struct inode *node, struct file *file)
{
    return 0;
}

long dispatch_ioctl(struct file *const file, unsigned int const cmd, unsigned long const arg)
{
    static COPY_MEMORY cm;
    static MODULE_BASE mb;
    static char key[0x100] = {0};
    static char name[0x100] = {0};
    static bool is_verified = false;

    if (cmd == OP_INIT_KEY && !is_verified)
    {
        if (copy_from_user(key, (void __user *)arg, sizeof(key) - 1) != 0)
        {
            return -1;
        }
    }
    switch (cmd)
    {
    case OP_READ_MEM:
    {
        if (copy_from_user(&cm, (void __user *)arg, sizeof(cm)) != 0)
        {
            return -1;
        }
        if (read_process_memory(cm.pid, cm.addr, cm.buffer, cm.size) == false)
        {
            return -1;
        }
        break;
    }
    case OP_WRITE_MEM:
    {
        if (copy_from_user(&cm, (void __user *)arg, sizeof(cm)) != 0)
        {
            return -1;
        }
        if (write_process_memory(cm.pid, cm.addr, cm.buffer, cm.size) == false)
        {
            return -1;
        }
        break;
    }
    case OP_MODULE_BASE:
    {
        if (copy_from_user(&mb, (void __user *)arg, sizeof(mb)) != 0 || copy_from_user(name, (void __user *)mb.name, sizeof(name) - 1) != 0)
        {
            return -1;
        }
        mb.base = getModuleBase(mb.pid, name);
        if (copy_to_user((void __user *)arg, &mb, sizeof(mb)) != 0)
        {
            return -1;
        }
        break;
    }
    default:
        break;
    }
    return 0;
}

struct file_operations dispatch_functions = {
    .owner = THIS_MODULE,
    .open = dispatch_open,
    .release = dispatch_close,
    .unlocked_ioctl = dispatch_ioctl,
};

/* 这里不再预定义设备名称，将在初始化时动态设置 */
static struct miscdevice misc;

int __init driver_entry(void)
{
    int ret;
    
    /* 生成随机设备名称 */
    generate_random_device_name();
    
    /* 初始化miscdevice结构 */
    misc.minor = MISC_DYNAMIC_MINOR;  /* 动态分配次设备号 */
    misc.name = random_device_name;   /* 使用随机生成的设备名称 */
    misc.fops = &dispatch_functions;
    
    ret = misc_register(&misc);
    if (ret == 0) {
      //  printk(KERN_INFO "[TearGame] Random device registered: /dev/%s\n", misc.name);
        //printk(KERN_INFO "[TearGame] Device path: /dev/%s\n", misc.name);
        //printk(KERN_INFO "[TearGame] Driver loaded successfully!\n");
    } else {
       // printk(KERN_ERR "[TearGame] Failed to register device! ret=%d\n", ret);
    }
    return ret;
}

void __exit driver_unload(void)
{
   /* printk(KERN_INFO "[TearGame] Driver unloading...\n");
    printk(KERN_INFO "[TearGame] Unregistering device: /dev/%s\n", misc.name);
    misc_deregister(&misc);
    printk(KERN_INFO "[TearGame] Device /dev/%s unregistered\n", misc.name);
    printk(KERN_INFO "[TearGame] Goodbye! - by 泪心\n");*/
}

module_init(driver_entry);
module_exit(driver_unload);

MODULE_DESCRIPTION("Telegram: t.me/chufa6  支持4.9~6.12");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("初罚");