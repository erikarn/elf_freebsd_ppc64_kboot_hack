#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>
#include <err.h>

/* TODO: endianness! */

const Elf64_Phdr *
find_phdr(const uint8_t *mem, size_t size, int hdrtype)
{
	const Elf64_Ehdr *ehdr = (Elf64_Ehdr *)mem;
	const Elf64_Phdr *phdr;
	int i;

	/* TODO: bounds checking! */
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = (const Elf64_Phdr *) (mem + ehdr->e_phoff +
		    (i * ehdr->e_phentsize));
		if (phdr->p_type == hdrtype)
			return phdr;
	}
	return NULL;
}

int main(int argc, const char *argv[])
{
    int fd;
    struct stat st;
    const Elf64_Phdr *phdr;
    Elf64_Addr eaddr;

    fd = open(argv[1], O_RDWR);
    fstat(fd, &st);

    printf("file: %s\n", argv[1]);
    printf("len: %ld\n", st.st_size);

    // Map the ELF file into memory
    uint8_t *mem = mmap(NULL, st.st_size,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        err(127, "%s: mmap", __func__);
    }

    phdr = find_phdr(mem, st.st_size, PT_PHDR);
    if (phdr == NULL) {
        printf("Couldn't find PT_PHDR!\n");
        exit(1);
    }

    // Cast to ELF64 header
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)mem;
    eaddr = ehdr->e_entry;
    eaddr -= phdr->p_vaddr;

    // Update fields
    ehdr->e_entry = eaddr;

    // Clean up
    msync(mem, st.st_size, MS_SYNC);
    munmap(mem, st.st_size);
    close(fd);
    exit(0);
}
