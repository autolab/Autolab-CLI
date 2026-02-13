#include "autolab/multi_server.h"
#include "logger.h"

namespace Autolab
{
    // TODO: when parsing, make sure that there are no duplicate names or courses.


bool AllServers::is_course(const std::string& course_name) {}

bool AllAuthInfo::set_tokens_for_server(const std::string& server_name, 
    const std::string& access_token, const std::string& refresh_token) {
        bool found = false;
        for (auto& server: m_auth_info_list) {
            if (server.server_name == server_name) {
                if (server.exists) {
                    Logger::info << "Overwriting tokens for server " << server_name << Logger::endl;
                }
                server.exists = true;
                server.access_token = access_token;
                server.refresh_token = refresh_token;
                found = true;
            }
        }
        if (!found) {
            Logger::info << "Could not find server " << server_name << " to set tokens for." << Logger::endl;
        }
        return found;
    }
    
} // namespace Autolab

Autolab::AllServers g_all_servers {};
