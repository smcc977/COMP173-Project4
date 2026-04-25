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
void tlb_FIFO(MMU *mmu, int page_number, int frame_number);
int search_tlb(MMU *mmu, int page_number);
int page_table_lookup(MMU *mmu, int page_number);
int allocation(MMU *mmu, int page_number);


void tlb_FIFO(MMU *mmu, int page_number, int frame_number) {
	mmu->tlb_pages[mmu->tlb_index] = page_number;
	mmu->tlb_frames[mmu->tlb_index] = frame_number;

	mmu->tlb_index = (mmu->tlb_index + 1) % 16;
}

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

int allocation(MMU *mmu, int page_number) {
	FILE *backing_bin = fopen("BACKING_STORE.bin", "rb");
	if (!backing_bin) {
		perror("BACKING_STORE.bin");
		return -1;
	}
	fseek(backing_bin, page_number * 256, SEEK_SET);
	page *new_page = &mmu->main_memory[mmu->next_open_frame];
	fread(new_page->bytes, sizeof(uint8_t), 256, backing_bin);
	fclose(backing_bin);
	mmu->page_table[page_number] = mmu->next_open_frame;
	tlb_FIFO(mmu, page_number, mmu->next_open_frame);	
	mmu->next_open_frame = (mmu->next_open_frame + 1) % 256;
	mmu->page_faults++;
	return mmu->next_open_frame - 1;
}

int main(void) {
	MMU mmu;
	init_mmu(&mmu);
	
	/*
	mmu.tlb_pages[0] = 66;
	mmu.tlb_frames[0] = 9;
	int frame_hit = search_tlb(&mmu, 66);
	printf("Hit test: frame=%d, tlb_hits=%d\n", frame_hit, mmu.tlb_hits);
	int frame_miss = search_tlb(&mmu, 77);
	printf("Miss test: frame=%d, tlb_hits=%d\n", frame_miss, mmu.tlb_hits);
	*/
	return 0;
}