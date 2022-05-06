#include <filesystem>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

#include <dtb.hh>

struct mapped_file {
    char* data;
    size_t filesize;
};

static mapped_file read_file(const char* path) {
    size_t file_size = std::filesystem::file_size(path);
        
    char* buffer = new char[file_size];

    int fd = open(path, O_RDONLY);
    char* mapped_file_addr = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    std::memcpy(buffer, mapped_file_addr, file_size);

    munmap(mapped_file_addr, file_size);
    close(fd);

    return mapped_file{buffer, file_size};
}

int main(int argc, char** argv) {
    auto mapped_dtb = read_file(argv[1]);

    dtb::reader dtb_reader{mapped_dtb.data};
    if(!dtb_reader.ok()) {
        printf("Something went wrong!\n");
        printf("  · Magic number: %x\n", dtb_reader.magic());
        printf("  · Version: %d\n", dtb_reader.version());
        printf("  · Last compatible version: %d\n", dtb_reader.last_compatible_version());
        std::exit(1);
    }

    // Print important information about the header
    printf("DTB Header:\n");
    printf("  · Version: %d\n", dtb_reader.version());
    printf("  · Last compatible version: %d\n", dtb_reader.last_compatible_version());

    printf("Reserved areas:\n");
    dtb_reader.iterate_over_reserved_areas([](const dtb::reserved_area& area) {
        printf("  · Area:\n");
        printf("    · Address: %x\n", area.address());
        printf("    · Size:    %d\n", area.size());
        fflush(stdout);
    });
}
