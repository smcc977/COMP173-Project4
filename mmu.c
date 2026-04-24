/*
Complie with:
gcc -pthread mmu.c -o mmu
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

/*
TLB is array of struct of page number and frames
Page table is array of frame number (index is page number)
Main Memory is array of value (index is frame #)

Program Flow:
Read address
Search TLB
If fault search page table
If fault allocate new frame in memory
Return value
*/

typedef struct {
	int page_table[256];

	int tlb_pages[16];
	int tlb_frames[16];

	int main_memory[256];

	int tlb_index;
	int next_open_frame;

	int tlb_hits;
	int page_faults;
} MMU;


void init_mmu(MMU *mmu)
