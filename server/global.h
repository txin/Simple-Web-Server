#ifndef GLOBAL_H
#define GLOBAL_H

#include "../common/file.h"
#include <unordered_map>
#include <boost/log/trivial.hpp>

#include <string>
#include <iostream>
#include <utility>

#include <ctime>
#include <chrono>

typedef struct {
    std::string username;
    bool check_in;
    bool check_out;

    std::time_t expire_time;
    std::time_t create_time;
        
    bool propagate_flag;
    bool is_owner;
    bool is_delegate;
} Rights;

// Each file is associated with some metadata
class Metadata {

public:
    Metadata(std::string t_request_file_name, std::string username, int t_security_flag) {
        request_file_name = t_request_file_name;
        std::string new_file_name(username + '_' + t_request_file_name);
        uid_file_name = new_file_name;
        BOOST_LOG_TRIVIAL(trace) << "meta_data: uid_file_name: " << uid_file_name;
        security_flag = t_security_flag;

        Rights t_right;
        t_right.username = username;
        t_right.check_in = true;
        t_right.check_out = true;

//        default 1 year
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        std::time_t timestamp = std::chrono::system_clock::to_time_t(start);
        end = start + std::chrono::hours(10000);
        std::time_t timestamp_new = std::chrono::system_clock::to_time_t(end);

        t_right.create_time = timestamp;
        t_right.expire_time = timestamp_new;

        t_right.propagate_flag = true;
        t_right.is_owner = true;

        users_list.push_back(t_right);
    }

    std::string request_file_name;
    std::string uid_file_name; //unique
    int fid;
    int security_flag; // 0, 1, 2
    std::vector<Rights> users_list;

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
    void print_users_list(std::vector<Rights> &list);
    void print_rights_str(Rights t_rights, std::string &out_str);
};

extern Global *global_ptr;

#endif 
