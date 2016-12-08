#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>

#include "client_https.hpp"
#include "../common/logger.h"

using namespace SimpleWeb;

void Client<HTTPS>::check_in(std::string const& t_path, int uid, int flag) {
    m_source_file.open(t_path, std::ios_base::binary | std::ios_base::ate);

    if (m_source_file.fail()) {
        throw std::fstream::failure("Failed opening file " + t_path);
    }

    boost::filesystem::path p(t_path);
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("UID", std::to_string(uid)));
    header.insert(std::make_pair("FileName", p.filename().string()));
    header.insert(std::make_pair("SecurityFlag", std::to_string(flag)));
    request("POST", "/upload", m_source_file, header);
    BOOST_LOG_TRIVIAL(trace) << "From open_file, finished uploading files";
}
