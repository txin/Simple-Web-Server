#include "global.h"

void Global::print_metadata() {
    for (auto it = meta_map.begin(); it != meta_map.end(); ++it ) {
        Metadata t_data = it->second;
        std::ostringstream ss;
        ss << "FID: " << t_data.fid << ", " << "FileName: " << it->first;
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

void Global::print_users_list(std::vector<Rights> &list) {
    std::ostringstream ss;

    for (auto it = list.begin(); it != list.end(); ++it) {
        std::string out_str;
        print_rights_str(*it, out_str);
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
       << ",Propagate_flag"
       << t_rights.propagate_flag << ",Is_owner: " << t_rights.is_owner
       << ",Is_delegate: " << t_rights.is_delegate << "]";
    out_str = ss.str();
    BOOST_LOG_TRIVIAL(trace) << out_str;
}
