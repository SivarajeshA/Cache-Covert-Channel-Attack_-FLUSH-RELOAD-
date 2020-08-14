#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include<assert.h>

#define ADDR_PTR void*
#define CYCLES uint64_t


typedef struct map_handle_s {
  int fd;
  size_t range;
  ADDR_PTR mapping;
} map_handle_t;

void clflush(ADDR_PTR addr);
void maccess(ADDR_PTR addr);
void* map_file(const char* filename, map_handle_t** handle);
	
//CYCLES measure_one_block_access_time(ADDR_PTR addr);
//CYCLES rdtscp(void);
/*
	
extern inline __attribute__((always_inline))
CYCLES measure_one_block_access_time(ADDR_PTR addr) {

  CYCLES cycles;

  asm volatile(
    " mfence           \n\t"
    " lfence           \n\t"
    " rdtsc            \n\t"
    " lfence            \n\t"
    " mov %%rax, %%rdi \n\t"
    " mov (%1), %%r8   \n\t"
    " lfence           \n\t"
    " rdtsc            \n\t"
    " sub %%rdi, %%rax \n\t"
    : "=a"(cycles) 
    : "c"(addr)
    : "r8", "rdi");

  return cycles;
}

extern inline __attribute__((always_inline))
CYCLES rdtscp(void) {

  CYCLES cycles;
  asm volatile (
    " rdtscp  \n\t"
    : "=a" (cycles));

	return cycles;
}
*/

/*
 * Flushes the cache block accessed by a virtual address out of the cache
 */
extern inline __attribute__((always_inline))
void clflush(ADDR_PTR addr) {

  asm volatile (
    "mfence       \n\t"
    "clflush (%0) \n\t"
    :
    :"r"(addr));
}


/*
 * Loads from virtual address addr
 */
void maccess(ADDR_PTR addr) {

  asm volatile (
    " movq (%0), %%rax  \n\t"
    :
    : "c"(addr)
    : "rax");
}

/*
 * Map a file in shared memory space
 */
void* map_file(const char* filename, map_handle_t** handle){

  if (filename == NULL || handle == NULL) {
    return NULL;
  }

  *handle = calloc(1, sizeof(map_handle_t));
  if (*handle == NULL) {
    return NULL;
  }

  (*handle)->fd = open(filename, O_RDONLY);
  if ((*handle)->fd == -1) {
    return NULL;
  }

  struct stat filestat;
  if (fstat((*handle)->fd, &filestat) == -1) {
    close((*handle)->fd);
    return NULL;
  }

  (*handle)->range = filestat.st_size;

  (*handle)->mapping = mmap(0, (*handle)->range, PROT_READ, MAP_SHARED, (*handle)->fd, 0);

  return (*handle)->mapping;
}


int main(int argc, char **argv) 
{
	double trans_time;
	double trans_rate;

	// TODO: Establish your cache covert channel	
	map_handle_t *handle;
	unsigned int *map;
	map = (unsigned int *) map_file("share_mem.txt",&handle);
	assert(map!=NULL);
	uint64_t a[10000000000];
	for(int i=0;i<10;i++)
	{
		clflush(&a);
		printf("\n%lu",measure_one_block_access_time(a));
		
		maccess(&a);
		printf("\n%lu",measure_one_block_access_time(a));
	}
		return 0;
}

