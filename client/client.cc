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
#include "../common/encrypt.h"
#include "../common/file.h"

#include <cryptopp/pssr.h>

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
    m_source_file.close();
}

void Client<HTTPS>::delegate(int fid, const std::string &client_name,
                             Rights &rights,
                             int time, bool propagation_flag) {

    std::map<std::string, std::string> header;
    header.insert(std::make_pair("FID", std::to_string(fid)));
    header.insert(std::make_pair("UserName", username));
    header.insert(std::make_pair("ClientName", client_name));
    header.insert(std::make_pair("check_in", std::to_string(rights.check_in)));
    header.insert(std::make_pair("check_out", std::to_string(rights.check_out)));
    header.insert(std::make_pair("is_delegate", std::to_string(rights.is_delegate)));
    header.insert(std::make_pair("is_owner", std::to_string(rights.is_owner)));
    header.insert(std::make_pair("time", std::to_string(time)));

    // Get message to sign
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(start);

    end = start + std::chrono::seconds(time);
    std::time_t timestamp_new = std::chrono::system_clock::to_time_t(end);

    std::string timestamp_str(std::ctime(&timestamp));
    std::string expire_time_str(std::ctime(&timestamp_new));
    
    std::ostringstream ss;

    // TODO: Sign the message (expire time) and send to the server
    ss << "FID:" << fid << " Timestamp:" << timestamp_str << " Timeframe(sec):"
       << time << " Expire:" << expire_time_str << " Propagation:" << propagation_flag;

    std::string message = ss.str();
    BOOST_LOG_TRIVIAL(trace) << message;

    std::string key_file = "certs/" + username + "_private.pem";
    CryptoPP::RSA::PrivateKey sk;
    load_private_key(key_file, sk);

    CryptoPP::AutoSeededRandomPool rng;
    // Sign the message
    CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA1>::Signer signer(sk);
    // Create signature space
    size_t length = signer.MaxSignatureLength();
    CryptoPP::SecByteBlock signature(length);
// Sign message
    length = signer.SignMessage(rng, (const byte*) message.c_str(),
                                message.length(), signature);
// Resize now we know the true size of the signature
    signature.resize(length);
    request("POST", "/delegate", std::string((const char *)signature.data(),
                                             signature.size()), header);
}

void Client<HTTPS>::check_out(int fid) {
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("UserName", username));    
    auto response = request("GET", "/upload", std::to_string(fid), header);
    std::string file_name = "temp";
//    global_ptr->lookup_check_out(fid, username, file_name);
    write_file(username + "/" + file_name, response);
}

void Client<HTTPS>::safe_delete(int fid) {
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("UserName", username));    
    request("POST", "/delete", std::to_string(fid), header);
}

void Client<HTTPS>::end_session() {
    std::map<std::string, std::string> header;
    header.insert(std::make_pair("UserName", username));    
    // end session
}
