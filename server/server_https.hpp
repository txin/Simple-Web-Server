#ifndef SERVER_HTTPS_HPP
#define	SERVER_HTTPS_HPP

#include <fstream>
#include <iostream>

#include "server_http.hpp"
#include "../common/encrypt.h"

#include <boost/asio/ssl.hpp>
#include <cryptopp/rsa.h>

namespace SimpleWeb {
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> HTTPS;    
    
    template<>
    class Server<HTTPS> : public ServerBase<HTTPS> {
    public:
        Server(unsigned short port, size_t num_threads, const std::string& cert_file, const std::string& private_key_file,
               long timeout_request=5, long timeout_content=300,
               const std::string& verify_file=std::string()) : 
            ServerBase<HTTPS>::ServerBase(port, num_threads, timeout_request, timeout_content), 
            context(boost::asio::ssl::context::tlsv12) { // 2016/08/13 only use tls12, see https://www.ssllabs.com/ssltest
            // For mutual authentication.
            context.set_verify_mode(boost::asio::ssl::verify_peer);
//                                    | boost::asio::ssl::context::verify_fail_if_no_peer_cert);

            context.use_certificate_chain_file(cert_file);
            context.use_private_key_file(private_key_file, boost::asio::ssl::context::pem);
            context.set_default_verify_paths();
            
            if (verify_file.size() > 0) {
                context.load_verify_file(verify_file);
            }

        }

    protected:
        boost::asio::ssl::context context;

        enum {Max_length = 40960};
        std::array<char, Max_length> m_buf;
        boost::asio::streambuf m_request_buf;
        std::ofstream m_output_file;
        size_t m_file_size;
        std::string m_file_name;
        
        void accept() {
            //Create new socket for this connection
            //Shared_ptr is used to pass temporary objects to the asynchronous functions
            auto socket=std::make_shared<HTTPS>(*io_service, context);

            acceptor->async_accept((*socket).lowest_layer(), [this, socket](const boost::system::error_code& ec) {
                    //Immediately start accepting a new connection (if io_service hasn't been stopped)
                    if (ec != boost::asio::error::operation_aborted)
                        accept();

                
                    if(!ec) {
                        boost::asio::ip::tcp::no_delay option(true);
                        socket->lowest_layer().set_option(option);
                    
                        //Set timeout on the following boost::asio::ssl::stream::async_handshake
                        auto timer=get_timeout_timer(socket, timeout_request);
                        (*socket).async_handshake(boost::asio::ssl::stream_base::server, [this, socket, timer]
                                                  (const boost::system::error_code& ec) {
                                                      if(timer)
                                                          timer->cancel();
                                                      if(!ec)
                                                          read_request_and_content(socket);
                                                  });
                    }
                });
        }
    };
}


#endif	/* SERVER_HTTPS_HPP */
