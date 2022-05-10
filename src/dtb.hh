#pragma once

#include <cstdint>
#include <cstdio>
#include <byteswap.h>

#include <string>

namespace dtb {
    constexpr uint32_t magic_number = 0xd00dfeed;

    typedef uint32_t* structure_node;

    static uint32_t is_begin_node(structure_node node) { return __bswap_32(*node) == 0x00000001; }
    static uint32_t is_end_node(structure_node node)   { return __bswap_32(*node) == 0x00000002; }
    static uint32_t is_prop(structure_node node)       { return __bswap_32(*node) == 0x00000003; }
    static uint32_t is_nop(structure_node node)        { return __bswap_32(*node) == 0x00000004; }
    static uint32_t is_end(structure_node node)        { return __bswap_32(*node) == 0x00000009; }

    struct header {
        uint32_t magic;
        uint32_t totalsize;
        uint32_t off_dt_struct;
        uint32_t off_dt_strings;
        uint32_t off_mem_rsvmap;
        uint32_t version;
        uint32_t last_comp_version;
        uint32_t boot_cpuid_phys;
        uint32_t size_dt_strings;
        uint32_t size_dt_struct;
    };

    class reserved_area {
    public:
        reserved_area() = delete;

        uint64_t address() const { return __bswap_64(m_address); }
        uint64_t size() const { return __bswap_64(m_size); }
    private:
        uint64_t m_address;
        uint64_t m_size;
    };


    class reader {
        friend char* prop_name(reader reader, structure_node node);
    public:
        reader() = delete;
        reader(void* ptr);

        bool ok();

        uint32_t magic() const { return __bswap_32(m_header->magic); }
        uint32_t version() const { return __bswap_32(m_header->version); }
        uint32_t last_compatible_version() const { return __bswap_32(m_header->last_comp_version); }
        uint32_t boot_cpuid() const { return __bswap_32(m_header->boot_cpuid_phys); }

        template<typename function>
        void iterate_over_reserved_areas(function func) {
            auto header_ptr = reinterpret_cast<uintptr_t>(m_header);
            reserved_area* area_node = reinterpret_cast<reserved_area*>(header_ptr + memory_reservation_map_offset());
            
            // SPEC: if both address and size are zero, thats the last node.
            while(!(area_node->address() == 0 && area_node->size() == 0)) {
                func(*area_node);

                area_node++;
            }
        }

        template<typename function>
        void iterate_over_structure_blocks(function func) {
            auto header_ptr = reinterpret_cast<uintptr_t>(m_header);
            uint32_t* something = reinterpret_cast<uint32_t*>(header_ptr + structure_block_offset());

            std::string tabs = "";

            while(!is_end(something)) {
                if(is_begin_node(something)) {
                    func(something);
                    something++;
                    while(*something != '\0') {
                        something++;
                    }
                } else if(is_prop(something)) {
                    uint32_t property_length = __bswap_32(*(something+1));
                    uint32_t property_string_offset = __bswap_32(*(something+2));
                    auto property_pointer = header_ptr + structure_string_offset() + property_string_offset;
                    printf("%sProperty: name = \"%s\"\n", tabs.c_str(), property_pointer);
                    
                    something += property_length / sizeof(uint32_t);
                } else if(is_end_node(something)) {
                    tabs.pop_back();
                    printf("%sEND_NODE\n", tabs.c_str());
                } else if(is_nop(something)) {
                    something++;
                    continue;
                } 

                something++;
            }
        }
    private:
        uint32_t memory_reservation_map_offset() { return __bswap_32(m_header->off_mem_rsvmap); }
        uint32_t structure_block_offset() { return __bswap_32(m_header->off_dt_struct); }
        uint32_t structure_string_offset() { return __bswap_32(m_header->off_dt_strings); }


        header* m_header;

        uint32_t* cpu_node_pointer = nullptr;
        uint32_t* memory_node_pointer = nullptr;
    };

    char* node_name(structure_node node);

    char* prop_name(reader reader, structure_node node);
}
