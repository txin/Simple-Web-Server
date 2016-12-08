#ifndef TYPE_H
#define TYPE_H

typedef struct {
    std::string username;
    bool check_in = false;
    bool check_out = false;

    std::time_t expire_time;
    std::time_t create_time;
        
    bool propagate_flag = false;
    bool is_owner = false;
    bool is_delegate = false;
} Rights;

#endif
