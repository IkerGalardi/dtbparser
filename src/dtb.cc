#include "dtb.hh"

#include <cstdio>

namespace dtb {
    reader::reader(void* ptr) 
        : m_header(static_cast<header*>(ptr))
    {

    }

    bool reader::ok() {
        // Magic number should be '0xd00dfeed', if not, not a real dtb file.
        if(magic() != dtb::magic_number) {
            return false;
        }

        if(version() != 17) {
            return false;
        }

        return true;
    }

    char* node_name(structure_node node) {
        node++;
        return reinterpret_cast<char*>(node);
    }

    char* prop_name(reader reader, structure_node node) {
        auto header_ptr = reinterpret_cast<uintptr_t>(reader.m_header);
        char* strings = reinterpret_cast<char*>(header_ptr + reader.structure_string_offset());

        uint32_t property_string_offset = __bswap_32(*(node+2));

        return strings + property_string_offset;
    }
}
