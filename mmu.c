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
#include <stdint.h>

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
	uint8_t bytes[256];
} page;

typedef struct {
	int page_table[256];

	int tlb_pages[16];
	int tlb_frames[16];

	page main_memory[256];

	int tlb_index;
	int next_open_frame;

	int tlb_hits;
	int page_faults;
} MMU;


void init_mmu(MMU *mmu);
int search_tlb(MMU *mmu, int page_number);
int page_table_lookup(MMU *mmu, int page_number);
int allocation (MMU *mmu, int page_number);
int tlb_FIFO (MMU *mmu);

void init_mmu(MMU *mmu) {
	for (int i = 0; i < 16; i++) {
		mmu->tlb_pages[i] = -1;
		mmu->tlb_frames[i] = -1;
	}

	mmu->tlb_index = 0;
	mmu->tlb_hits = 0;
}

int search_tlb(MMU *mmu, int page_number){
	for (int i = 0; i < 16; i++) {
		if (mmu->tlb_pages[i] == page_number) {
			mmu->tlb_hits++;
			return mmu-> tlb_frames[i];
		}
	}
	return -1;
}


int page_table_lookup(MMU *mmu, int page_number) {
	return mmu->page_table[page_number];
}

int allocation (MMU *mmu, int page_number) {
	page aPage = mmu->main_memory[mmu->next_open_frame];
	int evicted_tlb = tlb_FIFO(mmu);
	FILE *backing_bin = fopen("BACKING_STORE.bin", "r");
	fseek(backing_bin, (page_number*256), SEEK_SET);
	for (int i = 0; i < 256; i++) {
		aPage.bytes[i] = fgetc(backing_bin);
	}
	fclose("BACKING_STORE.bin");
	mmu->page_table[page_number] = mmu->next_open_frame;
	mmu->tlb_pages[evicted_tlb] = page_number;
	mmu->tlb_frames[evicted_tlb] = mmu->next_open_frame;
	mmu->next_open_frame = (mmu->next_open_frame + 1) % 256;
}