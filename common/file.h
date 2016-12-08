#pragma once
#include <fstream>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "../client/client_https.hpp"

#include "../server/server_http.hpp"
#include "../server/server_https.hpp"


typedef SimpleWeb::Server<SimpleWeb::HTTPS> HttpsServer;
typedef SimpleWeb::Client<SimpleWeb::HTTPS> HttpsClient;

enum SecurityFlag{CONFIDENTIALITY, INTEGRITY, NONE};

int write_file(std::shared_ptr<HttpsServer::Request> request,
               const CryptoPP::RSA::PublicKey &server_pk);
int write_file(std::string const& path, std::shared_ptr<HttpsClient::Response> response);

// Each file is associated with some metadata
class Metadata {
private:
    std::string request_file_name;
    std::string uid_file_name; //unique
    std::vector<int> owner_list; //uid
    int security_flag; // 0, 1, 2
    
public:
    Metadata(std::string t_request_file_name, int t_uid, int t_security_flag) {
        request_file_name = t_request_file_name;
        std::string new_file_name(std::to_string(t_uid) + '_' + t_request_file_name);
        uid_file_name = new_file_name;
        BOOST_LOG_TRIVIAL(trace) << "meta_data: uid_file_name: " << uid_file_name;
        security_flag = t_security_flag;
        owner_list.push_back(t_uid);
    }
};
