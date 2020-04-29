#include "gtest/gtest.h"
#include "MetadataBuffer.hpp"

using namespace std;

TEST(MetadataBuffer, basic_operations)
{
    int n_frames = 10;
    int n_modules = 3;

    unordered_map<string, HeaderDataType> header_values {
        {"frame", HeaderDataType("uint64")},
        {"module_number", HeaderDataType("uint64", n_modules)}
    };

    MetadataBuffer metadata_buffer(
            static_cast<const uint64_t>(n_frames),
            header_values);

    uint64_t base_frame_data = 12345678;
    uint64_t base_module_number = 0;

    for (int i = 0; i<n_frames; i++) {
        uint64_t frame_index = base_frame_data + i;
        metadata_buffer.add_metadata_to_buffer("frame", i, (char*)&frame_index);

        uint64_t module_number = base_module_number + i;
        uint64_t module_number_data[n_modules];

        for (int module_index=0; module_index<n_modules; module_index++) {
            module_number_data[module_index] = module_number;
        }
        
        metadata_buffer.add_metadata_to_buffer("module_number", i, (char*)&module_number_data);
    }

    uint64_t* frame_data = (uint64_t*) metadata_buffer.get_metadata_values("frame").get();
    uint64_t* module_number_data = (uint64_t*) metadata_buffer.get_metadata_values("module_number").get();

    for (int i = 0; i<n_frames; i++) {
	    EXPECT_EQ(frame_data[i], base_frame_data+i);

        for (int module_index=0; module_index<n_modules; module_index++) {
            EXPECT_EQ(module_number_data[n_modules*i + module_index], base_module_number + i);
        }
    }
}

TEST(MetadataBuffer, missing_keys)
{
    int n_frames = 10;
    int n_modules = 3;

    unordered_map<string, HeaderDataType> header_values = {
        {"frame", HeaderDataType("uint64")},
        {"module_number", HeaderDataType("uint64", n_modules)}
    };

    MetadataBuffer metadata_buffer(n_frames, header_values);

    uint64_t data = 0;

    metadata_buffer.add_metadata_to_buffer("frame", 0, (char*)&data);
    metadata_buffer.get_metadata_values("frame");

    EXPECT_THROW(
            metadata_buffer.get_metadata_values("non_existant"),
            runtime_error);
    EXPECT_THROW(
            metadata_buffer.add_metadata_to_buffer("non_existant", 0, nullptr),
            runtime_error);
    EXPECT_THROW(
            metadata_buffer.add_metadata_to_buffer("frame", n_frames, nullptr),
            runtime_error);
}