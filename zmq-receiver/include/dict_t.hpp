#ifndef __HASH_TYPE_HPP__
#define __HASH_TYPE_HPP__

#include <any>
#include <unordered_map>
#include <typeinfo>
#include <list>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <stdexcept>

namespace dict {

    /**Python-like dictionary class for C++
    It uses 'boost::any' types to provide common storage and ease the strict typing of C++.
    The exact type still must be known at insertion and retrieval but not during storage.

    NOTE: This hash was never meant to be fast, but clean and maintainable!
    NOTE: The variables are passed and retrieved by value or reference (see std::any for details)!**/
    class dict_t {
      public:
        dict_t() {};
        virtual ~dict_t() {};

        /**Add a variable to the hash
            The type must be known for insertion.
            NOTE: The variable is passed by value!**/
        template<typename TY>
        void set(std::string key, TY val) {
            vhash[key] = val;
        }

        /**Add a variable to the hash as a boost::any value
            NOTE: The variable is passed by value!**/
        void set_raw(std::string key, std::any val) {
            vhash[key] = val;
        }

        /**Retrieve a variable from the hash
            The type must be known beforehand for successful retrieval.
            NOTE: The variable is returned by value!**/
        template<typename TY>
        TY get(std::string key) {
            if(!this->has(key)){
                std::string msg = "KeyError: Unable to find key '" + key + "' in the current dictionary.";
                throw std::invalid_argument(msg);
            }

            try{
                return std::any_cast<TY>(vhash[key]);
            } catch (const std::bad_any_cast& e){
                std::string msg = e.what() + std::string(" thrown for key ") + key;
                std::cerr << msg << std::endl;
                throw std::bad_any_cast();
            }
        }

        /**Retrieve a variable from the hash as a boost::any value
            NOTE: The variable is returned by value!**/
        std::any get_raw(std::string key) {
            if(!this->has(key)){
                std::string msg = "KeyError: Unable to find key '" + key + "' in the current dictionary.";
                throw std::invalid_argument(msg);
            }
            return vhash[key];
        }

        /**Delete a key from the hash to free memory.**/
        void del(std::string key) {
            vhash.erase(key);
        }

        /**Get the list of keys in the hash**/
        std::list<std::string> keys() {
            std::list<std::string> keys;
            for( const auto& item: vhash ) {
                keys.push_back(item.first);
            }
            return keys;
        }

        /**Check if the key is in the hash**/
        bool has(std::string key) {
            auto h_keys = this->keys();
            return bool(std::find(h_keys.begin(), h_keys.end(), key) != h_keys.end());
        }

        /**Copy a list of parameters into a new dict_t.**/
        dict_t select(std::list<std::string> keys) {
            dict_t newTable;
            for( const auto& key: keys ) {
                newTable.set(key, this->vhash[key]);
            }
            return newTable;
        }

        /**Merge another dict_t into the current one
            NOTE: Duplicate keys will be overwritten! **/
        void merge(dict_t& other) {
            for( const auto& key: other.keys() ) {
                this->set_raw(key, other.get_raw(key));
            }
        }

        /**The number of elements in the hash**/
        size_t size() const {
            return this->vhash.size();
        }

        /**Helper functions to gather type information about the stored data.**/
        template<typename TY>
        bool is_type(std::string key) {
            std::any a = vhash[key];
            return (a.type()==typeid(TY));
        }

        /**Assignment operator**/
        dict_t& operator=(dict_t& other) {
            this->vhash.clear();
            for( auto key: other.keys() ) {
                this->set_raw(key, other.get_raw(key));
            }
            return *this;
        }

        /**Merge operators**/
        dict_t& operator+=(dict_t& other) {
            for( auto key: other.keys() ) {
                this->set_raw(key, other.get_raw(key));
            }
            return *this;
        }

        /**Merge operator**/
        dict_t& operator+=(dict_t other) {
            for( auto key: other.keys() ) {
                this->set_raw(key, other.get_raw(key));
            }
            return *this;
        }

        /**Array subscript operator is same as get()**/
        template<typename TY>
        TY operator[](const std::string key) {
            return this->get<TY>(key);
        }

      protected:
        /**The actual container that stores key - value pairs
            It uses std::any to allow lazy typing.**/
        std::unordered_map<std::string, std::any> vhash;
    };

    /**Print the array to the output stream**/
    std::ostream& operator<<( std::ostream&, dict_t&);

} /**End namespace**/


#endif /* __HASH_TYPE_HPP__ */

