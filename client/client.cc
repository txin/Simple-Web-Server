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

//     m_source_file.seekg(0, m_source_file.end);
//     auto file_size = m_source_file.tellg();
//     m_source_file.seekg(0, m_source_file.beg);

//     boost::asio::streambuf write_buffer;
//     std::ostream request_stream(&write_buffer);
// //    std::iostream write_stream(&write_buffer);



//     BOOST_LOG_TRIVIAL(trace) << "Filename: " << p.filename().string()
//                              << "file size" << file_size;
//     request_stream << p.filename().string() << "\n" << file_size << "\n\n";
//     BOOST_LOG_TRIVIAL(trace) << "Request size: " << m_request.size();

//     if (m_source_file) {
//         m_source_file.read(m_buf.data(), m_buf.size());
//         if (m_source_file.fail() && !m_source_file.eof()) {
//             auto msg = "Failed while reading file";
//             BOOST_LOG_TRIVIAL(error) << msg;
//             throw std::fstream::failure(msg);
//         }
//         // TODO: file_name
// //        request_stream << m_source_file.rdbuf();
//     }
//     std::stringstream ss;
    
//     ss << "Send " << m_source_file.gcount() << " bytes, total: "
//        << m_source_file.tellg() << " bytes";
//     BOOST_LOG_TRIVIAL(trace) << ss.str();

//     auto buf = boost::asio::buffer(m_buf.data(),
//                                    static_cast<size_t>(m_source_file.gcount()));
    boost::filesystem::path p(t_path);
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("FileName", p.filename().string()));
    request("POST", "/upload", m_source_file, header);
}

// void Client::write_file(const boost::system::error_code& t_ec) {
//     if (!t_ec) {
//         if (m_source_file) {
//             m_source_file.read(m_buf.data(), m_buf.size());
//             if (m_source_file.fail() && !m_source_file.eof()) {
//                 auto msg = "Failed while reading file";
//                 BOOST_LOG_TRIVIAL(error) << msg;
//                 throw std::fstream::failure(msg);
//             }
//         }
//         std::stringstream ss;
//         ss << "Send " << m_source_file.gcount() << " bytes, total: "
//            << m_source_file.tellg() << " bytes";

//         BOOST_LOG_TRIVIAL(trace) << ss.str();
//         std::cout << ss.str() << std::endl;

//         auto buf = boost::asio::buffer(m_buf.data(),
//                                        static_cast<size_t>(m_source_file.gcount()));
//         write_buffer(buf);
//     } else {
//         BOOST_LOG_TRIVIAL(error) << "Error: " << t_ec.message();
//     }
// }
