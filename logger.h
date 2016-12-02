#pragma once

#include <string>

class Logger {
    Logger() {}
public:
    static Logger& instance();
    static void set_options(std::string const& t_file_name,
                         unsigned t_rotation_size, unsigned t_max_size);
};
