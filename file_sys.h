#ifndef FILE_SYS
#define FILE_SYS

typedef struct file file;

file open(char* pathname);

void read(file curr_file, void* buffer, uint16_t num_bytes);

void lseek(file curr_file, uint32_t offset, uint8_t location_specifier);

#endif