//#include "FastQueue.hpp"
//#include "formats.hpp"
//#include "gtest/gtest.h"
//
//using namespace buffer_config;
//
//TEST(FastQueue, basic_interaction)
//{
//    size_t n_slots = 5;
//    size_t slot_data_n_bytes = MODULE_N_BYTES * 2;
//    FastQueue<ImageMetadata> queue(slot_data_n_bytes, n_slots);
//    int slot_id;
//
//    // The queue at the beginning should be empty.
//    ASSERT_EQ(queue.read(), -1);
//    // Cannot commit a slot until you reserve it.
//    ASSERT_THROW(queue.commit(), runtime_error);
//    // Cannot release a slot until its ready.
//    ASSERT_THROW(queue.release(), runtime_error);
//
//    // Reserve a slot.
//    slot_id = queue.reserve();
//    ASSERT_NE(slot_id, -1);
//    // But you cannot reserve 2 slots at once.
//    ASSERT_EQ(queue.reserve(), -1);
//    // And cannot read this slot until its committed.
//    ASSERT_EQ(queue.read(), -1);
//
//    auto detector_frame = queue.get_metadata_buffer(slot_id);
//    char* meta_ptr = (char*) detector_frame;
//    char* data_ptr = (char*) queue.get_data_buffer(slot_id);
//
//    queue.commit();
//
//    slot_id = queue.read();
//    // Once the slot is committed we should be able to read it.
//    ASSERT_NE(slot_id, -1);
//    // You can read the same slot multiple times.
//    ASSERT_NE(queue.read(), -1);
//    // The 2 buffers should match the committed slot.
//    ASSERT_EQ(meta_ptr, (char*)(queue.get_metadata_buffer(slot_id)));
//    ASSERT_EQ(data_ptr, (char*)(queue.get_data_buffer(slot_id)));
//
//    queue.release();
//}
//
//TEST(FastQueue, queue_full)
//{
//    size_t n_slots = 5;
//    size_t slot_data_n_bytes = MODULE_N_BYTES * 2;
//    FastQueue<ImageMetadata> queue(slot_data_n_bytes, n_slots);
//
//    // There is nothing to be read in the queue.
//    ASSERT_EQ(queue.read(), -1);
//
//    for (size_t i=0; i<n_slots; i++) {
//        // Business as usual here, we still have slots left.
//        ASSERT_NE(queue.reserve(), -1);
//        queue.commit();
//    }
//
//    // There are no more slots available.
//    ASSERT_EQ(queue.reserve(), -1);
//    // We now read the first slot.
//    ASSERT_EQ(queue.read(), 0);
//    // But until we release it we cannot re-use it.
//    ASSERT_EQ(queue.reserve(), -1);
//
//    queue.release();
//    // After the release, the first slot is again ready for writing.
//    ASSERT_EQ(queue.reserve(), 0);
//}
//
//TEST(FastQueue, data_transfer)
//{
//    size_t n_slots = 5;
//    size_t slot_data_n_bytes = MODULE_N_BYTES * 2;
//    FastQueue<ImageMetadata> queue(slot_data_n_bytes, n_slots);
//
//    int write_slot_id = queue.reserve();
//
//    auto w_metadata = queue.get_metadata_buffer(write_slot_id);
//    w_metadata->pulse_id = 1;
//    w_metadata->frame_index = 2;
//    w_metadata->daq_rec = 3;
//    w_metadata->is_good_frame = 4;
//
//    auto w_data = (uint16_t*)(queue.get_data_buffer(write_slot_id));
//    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
//        w_data[i] = (uint16_t) i;
//    }
//
//    queue.commit();
//
//    auto read_slot_id = queue.read();
//
//    auto r_metadata = queue.get_metadata_buffer(read_slot_id);
//    EXPECT_EQ(w_metadata->pulse_id,
//            r_metadata->pulse_id);
//    EXPECT_EQ(w_metadata->frame_index,
//            r_metadata->frame_index);
//    EXPECT_EQ(w_metadata->daq_rec,
//            r_metadata->daq_rec);
//    EXPECT_EQ(w_metadata->is_good_frame,
//            r_metadata->is_good_frame);
//
//    auto r_data = (uint16_t*)(queue.get_data_buffer(read_slot_id));
//    for (size_t i=0; i<MODULE_N_PIXELS; i++) {
//        ASSERT_EQ(r_data[i], (uint16_t) i);
//    }
//}
//
//TEST(FaseQueue, array_parameter)
//{
//    size_t n_modules = 32;
//    FastQueue<ModuleFrameBuffer> queue(
//            n_modules * MODULE_N_BYTES,
//            WRITER_FASTQUEUE_N_SLOTS);
//
//    ModuleFrame frame;
//
//    auto slot_id = queue.reserve();
//    auto metadata = queue.get_metadata_buffer(slot_id);
//
//    for (int i_module=0; i_module<n_modules; i_module++) {
//        auto& module_metadata = metadata->module[i_module];
//
//        frame.pulse_id = i_module;
//        frame.frame_index = i_module;
//        frame.daq_rec = i_module;
//        frame.n_recv_packets = i_module;
//        frame.module_id = i_module;
//
//        ModuleFrame* p_metadata = &module_metadata;
//
//        memcpy(p_metadata, &frame, sizeof(ModuleFrame));
//    }
//
//    for (int i_module=0; i_module<n_modules; i_module++) {
//        auto& module_metadata = metadata->module[i_module];
//
//        ASSERT_EQ(module_metadata.pulse_id, i_module);
//        ASSERT_EQ(module_metadata.frame_index, i_module);
//        ASSERT_EQ(module_metadata.daq_rec, i_module);
//        ASSERT_EQ(module_metadata.n_recv_packets, i_module);
//        ASSERT_EQ(module_metadata.module_id, i_module);
//    }
//}
//
//// TODO: Test with payload of zero (metadata only).