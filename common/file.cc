#include "file.h"
#include "encrypt.h"
#include "../server/global.h"

int write_file(std::shared_ptr<HttpsServer::Request> request,
               const CryptoPP::RSA::PublicKey &server_pk) {
    //Retrieve string:
    BOOST_LOG_TRIVIAL(trace) << "Upload resources content size: "
                             << (request->content).size();
    std::string file_name, user_name;
    int security_flag;

    for (auto& header: request->header) {
        if (header.first == "UserName") {
            user_name = header.second;
        } else if (header.first == "FileName") {
            file_name = header.second;
        } else if (header.first == "SecurityFlag") {
            security_flag = std::stoi(header.second);
        }
        BOOST_LOG_TRIVIAL(trace) << header.first << ": " << header.second << "\n";
    }
    std::string new_file = "web/upload/" + user_name + '_' + file_name;
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
    Metadata t_meta(file_name, user_name, security_flag);
    t_meta.set_fid(global_ptr->get_fid());
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

bool verify_delegation_request(std::shared_ptr<HttpsServer::Request> request) {
    bool result = false;

    std::string user_name, client_name;
    int fid;
    
    for (auto& header: request->header) {
        if (header.first == "FID") {
            fid = std::stoi(header.second);
        } else if (header.first == "UserName") {
            user_name = header.second;
        } else if (header.first == "ClientName") {
            client_name = header.second;
        }
    }

    BOOST_LOG_TRIVIAL(trace) << "Delegation request:" << " FID: " << fid
                             << ", from UserName: " << user_name
                             << ", ClientName: " << client_name;
    return result;
    // First check metadata
    // Then verify the delegation file
    // load public file
    // finally update the meta file for delegation list and also expiration time
    // update metafile
}
