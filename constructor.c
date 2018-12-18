#include <stdio.h>
#include <elf.h>

unsigned char dynstr_buf[8192] __attribute__((section(".data"))) =
    { [0 ... 8191] = 0};

extern unsigned long get_rip_label;
unsigned long get_rip(void);

#define PIC_RESOLVE_ADDR(target) (get_rip() - ((char *)&get_rip_label - (char *)target))

void
_memcpy(void *dst, void *src, unsigned int len)
{
	int i;
	unsigned char *s = (unsigned char *)src;
	unsigned char *d = (unsigned char *)dst;

	for (i = 0; i < len; i++) {
		*d = *s;
		s++, d++;
	}
	return;
}

int
_strcmp(const char *s1, const char *s2)
{
	int r = 0;

	while (!(r = (*s1 - *s2) && *s2))
		s1++, s2++;
	if (!r)
		return r;
	return r = (r < 0) ? -1 : 1;
}

unsigned long
get_rip(void)
{
	unsigned long ret;

	__asm__ __volatile__
	(
	"call get_rip_label     \n"
	".globl get_rip_label   \n"
	"get_rip_label:         \n"
	"pop %%rax              \n"
	"mov %%rax, %0" : "=r"(ret)
	);

        return ret;
}

void
restore_dynstr(void)
{
	char *strtab = NULL;
	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdr;
	int i;
	unsigned char *mem;

	ehdr = (void *)0x400000;
	shdr = (Elf64_Shdr *)&mem[ehdr->e_shoff];
	strtab = (char *)&mem[shdr[ehdr->e_shstrndx].sh_offset];

	for (i = 0; i < ehdr->e_shnum; i++) {
		if (_strcmp(&strtab[shdr[i].sh_name], ".dynstr") != 0)
			continue;
		_memcpy((char *)shdr[i].sh_addr,
		    (char *)PIC_RESOLVE_ADDR(&dynstr_buf), shdr[i].sh_size);
		break;
	}
	return;
}