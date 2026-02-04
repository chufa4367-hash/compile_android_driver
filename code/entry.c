#include <linux/module.h>
#include <linux/tty.h>
#include <linux/miscdevice.h>
#include "comm.h"
#include "memory.h"
#include "process.h"
#define CHU_RANDOM_SUFFIX_LEN 8
static char chu_device_name[32] = {0};

/* 生成随机字符的函数 - 不使用随机API，简单实现 */
static char get_random_char(void)
{
	static unsigned int seed = 0x12345678;
	seed = seed * 1103515245 + 12345;
	return 'a' + (seed >> 16) % 26;
}

/* 生成完整的设备名 */
static void generate_chu_device_name(void)
{
	int i;
	
	/* 组合设备名: chu + 随机后缀 */
	chu_device_name[0] = 'c';
	chu_device_name[1] = 'h';
	chu_device_name[2] = 'u';
	
	/* 生成随机后缀 */
	for (i = 0; i < CHU_RANDOM_SUFFIX_LEN; i++) {
		chu_device_name[3 + i] = get_random_char();
	}
	chu_device_name[3 + CHU_RANDOM_SUFFIX_LEN] = '\0';
}

#define DEVICE_NAME chu_device_name

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

static struct miscdevice misc;

int __init driver_entry(void)
{
	int ret;
	
	/* 生成随机设备名 */
	generate_chu_device_name();
	
	/* 初始化miscdevice结构体 */
	misc.minor = MISC_DYNAMIC_MINOR;
	misc.name = DEVICE_NAME;
	misc.fops = &dispatch_functions;
	

	ret = misc_register(&misc);
	if (ret == 0) {
	} else {
	
	}
	return ret;
}

void __exit driver_unload(void)
{

	misc_deregister(&misc);
}

module_init(driver_entry);
module_exit(driver_unload);

MODULE_DESCRIPTION("Tg t.me/chufa6");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("初罚");