#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <stdexcept>

template<typename T>
T convert(const std::string& s) {
    if (s.empty()) {
        throw std::runtime_error("Cannot convert empty value to requested type");
    }
    std::stringstream ss(s);
    T result;
    ss >> result;
    if (ss.fail()) {
        throw std::runtime_error("Cannot convert value '" + s + "' to requested type");
    }
    std::string remaining;
    if (ss >> remaining) {
        throw std::runtime_error("Cannot convert value '" + s + "' to requested type");
    }
    return result;
}

template<>
std::string convert<std::string>(const std::string& s) {
    return s;
}

class ini_parser {
private:
    std::map<std::string, std::map<std::string, std::string>> data;
    
    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
    
    void parse(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        
        std::string line;
        int line_num = 0;
        std::string current_section;
        bool section_started = false;
        
        while (std::getline(file, line)) {
            line_num++;
            std::string trimmed = trim(line);
            
            if (trimmed.empty() || trimmed[0] == ';') {
                continue;
            }
            
            if (trimmed[0] == '[') {
                size_t end = trimmed.find(']');
                if (end == std::string::npos) {
                    throw std::runtime_error("Invalid syntax at line " + std::to_string(line_num));
                }
                current_section = trim(trimmed.substr(1, end - 1));
                section_started = true;
                data[current_section];
                continue;
            }
            
            if (!section_started) {
                throw std::runtime_error("Invalid syntax at line " + std::to_string(line_num));
            }
            
            size_t eq_pos = trimmed.find('=');
            if (eq_pos == std::string::npos) {
                throw std::runtime_error("Invalid syntax at line " + std::to_string(line_num));
            }
            
            std::string key = trim(trimmed.substr(0, eq_pos));
            std::string value = trimmed.substr(eq_pos + 1);
            
            size_t comment_pos = value.find(';');
            if (comment_pos != std::string::npos) {
                value = value.substr(0, comment_pos);
            }
            value = trim(value);
            
            if (key.empty()) {
                throw std::runtime_error("Invalid syntax at line " + std::to_string(line_num));
            }
            
            data[current_section][key] = value;
        }
    }
    
public:
    ini_parser(const std::string& filename) {
        parse(filename);
    }
    
    template<typename T>
    T get_value(const std::string& path) {
        size_t dot_pos = path.find('.');
        if (dot_pos == std::string::npos) {
            throw std::runtime_error("Invalid path format: " + path);
        }
        
        std::string section = trim(path.substr(0, dot_pos));
        std::string key = trim(path.substr(dot_pos + 1));
        
        auto sec_it = data.find(section);
        if (sec_it == data.end()) {
            throw std::runtime_error("Section not found: " + section);
        }
        
        auto key_it = sec_it->second.find(key);
        if (key_it == sec_it->second.end()) {
            std::string msg = "Key not found: " + key + " in section " + section + ". Available keys: ";
            for (const auto& kv : sec_it->second) {
                msg += kv.first + " ";
            }
            throw std::runtime_error(msg);
        }
        
        return convert<T>(key_it->second);
    }
};