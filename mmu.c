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
int allocation(MMU *mmu, int page_number, const char *backing_store_path);
void init_page_table(MMU *mmu);
int print_address_trace(MMU *mmu, int logical_address, const char *backing_store_path);


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

void init_page_table(MMU *mmu) {
	for (int i = 0; i < 256; i++) {
		mmu->page_table[i] = -1;
	}
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

int allocation(MMU *mmu, int page_number, const char *backing_store_path) {
	FILE *backing_bin = fopen(backing_store_path, "rb");
	if (!backing_bin) {
		perror(backing_store_path);
		return -1;
	}
	int allocated_frame = mmu->next_open_frame;
	fseek(backing_bin, page_number * 256, SEEK_SET);
	page *new_page = &mmu->main_memory[allocated_frame];
	fread(new_page->bytes, sizeof(uint8_t), 256, backing_bin);
	fclose(backing_bin);
	mmu->page_table[page_number] = allocated_frame;
	tlb_FIFO(mmu, page_number, allocated_frame);
	mmu->next_open_frame = (mmu->next_open_frame + 1) % 256;
	mmu->page_faults++;
	return allocated_frame;
}

int print_address_trace(MMU *mmu, int logical_address, const char *backing_store_path) {
	int page_number = (logical_address >> 8) & 0xFF;
	int offset = logical_address & 0xFF;
	int frame_number;
	int value;

	printf("Reading logical address: %d\n", logical_address);
	printf("\tPage number: %d\n", page_number);
	printf("\tOffset: %d\n", offset);

	frame_number = search_tlb(mmu, page_number);
	if (frame_number != -1) {
		printf("\tTLB hit!\n");
	} else {
		printf("\tTLB miss!\n");
		frame_number = page_table_lookup(mmu, page_number);

		if (frame_number == -1) {
			printf("\tPage fault!\n");
			printf("\tFound free frame number: %d\n", mmu->next_open_frame);

			frame_number = allocation(mmu, page_number, backing_store_path);
			if (frame_number == -1) {
				return -1;
			}

			printf("\tPage loaded from disk to memory.\n");
			printf("\tPage table updated at index %d with frame number %d.\n", page_number, frame_number);
			printf("\tTLB updated with page number %d and frame number %d.\n", page_number, frame_number);
		} else {
			printf("\tPage table hit!\n");
			printf("\tTLB updated with page number %d and frame number %d.\n", page_number, frame_number);
		}
	}

	value = (int8_t)mmu->main_memory[frame_number].bytes[offset];
	printf("\tValue in the memory: %d\n", value);

	return 0;
}

int main(int argc, char *argv[]) {
	const char *backing_store_path = argv[1];
	const char *addresses_path = argv[2];

	MMU mmu;
	init_mmu(&mmu);
	mmu.next_open_frame = 0;
	mmu.page_faults = 0;
	init_page_table(&mmu);

	FILE *addresses = fopen(addresses_path, "r");
	if (!addresses) {
		perror(addresses_path);
		return 1;
	}

	int logical_address;

	while (fscanf(addresses, "%d", &logical_address) == 1) {
		if (print_address_trace(&mmu, logical_address, backing_store_path) == -1) {
			fclose(addresses);
			return 1;
		}
	}

	fclose(addresses);
	printf("\n\nAggregate page faults = %d\n", mmu.page_faults);
	printf("Aggregate TLB hits = %d\n", mmu.tlb_hits);

	return 0;
}