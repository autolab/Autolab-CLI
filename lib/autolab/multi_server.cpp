#include <autolab/multi_server.h>
#include "logger.h"
#include <rapidjson/document.h>
#include "../../src/file/file_utils.h"
#include <json_helpers.h>
#include <algorithm>
#include <unordered_set>
#include <autolab/autolab.h>
#include "../all_servers_dirname.h"
#include <dirent.h>

namespace Autolab
{

static constexpr size_t MAX_JSON_LENGTH = 1000;

static std::string sanitize_string(std::string str) {
    if (str.empty() || str == "." || str == "..") {
        Logger::fatal << "Invalid server name information. Contact your administrator" << Logger::endl;
        throw InvalidInputException {};
    }
    // Set of characters commonly disallowed or problematic in filenames on major OSes
    static const std::string invalid_chars = R"(\/:*?"<>|)";
    // Replace each invalid character with '_'
    for (char& c : str) {
        if (invalid_chars.find(c) != std::string::npos) {
            c = '_';
        }
    }

    // Prepend with '_'
    return "_" + str;
}

AllServers::AllServers(const std::string& all_servers_dirname) {
    if (all_servers_dirname.at(0) != '/') {
        Logger::fatal << "The directory path should be an absolute path (start"
                         " with /)." << Logger::endl;;
        throw InvalidInputException {};
    }
    if (all_servers_dirname.at(all_servers_dirname.length() - 1) != '/') {
        Logger::fatal << "The directory name should be terminated with a /"
                      << Logger::endl;
        throw InvalidInputException {};
    } 
    if (!dir_exists(all_servers_dirname.c_str())) {
        Logger::fatal << "The directory containing server information does not"
                         "exist. Contact your administrator." << Logger::endl;
        throw InvalidInputException {};
    }

    // For keeping track of duplicates
    std::unordered_set<std::string> seen_server_names;
    std::unordered_set<std::string> seen_course_names;

    DIR *dir = opendir(all_servers_dirname.c_str());
    assert(dir != NULL);
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_REG) continue;

        char json_string[MAX_JSON_LENGTH];
        size_t num_read = read_file((
            all_servers_dirname + std::string(entry->d_name)).c_str(), 
            json_string, MAX_JSON_LENGTH - 1);
        if (num_read <= 0) {
            Logger::fatal << "Not able to read file in directory" << Logger::endl;
            throw InvalidInputException {};
        }
        json_string[num_read] = '\0';
        rapidjson::Document document;
        document.Parse(json_string);
        require_is_object(document);

        rapidjson::Value& server_value = document;
        require_is_object(server_value);
        ServerInfo server_info;
        server_info.server_name = sanitize_string(std::move(
            get_string_force(server_value, "server_name")));
        auto [_, server_success] = seen_server_names.emplace(server_info.server_name);
        if (!server_success) {
            Logger::fatal << "Duplicate server name. Contact your administrator" << Logger::endl;
            throw InvalidInputException {};
        }

        server_info.base_uri = get_string_force(server_value, "base_uri");
        server_info.client_id = get_string_force(server_value, "client_id");
        server_info.client_secret = get_string_force(server_value, "client_secret");
        server_info.redirect_uri = get_string_force(server_value, "redirect_uri");
        if (!server_value.HasMember("courses")) {
            Logger::fatal << "JSON file not correctly formatted." << Logger::endl;
            throw InvalidInputException {};
        }
        rapidjson::Value& courses_value = server_value["courses"];
        require_is_array(courses_value);
        for (int course_idx = 0; course_idx < courses_value.Size(); course_idx++) {
            std::string course_name = courses_value[course_idx].GetString();
            auto [_, course_success] = seen_course_names.emplace(course_name);
            if (!course_success) {
                Logger::fatal << "Duplicate course name. Contact your administrator." << Logger::endl;
                throw InvalidInputException {};
            }
            server_info.courses.emplace_back(std::move(course_name));
        }
        m_server_list.push_back(server_info);
    }





}

std::vector<std::string> AllServers::get_all_courses() const {
    std::vector<std::string> result;
    for (const ServerInfo& server : m_server_list) {
        result.insert(result.end(), server.courses.begin(), server.courses.end());
    }
    return result;
}

std::vector<std::string> AllServers::get_all_server_names() const {
    std::vector<std::string> result;
    std::transform(m_server_list.begin(), m_server_list.end(), std::back_inserter(result), 
                    [](const ServerInfo& server) {return server.server_name;});
    return result;
}

ServerInfo AllServers::get_server_from_course(const std::string& course_name) const {
    for (const ServerInfo& server_info : m_server_list) {
        if (std::find(server_info.courses.begin(), server_info.courses.end(), 
            course_name) != server_info.courses.end()) {
                return server_info;
            }
    }
    // Course not found
    Logger::fatal << "Please specify a valid course. List of courses: " << Logger::endl;
    std::vector<std::string> all_courses = g_all_servers.get_all_courses();
    for (const auto& course: all_courses) {
      Logger::info << course << " | ";
    }
    Logger::info << Logger::endl;
    throw Autolab::InvalidInputException {};
}

ServerInfo AllServers::get_server_from_name(const std::string& server_name) const {
    for (const ServerInfo& server_info : m_server_list) {
        if (server_info.server_name == server_name) {
            return server_info;
        }
    }
    Logger::fatal << "Please specify a valid server." << Logger::endl;
    throw Autolab::InvalidInputException {};
}

bool AllAuthInfo::has_auth_for_server(const std::string& server_name) {
    for (const AuthInfo& info : m_auth_info_list) {
        if (server_name == info.server_name) return info.exists;
    }
    return false;
}

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

std::string AllAuthInfo::get_access_token_from_server(const std::string& server_name) {
    for (const AuthInfo& info : m_auth_info_list) {
        if (server_name == info.server_name && info.exists) {
            return info.access_token;
        }
    }
    Logger::fatal << "Could not find access token. Please rerun autolab setup for this course." << Logger::endl;
    throw std::runtime_error("fatal");
}
    

std::string AllAuthInfo::get_refresh_token_from_server(const std::string& server_name) {
    for (const AuthInfo& info : m_auth_info_list) {
        if (server_name == info.server_name && info.exists) {
            return info.refresh_token;
        }
    }
    return "";
}

} // namespace Autolab

Autolab::AllServers g_all_servers {all_servers_dirname};
