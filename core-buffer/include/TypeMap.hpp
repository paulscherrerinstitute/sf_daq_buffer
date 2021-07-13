#ifndef SF_DAQ_BUFFER_TYPEMAP_HPP
#define SF_DAQ_BUFFER_TYPEMAP_HPP

#include <typeindex>
#include <unordered_map>
#include <complex>


enum class TypeList {
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
    COMPLEX_DOUBLE,
};


struct DataType{
    const size_t size;
    const int value;
};


const std::unordered_map<std::type_index, DataType> TypeTable = {
    { typeid(void), {sizeof(char), (int)TypeList::VOID} },
    { typeid(char), {sizeof(char), (int)TypeList::CHAR} },
    { typeid(int8_t), {sizeof(int8_t), (int)TypeList::INT8} },
    { typeid(uint8_t), {sizeof(uint8_t), (int)TypeList::UINT8} },
    { typeid(int16_t), {sizeof(int16_t), (int)TypeList::INT16} },
    { typeid(uint16_t), {sizeof(uint16_t), (int)TypeList::UINT16} },
    { typeid(int32_t), {sizeof(int32_t), (int)TypeList::INT32} },
    { typeid(uint32_t), {sizeof(uint32_t), (int)TypeList::UINT32} },
    { typeid(int64_t), {sizeof(int64_t), (int)TypeList::INT64} },
    { typeid(uint64_t), {sizeof(uint64_t), (int)TypeList::UINT64} },
    { typeid(float), {sizeof(float), (int)TypeList::FLOAT} },
    { typeid(double), {sizeof(double), (int)TypeList::DOUBLE} },
    { typeid(std::complex<float>), {sizeof(std::complex<float>), (int)TypeList::COMPLEX_FLOAT} },
    { typeid(std::complex<double>), {sizeof(std::complex<double>), (int)TypeList::COMPLEX_DOUBLE} },
};


const std::unordered_map<int , DataType> TypeMap = {
    { (int)TypeList::VOID, {sizeof(char), (int)TypeList::VOID} },
    { (int)TypeList::CHAR, {sizeof(char), (int)TypeList::CHAR} },
    { (int)TypeList::INT8, {sizeof(int8_t), (int)TypeList::INT8} },
    { (int)TypeList::UINT8, {sizeof(uint8_t), (int)TypeList::UINT8} },
    { (int)TypeList::INT16, {sizeof(int16_t), (int)TypeList::INT16} },
    { (int)TypeList::UINT16, {sizeof(uint16_t), (int)TypeList::UINT16} },
    { (int)TypeList::INT32, {sizeof(int32_t), (int)TypeList::INT32} },
    { (int)TypeList::UINT32, {sizeof(uint32_t), (int)TypeList::UINT32} },
    { (int)TypeList::INT64, {sizeof(int64_t), (int)TypeList::INT64} },
    { (int)TypeList::UINT64, {sizeof(uint64_t), (int)TypeList::UINT64} },
    { (int)TypeList::FLOAT, {sizeof(float), (int)TypeList::FLOAT} },
    { (int)TypeList::DOUBLE, {sizeof(double), (int)TypeList::DOUBLE} },
    { (int)TypeList::COMPLEX_FLOAT, {sizeof(std::complex<float>), (int)TypeList::COMPLEX_FLOAT} },
    { (int)TypeList::COMPLEX_DOUBLE, {sizeof(std::complex<double>), (int)TypeList::COMPLEX_DOUBLE} },
};


#endif // SF_DAQ_BUFFER_TYPEMAP_HPP
