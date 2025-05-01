#ifndef UTILS_HPP
#define UTILS_HPP

#pragma once

#include <npp.h>

///
/// \brief The Buffer_t class
///
/// This class represent a 1D buffer.
/// It was desgin to be either a filter
/// kernel of a scratch buffer. For this
/// purpose the copy construction of the
/// non-move assignation operator have
/// been deleted.
/// The destruction will deallocate
/// any allocated memory if any.
///
template<class T>
struct Buffer_t
{
    using pointer = T*;
    using self = Buffer_t;

    ///
    /// \brief Buffer_t: Default Constructor.
    ///
    /// Properly initialiaze the attribute.
    /// This is required because none of the
    /// attributes is an object.
    ///
    __forceinline__ Buffer_t():
        data(nullptr),
        size(0)
    {}

    ///
    /// \brief Buffer_t : Parametric Constructor:
    ///  Allocate the specified amount of memory
    /// \param _size : Number of element of type T to allocate.
    ///
    __forceinline__ Buffer_t(const Npp32s& _size)
    {
        this->resize(_size);
    }

    Buffer_t(const self&) = delete;
    Buffer_t(self&&) = default;

    ///
    /// \brief Destructor : will deallocate the memory, if any memory was allocated.
    ///
    ~Buffer_t()
    {
        this->release();
    }

    self& operator=(const self&) = delete;
    self& operator=(self&&) = default;

    ///
    /// \brief empty : accessor
    /// \return true if no memory was allocate, false otherwise.
    ///
    __host__ __device__ __forceinline__ bool empty() const
    {
        return !this->data;
    }

    ///
    /// \brief release : release all the allocated memory if
    /// any was allocated, and reset the size to 0.
    ///
    __host__ void release()
    {
        if(this->data)
            nppsFree(this->data);
        this->size = 0;
    }

    ///
    /// \brief resize : allocate memory
    /// \param _size : number of elements of type T to allocate.
    ///
    /// \note If the _size if equal to the current size nothing
    /// is done, otherwise the current memory is deallocated
    /// and another allocation is made with the proper size.
    /// In this case the current elements are not preserved.
    ///
    __host__ void resize(const Npp32s& _size)
    {
        if(this->size != _size)
            this->release();

        this->data = reinterpret_cast<T*>(nppsMalloc_8u(_size * sizeof(T)));
        this->size = _size;
    }

    ///
    /// \brief operator pointer : cast operator
    ///
    /// If an object is assign to a T* this operator is called.
    ///
    __host__ __device__ __forceinline__ operator pointer()
    {
        return this->data;
    }


    // Address of the first element in the memory.
    pointer data;
    // Number of element of T that were allocated.
    Npp32s size;
};


///
/// \brief The Buffer2D_t class
///
template<class T, int cn>
struct Buffer2D_t
{

    using pointer = T*;
    using self = Buffer2D_t;

    ///
    /// \brief Buffer2D_t : Default
    ///
    /// Properly initialiaze the attribute.
    /// This is required because none of the
    /// attributes is an object.
    ///
    __forceinline__ Buffer2D_t():
        data(nullptr),
        step(0),
        size({0,0}),
        flat_size({0,0})
    {}

    ///
    /// \brief Buffer2D_t : Parametric Constructor:
    ///
    ///  Allocate the specified amount of memory
    /// \param _size : Number of element of type T to allocate.
    ///
    __forceinline__ Buffer2D_t(const NppiSize& _size)
    {
        this->create(_size);
    }

    Buffer2D_t(const self&) = delete;
    Buffer2D_t(self&&) = default;

    ///
    /// \brief Destructor : will deallocate the memory, if any memory was allocated.
    ///
    ~Buffer2D_t()
    {
        this->release();
    }

    self& operator=(const self&) = delete;
    self& operator=(self&&) = default;

    ///
    /// \brief empty : accessor
    /// \return true if no memory was allocate, false otherwise.
    ///
    __host__ __device__ __forceinline__ bool empty() const
    {
        return !this->data;
    }


    ///
    /// \brief release : release all the allocated memory if
    /// any was allocated, and reset the sizes and the step to 0.
    ///
    __host__ void release()
    {
        if(this->data)
        {
            nppiFree(this->data);
            this->step = 0;
            this->size = this->flat_size = {0, 0};
        }
    }

    ///
    /// \brief create : allocate memory.
    /// \param _size : width and height to represent.
    ///
    __host__ void create(const NppiSize& _size)
    {
        if(this->data && ((this->size.width != _size.width) || (this->size.height != _size.height)))
            this->release();

        this->data = reinterpret_cast<pointer>(nppiMalloc_8u_C1(_size.width * cn * sizeof(T), _size.height, std::addressof(this->step)));
        this->size = _size;
        this->flat_size = {_size.width * cn, _size.height};
    }

    ///
    /// \brief operator pointer : cast operator
    ///
    /// If an object is assign to a T* this operator is called.
    ///
    __host__ __device__ __forceinline__ operator pointer()
    {
        return this->data;
    }

    // Address of the first element in the memory.
    pointer data;
    // Number of bytes (not elements) to add to the address of the first element for move from one row to another.
    Npp32s step;
    // Size considering the channels.
    NppiSize size;
    // Size if the data had a single channel.
    NppiSize flat_size;

};



#endif // UTILS_HPP
