#include <fstream>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "../server/server_http.hpp"
#include "../server/server_https.hpp"


typedef SimpleWeb::Server<SimpleWeb::HTTPS> HttpsServer;
typedef SimpleWeb::Client<SimpleWeb::HTTPS> HttpsClient;

int write_file(std::string const& path, std::shared_ptr<HttpsServer::Request> request) {
    // write file
    std::ofstream m_output_file;
    enum {Max_length = 40960};
    std::array<char, Max_length> m_buf;

//    BOOST_LOG_TRIVIAL(trace) << "filename: " << file_name;
    m_output_file.open(path, std::ios_base::binary);
//    m_output_file.open("web/upload/" + file_name, std::ios_base::binary);

    do {
        request->content.read(m_buf.data(), m_buf.size());
        BOOST_LOG_TRIVIAL(trace) << __func__ << " write " << request->content.gcount() <<
            " bytes.";
        m_output_file.write(m_buf.data(), request->content.gcount());
    } while (request->content.gcount() > 0);
        
    if (!m_output_file){
        BOOST_LOG_TRIVIAL(error) << __LINE__ << ": Failed to create: "   << path;
        return -1;
    }
    return 0;
};


int write_file(std::string const& path, std::shared_ptr<HttpsClient::Response> response) {
    // write file
    std::ofstream m_output_file;
    enum {Max_length = 40960};
    std::array<char, Max_length> m_buf;

//    BOOST_LOG_TRIVIAL(trace) << "filename: " << file_name;
    m_output_file.open(path, std::ios_base::binary);
//    m_output_file.open("web/upload/" + file_name, std::ios_base::binary);

    do {
        response->content.read(m_buf.data(), m_buf.size());
        BOOST_LOG_TRIVIAL(trace) << __func__ << " write " << response->content.gcount() <<
            " bytes.";
        m_output_file.write(m_buf.data(), response->content.gcount());
    } while (response->content.gcount() > 0);
        
    if (!m_output_file){
        BOOST_LOG_TRIVIAL(error) << __LINE__ << ": Failed to create: "   << path;
        return -1;
    }
    return 0;
};


