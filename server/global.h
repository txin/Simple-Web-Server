#ifndef GLOBAL_H
#define GLOBAL_H

#include "../common/file.h"
#include <unordered_map>
#include <boost/log/trivial.hpp>
#include <string>
#include <iostream>
#include <utility>


// Each file is associated with some metadata
class Metadata {
public:
    Metadata(std::string t_request_file_name, std::string username, int t_security_flag) {
        request_file_name = t_request_file_name;
        std::string new_file_name(username + '_' + t_request_file_name);
        uid_file_name = new_file_name;
        BOOST_LOG_TRIVIAL(trace) << "meta_data: uid_file_name: " << uid_file_name;
        security_flag = t_security_flag;

        owners_list.push_back(username);
        check_in_list.push_back(username);
        check_out_list.push_back(username);
    }
    std::string request_file_name;
    std::string uid_file_name; //unique
    int fid;
    std::vector<std::string> owners_list; 
    std::vector<std::string> check_in_list;
    std::vector<std::string> check_out_list;
    std::vector<std::string> delegates_list; 
    int security_flag; // 0, 1, 2

    void set_fid(int t_fid) {
        fid = t_fid;
    }; 
};

class Global {
    std::unordered_multimap<std::string, Metadata> meta_map;
    int fid_ctr;
    
public:
    Global() {fid_ctr = 0;}

    int get_fid() {
        return fid_ctr++;
    }
    
    void insert_meta(Metadata t_meta) {
        meta_map.insert(std::make_pair(t_meta.request_file_name, t_meta));
    }
    
    bool lookup_meta(const std::string &file_name) {
        bool result = false;
        if (meta_map.count(file_name) > 0) {
            result = true;
        }
        return result;
    }
    void print_metadata();
    void print_vector(std::string list_name, std::vector<std::string> &list);
};

extern Global *global_ptr;

#endif 
