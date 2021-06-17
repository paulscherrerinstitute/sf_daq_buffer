#ifndef IMAGE_HPP
#define JUNGFRAU_H

#include <cstdint>
#include <vector>
#include "formats.hpp"


class array_t {
    public:
        // Constructor
        array_t(size_t i_size): m_container(i_size) {};

        // Access methods
        ModuleFrame* meta(){ return &m_metadata; };
        char* data(){ return m_container.data(); };
        size_t size(){ return m_container.size(); };
    protected:
        std::vector<char> m_container;
        ModuleFrame m_metadata;
};







#endif  //IMAGE_HPP
