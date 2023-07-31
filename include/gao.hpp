/*
 * gao.hpp
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


#ifndef RING_GAO_HPP
#define RING_GAO_HPP


#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <cltj.hpp>
#include <utils.hpp>
#include <queue>
#include <functional>
#include <stack>
#include <algorithm>
#include <ltj_iterator.hpp>
namespace ltj {

    template<class info_var_t, class var_to_iterators_t, class index_scheme_t = index_scheme::compactLTJ, class var_t = uint8_t, class cons_t = uint64_t, class ltj_iterator_t = ltj_iterator<index_scheme_t, var_t, cons_t>>
    class gao_size {

    public:
        typedef var_t var_type;
        typedef cons_t cons_type;
        typedef uint64_t size_type;
        typedef index_scheme_t index_type;
        typedef ltj_iterator_t ltj_iter_type;
        typedef info_var_t info_var_type;
        typedef var_to_iterators_t var_to_iterators_type;
        typedef std::pair<size_type, var_type> pair_type;
        typedef std::priority_queue<pair_type, std::vector<pair_type>, std::greater<pair_type>> min_heap_type;
    private:
        const std::vector<rdf::triple_pattern>* m_ptr_triple_patterns;
        const std::vector<ltj_iter_type>* m_ptr_iterators;//TODO: remove?
        std::vector<info_var_type> m_var_info;
        std::unordered_map<var_type, size_type> m_hash_table_position;
        index_type* m_ptr_index;
        size_type m_lonely_start;
        std::vector<var_type> m_lonely_variables;
        std::stack<std::vector<std::pair<var_type, size_type>>> m_previous_values_stack;
        var_type m_starting_var;
        var_to_iterators_type* m_var_to_iterators;//TODO: Ojo, quizas m_var_info tambien tiene que ser un puntero!
        size_type m_number_of_variables;
        size_type m_number_of_triples;
        //TODO: move to another function / class that manages the variable information.
        void update_hash_var_index(typename std::vector<info_var_type>::iterator it_start, typename std::vector<info_var_type>::iterator it_end, std::unordered_map<var_type, size_type> &hash_table){
            int i=0;
            typename std::vector<info_var_type>::iterator it = it_start;
            while(it != it_end){
                auto hash_it = hash_table.find(it->name);
                if(hash_it != hash_table.end()){
                    hash_it->second = i;
                    i++;
                }
                it = std::next(it);
            }
        }

        void fill_heap(const var_type var,
                        std::unordered_map<var_type, size_type> &hash_table,
                        std::vector<info_var_type> &vec,
                        std::vector<bool> &checked,
                        min_heap_type &heap){

            auto pos_var = hash_table[var];
            for(const auto &e : vec[pos_var].related){
                auto pos_rel = hash_table[e];
                if(!checked[pos_rel] && vec[pos_rel].n_triples > 1){
                    heap.push({vec[pos_rel].weight, e});
                    checked[pos_rel] = true;
                }
            }
        }

        struct compare_var_info
        {
            inline bool operator() (const info_var_type& linfo, const info_var_type& rinfo)
            {
                if(linfo.n_triples>1 && rinfo.n_triples==1){
                    return true;
                }
                if(linfo.n_triples==1 && rinfo.n_triples>1){
                    return false;
                }
                return linfo.weight < rinfo.weight;
            }
        };

        void copy(const gao_size &o) {
            m_ptr_triple_patterns = std::move(o.m_ptr_triple_patterns);
            m_ptr_iterators = std::move(o.m_ptr_iterators);
            m_ptr_index = std::move(o.m_ptr_index);
            m_var_info = std::move(o.m_var_info);
            m_lonely_variables = std::move(o.m_lonely_variables);
            m_hash_table_position = std::move(o.m_hash_table_position);
            m_number_of_variables = std::move(o.m_number_of_variables);
            m_lonely_start = std::move(o.m_lonely_start);
            m_starting_var = std::move(o.m_starting_var);
            m_var_to_iterators = std::move(o.m_var_to_iterators);
        }
    public:
        const size_type &number_of_variables = m_number_of_variables;
        const size_type &number_of_triples = m_number_of_triples;
        const size_type get_number_of_lonely() const{
            return number_of_variables - m_lonely_start;
        }
        gao_size() = default;

        gao_size(const std::vector<rdf::triple_pattern>* triple_patterns,
                    std::vector<info_var_type> &var_info,
                    std::unordered_map<var_type, size_type> &hash_table_position,
                    var_to_iterators_type *var_to_iterators,
                    index_type* r,
                    std::vector<var_type> &gao) : m_number_of_variables(0){
            m_ptr_triple_patterns = triple_patterns;//TODO: to be removed.
            m_number_of_triples = m_ptr_triple_patterns->size();
            //m_ptr_iterators = iterators;
            m_hash_table_position = hash_table_position;
            m_var_info = var_info;
            m_ptr_index = r;
            m_var_to_iterators = var_to_iterators;
            size_type i = 0;

            //1. Sorting variables according to their weights.
            //std::cout << "Sorting... " << std::flush;
            std::sort(m_var_info.begin(), m_var_info.end(), compare_var_info());
            update_hash_var_index(m_var_info.begin(), m_var_info.end(), m_hash_table_position);
            m_lonely_start = m_var_info.size();
            m_number_of_variables = m_var_info.size();
            for(i = 0; i < m_var_info.size(); ++i){
                m_hash_table_position[m_var_info[i].name] = i;
                if(m_var_info[i].n_triples == 1 && i < m_lonely_start){
                    m_lonely_start = i;
                }
            }
            gao.reserve(m_var_info.size());
            m_lonely_variables.reserve(m_var_info.size() - m_lonely_start);
            //std::cout << "Done. " << std::endl;
            i = 0;
            if(index_scheme::util::configuration.is_adaptive()){
                m_starting_var = m_var_info[0].name;
                //m_ptr_index->clear_cache();
            }else{
                //2. Choosing the variables
                //std::cout << "Choosing GAO ... " << std::flush;
                std::vector<bool> checked(m_var_info.size(), false);
                while(i < m_lonely_start){ //Regular variables (not lonely)
                    if(!checked[i]){
                        gao.push_back(m_var_info[i].name); //Adding var to gao
                        checked[i] = true;
                        min_heap_type heap; //Stores the regular variables that are related with the chosen ones
                        auto var_name = m_var_info[i].name;
                        fill_heap(var_name, m_hash_table_position, m_var_info, checked,heap);
                        while(!heap.empty()){
                            var_name = heap.top().second;
                            heap.pop();
                            gao.push_back(var_name);
                            fill_heap(var_name, m_hash_table_position, m_var_info, checked, heap);
                        }
                    }
                    ++i;
                }

                while(i < m_var_info.size()){ //Lonely variables
                    m_lonely_variables.emplace_back(m_var_info[i].name);
                    gao.push_back(m_var_info[i].name); //Adding var to gao
                    ++i;
                }
                m_starting_var = gao[0];
            }
            //3. Up to here the gao is calculated. Now we need to set the iterators per variable,
            // based on the idea of 1 iterator per triple (For LTJ usage).
/*
            //We want to know per each v in gao and t in Q_{v}. Was t checked before? if no, check it, otherwise omit it.
            std::vector<bool> checked(m_number_of_triples, false);
            for(auto& var: gao){
                //per each var in gao.
                auto& triple_iter_related = m_var_info[m_hash_table_position[var]].triple_iter_related_details;
                //per each triple t in Q_{v}, where Q_{v} are triple patterns in which v participates (!t.empty).
                for(auto& t : triple_iter_related){
                    if(!t.empty && !checked[t.triple_index]){
                        auto& winning_iter = t.iterator;
                        add_var_to_iterator(var, winning_iter);
                        checked[t.triple_index] = true;
                        for(auto& rel_var : t.related){//related variables in t.
                            add_var_to_iterator(rel_var, winning_iter);
                        }

                    }
                }
            }*/
            //std::cout << "done."<< std::endl;
        }
        /*
        inline void add_var_to_iterator(const var_type var, ltj_iter_type* ptr_iterator){
            auto it =  m_var_to_iterators->find(var);
            if(it != m_var_to_iterators->end()){
                it->second.push_back(ptr_iterator);
            }else{
                std::vector<ltj_iter_type*> vec = {ptr_iterator};
                m_var_to_iterators->insert({var, vec});
            }
        }
        */
        //! Copy constructor
        gao_size(const gao_size &o) {
            copy(o);
        }

        //! Move constructor
        gao_size(gao_size &&o) {
            *this = std::move(o);
        }

        //! Copy Operator=
        gao_size &operator=(const gao_size &o) {
            if (this != &o) {
                copy(o);
            }
            return *this;
        }
        //! Move Operator=
        gao_size &operator=(gao_size &&o) {
            if (this != &o) {
                m_ptr_triple_patterns = std::move(o.m_ptr_triple_patterns);
                m_ptr_iterators = std::move(o.m_ptr_iterators);
                m_ptr_index = std::move(o.m_ptr_index);
                m_var_info = std::move(o.m_var_info);
                m_lonely_variables = std::move(o.m_lonely_variables);
                m_hash_table_position = std::move(o.m_hash_table_position);
                m_number_of_variables = std::move(o.m_number_of_variables);
                m_lonely_start = std::move(o.m_lonely_start);
                m_starting_var = std::move(o.m_starting_var);
                m_var_to_iterators = std::move(o.m_var_to_iterators);
            }
            return *this;
        }

        void swap(gao_size &o) {
            std::swap(m_ptr_triple_patterns, o.m_ptr_triple_patterns);
            std::swap(m_ptr_iterators, o.m_ptr_iterators);
            std::swap(m_ptr_index, o.m_ptr_index);
            std::swap(m_var_info, o.m_var_info);
            std::swap(m_lonely_variables, o.m_lonely_variables);
            std::swap(m_hash_table_position, o.m_hash_table_position);
            std::swap(m_number_of_variables, o.m_number_of_variables);
            std::swap(m_lonely_start, o.m_lonely_start);
            std::swap(m_starting_var, o.m_starting_var);
            std::swap(m_var_to_iterators, o.m_var_to_iterators);
        }
        std::unordered_set<var_type> get_related_variables(const var_type& var){
            return m_var_info[m_hash_table_position[var]].related;
        }
        std::vector<var_type> get_lonely_variables() const{
            return m_lonely_variables;
        }
        /*Updates weights of the related vars of ´cur_var´*/
        void update_weights(const size_type& j, const var_type& cur_var, const std::unordered_map<var_type, bool> &gao_vars,const var_to_iterators_type &m_var_to_iterators){
            std::vector<std::pair<var_type, size_type>> previous_values;
            //Lonely vars are excluded of this process.
            if(j > 0 && j < m_lonely_start){
                //Non-lonely vars.
                const auto& rel_vars = get_related_variables(cur_var);
                for(const auto &rel_var : rel_vars){
                    if(!is_var_bound(rel_var, gao_vars)){
                        size_type index = m_hash_table_position[rel_var];
                        info_var_type& var_info = m_var_info[index];
                        size_type min_weight = -1ULL;
                        //All iterators of 'var'
                        auto iters =  m_var_to_iterators.find(rel_var);
                        if(iters != m_var_to_iterators.end()){
                            std::vector<ltj_iter_type*> var_iters = iters->second;
                            for(ltj_iter_type* it : var_iters){
                                //The iterator has a reference to its triple pattern.
                                //const triple_pattern& triple_pattern = *(it->get_triple_pattern());
                                const ltj_iter_type &iter = *it;
                                size_type weight = 0;
                                weight = iter.get_weight();
                                if(weight < min_weight){
                                    min_weight = weight;
                                }
                                //std::cout << "Cur var: " << int(cur_var) << " rel var: " << int(rel_var) << " min weight : " << min_weight << std::endl;
                            }
                        }
                        if(min_weight != -1ULL){
                            //Storing the previous value on the stack.
                            std::pair<var_type, size_type> p{rel_var, var_info.weight};
                            previous_values.emplace_back(p);
                            //Updating the minimum weight in m_var_info.
                            var_info.weight = min_weight;
                        }
                    }
                }
            }
            m_previous_values_stack.emplace(std::move(previous_values));
        }
        bool is_var_bound(const size_type& var, const std::unordered_map<var_type,bool> &m_gao_vars) const{
            auto it = m_gao_vars.find(var);
            //auto it = std::find(m_gao_vars.begin(), m_gao_vars.end(), var);
            if (it != m_gao_vars.end())
                if(it->second)
                    return true;
            return false;
        }
        //Linear search on 'm_var_info' for the non-bound variable with minimum weight.
        var_type get_next_var(const size_type& j, const std::unordered_map<var_type,bool> &m_gao_vars) const{
            size_type min_weight = -1ULL;
            var_type min_var = '\0';

            if(j == 0){
                min_var = m_starting_var;
            }else{
                for(const auto& v : m_var_info){
                    if(!is_var_bound(v.name, m_gao_vars)){
                        //Non-lonely first.
                        if(j < m_lonely_start){
                            if(v.n_triples > 1 && v.weight <= min_weight){
                                min_weight = v.weight;
                                min_var = v.name;
                            }
                        }else{
                            //Lonely variables after all the non-lonely are instantiated.
                            if(v.n_triples == 1){
                                min_var = v.name;
                                break;
                            }
                        }
                    }
                }
            }
            return min_var;
        }
        //Sets back previous weight value in constant time*.
        void set_previous_weight(){
            auto& vec = m_previous_values_stack.top();
            
            for(auto &pair: vec){
                size_type index = m_hash_table_position[pair.first];
                //std::cout << "replacing weight : " << m_var_info[index].weight << " with previous value : " << pair.second << std::endl;
                m_var_info[index].weight = pair.second;
            }
            m_previous_values_stack.pop();
        }

        const var_type get_starting_var() const{
            return m_starting_var;
        }
    };

}

#endif //RING_GAO_HPP
