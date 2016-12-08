#include "file.h"
#include "encrypt.h"
#include "../server/global.h"

int write_file(std::shared_ptr<HttpsServer::Request> request,
               const CryptoPP::RSA::PublicKey &server_pk) {
    //Retrieve string:
    BOOST_LOG_TRIVIAL(trace) << "Upload resources content size: "
                             << (request->content).size();
    std::string file_name;
    std::string uid_str;
    int security_flag;
    int uid;

    for(auto& header: request->header) {
        if (header.first == "UID") {
            uid_str = header.second;
            uid = std::stoi(uid_str);
        } else if (header.first == "FileName") {
            file_name = header.second;
        } else if (header.first == "SecurityFlag") {
            security_flag = std::stoi(header.second);
        }
        BOOST_LOG_TRIVIAL(trace) << header.first << ": " << header.second << "\n";
    }
    std::string new_file = "web/upload/" + uid_str + '_' + file_name;
    // write file
    std::ofstream m_output_file;
    enum {Max_length = 40960};
    std::array<char, Max_length> m_buf;

    m_output_file.open(new_file, std::ios_base::binary);

    do {
        request->content.read(m_buf.data(), m_buf.size());
        BOOST_LOG_TRIVIAL(trace) << __func__ << " write " << request->content.gcount() <<
            " bytes.";
        m_output_file.write(m_buf.data(), request->content.gcount());
    } while (request->content.gcount() > 0);
        
    if (!m_output_file){
        BOOST_LOG_TRIVIAL(error) << __LINE__ << ": Failed to create: "   << new_file;
        return -1;
    }
    
    BOOST_LOG_TRIVIAL(trace) << "Files have been uploaded.";
// write metadata
    Metadata t_meta(file_name, uid, security_flag);
    global_ptr->insert_meta(t_meta);

    const std::string lookup_file = "test.jpg";
    BOOST_LOG_TRIVIAL(trace) << "Lookup: " << global_ptr->lookup_meta(lookup_file);
        
    if (security_flag == 0) { //Plaintext
        BOOST_LOG_TRIVIAL(trace) << "Security flag: NONE" ;
    } else if (security_flag == 1) {                     // Confidentiality
        BOOST_LOG_TRIVIAL(trace) << "Security flag: CONFIDENTIALITY" ;
        // Generate a new key, encrypt
        encrypt_file_1(new_file, server_pk);
    } else if (security_flag == 2) {                     // Integrity
        BOOST_LOG_TRIVIAL(trace) << "Security flag: INTEGRITY" ;
        encrypt_file_2(new_file, server_pk);
    } else {
        BOOST_LOG_TRIVIAL(error) << "Invalid security flag.";
    }
    
    return 0;
};


int write_file(std::string const& path, std::shared_ptr<HttpsClient::Response> response) {
    // write file
    std::ofstream m_output_file;
    enum {Max_length = 40960};
    std::array<char, Max_length> m_buf;

    m_output_file.open(path, std::ios_base::binary);

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
