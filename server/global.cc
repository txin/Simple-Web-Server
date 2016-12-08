#include "global.h"

void Global::print_metadata() {
    for (auto it = meta_map.begin(); it != meta_map.end(); ++it ) {
        Metadata t_data = it->second;

        std::ostringstream ss;
        ss << "[ FID: " << t_data.fid << ", " << "FileName: " << it->first << " ]";
        BOOST_LOG_TRIVIAL(trace) << ss.str();
        print_vector("Owner_list", t_data.owner_list);
        print_vector("Checkin_list", t_data.check_in_list);
        print_vector("Checkout_list", t_data.check_out_list);
    }
}

void Global::print_vector(std::string list_name,
                          std::vector<int> &list) {
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
