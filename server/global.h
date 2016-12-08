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
    Metadata(std::string t_request_file_name, int t_uid, int t_security_flag) {
        request_file_name = t_request_file_name;
        std::string new_file_name(std::to_string(t_uid) + '_' + t_request_file_name);
        uid_file_name = new_file_name;
        BOOST_LOG_TRIVIAL(trace) << "meta_data: uid_file_name: " << uid_file_name;
        security_flag = t_security_flag;
        owner_list.push_back(t_uid);
    }
    std::string request_file_name;
    std::string uid_file_name; //unique
    std::vector<int> owner_list; //uid
    int security_flag; // 0, 1, 2
};

class Global {
    std::unordered_multimap<std::string, Metadata> meta_map;
public:
    Global() {}
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
};


extern Global *global_ptr;
#endif 
