#ifndef SF_DAQ_BUFFER_TYPEMAP_HPP
#define SF_DAQ_BUFFER_TYPEMAP_HPP

#include <unordered_map>


enum class TypeMap {
    VOID,
    CHAR,
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    INT64,
    UINT64,
    FLOAT,
    DOUBLE,
    COMPLEX_FLOAT,
    COMPLEX_DOUBLE
};

struct Type{
    const size_t size;
    const int value;
};


const std::unordered_map<std::type_index, Type> TypeTable = {
    { typeid(void), {sizeof(void), TypeMap::VOID} },
    { typeid(char), {sizeof(char), TypeMap::CHAR} },
    { typeid(int8_t), {sizeof(int8_t), TypeMap::INT8} },
    { typeid(uint8_t), {sizeof(uint8_t), TypeMap::UINT8} },
    { typeid(int16_t), {sizeof(int16_t), TypeMap::INT16} },
    { typeid(uint16_t), {sizeof(uint16_t), TypeMap::UINT16} },
    { typeid(int32_t), {sizeof(int32_t), TypeMap::INT32} },
    { typeid(uint32_t), {sizeof(uint32_t), TypeMap::UINT32} },
    { typeid(int64_t), {sizeof(int64_t), TypeMap::INT64} },
    { typeid(uint64_t), {sizeof(uint64_t), TypeMap::UINT64} },
    { typeid(float), {sizeof(float), TypeMap::float} },
    { typeid(double), {sizeof(double), TypeMap::DOUBLE} },
    { typeid(std::complex<float>), {sizeof(std::complex<float>), TypeMap::COMPLEX_FLOAT} },
    { typeid(std::complex<double>), {sizeof(std::complex<double>), TypeMap::COMPLEX_DOUBLE} }
};

const std::unordered_map<int , Type> TypeTable = {
    { TypeMap::VOID, {sizeof(void), TypeMap::VOID} },
    { TypeMap::CHAR, {sizeof(char), TypeMap::CHAR} },
    { TypeMap::INT8, {sizeof(int8_t), TypeMap::INT8} },
    { TypeMap::UINT8, {sizeof(uint8_t), TypeMap::UINT8} },
    { TypeMap::INT16, {sizeof(int16_t), TypeMap::INT16} },
    { TypeMap::UINT16, {sizeof(uint16_t), TypeMap::UINT16} },
    { TypeMap::INT32, {sizeof(int32_t), TypeMap::INT32} },
    { TypeMap::UINT32, {sizeof(uint32_t), TypeMap::UINT32} },
    { TypeMap::INT64, {sizeof(int64_t), TypeMap::INT64} },
    { TypeMap::UINT64, {sizeof(uint64_t), TypeMap::UINT64} },
    { TypeMap::FLOAT, {sizeof(float), TypeMap::float} },
    { TypeMap::DOUBLE, {sizeof(double), TypeMap::DOUBLE} },
    { TypeMap::COMPLEX_FLOAT, {sizeof(std::complex<float>), TypeMap::COMPLEX_FLOAT} },
    { TypeMap::COMPLEX_DOUBLE, {sizeof(std::complex<double>), TypeMap::COMPLEX_DOUBLE} }
};

#endif // SF_DAQ_BUFFER_TYPEMAP_HPP
