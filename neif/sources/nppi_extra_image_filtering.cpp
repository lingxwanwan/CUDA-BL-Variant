#include "nppi_extra_image_filtering.hpp"
#include "gpa.internal.hpp"

namespace 
{


template<int cn, bool has_alpha, class T>
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_(const T* pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, T* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
    using image_type = Buffer2D_t<Npp32f, cn>;
    using buffer_type = Buffer_t<Npp32f>;

    NppiSize oFlatSizeRoi = {oSizeROI.width * cn, oSizeROI.height};

    Npp32s N = getN<T>(sigma_range, epsilon);

    Npp32f midRange = getT<T>();

    image_type H, F, FBar, tmp, G, P, Q;

    NppiMaskSize GMasksize;

    Npp32s nFilterTap = 0;

    NppiPoint offset = {0,0};
    NppiSize BMaskSize;
    NppiPoint BAnchor;

    buffer_type Hs;

    bool is_gaussian = flag == GPA_FLAG::GAUSSIAN;


    //////// PREPARE FILTERING //////////////

    if(is_gaussian)
    {
        if constexpr (std::is_same<T, Npp8u>())
        {
            nFilterTap = static_cast<int>(std::ceil(4.f * sigma_spatial_or_box_width + 0.5f) );
        }
        else
        {
            nFilterTap = static_cast<int>(std::ceil(3.f * sigma_spatial_or_box_width + 0.5f) );
        }

        if(nFilterTap <= 15)
        {
            switch(nFilterTap)
            {

            case 3:
                GMasksize = NPP_MASK_SIZE_3_X_3;
                break;

            case 5:
                GMasksize = NPP_MASK_SIZE_5_X_5;
                break;

            case 7:
                GMasksize = NPP_MASK_SIZE_7_X_7;
                break;

            case 9:
                GMasksize = NPP_MASK_SIZE_9_X_9;
                break;

            case 11:
                GMasksize = NPP_MASK_SIZE_11_X_11;
                break;

            case 13:
                GMasksize = NPP_MASK_SIZE_13_X_13;
                break;

            case 15:
                GMasksize = NPP_MASK_SIZE_15_X_15;
                break;

            }

        }
        else
        {
            Hs.resize(nFilterTap);

            Npp32f* pHs = Hs;

            generate_gaussian_kernel(pHs, sigma_spatial_or_box_width, nFilterTap);

        }

    }
    else
    {
        nFilterTap = static_cast<int>(std::ceil(2.f * sigma_spatial_or_box_width) + 1.f);

        BMaskSize = {nFilterTap, nFilterTap};
        BAnchor = {nFilterTap>>1, nFilterTap>>1};

    }

    H.create(oSizeROI);
    F.create(oSizeROI);
    FBar.create(oSizeROI);
    tmp.create(oSizeROI);

    if constexpr (!has_alpha)
    {
        if constexpr (std::is_same<T, Npp8u>())
        {
            CHECK_NPPI_RETURN(nppiConvert_8u32f_C1R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
        }
        else if constexpr (std::is_same<T, Npp16u>())
        {
            CHECK_NPPI_RETURN(nppiConvert_16u32f_C1R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
        }
        else if constexpr (std::is_same<T, Npp16s>())
        {
            CHECK_NPPI_RETURN(nppiConvert_16s32f_C1R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
        }
        else // Npp32f
        {
            CHECK_NPPI_RETURN(nppiCopy_32f_C1R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
        }

        CHECK_NPPI_RETURN(nppiSubC_32f_C1R_Ctx(tmp, tmp.step, midRange, H, H.step, oFlatSizeRoi, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiMulC_32f_C1R_Ctx(H, H.step, 1.f / static_cast<Npp32f>(sigma_range), H, H.step, oFlatSizeRoi, nppStreamCtx))



        CHECK_NPPI_RETURN(nppiSqr_32f_C1R_Ctx(H, H.step, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiMulC_32f_C1R_Ctx(tmp, tmp.step, -0.5, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiExp_32f_C1R_Ctx(tmp, tmp.step,F, F.step, oFlatSizeRoi, nppStreamCtx))


                //////// PREPARE FILTERING //////////////
        if constexpr (cn == 1)
        {
            CHECK_NPPI_RETURN(is_gaussian ?
                                  !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_C1R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterGaussBorder_32f_C1R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterBoxBorder_32f_C1R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))
        }
        else if constexpr (cn == 3)
        {
            CHECK_NPPI_RETURN(is_gaussian ?
                                  !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_C3R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterGaussBorder_32f_C3R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterBoxBorder_32f_C3R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))
        }
        else
        {
            CHECK_NPPI_RETURN(is_gaussian ?
                                  !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_C4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterGaussBorder_32f_C4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterBoxBorder_32f_C4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))
        }
        /////////////////

        P.create(oSizeROI);
        Q.create(oSizeROI);
        G.create(oSizeROI);


        CHECK_NPPI_RETURN(nppiSet_32f_C1R_Ctx(0.f, P, P.step, oFlatSizeRoi, nppStreamCtx));
        CHECK_NPPI_RETURN(nppiSet_32f_C1R_Ctx(0.f, Q, Q.step, oFlatSizeRoi, nppStreamCtx));
        CHECK_NPPI_RETURN(nppiSet_32f_C1R_Ctx(1.f, G, G.step, oFlatSizeRoi, nppStreamCtx));


        for(int i=0; i<N;i++)
        {
            double sqrt_n = std::sqrt(static_cast<double>(i) + 1.);
            double inv_sqrt_n = 1. / sqrt_n;

            //  Q += G.mul(Fbar);

            CHECK_NPPI_RETURN(nppiMul_32f_C1R_Ctx(G, G.step, FBar, FBar.step, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiAdd_32f_C1R_Ctx(Q, Q.step, tmp, tmp.step, Q, Q.step, oFlatSizeRoi, nppStreamCtx))

            //  F = H.mul(F, inv_sqrt_n);

            CHECK_NPPI_RETURN(nppiMulC_32f_C1R_Ctx(F, F.step, inv_sqrt_n, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiMul_32f_C1R_Ctx(H, H.step, tmp, tmp.step, F, F.step, oFlatSizeRoi, nppStreamCtx))

            // sepFilter2D(F, Fbar, CV_32F, Hs.t(), Hs);

            if constexpr (cn == 1)
            {
                CHECK_NPPI_RETURN(is_gaussian ?
                                      !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_C1R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                    nppiFilterGaussBorder_32f_C1R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                    nppiFilterBoxBorder_32f_C1R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))
            }
            else if constexpr (cn == 3)
            {
                CHECK_NPPI_RETURN(is_gaussian ?
                                      !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_C3R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                    nppiFilterGaussBorder_32f_C3R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                    nppiFilterBoxBorder_32f_C3R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))
            }
            else
            {
                CHECK_NPPI_RETURN(is_gaussian ?
                                      !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_C4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                    nppiFilterGaussBorder_32f_C4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                    nppiFilterBoxBorder_32f_C4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))
            }


            //  P += G.mul(Fbar, sqrt_n);

            CHECK_NPPI_RETURN(nppiMulC_32f_C1R_Ctx(FBar, FBar.step, sqrt_n, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiMul_32f_C1IR_Ctx(G, G.step, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiAdd_32f_C1R_Ctx(P, P.step, tmp, tmp.step, P, P.step, oFlatSizeRoi, nppStreamCtx))

            //  G += H.mul(G, inv_sqrt_n);

            CHECK_NPPI_RETURN(nppiMulC_32f_C1R_Ctx(G, G.step, inv_sqrt_n, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiMul_32f_C1IR_Ctx(H, H.step, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiAdd_32f_C1R_Ctx(G, G.step, tmp, tmp.step, G, G.step, oFlatSizeRoi, nppStreamCtx))


        }

        G.release();
        F.release();
        FBar.release();

        // g = 128. + sigma_range * (P / Q); Note: 128 if uint8, 32768 if uint16, ...

        CHECK_NPPI_RETURN(nppiDiv_32f_C1R_Ctx(Q, Q.step, P, P.step, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiMulC_32f_C1R_Ctx(tmp, tmp.step, static_cast<float>(sigma_range), tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))

        CHECK_NPPI_RETURN(nppiAddC_32f_C1IR_Ctx(midRange, tmp, tmp.step, oFlatSizeRoi, nppStreamCtx))


        // Clamp the values.

        if constexpr (std::is_same<T, Npp8u>())
        {
            if constexpr (cn == 1)
            {
                CHECK_NPPI_RETURN(nppiScale_32f8u_C1R_Ctx(tmp, tmp.step, pDst, nDstStep, oSizeROI, 0.f, 255.f, nppStreamCtx))
            }
            else if constexpr (cn == 3)
            {
                CHECK_NPPI_RETURN(nppiScale_32f8u_C3R_Ctx(tmp, tmp.step, pDst, nDstStep, oSizeROI, 0.f, 255.f, nppStreamCtx))
            }
            else
            {
                CHECK_NPPI_RETURN(nppiScale_32f8u_C4R_Ctx(tmp, tmp.step, pDst, nDstStep, oSizeROI, 0.f, 255.f, nppStreamCtx))
            }
        }
        else if constexpr (std::is_same<T, Npp16u>())
        {
            CHECK_NPPI_RETURN(nppiThreshold_LTValGTVal_32f_C1IR_Ctx(tmp, tmp.step, oFlatSizeRoi, 0.f, 0.f, 65535.f, 65535.f, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiConvert_32f16u_C1R_Ctx(tmp, tmp.step, pDst, nDstStep, oFlatSizeRoi, NPP_RND_NEAR, nppStreamCtx))
        }
        else if constexpr (std::is_same<T, Npp16s>())
        {
            CHECK_NPPI_RETURN(nppiThreshold_LTValGTVal_32f_C1IR_Ctx(tmp, tmp.step, oFlatSizeRoi, -32768.f, -32768.f, 32767.f, 32767.f, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiConvert_32f16s_C1R_Ctx(tmp, tmp.step, pDst, nDstStep, oFlatSizeRoi, NPP_RND_NEAR, nppStreamCtx))
        }
        else
        {
            CHECK_NPPI_RETURN(nppiThreshold_LTValGTVal_32f_C1IR_Ctx(tmp, tmp.step, oFlatSizeRoi, 0.f, 0.f, 1.f, 1.f, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiCopy_32f_C1R_Ctx(tmp, tmp.step, pDst, nDstStep, oFlatSizeRoi, nppStreamCtx))
        }

    }
    else
    {
#if 1
        using buffer_type = Buffer2D_t<T, 1>;

        // Allocate a buffer.
        // The allocation is like the channels were concatenate horizontaly.
        // Why is that? Because like this the step will be the same as
        // the one we would have allocating an image independently.
        buffer_type buffer_src({oSizeROI.width, oSizeROI.height * 4});

        T* pBuffer = buffer_src;
        Npp32s nBufferStep = buffer_src.step;
        Npp32s nBufferOffset = oSizeROI.height * nBufferStep;

        // The original image has interlieaved channels (i.e. Blue, Green, Red, Alpha, Blue, Green, Red, Alpha, ...).
        // The goal of the Step 1) is to deinterlieve the channels, i.e. write all the Blue consequently in a buffer,
        // then all the green and so on. I am setting each one of the 4 buffers consequenly one after the other like if
        // the channels were concatenate horizontaly.
        T* const pChannelsSrc[4] = {pBuffer, pBuffer + nBufferOffset, pBuffer + 2 * nBufferOffset, pBuffer + 3 * nBufferOffset};


        // Here each channels will be deinterleved and copied into the buffer.
        if constexpr (std::is_same<T, Npp8u>())
        {
            // Step 1) de-interleave.

            CHECK_NPPI_RETURN(nppiCopy_8u_C4P4R_Ctx(pSrc, nSrcStep, pChannelsSrc, buffer_src.step, oSizeROI, nppStreamCtx))

            // Step 2) Process each channel independently.

            // One can note that the source and destination pointer are the same.
            // It is not a mistake. Also the alpha channel is not process... which makes sense.
            for(int i=0; i<3;i++)
                CHECK_NPPI_RETURN(nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R_Ctx(pChannelsSrc[i], nBufferStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pChannelsSrc[i], nBufferStep, oSizeROI, nppStreamCtx))

            // Step 3) re-interleaved
            CHECK_NPPI_RETURN(nppiCopy_8u_P4C4R_Ctx(pChannelsSrc, nBufferStep, pDst, nDstStep, oSizeROI, nppStreamCtx));
        }
        else if constexpr (std::is_same<T, Npp16u>())
        {
            // Step 1) de-interleave.

            CHECK_NPPI_RETURN(nppiCopy_16u_C4P4R_Ctx(pSrc, nSrcStep, pChannelsSrc, buffer_src.step, oSizeROI, nppStreamCtx))

            // Step 2) Process each channel independently.

            // One can note that the source and destination pointer are the same.
            // It is not a mistake. Also the alpha channel is not process... which makes sense.
            for(int i=0; i<3;i++)
                CHECK_NPPI_RETURN(nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R_Ctx(pChannelsSrc[i], nBufferStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pChannelsSrc[i], nBufferStep, oSizeROI, nppStreamCtx))

            // Step 3) re-interleaved
            CHECK_NPPI_RETURN(nppiCopy_16u_P4C4R_Ctx(pChannelsSrc, nBufferStep, pDst, nDstStep, oSizeROI, nppStreamCtx));
        }
        else if constexpr (std::is_same<T, Npp16s>())
        {
            // Step 1) de-interleave.

            CHECK_NPPI_RETURN(nppiCopy_16s_C4P4R_Ctx(pSrc, nSrcStep, pChannelsSrc, buffer_src.step, oSizeROI, nppStreamCtx))

            // Step 2) Process each channel independently.

            // One can note that the source and destination pointer are the same.
            // It is not a mistake. Also the alpha channel is not process... which makes sense.
            for(int i=0; i<3;i++)
                CHECK_NPPI_RETURN(nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R_Ctx(pChannelsSrc[i], nBufferStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pChannelsSrc[i], nBufferStep, oSizeROI, nppStreamCtx))

            // Step 3) re-interleaved
            CHECK_NPPI_RETURN(nppiCopy_16s_P4C4R_Ctx(pChannelsSrc, nBufferStep, pDst, nDstStep, oSizeROI, nppStreamCtx));
        }
        else
        {
            // Step 1) de-interleave.

            CHECK_NPPI_RETURN(nppiCopy_32f_C4P4R_Ctx(pSrc, nSrcStep, pChannelsSrc, buffer_src.step, oSizeROI, nppStreamCtx))

            // Step 2) Process each channel independently.

            // One can note that the source and destination pointer are the same.
            // It is not a mistake. Also the alpha channel is not process... which makes sense.
            for(int i=0; i<3;i++)
                CHECK_NPPI_RETURN(nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R_Ctx(pChannelsSrc[i], nBufferStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pChannelsSrc[i], nBufferStep, oSizeROI, nppStreamCtx))

            // Step 3) re-interleaved
            CHECK_NPPI_RETURN(nppiCopy_32f_P4C4R_Ctx(pChannelsSrc, nBufferStep, pDst, nDstStep, oSizeROI, nppStreamCtx));
        }
#else
        Npp32f scaleBuf[8];

        Npp32f* pScaleBuf1 = scaleBuf;
        Npp32f* pScaleBuf2 = pScaleBuf1 + 4;

        Npp32s nScaleSize = 3;

        // Create a lmabda because this function does not exists.
        auto nppiExp_32f_AC4R_Ctx = [](const Npp32f * pSrc, int nSrcStep, Npp32f * pDst,  int nDstStep,  NppiSize oSizeROI, NppStreamContext nppStreamCtx) -> NppStatus
        {
            using buffer_type_planar = Buffer2D_t<Npp32f, 1>;

            buffer_type_planar buffer({oSizeROI.width, oSizeROI.height * 4});

            Npp32f* pBufferPlanar = buffer;
            Npp32s nBufferStep = buffer.step;
            Npp32s nBufferOffset = oSizeROI.height * nBufferStep;

            // In this memory representation each planar channels is one under the other.
            Npp32f* const pChannels[4] = {pBufferPlanar, pBufferPlanar + nBufferOffset, pBufferPlanar + 2 * nBufferStep, pBufferPlanar + 3 * nBufferStep};

            // De-interleave the channels into 4 planar channels.
            CHECK_NPPI_RETURN(nppiCopy_32f_C4P4R_Ctx(pSrc, nSrcStep, pChannels, nBufferStep, oSizeROI, nppStreamCtx))

            // Apply the exponential elementwisely on the 3 first channels.
            CHECK_NPPI_RETURN(nppiExp_32f_C1IR_Ctx(pBufferPlanar, nBufferStep, {oSizeROI.width, oSizeROI.height * 3}, nppStreamCtx))

            // Re-create a 4 channels interleaves image.
            CHECK_NPPI_RETURN(nppiCopy_32f_P4C4R_Ctx(pChannels, nBufferStep, pDst, nDstStep, oSizeROI, nppStreamCtx))

            // NO ERROR :)
            return NPP_SUCCESS;
        };

        // Set the coefficients.
        std::fill_n(pScaleBuf1, nScaleSize, midRange);
        std::fill_n(pScaleBuf2, nScaleSize, 1.f / sigma_range);

        if constexpr (std::is_same<T, Npp8u>())
        {
            CHECK_NPPI_RETURN(nppiConvert_8u32f_AC4R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oSizeROI, nppStreamCtx))
        }
        else if constexpr (std::is_same<T, Npp16u>())
        {
            CHECK_NPPI_RETURN(nppiConvert_16u32f_AC4R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oSizeROI, nppStreamCtx))
        }
        else if constexpr (std::is_same<T, Npp16s>())
        {
            CHECK_NPPI_RETURN(nppiConvert_16s32f_AC4R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oSizeROI, nppStreamCtx))
        }
        else // Npp32f
        {
            CHECK_NPPI_RETURN(nppiCopy_32f_AC4R_Ctx(pSrc, nSrcStep, tmp, tmp.step, oSizeROI, nppStreamCtx))
        }

        CHECK_NPPI_RETURN(nppiSubC_32f_AC4R_Ctx(tmp, tmp.step, pScaleBuf1, H, H.step, oSizeROI, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiMulC_32f_AC4R_Ctx(H, H.step, pScaleBuf2, H, H.step, oSizeROI, nppStreamCtx))

        // Set the coefficients.
        std::fill_n(pScaleBuf1, nScaleSize, -0.5f);


        CHECK_NPPI_RETURN(nppiSqr_32f_AC4R_Ctx(H, H.step, tmp, tmp.step, oSizeROI, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiMulC_32f_AC4R_Ctx(tmp, tmp.step, pScaleBuf1, tmp, tmp.step, oSizeROI, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiExp_32f_AC4R_Ctx(tmp, tmp.step,F, F.step, oSizeROI, nppStreamCtx))


        //////// PREPARE FILTERING //////////////
        CHECK_NPPI_RETURN(is_gaussian ?
                              !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_AC4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                            nppiFilterGaussBorder_32f_AC4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                            nppiFilterBoxBorder_32f_AC4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))

        /////////////////

        P.create(oSizeROI);
        Q.create(oSizeROI);
        G.create(oSizeROI);

        // Set the coefficients.
        std::fill_n(pScaleBuf1, nScaleSize, 0.f);
        std::fill_n(pScaleBuf2, nScaleSize, 1.f);

        CHECK_NPPI_RETURN(nppiSet_32f_AC4R_Ctx(pScaleBuf1, P, P.step, oSizeROI, nppStreamCtx));
        CHECK_NPPI_RETURN(nppiSet_32f_AC4R_Ctx(pScaleBuf1, Q, Q.step, oSizeROI, nppStreamCtx));
        CHECK_NPPI_RETURN(nppiSet_32f_AC4R_Ctx(pScaleBuf2, G, G.step, oSizeROI, nppStreamCtx));


        for(int i=0; i<N;i++)
        {
            Npp32f sqrt_n = std::sqrt(static_cast<Npp32f>(i) + 1.f);
            Npp32f inv_sqrt_n = 1.f / sqrt_n;

            // Set the coefficients.
            std::fill_n(pScaleBuf1, nScaleSize, sqrt_n);
            std::fill_n(pScaleBuf2, nScaleSize, inv_sqrt_n);


            //  Q += G.mul(Fbar);

            CHECK_NPPI_RETURN(nppiMul_32f_AC4R_Ctx(G, G.step, FBar, FBar.step, tmp, tmp.step, oSizeROI, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiAdd_32f_AC4R_Ctx(Q, Q.step, tmp, tmp.step, Q, Q.step, oSizeROI, nppStreamCtx))

            //  F = H.mul(F, inv_sqrt_n);

            CHECK_NPPI_RETURN(nppiMulC_32f_AC4R_Ctx(F, F.step, pScaleBuf2, tmp, tmp.step, oSizeROI, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiMul_32f_AC4R_Ctx(H, H.step, tmp, tmp.step, F, F.step, oSizeROI, nppStreamCtx))

            // sepFilter2D(F, Fbar, CV_32F, Hs.t(), Hs);

            CHECK_NPPI_RETURN(is_gaussian ?
                                  !Hs.empty() ? nppiFilterGaussAdvancedBorder_32f_AC4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, nFilterTap, Hs, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterGaussBorder_32f_AC4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, GMasksize, NPP_BORDER_REPLICATE, nppStreamCtx) :
                                                nppiFilterBoxBorder_32f_AC4R_Ctx(F, F.step, oSizeROI, offset, FBar, FBar.step, oSizeROI, BMaskSize, BAnchor, NPP_BORDER_REPLICATE,  nppStreamCtx))



            //  P += G.mul(Fbar, sqrt_n);

            CHECK_NPPI_RETURN(nppiMulC_32f_AC4R_Ctx(FBar, FBar.step, pScaleBuf1, tmp, tmp.step, oSizeROI, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiMul_32f_AC4IR_Ctx(G, G.step, tmp, tmp.step, oSizeROI, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiAdd_32f_AC4R_Ctx(P, P.step, tmp, tmp.step, P, P.step, oSizeROI, nppStreamCtx))

            //  G += H.mul(G, inv_sqrt_n);

            CHECK_NPPI_RETURN(nppiMulC_32f_AC4R_Ctx(G, G.step, pScaleBuf2, tmp, tmp.step, oSizeROI, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiMul_32f_AC4IR_Ctx(H, H.step, tmp, tmp.step, oSizeROI, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiAdd_32f_AC4R_Ctx(G, G.step, tmp, tmp.step, G, G.step, oSizeROI, nppStreamCtx))

        }

        G.release();
        F.release();
        FBar.release();

        // g = 128. + sigma_range * (P / Q); Note: 128 if uint8, 32768 if uint16, ...

        // Set the coefficients.
        std::fill_n(pScaleBuf1, nScaleSize, sigma_range);
        std::fill_n(pScaleBuf2, nScaleSize, midRange);

        CHECK_NPPI_RETURN(nppiDiv_32f_AC4R_Ctx(Q, Q.step, P, P.step, tmp, tmp.step, oSizeROI, nppStreamCtx))
        CHECK_NPPI_RETURN(nppiMulC_32f_AC4R_Ctx(tmp, tmp.step, pScaleBuf1, tmp, tmp.step, oSizeROI, nppStreamCtx))

        CHECK_NPPI_RETURN(nppiAddC_32f_AC4IR_Ctx(pScaleBuf2, tmp, tmp.step, oSizeROI, nppStreamCtx))


        // Clamp the values.
        if constexpr (std::is_same<T, Npp8u>())
        {
            CHECK_NPPI_RETURN(nppiScale_32f8u_AC4R_Ctx(tmp, tmp.step, pDst, nDstStep, oSizeROI, 0.f, 255.f, nppStreamCtx))
        }
        else if constexpr (std::is_same<T, Npp16u>())
        {
            // Set the range.
            std::fill_n(pScaleBuf1, nScaleSize, 0.f);
            std::fill_n(pScaleBuf2, nScaleSize, 65535.f);

            CHECK_NPPI_RETURN(nppiThreshold_LTValGTVal_32f_AC4IR_Ctx(tmp, tmp.step, oSizeROI, pScaleBuf1, pScaleBuf1, pScaleBuf2, pScaleBuf2, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiConvert_32f16u_AC4R_Ctx(tmp, tmp.step, pDst, nDstStep, oSizeROI, NPP_RND_NEAR, nppStreamCtx))
        }
        else if constexpr (std::is_same<T, Npp16s>())
        {
            // Set the range.
            std::fill_n(pScaleBuf1, nScaleSize, -32768.f);
            std::fill_n(pScaleBuf2, nScaleSize, 32767.f);

            CHECK_NPPI_RETURN(nppiThreshold_LTValGTVal_32f_AC4IR_Ctx(tmp, tmp.step, oSizeROI, pScaleBuf1, pScaleBuf1, pScaleBuf2, pScaleBuf2, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiConvert_32f16s_AC4R_Ctx(tmp, tmp.step, pDst, nDstStep, oSizeROI, NPP_RND_NEAR, nppStreamCtx))
        }
        else
        {
            // Set the range.
            std::fill_n(pScaleBuf1, nScaleSize, 0.f);
            std::fill_n(pScaleBuf2, nScaleSize, 1.f);

            CHECK_NPPI_RETURN(nppiThreshold_LTValGTVal_32f_AC4IR_Ctx(tmp, tmp.step, oSizeROI, pScaleBuf1, pScaleBuf1, pScaleBuf2, pScaleBuf2, nppStreamCtx))
            CHECK_NPPI_RETURN(nppiCopy_32f_AC4R_Ctx(tmp, tmp.step, pDst, nDstStep, oSizeROI, nppStreamCtx))
        }

#endif
    }

    return NPP_SUCCESS;
}

} // anonymous
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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<1, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<3, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C4R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_C4R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_8u_C4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R_Ctx(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, true>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R(const Npp8u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp8u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<1, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<3, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C4R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_C4R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16u_C4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R_Ctx(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, true>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R(const Npp16u *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16u* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<1, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<3, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C4R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_C4R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16s_C4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R_Ctx(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, true>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R(const Npp16s *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp16s* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<1, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<3, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C4R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, false>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_C4R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_32f_C4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R_Ctx(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI, NppStreamContext nppStreamCtx)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_Ctx_<4, true>(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, nppStreamCtx);
}



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
NppStatus nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R(const Npp32f *pSrc, Npp32s nSrcStep, Npp32f sigma_range, Npp32f sigma_spatial_or_box_width, GPA_FLAG flag, Npp32f epsilon, Npp32f* pDst, Npp32s nDstStep, NppiSize oSizeROI)
{
return nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R_Ctx(pSrc, nSrcStep, sigma_range, sigma_spatial_or_box_width, flag, epsilon, pDst, nDstStep, oSizeROI, getDefaultStreamContext());
}




