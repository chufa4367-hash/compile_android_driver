[file name]: entry.c
[file content begin]
/**
 * ============================================================================
 * 泪心开源驱动 - TearGame Open Source Driver
 * ============================================================================
 * 作者 (Author): 泪心 (Tear)
 * QQ: 2254013571
 * 邮箱 (Email): tearhacker@outlook.com
 * 电报 (Telegram): t.me/TearGame
 * GitHub: github.com/tearhacker
 * ============================================================================
 * 本项目完全免费开源，代码明文公开
 * This project is completely free and open source with clear code
 * 
 * 禁止用于引流盈利，保留开源版权所有
 * Commercial use for profit is prohibited, all open source rights reserved
 * 
 * 凡是恶意盈利者需承担法律责任
 * Those who maliciously profit will bear legal responsibility
 * ============================================================================
 */

#include <linux/module.h>
#include <linux/tty.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include "comm.h"
#include "memory.h"
#include "process.h"

//原作者JiangNight  源码存在严重问题 加载格机 重启  黑砖    加载失败  kernel pacni 各种问题
//泪心已经彻底修复优化

//原项目链接 https://github.com/Jiang-Night/Kernel_driver_hack
//泪心驱动完整开源读写内核源码新项目链接 https://github.com/tearhacker/TearGame_KernelDriver_Android_WriteReadMemory

/* 全局变量用于存储随机生成的设备名称 */
static char random_device_name[64] = {0};

/* 简化版的随机字符生成 */
static char get_random_char(unsigned int *seed)
{
    /* 字符集: 小写字母和数字 */
    const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    
    /* 使用简单的伪随机算法 */
    *seed = *seed * 1103515245 + 12345;
    int index = (*seed >> 16) % (sizeof(charset) - 1);
    
    return charset[index];
}

/* 生成随机设备名称: chu + 6位随机字符 */
static void generate_random_device_name(void)
{
    int i;
    unsigned int seed;
    
    /* 使用jiffies作为随机种子（所有内核版本都可用） */
    seed = (unsigned int)jiffies;
    
    /* 固定前缀 "chu" */
    strcpy(random_device_name, "chu");
    
    /* 添加6位随机字符作为后缀 */
    for (i = 0; i < 6; i++) {
        random_device_name[3 + i] = get_random_char(&seed);
    }
    random_device_name[9] = '\0';  /* 确保以NULL结尾 */
    
    printk(KERN_INFO "[TearGame] Generated random device name: %s\n", random_device_name);
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
        mb.base = get_module_base(mb.pid, name);
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
    
    printk(KERN_INFO "=============================================\n");
    printk(KERN_INFO "[TearGame] Driver loading...\n");
    printk(KERN_INFO "[TearGame] Author: 泪心 (Tear)\n");
    printk(KERN_INFO "[TearGame] QQ: 2254013571\n");
    printk(KERN_INFO "[TearGame] Email: tearhacker@outlook.com\n");
    printk(KERN_INFO "[TearGame] Telegram: t.me/TearGame\n");
    printk(KERN_INFO "[TearGame] GitHub: github.com/tearhacker\n");
    printk(KERN_INFO "=============================================\n");
    
    /* 生成随机设备名称 */
    generate_random_device_name();
    
    /* 初始化miscdevice结构 */
    memset(&misc, 0, sizeof(misc));
    misc.minor = MISC_DYNAMIC_MINOR;  /* 动态分配次设备号 */
    misc.name = random_device_name;   /* 使用随机生成的设备名称 */
    misc.fops = &dispatch_functions;
    
    ret = misc_register(&misc);
    if (ret == 0) {
        printk(KERN_INFO "[TearGame] Random device registered: /dev/%s\n", misc.name);
        printk(KERN_INFO "[TearGame] Device path: /dev/%s\n", misc.name);
        printk(KERN_INFO "[TearGame] Driver loaded successfully!\n");
    } else {
        printk(KERN_ERR "[TearGame] Failed to register device! ret=%d\n", ret);
    }
    return ret;
}

void __exit driver_unload(void)
{
    printk(KERN_INFO "[TearGame] Driver unloading...\n");
    printk(KERN_INFO "[TearGame] Unregistering device: /dev/%s\n", misc.name);
    misc_deregister(&misc);
    printk(KERN_INFO "[TearGame] Device /dev/%s unregistered\n", misc.name);
    printk(KERN_INFO "[TearGame] Goodbye! - by 泪心\n");
}

module_init(driver_entry);
module_exit(driver_unload);

MODULE_DESCRIPTION("Telegram: t.me/chufa6  支持4.9~6.12");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("初罚");