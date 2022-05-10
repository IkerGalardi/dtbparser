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

struct parser_flags {
    bool show_header = false;
    bool show_reservation_map = false;
    bool show_tree_nodes = false;
};

static void print_helpfull_error_message() {
    fprintf(stderr, "Usage: dtbparser [-hrt] file\n");
    fprintf(stderr, "Where the arguments specify...\n");
    fprintf(stderr, "  -h : show the header of the DTB file.\n");
    fprintf(stderr, "  -r : show the reservation map of the DTB file\n");
    fprintf(stderr, "  -t : show the tree nodes of the DTB file\n");
}

parser_flags get_flags(int argc, char** argv) {
    parser_flags flags;

    int option = -1;
    while((option = getopt(argc, argv, "hrtH")) != -1) {
        switch(option) {
        case 'h':
            flags.show_header = true;
            break;

        case 'r':
            flags.show_reservation_map = true;
            break;

        case 't':
            flags.show_tree_nodes = true;
            break;

        case 'H':
            print_helpfull_error_message();
            std::exit(1);
        }
    }

    return flags;
}

int main(int argc, char** argv) {
    auto flags = get_flags(argc, argv);

    // If no file argument has been passed, then print an error message and quit.
    if(optind >= argc) {
        print_helpfull_error_message();
        std::exit(1);
    }

    auto mapped_dtb = read_file(argv[optind]);

    dtb::reader dtb_reader{mapped_dtb.data};
    if(!dtb_reader.ok()) {
        fprintf(stderr, "dtbparser: File %s is not a correct DTB file.\n", argv[optind]);
        std::exit(2);
    }

    // Print important information about the header
    if(flags.show_header) {
        printf("Header:\n");
        printf("  · Magic: %d\n", dtb_reader.magic());
        printf("  · Version: %d\n", dtb_reader.version());
        printf("  · Last compatible version: %d\n", dtb_reader.last_compatible_version());
    }

    if(flags.show_reservation_map) {
        printf("Reserved areas:\n");
        dtb_reader.iterate_over_reserved_areas([](const dtb::reserved_area& area) {
            printf("  · Area:\n");
            printf("    · Address: %x\n", area.address());
            printf("    · Size:    %d\n", area.size());
            fflush(stdout);
        });
    }

    if(flags.show_tree_nodes) {
        printf("Structure Blocks:\n");
        dtb_reader.iterate_over_structure_blocks([&](dtb::structure_node node) {
            std::string tabs{};
            if(dtb::is_begin_node(node)) {
                printf("%sNode: \n", tabs.c_str(), dtb::node_name(node));
                tabs.push_back('\t');
            } else if(dtb::is_end_node(node)) {
                tabs.pop_back();
            } else if(dtb::is_prop(node)) {
                printf("%s%s", tabs.c_str(), dtb::prop_name(dtb_reader, node));
            } 
        });
    }
}
