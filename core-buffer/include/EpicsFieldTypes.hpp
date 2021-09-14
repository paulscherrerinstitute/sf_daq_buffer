#ifndef SF_DAQ_EPICS_FIELD_TYPES_HPP
#define SF_DAQ_EPICS_FIELD_TYPES_HPP

#include <typeindex>
#include <unordered_map>
#include <complex>


typedef enum {
    DBF_STRING,
    DBF_CHAR,
    DBF_UCHAR,
    DBF_SHORT,
    DBF_USHORT,
    DBF_LONG,
    DBF_ULONG,
    DBF_INT64,
    DBF_UINT64,
    DBF_FLOAT,
    DBF_DOUBLE,
    DBF_ENUM,
    DBF_MENU,
    DBF_DEVICE,
    DBF_INLINK,
    DBF_OUTLINK,
    DBF_FWDLINK,
    DBF_NOACCESS
}dbfType;


const std::unordered_map<std::type_index, int> dbfTypeMap = {
    { typeid(void),     (int)dbfType::DBF_CHAR      },
    { typeid(char),     (int)dbfType::DBF_CHAR      },
    { typeid(int8_t),   (int)dbfType::DBF_CHAR      },
    { typeid(uint8_t),  (int)dbfType::DBF_UCHAR     },
    { typeid(int16_t),  (int)dbfType::DBF_SHORT     },
    { typeid(uint16_t), (int)dbfType::DBF_USHORT    },
    { typeid(int32_t),  (int)dbfType::DBF_LONG      },
    { typeid(uint32_t), (int)dbfType::DBF_ULONG     },
    { typeid(int64_t),  (int)dbfType::DBF_INT64     },
    { typeid(uint64_t), (int)dbfType::DBF_UINT64    },
    { typeid(float),    (int)dbfType::DBF_FLOAT     },
    { typeid(double),   (int)dbfType::DBF_DOUBLE    },
};


const std::unordered_map<int , size_t> dbfSize = {
    { (int)dbfType::DBF_CHAR,   sizeof(char) },
    { (int)dbfType::DBF_CHAR,   sizeof(char) },
    { (int)dbfType::DBF_CHAR,   sizeof(int8_t) },
    { (int)dbfType::DBF_UCHAR,  sizeof(uint8_t) },
    { (int)dbfType::DBF_SHORT,  sizeof(int16_t) },
    { (int)dbfType::DBF_USHORT, sizeof(uint16_t) },
    { (int)dbfType::DBF_LONG,   sizeof(int32_t) },
    { (int)dbfType::DBF_ULONG,  sizeof(uint32_t) },
    { (int)dbfType::DBF_INT64,  sizeof(int64_t) },
    { (int)dbfType::DBF_UINT64, sizeof(uint64_t) },
    { (int)dbfType::DBF_FLOAT,  sizeof(float) },
    { (int)dbfType::DBF_DOUBLE, sizeof(double) },
};

#endif // SF_DAQ_EPICS_FIELD_TYPES_HPP
