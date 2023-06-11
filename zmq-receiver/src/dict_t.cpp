
#include "dict_t.hpp"


/**Print the hash into the output stream**/
std::ostream& dict::operator<<( std::ostream &output, dict::dict_t& hsh) {
    output << "Printing known key-value pairs in hash table:\n";
	
    for( const auto& key: hsh.keys() ) {
        if( hsh.is_type<std::string>(key)) {
			output << key << "\t" << hsh.get<std::string>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";}
        else if( hsh.is_type<double>(key)) {
			output << key << "\t" << hsh.get<double>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";		}
        else if( hsh.is_type<float>(key)) {
			output << key << "\t" << hsh.get<float>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";		}
        else if( hsh.is_type<int64_t>(key)) {
			output << key << "\t" << hsh.get<int64_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";	}
        else if( hsh.is_type<uint64_t>(key)) {
			output << key << "\t" << hsh.get<uint64_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";	}
        else if( hsh.is_type<int32_t>(key)) {
			output << key << "\t" << hsh.get<int32_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";	}
        else if( hsh.is_type<uint32_t>(key)) {
			output << key << "\t" << hsh.get<uint32_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";	}
        else if( hsh.is_type<int16_t>(key)) {
			output << key << "\t" << hsh.get<int16_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n"; 	}
        else if( hsh.is_type<uint16_t>(key)) {
			output << key << "\t" << hsh.get<uint16_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";	}
        else if( hsh.is_type<int8_t>(key)) {
			output << key << "\t" << hsh.get<int8_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n"; 	}
        else if( hsh.is_type<uint8_t>(key)) {
			output << key << "\t" << hsh.get<uint8_t>(key) << "\t" << hsh.get_raw(key).type().name() << "\n";	}
        else {
            output << key << "\t" << hsh.get_raw(key).type().name() << "\n";
        }
    }
	// Flush buffer
	output << std::endl;
    return output;
}
