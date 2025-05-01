#ifndef CUDA_GPABF_HPP
#define CUDA_GPABF_HPP 1

#pragma once

#include <cuda_runtime.h>


namespace cas
{

enum class GPA_FLAG
{
GAUSSIAN,
BOX
};



///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_8u_C1_Ctx(const unsigned char* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned char* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_8u_C2_Ctx(const unsigned char* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned char* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_8u_C3_Ctx(const unsigned char* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned char* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_8u_C4_Ctx(const unsigned char* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned char* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16u_C1_Ctx(const unsigned short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16u_C2_Ctx(const unsigned short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16u_C3_Ctx(const unsigned short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16u_C4_Ctx(const unsigned short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, unsigned short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16s_C1_Ctx(const short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16s_C2_Ctx(const short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16s_C3_Ctx(const short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_16s_C4_Ctx(const short* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, short* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_32f_C1_Ctx(const float* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, float* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_32f_C2_Ctx(const float* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, float* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_32f_C3_Ctx(const float* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, float* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);

///
/// \brief gaussianApproximationBilateralFilter_8u_C1_Ctx
/// \param const psrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param rows: number of rows.
/// \param cols: number of columns.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag: Gaussian or Box filter to use in order to blur the data.
/// \param const pdst: pointer on the adress were to write the data.
/// \param nDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream: stream
///
void gaussianApproximationBilateralFilter_32f_C4_Ctx(const float* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, float* const pdst, const int& nDstStep, const double& eps = 1e-3, cudaStream_t stream = nullptr);



} // cuda



#endif // CUDA_GPABF_H
