#ifndef UTILS_HPP
#define UTILS_HPP

#pragma once


#include <stdexcept>
#include <cmath>
#include <iostream>

#include <cuda_runtime.h>
#include <vector_types.h>
#include <cuda_fp16.h>



namespace cas
{

///
/// \brief check_error_cuda : check if an error has happen or not.
/// If an error happened an exception is thrown.
/// \param error : error to check.
///
__host__ __forceinline__ void check_error_cuda(const cudaError_t& error)
{
    if(error)
        throw std::runtime_error(cudaGetErrorString(error));
}

///
/// \brief div_up : return the round-up division
/// \param num : numerator.
/// \param den : denominator.
/// \return the round-up division.
///
__host__ __forceinline__ int div_up(const int& num, const int& den)
{
    float numf = static_cast<float>(num);
    float denf = static_cast<float>(den);

    return static_cast<int>(std::ceil(numf/denf));
}

///
/// \brief isDevicePointer : check if an address is allocated on the device or not.
/// \param ptr : address to assess.
/// \return true if the address is allocated on the device, false otherwise.
///
__host__ __forceinline__ bool isDevicePointer(const void* ptr)
{
    cudaPointerAttributes attributes;
    cudaError_t err = cudaPointerGetAttributes(&attributes, ptr);

    if (err != cudaSuccess) {
        // If the error is not cudaSuccess, it means the pointer is not recognized by CUDA
        cudaGetLastError(); // Reset the CUDA error state
        return false;
    }

    return attributes.type != cudaMemoryTypeHost;

//    return attributes.type == cudaMemoryTypeDevice;
}

using uchar = unsigned char;
using schar = signed char;
using uint = unsigned;
using ulong = unsigned long;
using longlong = long long;
using ulonglong = unsigned long long;


///
/// \brief Traits class for CUDA's vector type. It also offer an interface with fundamental types.
///
template<class T>
struct vectorTraits;

#define IMPL_VECTOR_TRAITS_SPEC(ftype)\
template<> struct vectorTraits<ftype> { static constexpr int channels = 1; using type=ftype; using lane_type = ftype;};\
template<> struct vectorTraits<ftype ## 1> { static constexpr int channels = 1; using type=ftype ## 1; using lane_type = ftype;};\
template<> struct vectorTraits<ftype ## 2> { static constexpr int channels = 2; using type=ftype ## 2; using lane_type = ftype;};\
template<> struct vectorTraits<ftype ## 3> { static constexpr int channels = 3; using type=ftype ## 3; using lane_type = ftype;};\
template<> struct vectorTraits<ftype ## 4> { static constexpr int channels = 4; using type=ftype ## 4; using lane_type = ftype;};

IMPL_VECTOR_TRAITS_SPEC(uchar)
//IMPL_VECTOR_TRAITS_SPEC(char)
template<> struct vectorTraits<signed char> { static constexpr int channels = 1; using type = schar; using lane_type = schar;};
template<> struct vectorTraits<char1> { static constexpr int channels = 1; using type = char1; using lane_type = schar;};
template<> struct vectorTraits<char2> { static constexpr int channels = 2; using type = char2; using lane_type = schar;};
template<> struct vectorTraits<char3> { static constexpr int channels = 3; using type = char3; using lane_type = schar;};
template<> struct vectorTraits<char4> { static constexpr int channels = 4; using type = char4; using lane_type = schar;};
IMPL_VECTOR_TRAITS_SPEC(ushort)
IMPL_VECTOR_TRAITS_SPEC(short)
IMPL_VECTOR_TRAITS_SPEC(int)
IMPL_VECTOR_TRAITS_SPEC(uint)
IMPL_VECTOR_TRAITS_SPEC(long)
IMPL_VECTOR_TRAITS_SPEC(ulong)
IMPL_VECTOR_TRAITS_SPEC(longlong)
IMPL_VECTOR_TRAITS_SPEC(ulonglong)
IMPL_VECTOR_TRAITS_SPEC(float)
IMPL_VECTOR_TRAITS_SPEC(double)

#undef IMPL_VECTOR_TRAITS_SPEC



///
/// \brief CUDA's vectorized type class creation.
///
template<class T, int cn>
struct make_vector_type;

#define SPEC_MAKE_VECTOR_TYPE_DECL_CN_(ftype, vtype)\
    template<> struct make_vector_type <ftype, 1>{ using type = vtype ## 1; };\
    template<> struct make_vector_type <ftype, 2>{ using type = vtype ## 2; };\
    template<> struct make_vector_type <ftype, 3>{ using type = vtype ## 3; };\
    template<> struct make_vector_type <ftype, 4>{ using type = vtype ## 4; };

#define SPEC_MAKE_VECTOR_TYPE_DECL\
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(unsigned char, uchar) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(signed char, char) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(unsigned short, ushort) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(short, short) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(unsigned int, uint) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(int, int) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(unsigned long, ulong) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(long, long) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(long long, longlong) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(unsigned long long, ulonglong) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(float, float) \
    SPEC_MAKE_VECTOR_TYPE_DECL_CN_(double, double)

SPEC_MAKE_VECTOR_TYPE_DECL

// Less typename everywhere.
template<class T, int cn>
using make_vector_type_t = typename make_vector_type<T, cn>::type;

///
/// \brief The is_cuda_vector_type class : check if the provided argument is a template or not.
///
template<class T>
struct is_cuda_vector_type : std::false_type {};

#define SEPC_IS_VECT_TYPE(vtype)\
    template<> struct is_cuda_vector_type<vtype ## 1> : std::true_type{};\
    template<> struct is_cuda_vector_type<vtype ## 2> : std::true_type{};\
    template<> struct is_cuda_vector_type<vtype ## 3> : std::true_type{};\
    template<> struct is_cuda_vector_type<vtype ## 4> : std::true_type{};

SEPC_IS_VECT_TYPE(uchar)
SEPC_IS_VECT_TYPE(char)
SEPC_IS_VECT_TYPE(ushort)
SEPC_IS_VECT_TYPE(short)
SEPC_IS_VECT_TYPE(int)
SEPC_IS_VECT_TYPE(uint)
SEPC_IS_VECT_TYPE(long)
SEPC_IS_VECT_TYPE(ulong)
SEPC_IS_VECT_TYPE(longlong)
SEPC_IS_VECT_TYPE(ulonglong)
SEPC_IS_VECT_TYPE(float)
SEPC_IS_VECT_TYPE(double)



///
/// \brief operator << : ostream operator << overlod.
/// \param ostr : stream to write on.
/// \param obj : object from a cuda's vectorized type, to write on the stream.
/// \return ostr
///
template<class T, std::enable_if_t<is_cuda_vector_type<T>::value, bool> = true >
__host__ __forceinline__ std::ostream& operator<<(std::ostream& ostr, const T& obj)
{
    if constexpr (vectorTraits<T>::channels == 1)
    {
        if constexpr (sizeof(typename vectorTraits<T>::lane_type) == 1)
        {
            ostr<<"x: "<<static_cast<short>(obj.x)<<std::endl;
        }
        else
        {
            ostr << obj.x << std::endl;
        }
    }
    else if constexpr (vectorTraits<T>::channels == 2)
    {
        if constexpr (sizeof(typename vectorTraits<T>::lane_type) == 1)
        {
            ostr<<"x: "<<static_cast<short>(obj.x)<<" y: "<<static_cast<short>(obj.y)<<std::endl;
        }
        else
        {
            ostr<<"x: "<<obj.x<<" y: "<<obj.y<<std::endl;
        }
    }
    else if constexpr (vectorTraits<T>::channels == 3)
    {
        if constexpr (sizeof(typename vectorTraits<T>::lane_type) == 1)
        {
            ostr<<"x: "<<static_cast<short>(obj.x)<<" y: "<<static_cast<short>(obj.y)<<" z: "<<static_cast<short>(obj.z)<<std::endl;
        }
        else
        {
            ostr<<"x: "<<obj.x<<" y: "<<obj.y<<" z: "<<obj.z<<std::endl;
        }
    }
    else
    {
        if constexpr (sizeof(typename vectorTraits<T>::lane_type) == 1)
        {
            ostr<<"x: "<<static_cast<short>(obj.x)<<" y: "<<static_cast<short>(obj.y)<<" z: "<<static_cast<short>(obj.z)<<" w: "<<static_cast<short>(obj.w)<<std::endl;
        }
        else
        {
            ostr<<"x: "<<obj.x<<" y: "<<obj.y<<" z: "<<obj.z<<" w: "<<obj.w<<std::endl;
        }
    }

    return ostr;
}



} // cuda

#endif // UTILS_HPP
