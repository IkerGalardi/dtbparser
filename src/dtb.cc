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
}
