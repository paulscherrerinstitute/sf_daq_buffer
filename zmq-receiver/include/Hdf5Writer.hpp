#ifndef SF_DAQ_BUFFER_HDF5_WRITER_HPP
#define SF_DAQ_BUFFER_HDF5_WRITER_HPP

#include <hdf5.h>
#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <typeinfo>


class Hdf5Writer {
protected:
    H5::H5File m_file;
    
    template <typename TY>
    H5::DataType get_datatype_for(){
        if(typeid(TY) == typeid(float) ) return H5::PredType::NATIVE_FLOAT;
        if(typeid(TY) == typeid(double) ) return H5::PredType::NATIVE_DOUBLE;
        if(typeid(TY) == typeid(char) ) return H5::PredType::NATIVE_CHAR;
        if(typeid(TY) == typeid(short) ) return H5::PredType::NATIVE_SHORT;
        if(typeid(TY) == typeid(long) ) return H5::PredType::NATIVE_LONG;
        
        if(typeid(TY) == typeid(int8_t) ) return H5::PredType::NATIVE_INT8;
        if(typeid(TY) == typeid(uint8_t) ) return H5::PredType::NATIVE_UINT8;
        if(typeid(TY) == typeid(int16_t) ) return H5::PredType::NATIVE_INT16;
        if(typeid(TY) == typeid(uint16_t) ) return H5::PredType::NATIVE_UINT16;
        
        if(typeid(TY) == typeid(int32_t) ) return H5::PredType::NATIVE_INT32;
        if(typeid(TY) == typeid(uint32_t) ) return H5::PredType::NATIVE_UINT32;
        if(typeid(TY) == typeid(int64_t) ) return H5::PredType::NATIVE_INT64;
        if(typeid(TY) == typeid(uint64_t) ) return H5::PredType::NATIVE_UINT64;
        
        // If all fails
        return H5::PredType::NATIVE_CHAR;
        };

public:
	Hdf5Writer( std::string filename): m_file(filename, H5F_ACC_TRUNC) {
	    // Stop vomiting to console
        H5::Exception::dontPrint();
	};
	
    void createGroup(std::string groupname){ m_file.createGroup(groupname); };

    template <typename TY>
    void writeVector(const std::vector<TY>& data_ref, std::string ipath){
        /* Allocating containers for the data */
        H5::DataType ds_type = this->get_datatype_for<TY>();
        hsize_t      ds_dims[1] = { data_ref.size() };
        H5::DataSpace ds_space(1, ds_dims);
        H5::DataSet dataset = m_file.createDataSet(ipath, ds_type, ds_space);

        /* Writing array to hdf5 file */
        dataset.write(data_ref.data(), ds_type, H5S_ALL, ds_space);
        /* Close dataset */
        dataset.close();
    };
	
    template <typename TY>
    void writeArray(const std::vector<TY>& data_ref, const std::vector<TY>& shape_ref, std::string ipath){
        /* Allocating containers for the data */
		int64_t n_dim = shape_ref.size();
        hsize_t      ds_dims[n_dim];
		for(int64_t dd=0; dd<n_dim; dd++) { ds_dims[dd]=shape_ref[dd]; }			
        H5::DataType ds_type = this->get_datatype_for<TY>();

        H5::DataSpace ds_space(n_dim, ds_dims);
        H5::DataSet dataset = m_file.createDataSet(ipath, ds_type, ds_space);

        /* Writing array to hdf5 file */
        dataset.write(data_ref.data(), ds_type, H5S_ALL, ds_space);
        /* Close dataset */
        dataset.close();
    };
};   // class Hdf5Writer


#endif //SF_DAQ_BUFFER_HDF5_WRITER_HPP
