#pragma once


#include <nppi.h>


enum class GPA_FLAG
{
GAUSSIAN,
BOX
};

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C4R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C4R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C4R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C4R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C4R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C4R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C4R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C4R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI);


///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R_Ctx
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \param nppStreamCtx: stream context
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx);

///
/// \brief NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R
/// \param pSrc: pointer on the data to process.
/// \param nSrcStep: number of byte to pass from a row to the next.
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param epsilon: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param pDst: pointer on the adress were to write the data.
/// \param pDstStep: number of bytes to pass from a row to the next, for the destination storage.
/// \param oSizeROI: Size of the source and destination images.
/// \param anchor: anchor point.
/// \return : the error code.
///
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI);


