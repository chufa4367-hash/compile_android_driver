
typedef struct _COPY_MEMORY
{
	pid_t pid;
	uintptr_t addr;
	void *buffer;
	size_t size;
} COPY_MEMORY, *PCOPY_MEMORY;

typedef struct _MODULE_BASE
{
	pid_t pid;
	char *name;
	uintptr_t base;
} MODULE_BASE, *PMODULE_BASE;

enum OPERATIONS
{
	OP_INIT_KEY = 0x6368750,
	OP_READ_MEM = 0x63687501,
	OP_WRITE_MEM = 0x63687502,
	OP_MODULE_BASE = 0x63687503,
};