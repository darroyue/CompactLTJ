/*
 * utils.hpp
 * Copyright (C) 2020 Author removed for double-blind evaluation
 *
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <set>
#include <unordered_map>
#include <string>
namespace index_scheme {
    namespace util {
        typedef uint64_t size_type;//TODO: shouldn't they all get their values from a single source?
        typedef uint8_t var_type;
/*Classes*/
        class configuration{
            private:
                enum class execution_mode { cltj, adaptive_cltj, cltj_subtree, adaptive_cltj_subtree };
                std::unordered_map<execution_mode, std::string> mode_enum_to_str;
                std::unordered_map<std::string, execution_mode> mode_str_to_enum;
                execution_mode m_mode;
                bool m_print_gao;
                bool m_verbose;
                bool m_adaptive;
                bool m_subtree;
                const size_type m_threshold;
                size_type m_number_of_results;
                size_type m_timeout;
                execution_mode get_execution_mode(std::string &mode){
                    execution_mode ex_mode = execution_mode::cltj;
                    std::unordered_map<std::string,execution_mode>::iterator item;
                    item = mode_str_to_enum.find(mode);
                    if(item != mode_str_to_enum.end()){
                        ex_mode = item->second;
                    }
                    return ex_mode;
                }
                std::string get_mode_label() {
                    std::unordered_map<execution_mode, std::string>::iterator item;
                    item = mode_enum_to_str.find(m_mode);
                    if(item != mode_enum_to_str.end()){
                        return item->second;
                    }
                    return "";
                }
                std::string get_mode_options() const{
                    std::string str="";
                    for(auto& item : mode_enum_to_str){
                        str += item.second + "|";
                    }
                    return str;
                }
                std::string get_default_mode() const{
                    auto item = mode_enum_to_str.find(execution_mode::cltj);
                    if(item != mode_enum_to_str.end()){
                        return item->second;
                    }
                    return "";//Handle this better
                }
            public:
                configuration() :
                m_mode (execution_mode::cltj),
                m_print_gao(false),
                m_verbose(true),
                m_adaptive(false),
                m_number_of_results(1000),
                m_timeout(600),
                m_threshold(1){
                    mode_enum_to_str = {
                                        {execution_mode::cltj, "cltj"},
                                        {execution_mode::adaptive_cltj, "adaptive_cltj"},
                                        {execution_mode::cltj_subtree, "cltj_subtree"},
                                        {execution_mode::adaptive_cltj_subtree, "adaptive_cltj_subtree"}
                                    };
                    mode_str_to_enum = {
                                        {"cltj", execution_mode::cltj},
                                        {"adaptive_cltj", execution_mode::adaptive_cltj},
                                        {"cltj_subtree", execution_mode::cltj_subtree},
                                        {"adaptive_cltj_subtree", execution_mode::adaptive_cltj_subtree}
                                    };
                };
                inline size_type get_threshold() const{
                    return m_threshold;
                }
                bool is_adaptive() const{
                    return m_adaptive;
                }
                bool print_gao() const{
                    return m_print_gao;
                }
                bool is_verbose() const{
                    return m_verbose;
                }
                bool uses_subtree_mode() const{
                    return m_subtree;
                }
                size_type get_number_of_results() const{
                    return m_number_of_results;
                }
                void print_configuration() {//It cannot be declared as 'const' because 'get_mode_label' function uses 'm_mode' member, which is not const as well.
                    if(m_verbose){
                        std::cout << "Configuration" << std::endl << "=============" << std::endl;
                        std::cout << "Execution Mode: " << get_mode_label() << std::endl;
                        std::cout << "Print gao: " << (m_print_gao ? "true" : "false") << std::endl;
                        std::cout << "Verbose: " << (m_verbose ? "true" : "false") << std::endl;
                        std::cout << "Adaptive gao : " << (m_adaptive ? "true" : "false") << std::endl;
                    }
                }
                void configure(std::string &mode, bool print_gao, bool verbose,size_type number_of_results, size_type timeout){
                    m_mode = get_execution_mode(mode);
                    m_print_gao = print_gao;
                    m_verbose = verbose;
                    m_number_of_results = number_of_results;
                    m_timeout = timeout;
                    if(m_mode == execution_mode::adaptive_cltj || m_mode == execution_mode::adaptive_cltj_subtree){
                        m_adaptive = true;
                    }
                    if(m_mode == execution_mode::cltj_subtree){
                        m_subtree = true;
                    }
                }
                std::string get_configuration_options() const{
                    return "[execution_mode="+get_mode_options()+" default="+get_default_mode()+"] [print_gao=0|1 default=0] [verbose=0|1 (default) ] [number_of_results=1000 (default)] [timeout=600 (default)]";
                }
        };
        static configuration configuration;

        // trim from end (in place)
         static inline void rtrim(std::string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
        }
    }
}
#endif
