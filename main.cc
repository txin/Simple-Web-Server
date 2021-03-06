#include "server/server_https.hpp"
#include "server/global.h"
#include "client/client_https.hpp"

#include "common/logger.h"
#include "common/file.h"
#include "common/encrypt.h"

//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/log/trivial.hpp>

//Added for the default_resource example
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <vector>
#include <algorithm>

using namespace std;
//Added for the json-example:
using namespace boost::property_tree;

typedef SimpleWeb::Server<SimpleWeb::HTTPS> HttpsServer;
typedef SimpleWeb::Client<SimpleWeb::HTTPS> HttpsClient;

Global *global_ptr = 0;

//Added for the default_resource example
void default_resource_send(const HttpsServer &server,
                           const shared_ptr<HttpsServer::Response> &response,
                           const shared_ptr<ifstream> &ifs);
int main() {
    if (!global_ptr) {
        global_ptr = new Global;
    }
    
    //HTTPS-server at port 8080 using 1 thread
    //Unless you do more heavy non-threaded processing in the resources,
    //1 thread is usually faster than several threads
    
    Logger::instance().set_options("server_%3N.log", 1 * 1024 * 1024, 10 * 1024 * 1024);
    HttpsServer server(8081, 1, "certs/server_cert.crt", "certs/server_private.pem",
                       5, 300, "certs/demoCA/cacert.pem");

//Add resources using path-regex and method-string, and an anonymous function
    //POST-example for the path /string, responds the posted string
    server.resource["^/string$"]["POST"]=[](shared_ptr<HttpsServer::Response> response,
                                            shared_ptr<HttpsServer::Request> request) {
        
        //Retrieve string:
        auto content=request->content.string();
        
        //request->content.string() is a convenience function for:
        //stringstream ss;
        //ss << request->content.rdbuf();
        //string content=ss.str();
        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length()
        << "\r\n\r\n" << content;
    };

    CryptoPP::RSA::PublicKey server_pk;
    load_public_key("certs/server_public.pem", server_pk);
    
    server.resource["^/upload$"]["POST"]=[server_pk]
        (shared_ptr<HttpsServer::Response> response,
                                            shared_ptr<HttpsServer::Request> request) {
        thread work_thread([response, request, server_pk] {
                write_file(request, server_pk);
                string str = "OK";
                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << str.length()
                          << "\r\n\r\n" << str;
            });
        work_thread.detach();
    };

    server.resource["^/delete"]["POST"]=[]
        (shared_ptr<HttpsServer::Response> response,
         shared_ptr<HttpsServer::Request> request) {
//         auto content=request->content.string();
        auto web_root_path=boost::filesystem::canonical("web");
        int fid = std::stoi(request->content.string());

// look up metadata
        std::string user_name;
        for (auto& header: request->header) {
            if (header.first == "UserName") {
                user_name = header.second;
            }
        }
        std::string file_name;
        bool allow_delete = global_ptr->lookup_delete(fid, user_name, file_name);
        if (allow_delete) {
            string path_upload = "upload";
            auto path = boost::filesystem::canonical(web_root_path/path_upload/file_name);
            BOOST_LOG_TRIVIAL(trace) << "File removing: " << path;            
            try {
                if (boost::filesystem::exists(path)) {
                    boost::filesystem::remove(path);
                }
            }  catch(boost::filesystem::filesystem_error const & e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
            }
            std::string str = "Server: File Removed";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << str.length()
                      << "\r\n\r\n" << str;
            
            BOOST_LOG_TRIVIAL(trace) << "File removed: " << path;            
        } else {
            std::string str = "No checkout";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << str.length() << "\r\n\r\n"
            << str;
        }
    };
    
    server.resource["^/delegate$"]["POST"]=[](shared_ptr<HttpsServer::Response> response,
                                              shared_ptr<HttpsServer::Request> request) {
        thread work_thread([response, request] {
                auto content = request->content.string();
                bool result = verify_delegation_request(request);
                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << 1
                          << "\r\n\r\n" << result;
            });
        work_thread.detach();
    };

    server.resource["^/json$"]["POST"]=[](shared_ptr<HttpsServer::Response> response,
                                          shared_ptr<HttpsServer::Request> request) {
        try {
            ptree pt;
            read_json(request->content, pt);
            string name=pt.get<string>("firstName")+" "+pt.get<string>("lastName");

            *response << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << name.length() << "\r\n\r\n"
            << name;
        }
        catch(exception& e) {
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: "
            << strlen(e.what()) << "\r\n\r\n" << e.what();
        }
    };
    
    //GET-example for the path /info
    //Responds with request-information
    server.resource["^/info$"]["GET"]=[](shared_ptr<HttpsServer::Response> response,
                                         shared_ptr<HttpsServer::Request> request) {
        stringstream content_stream;
        content_stream << "<h1>Request from " << request->remote_endpoint_address
        << " (" << request->remote_endpoint_port << ")</h1>";
        content_stream << request->method << " " << request->path << " HTTP/"
        << request->http_version << "<br>";

        for(auto& header: request->header) {
            content_stream << header.first << ": " << header.second << "<br>";
        }
        
        //find length of content_stream (length received using content_stream.tellp())
        content_stream.seekp(0, ios::end);
        
        *response <<  "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp()
        << "\r\n\r\n" << content_stream.rdbuf();
    };
    
    //GET-example for the path /match/[number], responds with the matched string
    //in path (number)
    //For instance a request GET /match/123 will receive: 123
    server.resource["^/match/([0-9]+)$"]["GET"] =
        [&server](shared_ptr<HttpsServer::Response> response,
                  shared_ptr<HttpsServer::Request> request) {
        string number=request->path_match[1];
        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << number.length()
        << "\r\n\r\n" << number;
    };
    
    //Get example simulating heavy work in a separate thread
    server.resource["^/work$"]["GET"]=[&server](shared_ptr<HttpsServer::Response> response,
                                                shared_ptr<HttpsServer::Request> /*request*/) {
        thread work_thread([response] {
                this_thread::sleep_for(chrono::seconds(5));
                string message="Work done";
                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << message.length()
                          << "\r\n\r\n" << message;
            });
        work_thread.detach();
    };


    //Get example simulating heavy work in a separate thread
    server.resource["^/upload$"]["GET"]=[&server](shared_ptr<HttpsServer::Response> response,
                                                  shared_ptr<HttpsServer::Request> request) {
        auto web_root_path=boost::filesystem::canonical("web");
        int fid = std::stoi(request->content.string());

// look up metadata
        std::string user_name;
        for (auto& header: request->header) {
            if (header.first == "UserName") {
                user_name = header.second;
            }
        }
        std::string file_name;
        bool allow_checkout = global_ptr->lookup_check_out(fid, user_name, file_name);
        if (allow_checkout) {
//            auto file_name = request->content.string();
            auto path = boost::filesystem::canonical(web_root_path/request->path/file_name);
            auto ifs = make_shared<ifstream>();

            ifs->open(path.string(), ifstream::in | ios::binary);
            if(*ifs) {
                BOOST_LOG_TRIVIAL(trace) << "File is opened";
                ifs->seekg(0, ios::end);
                auto length=ifs->tellg();
                ifs->seekg(0, ios::beg);
                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << length << "\r\n\r\n";
                BOOST_LOG_TRIVIAL(trace) << "File length" << length;
                default_resource_send(server, response, ifs);
            }  else {
                throw invalid_argument("could not read file");
            }
            BOOST_LOG_TRIVIAL(trace) << "Download" << path;            
        } else {
            std::string str = "No checkout";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << str.length() << "\r\n\r\n"
            << str;
        }
    };

    //Default GET-example. If no other matches, this anonymous function will be called. 
    //Will respond with content in the web/-directory, and its subdirectories.
    //Default file: index.html
    //Can for instance be used to retrieve an HTML 5 client that
    //uses REST-resources on this server
    server.default_resource["GET"]=[&server](shared_ptr<HttpsServer::Response> response,
                                             shared_ptr<HttpsServer::Request> request) {
        try {
            auto web_root_path=boost::filesystem::canonical("web");
            auto path=boost::filesystem::canonical(web_root_path/request->path);
            //Check if path is within web_root_path
            if(distance(web_root_path.begin(),
                        web_root_path.end())>distance(path.begin(), path.end()) ||
               !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw invalid_argument("path must be within root path");
            if(boost::filesystem::is_directory(path))
                path/="index.html";
            if(!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
                throw invalid_argument("file does not exist");
            
            auto ifs=make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary);
            
            if(*ifs) {
                ifs->seekg(0, ios::end);
                auto length=ifs->tellg();
                
                ifs->seekg(0, ios::beg);
                
                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << length << "\r\n\r\n";
                default_resource_send(server, response, ifs);
            }
            else
                throw invalid_argument("could not read file");
        }
        catch(const exception &e) {
            string content="Could not open path "+request->path+": "+e.what();
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: "
            << content.length() << "\r\n\r\n" << content;
        }
    };
    
    thread server_thread([&server](){
            //Start server
            server.start();
        });
    
    //Wait for server to start so that the client can connect
    this_thread::sleep_for(chrono::seconds(1));
    
    //Client examples
    HttpsClient Alice("localhost:8081", true,
                       "certs/Alice_cert.pem", "certs/Alice_private.pem",
                       "certs/demoCA/cacert.pem");

    HttpsClient Bob("localhost:8081", true,
                    "certs/Bob_cert.pem", "certs/Bob_private.pem",
                       "certs/demoCA/cacert.pem");

    HttpsClient Eve("localhost:8081", true,
                    "certs/Eve_cert.pem", "certs/Eve_private.pem",
                       "certs/demoCA/cacert.pem");
    // upload file
    // uid
    // security flag: 0 NONE, 1 CONFIDENTIALITY, 2, INTEGRITY
    // TODO: change uid as part of start session.
    Alice.check_in("Alice/test.jpg", "Alice", 0);
    Alice.check_in("Alice/a.txt", "Alice", 0);
    
//    Bob.check_in("Bob/test.jpg", "Bob", 1);
    Bob.check_in("Bob/b.txt", "Bob", 1);
//    Eve.check_in("Eve/test.jpg", "Eve", 2);
    Eve.check_in("Eve/c.txt", "Eve", 2);

    // time: 10000 secs
    // Alice delegate Bob
// make request
    BOOST_LOG_TRIVIAL(trace) << "finished uploading files";
    global_ptr->print_metadata();

    Rights test_rights;
    test_rights.is_delegate = true;
    test_rights.check_in = true;
    Alice.delegate(0, "Bob", test_rights, 10000, false);
    Alice.check_out(2);
// checkout file
    int test_fid = 0;
    Alice.check_out(test_fid);
    Alice.safe_delete(test_fid);    
    Alice.close();
    
    server_thread.join();
    return 0;
}

void default_resource_send(const HttpsServer &server,
                           const shared_ptr<HttpsServer::Response> &response,
                           const shared_ptr<ifstream> &ifs) {
    //read and send 128 KB at a time
    static vector<char> buffer(131072); // Safe when server is running on one thread
    streamsize read_length;
    if((read_length=ifs->read(&buffer[0], buffer.size()).gcount())>0) {
        response->write(&buffer[0], read_length);
        if(read_length==static_cast<streamsize>(buffer.size())) {
            server.send(response,
                        [&server, response, ifs](const boost::system::error_code &ec) {
                            if(!ec)
                                default_resource_send(server, response, ifs);
                            else
                                cerr << "Connection interrupted" << endl;
                        });
        }
    }
}
