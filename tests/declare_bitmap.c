// See linux-6.1/include/linux/cpumask.h:910:0
// Commit id: fd7e2a25863d3a8104dc1e414b2d49e2418e250c (from GitHub)

#define CONFIG_NR_CPUS	1
#define NR_CPUS		CONFIG_NR_CPUS
#define __KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BITS_PER_BYTE		8
#define BITS_PER_TYPE(type)	(sizeof(type) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)	__KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(long))

#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]

extern const DECLARE_BITMAP(cpu_all_bits, NR_CPUS);

