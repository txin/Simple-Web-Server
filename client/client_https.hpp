#ifndef CLIENT_HTTPS_HPP
#define	CLIENT_HTTPS_HPP

#include <fstream>
#include <iostream>

#include "../common/type.h"

#include "client_http.hpp"

#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>


namespace SimpleWeb {
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> HTTPS;

    template<>
    class Client<HTTPS> : public ClientBase<HTTPS> {
    public:
        
        Client(const std::string& server_port_path, bool verify_certificate=true, 
               const std::string& cert_file=std::string(), const std::string& private_key_file=std::string(), 
               const std::string& verify_file=std::string()) : 
            ClientBase<HTTPS>::ClientBase(server_port_path, 443), context(boost::asio::ssl::context::tlsv12) {

            
            if(verify_certificate) {
                context.set_verify_mode(boost::asio::ssl::verify_peer
                                        | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
                context.set_default_verify_paths();
            }
            else
                context.set_verify_mode(boost::asio::ssl::verify_none);
            
            if(cert_file.size()>0 && private_key_file.size()>0) {
                context.use_certificate_chain_file(cert_file);
                context.use_private_key_file(private_key_file, boost::asio::ssl::context::pem);
            }
            
            if(verify_file.size()>0)
                context.load_verify_file(verify_file);

            std::size_t found_0 = cert_file.find("/");
            std::size_t found_1 = cert_file.find("_");
            if (found_0 != std::string::npos && found_1 != std::string::npos) {
                std::string folder_name = cert_file.substr(found_0 + 1,
                                                           found_1 - found_0 - 1);
                username = folder_name;
                BOOST_LOG_TRIVIAL(trace) << folder_name;
                create_folder(folder_name.c_str());
            } else {
                std::cerr << "Error: wrong certificate input.";
                exit(1);
            }
        }

// create folder if it does not exsits
        void create_folder(const char* name) {
            // get working dir
            char cwd[1024];
            char path[1024];
            getcwd(cwd, sizeof(cwd));
            strcpy(path, cwd);
            strcat(path, "/");
            strcat(path, name);
            puts(path);
            struct stat st = {0};
            if (stat(path, &st) == -1) {
                mkdir(path, 0700);
            }
        }

        void start_session(const std::string &hostname);
        void check_in(const std::string & t_path, const std::string &username, int flag);
        void delegate(int fid, const std::string &client_name,
                      Rights &rights,
                      int time, bool propagation_flag);
        void safe_delete(int fid);
        void check_out(int fid);
        void end_session();
        

        std::ifstream m_source_file;
        boost::asio::streambuf m_request;
        void send_file(const Client<HTTPS> &client, const std::shared_ptr<std::ifstream> &ifs);
        enum {Message_size = 1024};
        std::array<char, Message_size> m_buf;
        
    protected:
        boost::asio::ssl::context context;
        std::string username;
        
        void connect() {
            if(!socket || !socket->lowest_layer().is_open()) {
                std::unique_ptr<boost::asio::ip::tcp::resolver::query> query;
                if(config.proxy_server.empty())
                    query=std::unique_ptr<boost::asio::ip::tcp::resolver::query>(new boost::asio::ip::tcp::resolver::query(host, std::to_string(port)));
                else {
                    auto proxy_host_port=parse_host_port(config.proxy_server, 8080);
                    query=std::unique_ptr<boost::asio::ip::tcp::resolver::query>(new boost::asio::ip::tcp::resolver::query(proxy_host_port.first, std::to_string(proxy_host_port.second)));
                }
                resolver.async_resolve(*query, [this]
                                       (const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it){
                                           if(!ec) {
                                               {
                                                   std::lock_guard<std::mutex> lock(socket_mutex);
                                                   socket=std::unique_ptr<HTTPS>(new HTTPS(io_service, context));
                                               }
                        
                                               boost::asio::async_connect(socket->lowest_layer(), it, [this]
                                                                          (const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator /*it*/){
                                                                              if(!ec) {
                                                                                  boost::asio::ip::tcp::no_delay option(true);
                                                                                  this->socket->lowest_layer().set_option(option);
                                                                              }
                                                                              else {
                                                                                  std::lock_guard<std::mutex> lock(socket_mutex);
                                                                                  this->socket=nullptr;
                                                                                  throw boost::system::system_error(ec);
                                                                              }
                                                                          });
                                           }
                                           else {
                                               std::lock_guard<std::mutex> lock(socket_mutex);
                                               socket=nullptr;
                                               throw boost::system::system_error(ec);
                                           }
                                       });
                io_service.reset();
                io_service.run();
                
                if(!config.proxy_server.empty()) {
                    boost::asio::streambuf write_buffer;
                    std::ostream write_stream(&write_buffer);
                    auto host_port=host+':'+std::to_string(port);
                    write_stream << "CONNECT "+host_port+" HTTP/1.1\r\n" << "Host: " << host_port << "\r\n\r\n";
                    auto timer=get_timeout_timer();
                    boost::asio::async_write(*socket, write_buffer,
                                             [this, timer](const boost::system::error_code &ec, size_t /*bytes_transferred*/) {
                                                 if(timer)
                                                     timer->cancel();
                                                 if(ec) {
                                                     std::lock_guard<std::mutex> lock(socket_mutex);
                                                     socket=nullptr;
                                                     throw boost::system::system_error(ec);
                                                 }
                                             });
                    io_service.reset();
                    io_service.run();
                    
                    auto response=request_read();
                    if (response->status_code.empty() || response->status_code.substr(0,3) != "200") {
                        std::lock_guard<std::mutex> lock(socket_mutex);
                        socket=nullptr;
                        throw boost::system::system_error(boost::system::error_code(boost::system::errc::permission_denied, boost::system::generic_category()));
                    }
                }
                
                auto timer=get_timeout_timer();
                this->socket->async_handshake(boost::asio::ssl::stream_base::client,
                                              [this, timer](const boost::system::error_code& ec) {
                                                  if(timer)
                                                      timer->cancel();
                                                  if(ec) {
                                                      std::lock_guard<std::mutex> lock(socket_mutex);
                                                      socket=nullptr;
                                                      throw boost::system::system_error(ec);
                                                  }
                                              });
                io_service.reset();
                io_service.run();
            }
        }

   
    };
}

#endif	/* CLIENT_HTTPS_HPP */
