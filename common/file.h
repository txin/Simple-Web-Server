#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "../client/client_https.hpp"
#include "../server/server_https.hpp"

using namespace SimpleWeb;
typedef SimpleWeb::Server<SimpleWeb::HTTPS> HttpsServer;
typedef SimpleWeb::Client<SimpleWeb::HTTPS> HttpsClient;

enum SecurityFlag{CONFIDENTIALITY, INTEGRITY, NONE};

int write_file(std::shared_ptr<HttpsServer::Request> request,
               const CryptoPP::RSA::PublicKey &server_pk);
int write_file(std::string const& path, std::shared_ptr<HttpsClient::Response> response);

void create_folder(const char *name);

#endif
