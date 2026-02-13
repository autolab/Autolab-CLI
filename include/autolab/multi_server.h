#ifndef _MULTI_SERVER_H_
#define _MULTI_SERVER_H_

#include <string>
#include <vector>

namespace Autolab {
struct ServerInfo {
    std::string server_name;
    std::string base_uri; // domain of the autolab service
    std::string client_id;
    std::string client_secret;
    std::string redirect_uri;
    std::vector<std::string> courses;
};


class AllServers {
public:
    std::vector<ServerInfo> m_server_list;

    AllServers(); // parses the JSON file to populate itself

    std::vector<std::string> get_all_courses();
    std::vector<std::string> get_all_server_names();

    // Returns "" if there is no such course
    ServerInfo get_server_from_course(const std::string& course_name);
    ServerInfo get_server_from_name(const std::string& server_name);
};

struct AuthInfo {
    std::string server_name;
    bool exists;
    std::string access_token;
    std::string refresh_token;
};

class AllAuthInfo {
public:
    std::vector<AuthInfo> m_auth_info_list;
    bool has_auth_for_server(const std::string& server_name);
    bool set_tokens_for_server(const std::string& server_name, 
        const std::string& access_token, const std::string& refresh_token)

    // Returns "" if it does not exist
    std::string get_access_token_from_server(const std::string& server_name);
    std::string get_refresh_token_from_server(const std::string& server_name);
}

}

extern Autolab::AllServers g_all_servers;


#endif /* _MULTI_SERVER_H_ */