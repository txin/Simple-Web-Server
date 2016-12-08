#ifndef GLOBAL_H
#define GLOBAL_H

#include <unordered_map>
#include "../common/file.h"

class Global {
    std::unordered_map<std::string, Metadata> meta_map;
public:
    Global() {}
}

extern Global *global_ptr;
#endif
