#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "logger.h"

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::set_options(std::string const& t_file_name,
                         unsigned t_rotation_size, unsigned t_max_size) {

    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
//    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    
    // boost::log::add_file_log(
    //     boost::log::keywords::file_name = t_file_name,
    //     boost::log::keywords::rotation_size = t_rotation_size,
    //     boost::log::keywords::max_size = t_max_size,
    //     boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0), // ?? TODO: necessayr
    //     boost::log::keywords::format = "[%TimeStamp%]: %Message%", 
    //     boost::log::keywords::auto_flush = true
    //     );
    // boost::log::add_common_attributes();
}
