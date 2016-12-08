#include <iostream>
#include <string>

#include <chrono>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>

#include "client_https.hpp"
#include "../common/logger.h"

using namespace SimpleWeb;

void Client<HTTPS>::check_in(std::string const& t_path,
                             const std::string &username, int flag) {
    m_source_file.open(t_path, std::ios_base::binary | std::ios_base::ate);

    if (m_source_file.fail()) {
        throw std::fstream::failure("Failed opening file " + t_path);
    }

    boost::filesystem::path p(t_path);
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("UserName", username));
    header.insert(std::make_pair("FileName", p.filename().string()));
    header.insert(std::make_pair("SecurityFlag", std::to_string(flag)));
    request("POST", "/upload", m_source_file, header);
}

void Client<HTTPS>::delegate(int fid, const std::string &client_name,
                             int time, bool propagation_flag) {
    // TODO: Server first check the delegate right
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("FID", std::to_string(fid)));
    header.insert(std::make_pair("ClientName", client_name));
    request("POST", "/delegate", "No Content", header);

    // If is OK then proceed, otherwise, end request;

    // Get message to sign
    std::chrono::time_point<std::chrono::system_clock> clock;
    clock = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(clock);
    std::string timestamp_str(std::ctime(&timestamp));

    std::ostringstream ss;
    ss << "FID:" << fid << " Timestamp:" << timestamp_str << " Timeframe:"
       << time << " Propagation:" << propagation_flag;
    BOOST_LOG_TRIVIAL(trace) << ss.str();

    // TODO: Sign the message and send to the server
        // Sign the time stamp and the expiration time and the propatation_flag;
    // TODO: Server verify the signature, and update the metadata
}

void Client<HTTPS>::safe_delete(int fid) {
    
}

void Client<HTTPS>::check_out(int fid) {
    
}
