#ifndef TYPES_CUH
#define TYPES_CUH

//#pragma once

#include <cuda_runtime.h>
#include <memory>

#include <iostream>
#include <iomanip>

#include "utils.hpp"

namespace cas
{
/////////////////////////////////////////////////////
/// SAFE STREAM
////////////////////////////////////////////////////

class safe_event;

class safe_stream
{

public:

    __host__ safe_stream();

    __host__ safe_stream(const cudaStream_t& _stream, const bool& _own = false);

    __host__ ~safe_stream();

    __host__ void create(const unsigned int& flags = cudaStreamDefault, const int& priority=-1);

    __host__ void destroy();

    __host__ void waitEvent(const safe_event& event, const unsigned int& flags = cudaEventWaitDefault);

    __host__ operator cudaStream_t() const;

    __host__ operator bool() const;

private:
    cudaStream_t stream;
    bool own;
};


/////////////////////////////////////////////////////
/// SAFE EVENT
////////////////////////////////////////////////////

class safe_event
{

public:

    __host__ safe_event();

    __host__ safe_event(const cudaEvent_t& event, const bool& _own = false);

    __host__ ~safe_event();

    __host__ void create(const unsigned int& flags = cudaEventDefault);

    __host__ void destroy();

    __host__ void record(const safe_stream& _stream = safe_stream(0), const unsigned int& _flags = cudaEventDefault);

    __host__ void synchonize();

    __host__ operator cudaEvent_t() const;

private:

    cudaEvent_t event;
    bool own;
};

/////////////////////////////////////////////////////
/// PTRSTEP
////////////////////////////////////////////////////


template<class T>
class PtrStep
{
public:
    using pointer = T*;
    using const_pointer = const T*;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    __host__ __device__ PtrStep();

    __host__ __device__ PtrStep(pointer _data, const size_t& _step);

    PtrStep(const PtrStep&) = default;
    PtrStep(PtrStep&&) = default;

    ~PtrStep() = default;

    PtrStep& operator =(const PtrStep&) = default;
    PtrStep& operator =(PtrStep&&) = default;

    __host__ __device__ pointer ptr(const int& y);

    __host__ __device__ pointer ptr(const int& y, const int& x);

    __device__ reference at(const int& y, const int& x);

    __host__ __device__ const_pointer ptr(const int& y) const;

    __host__ __device__ const_pointer ptr(const int& y, const int& x) const;

    __device__ const_reference at(const int& y, const int& x) const;

    __host__ __device__ int pitch() const;

private:

    using pointer_byte = unsigned char*;

    pointer_byte data;
    size_t step;
};



} // cas

#endif // TYPES_H
