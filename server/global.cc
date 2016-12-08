#include "global.h"
#include <time.h>

void Global::print_metadata() {
    for (auto it = meta_map.begin(); it != meta_map.end(); ++it ) {
        Metadata t_data = it->second;
        std::ostringstream ss;
        ss << "FID: " << t_data.fid;
        BOOST_LOG_TRIVIAL(trace) << ss.str();
        print_users_list(t_data.users_list);
    }
}

void Global::print_vector(std::string list_name,
                          std::vector<std::string> &list) {
    std::ostringstream ss;
    std::string out_str;
    ss << list_name << ": [ ";
    for (auto it = list.begin(); it != list.end(); ++it) {
        ss << *it << " ";
    }
    ss << "]";
    out_str = ss.str();
    BOOST_LOG_TRIVIAL(trace) << out_str;
}

void Global::print_users_list(std::unordered_map<std::string, Rights> &list) {
    std::ostringstream ss;

    for (auto it = list.begin(); it != list.end(); ++it) {
        std::string out_str;
        print_rights_str(it->second, out_str);
        ss << out_str << " \n";
        BOOST_LOG_TRIVIAL(trace) << out_str;
    }
}

void Global::print_rights_str(Rights t_rights, std::string &out_str) {
    std::ostringstream ss;
    ss << "[Username: " << t_rights.username 
       << ",Check_in: " << t_rights.check_in << ",Check_out: " << t_rights.check_out
       << ",Create_time: " << std::string(std::ctime(&(t_rights.create_time)))
       << ",Expire_time: " << std::string(std::ctime(&(t_rights.expire_time)))
       << ",Propagate_flag: "
       << t_rights.propagate_flag << ",Is_owner: " << t_rights.is_owner
       << ",Is_delegate: " << t_rights.is_delegate << "]";
    out_str = ss.str();
}

bool Global::lookup_delegation(int t_fid, std::string &username) {
    bool result = false;

    auto it = meta_map.find(t_fid);
    if (it != meta_map.end()) {
        Metadata t_data = it->second;
        // check whether it is in the user list and delegation rights
        auto it_2 =  t_data.users_list.find(username);
        if (it_2 != t_data.users_list.end()) {
            Rights t_rights = it_2->second;
            std::chrono::time_point<std::chrono::system_clock> start;
            start = std::chrono::system_clock::now();
            std::time_t timestamp = std::chrono::system_clock::to_time_t(start);
            double time_left = difftime(t_rights.expire_time, timestamp);
            if (time_left < 0) {
                // expired
                return false;
            } else {
                bool condition = t_rights.is_owner ||
                    (t_rights.is_delegate && t_rights.propagate_flag);
                result = condition;
            }
        }
    }
    return result;
}

void Global::update_rights(int t_fid, std::string clientname, Rights &t_rights,
                           int time) {
    auto it = meta_map.find(t_fid);

    // definitely exists

    Metadata t_data = it->second;
    auto it_2 = t_data.users_list.find(clientname);
    t_rights.username = clientname;


    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(start);

    end = start + std::chrono::seconds(time);
    std::time_t timestamp_new = std::chrono::system_clock::to_time_t(end);

    t_rights.create_time = timestamp;
    t_rights.expire_time = timestamp_new;
    
    it->second.users_list.insert(std::make_pair(clientname, t_rights));
}

bool Global::lookup_check_out(int t_fid, std::string &username, std::string &file_name) {
    bool result = false;
    auto it = meta_map.find(t_fid);
    if (it != meta_map.end()) {
        Metadata t_data = it->second;
        // check whether it is in the user list and delegation rights
        auto it_2 =  t_data.users_list.find(username);
        if (it_2 != t_data.users_list.end()) {
            Rights t_rights = it_2->second;
            std::chrono::time_point<std::chrono::system_clock> start;
            start = std::chrono::system_clock::now();
            std::time_t timestamp = std::chrono::system_clock::to_time_t(start);
            double time_left = difftime(t_rights.expire_time, timestamp);
            if (time_left < 0) {
                // expired
                return false;
            } else {
                bool condition = t_rights.is_owner ||
                    (t_rights.is_delegate && t_rights.check_out);
                result = condition;
                if (condition) {
                    file_name = t_data.uid_file_name;
                }
            }
        }
    }
    
    return result;
}

bool Global::lookup_check_in(int t_fid, std::string &username, std::string &file_name) {
    bool result = true;
    auto it = meta_map.find(t_fid);
    if (it != meta_map.end()) {
        Metadata t_data = it->second;
        // check whether it is in the user list and delegation rights
        auto it_2 =  t_data.users_list.find(username);
        if (it_2 != t_data.users_list.end()) {
            Rights t_rights = it_2->second;
            std::chrono::time_point<std::chrono::system_clock> start;
            start = std::chrono::system_clock::now();
            std::time_t timestamp = std::chrono::system_clock::to_time_t(start);
            double time_left = difftime(t_rights.expire_time, timestamp);
            if (time_left < 0) {
                // expired
                return false;
            } else {
                bool condition = t_rights.is_owner ||
                    (t_rights.is_delegate && t_rights.check_in);
                result = condition;
            }
        }
    }
    return result;
}

