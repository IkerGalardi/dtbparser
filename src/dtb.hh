#pragma once

#include <cstdint>
#include <cstdio>
#include <byteswap.h>

namespace dtb {
    constexpr uint32_t magic_number = 0xd00dfeed;

    namespace node_type {
        static uint32_t begin_node() { return __bswap_64(0x00000001); }
        static uint32_t end_node()   { return __bswap_64(0x00000002); }
        static uint32_t nop()        { return __bswap_64(0x00000001); }
        static uint32_t end()        { return __bswap_64(0x00000001); }
    };

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
        uint64_t address() const { return __bswap_64(m_address); }
        uint64_t size() const { return __bswap_64(m_size); }
    private:
        uint64_t m_address;
        uint64_t m_size;
    };

    class reader {
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

        void do_random_stuff();
    private:
        uint32_t memory_reservation_map_offset() { return __bswap_32(m_header->off_mem_rsvmap); }
        uint32_t structure_block_offset() { return __bswap_32(m_header->off_dt_struct); }
        uint32_t structure_strings_offset() { return __bswap_32(m_header->off_dt_strings); }

        header* m_header;
    };
}
