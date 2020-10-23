#include "file_sys.h"

struct file {

	uint32_t current_addr;	
	uint32_t current_FAT_cluster;
};


file open(char* filename) {
	
}

void read(file curr_file, void* buffer, uint16_t num_bytes) {
	
}

void lseek(file curr_file, uint32_t offset, uint8_t location_specifier) {
	
}