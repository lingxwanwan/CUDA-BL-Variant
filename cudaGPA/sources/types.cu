#include "types.cuh"
#include "utils.hpp"
#include <cstring>
#include <algorithm>

#define DEF_CLASS_SPEC(name)\
template class name<unsigned char>;\
template class name<signed char>;\
template class name<unsigned short>;\
template class name<short>;\
template class name<unsigned>;\
template class name<int>;\
template class name<unsigned long>;\
template class name<long>;\
template class name<unsigned long long>;\
template class name<long long>;\
template class name<float>;\
template class name<double>;\
\
template class name<uchar1>;\
template class name<char1>;\
template class name<ushort1>;\
template class name<short1>;\
template class name<uint1>;\
template class name<int1>;\
template class name<ulong1>;\
template class name<long1>;\
template class name<ulonglong1>;\
template class name<longlong1>;\
template class name<float1>;\
template class name<double1>;\
\
template class name<uchar2>;\
template class name<char2>;\
template class name<ushort2>;\
template class name<short2>;\
template class name<uint2>;\
template class name<int2>;\
template class name<ulong2>;\
template class name<long2>;\
template class name<ulonglong2>;\
template class name<longlong2>;\
template class name<float2>;\
template class name<double2>;\
\
template class name<uchar3>;\
template class name<char3>;\
template class name<ushort3>;\
template class name<short3>;\
template class name<uint3>;\
template class name<int3>;\
template class name<ulong3>;\
template class name<long3>;\
template class name<ulonglong3>;\
template class name<longlong3>;\
template class name<float3>;\
template class name<double3>;\
\
template class name<uchar4>;\
template class name<char4>;\
template class name<ushort4>;\
template class name<short4>;\
template class name<uint4>;\
template class name<int4>;\
template class name<ulong4>;\
template class name<long4>;\
template class name<ulonglong4>;\
template class name<longlong4>;\
template class name<float4>;\
template class name<double4>;


namespace  cas
{

/////////////////////////////////////////////////////
/// SAFE STREAM
////////////////////////////////////////////////////

//int safe_stream::id_ = 0;

__host__ safe_stream::safe_stream():
    stream(nullptr),
    own(false)
{

}

__host__ safe_stream::safe_stream(const cudaStream_t& _stream, const bool &_own):
    stream(_stream),
    own(_own)
{

}

__host__ safe_stream::~safe_stream()
{
    this->destroy();
}


__host__ void safe_stream::create(const unsigned int& flags, const int& priority)
{
    // If a stream already exists
    // it must be destroied before
    // continuing.
    if(this->stream)
        this->destroy();

    if(flags == cudaStreamDefault && priority<0)
    {
        cas::check_error_cuda(cudaStreamCreate(std::addressof(this->stream)));
    }
    else if(flags != cudaStreamDefault && priority<0)
    {
        cas::check_error_cuda(cudaStreamCreateWithFlags(std::addressof(this->stream), flags));
    }
    else if(flags == cudaStreamDefault && priority>=0)
    {
        cas::check_error_cuda(cudaStreamCreateWithPriority(std::addressof(this->stream), cudaStreamDefault, priority));
    }
    else
    {
        cas::check_error_cuda(cudaStreamCreateWithPriority(std::addressof(this->stream), flags, priority));
    }
    this->own = true;
}

__host__ void safe_stream::destroy()
{
    if(this->stream && this->own)
    {
        cas::check_error_cuda(cudaStreamDestroy(this->stream));
    }
    this->stream = nullptr;
    this->own = false;
}

__host__ void safe_stream::waitEvent(const safe_event& event, const unsigned int& flags)
{
    check_error_cuda(cudaStreamWaitEvent(this->stream, event, flags));
}

__host__ safe_stream::operator cudaStream_t() const
{
    return this->stream;
}

__host__ safe_stream::operator bool() const
{
    return this->stream;
}



/////////////////////////////////////////////////////
/// SAFE EVENT
////////////////////////////////////////////////////

__host__ safe_event::safe_event():
    event(nullptr)
{
//    this->create();
}

__host__ safe_event::safe_event(const cudaEvent_t &_event, const bool &_own):
    event(_event),
    own(_own)
{}

__host__ safe_event::~safe_event()
{
    this->destroy();
}

__host__ void safe_event::create(const unsigned int& flags)
{
    if(this->event)
        this->destroy();

    if(flags!=cudaEventDefault)
    {
        cudaEventCreate(std::addressof(this->event));
    }
    else
    {
        cudaEventCreateWithFlags(std::addressof(this->event), flags);
    }
    this->own = true;
}

__host__ void safe_event::destroy()
{
    if(this->event && this->own)
    {
        cudaEventDestroy(this->event);
    }
    this->event = nullptr;
    this->own = false;
}

__host__ void safe_event::record(const safe_stream& _stream, const unsigned int& _flags)
{
    if(_flags!=cudaEventDefault)
    {
        cudaEventRecord(this->event, _stream);
    }
    else
    {
        cudaEventRecordWithFlags(this->event, _stream, _flags);
    }
}

__host__ void safe_event::synchonize()
{
    cudaEventSynchronize(this->event);
}

__host__ safe_event::operator cudaEvent_t() const
{
    return this->event;
}

/////////////////////////////////////////////////////
/// PTRSTEP
////////////////////////////////////////////////////

template<class Type>
__host__ __device__ PtrStep<Type>::PtrStep():
    data(nullptr),
    step(0)
{}

template<class Type>
__host__ __device__ PtrStep<Type>::PtrStep(pointer _data, const size_t& _step):
    data(reinterpret_cast<pointer_byte>(_data)),
    step(_step)
{}


template<class Type>
__host__ __device__ typename PtrStep<Type>::pointer PtrStep<Type>::ptr(const int& y)
{
    return reinterpret_cast<pointer>(this->data + y * step);
}

template<class Type>
__host__ __device__ typename PtrStep<Type>::pointer PtrStep<Type>::ptr(const int& y, const int& x)
{
    return this->ptr(y) + x;
}

template<class Type>
__device__  typename PtrStep<Type>::reference PtrStep<Type>::at(const int& y, const int& x)
{
    return *this->ptr(y, x);
}

template<class Type>
__host__ __device__ typename PtrStep<Type>::const_pointer PtrStep<Type>::ptr(const int& y) const
{
    return reinterpret_cast<const_pointer>(this->data + y * step);
}

template<class Type>
__host__ __device__ typename PtrStep<Type>::const_pointer PtrStep<Type>::ptr(const int& y, const int& x) const
{
    return this->ptr(y) + x;
}

template<class Type>
__device__ typename PtrStep<Type>::const_reference PtrStep<Type>::at(const int& y, const int& x) const
{
    return *this->ptr(y, x);
}

template<class Type>
__host__ __device__ int PtrStep<Type>::pitch() const
{
    return this->step;
}

DEF_CLASS_SPEC(PtrStep)



} // cuda
