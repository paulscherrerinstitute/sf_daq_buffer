#ifndef SF_DAQ_BUFFER_TYPES_HPP
#define SF_DAQ_BUFFER_TYPES_HPP

#include <chrono>
#include <cstring>
#include "rapidjson/document.h"

#include "../../core-buffer/include/buffer_config.hpp"
#include "../../core-buffer/include/formats.hpp"
#include "Hdf5Writer.hpp"
#include "dict_t.hpp"


class ImageMetadataCache{
protected:
    // General container
    dict::dict_t hsh;
    // Metadata for file IO
    std::string base_path;
    std::string detector_name;

    // Block metadata
    uint64_t block_start_pulse_id;
    uint64_t block_stop_pulse_id;

    // Caching indices
    bool is_first_run = true;
    size_t m_buffer_size;
    size_t m_block_size;
    uint64_t write_idx = 0;;
    uint64_t run_id = 0;

public:
    ImageMetadataCache(std::string base_path, std::string detector_name, size_t bs = buffer_config::BUFFER_BLOCK_SIZE):
        base_path(base_path), detector_name(detector_name) {
        m_buffer_size = bs;

        // Nice usability feature
        if(&base_path.back()=="/"){ base_path.pop_back(); }

        // Fill up the hash according to ImageMetadata schema
        hsh.set("version", std::vector<uint64_t>(bs) );
        hsh.set("id", std::vector<uint64_t>(bs) );
        hsh.set("height", std::vector<uint64_t>(bs) );
        hsh.set("width", std::vector<uint64_t>(bs) );
        hsh.set("dtype", std::vector<uint16_t>(bs) );
        hsh.set("encoding", std::vector<uint16_t>(bs) );
        hsh.set("array_id", std::vector<uint16_t>(bs) );
        hsh.set("status", std::vector<uint16_t>(bs) );
        hsh.set("user_1", std::vector<uint64_t>(bs) );
        hsh.set("user_2", std::vector<uint64_t>(bs) );
        hsh.set("shape", std::vector<std::vector<uint64_t>>(bs) );
    };

    bool is_full(){
        return write_idx >= buffer_config::BUFFER_BLOCK_SIZE;
    };

    void append(void* meta, size_t meta_size, void* data, size_t data_size){
        std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
        std::chrono::duration<double, std::milli> ms_double;
        
        t1 = std::chrono::high_resolution_clock::now();
        
        std::string jason_string((char*)meta, meta_size);
        rapidjson::Document jason_parsed;
        jason_parsed.Parse(jason_string.c_str());
        if(jason_parsed["id"].GetInt() % 20 ==0){ std::cout << jason_string << std::endl; }
        
        t2 = std::chrono::high_resolution_clock::now();
        ms_double = t2 - t1;        
        std::cout << "    JSON parsing took: " << ms_double.count() << " ms" << std::endl;


        // Enforce flushing when full
        if(is_full()){ write_to_disk(); }
        if(is_first_run){ initBuffer(jason_parsed); is_first_run=false; }

        //std::cout << "init" << std::endl;

        //for (rapidjson::Value::ConstMemberIterator itr = jason_parsed.MemberBegin(); itr != jason_parsed.MemberEnd(); ++itr){
        //        std::string key(itr->name.GetString());
        //        std::cout << key << std::endl;
        //}

        // Update the hash
        t1 = std::chrono::high_resolution_clock::now();

            
        hsh.get<std::vector<uint64_t>&>("version")[write_idx] = jason_parsed["version"].GetInt();
        hsh.get<std::vector<uint64_t>&>("id")[write_idx] = jason_parsed["id"].GetInt();
        hsh.get<std::vector<uint64_t>&>("height")[write_idx] = jason_parsed["height"].GetInt();
        hsh.get<std::vector<uint64_t>&>("width")[write_idx] = jason_parsed["width"].GetInt();
        hsh.get<std::vector<uint16_t>&>("dtype")[write_idx] = jason_parsed["dtype"].GetInt();
        hsh.get<std::vector<uint16_t>&>("encoding")[write_idx] = jason_parsed["encoding"].GetInt();
        hsh.get<std::vector<uint16_t>&>("array_id")[write_idx] = jason_parsed["array_id"].GetInt();
        hsh.get<std::vector<uint16_t>&>("status")[write_idx] = jason_parsed["status"].GetInt();
        hsh.get<std::vector<uint64_t>&>("user_1")[write_idx] = jason_parsed["user_1"].GetInt();
        hsh.get<std::vector<uint64_t>&>("user_2")[write_idx] = jason_parsed["user_2"].GetInt();
        

        
        std::vector<uint64_t> shape;
        const auto& s = jason_parsed["shape"];
        for(auto& it : s.GetArray()){   shape.push_back(it.GetInt());   }
        hsh.get<std::vector<std::vector<uint64_t>>&>("shape")[write_idx] = shape;

        t2 = std::chrono::high_resolution_clock::now();
        ms_double = t2 - t1;        
        std::cout << "    Hash update took: " << ms_double.count() << " ms" << std::endl;

        // Hard coded type for now        
        // NOTE: There's a massive performance bottleneck here if this is pre fetched!
        //std::vector<uint64_t>& data_buf = hsh.get<std::vector<uint16_t>&>("data");

        t1 = std::chrono::high_resolution_clock::now();

        std::memcpy(&hsh.get<std::vector<uint16_t>&>("data")[write_idx*m_block_size], data, std::min(data_size, m_block_size));

        t2 = std::chrono::high_resolution_clock::now();
        ms_double = t2 - t1;        
        std::cout << "    Data copy took: " << ms_double.count() << " ms (size: " << std::min(data_size, m_block_size) << " )" << std::endl;

        // Pop index
        write_idx++;
    };

    void initBuffer(rapidjson::Document& meta){
        size_t dsize = 2;
        m_block_size = meta["width"].GetInt() * meta["height"].GetInt() * dsize;
        std::cout << "Block size: " << m_block_size << " ( " << meta["width"].GetInt() << " x " << meta["height"].GetInt() << " ) " << std::endl;


        // Hard coded type for now
        hsh.set("data", std::vector<uint16_t>(m_buffer_size * m_block_size) );
    }


    void write_to_disk(){
        std::cout << "Writing ImageMetadata cache to disk" << std::endl;
        write_idx = 0;

        std::string filename = base_path + "/Run" + std::to_string(run_id) + "_Batch.hdf5";
        Hdf5Writer writer(filename);
        writer.createGroup("/data/");
        writer.createGroup("/data/" + detector_name);
        writer.createGroup("/general/");
        writer.createGroup("/general/" + detector_name);

        writer.writeVector(hsh.get<std::vector<uint64_t>&>("version"),      "/data/" + detector_name + "/version");
        writer.writeVector(hsh.get<std::vector<uint64_t>&>("id"),      "/data/" + detector_name + "/id");
        writer.writeVector(hsh.get<std::vector<uint64_t>&>("width"),      "/data/" + detector_name + "/width");
        writer.writeVector(hsh.get<std::vector<uint64_t>&>("height"),      "/data/" + detector_name + "/height");
        writer.writeVector(hsh.get<std::vector<uint16_t>&>("dtype"),      "/data/" + detector_name + "/dtype");
        writer.writeVector(hsh.get<std::vector<uint16_t>&>("encoding"),      "/data/" + detector_name + "/encoding");
        writer.writeVector(hsh.get<std::vector<uint16_t>&>("array_id"),      "/data/" + detector_name + "/array_id");
        writer.writeVector(hsh.get<std::vector<uint16_t>&>("status"),      "/data/" + detector_name + "/status");
        writer.writeVector(hsh.get<std::vector<uint64_t>&>("user_1"),      "/data/" + detector_name + "/user_1");
        writer.writeVector(hsh.get<std::vector<uint64_t>&>("user_2"),      "/data/" + detector_name + "/user_2");

        std::cout << "Writing data: " << hsh.get<std::vector<uint16_t>&>("data").size() << std::endl;
        std::vector<uint64_t> array_shape = {m_buffer_size};
        auto slice_shape =hsh.get<std::vector<std::vector<uint64_t>>&>("shape")[0];
        array_shape.insert(array_shape.end(), slice_shape.begin(), slice_shape.end());
        writer.writeArray(hsh.get<std::vector<uint16_t>&>("data"), array_shape,      "/data/" + detector_name + "/data");
        
        run_id++;
    };

};





#endif //SF_DAQ_BUFFER_TYPES_HPP
