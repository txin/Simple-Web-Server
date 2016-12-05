#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>

#include "client_https.hpp"
#include "../common/logger.h"

using namespace SimpleWeb;

void Client<HTTPS>::open_file(std::string const& t_path) {
    m_source_file.open(t_path, std::ios_base::binary | std::ios_base::ate);

    if (m_source_file.fail()) {
        throw std::fstream::failure("Failed opening file " + t_path);
    }

    boost::filesystem::path p(t_path);
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("FileName", p.filename().string()));
    request("POST", "/upload", m_source_file, header);
    BOOST_LOG_TRIVIAL(trace) << "From open_file, finished uploading files";
}
