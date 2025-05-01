
#include "cuda_gpabf.hpp"
#include "utils.hpp"
#include "types.cuh"

#include <stdexcept>
#include <type_traits>
#include <numeric>
#include <vector>
#include <algorithm>

#include <vector_types.h>




namespace cas
{

namespace
{

///
/// \brief getT
/// \return the minimum number that can be represented for the given type.
///
template<class Type>
__host__ constexpr float getT()
{
    using lane_type = typename vectorTraits<Type>::lane_type;

    return std::is_signed<lane_type>() ? -static_cast<float>(std::numeric_limits<lane_type>::min()) : std::fmaf(static_cast<float>(std::numeric_limits<lane_type>::max()), 0.5f, 0.5f);
}


///
/// \brief getN : return the number of iteration to do
///  based on the precision set.
/// \param sigma_range : standard deviation on the range
/// \param epsilon : accuracy to reach
/// \return Number of iteration to do in order to reach the accuracy.
///
template<class Type>
__host__ int getN(float sigma_range, float epsilon)
{
    float lam = getT<Type>() / sigma_range;

    lam *= lam;

    float p = std::log(std::exp(1) * lam);
    float q = -lam - std::log(epsilon);
    float t = q * std::exp(-1) / lam;

    float t_sq = t*t;

    // t + 1.5 * t^3 - (8/3) t^4 - t^2
    float W = t + std::fma(t_sq, 1.5f * t, std::fma((-8.f/3.f) * t_sq, t_sq, -t_sq));

    float N = std::min(std::max(q / W, 10.f), 300.f);

    if(sigma_range < 30.f)
    {
        for(int i=0; i<5; i++)
        {
            float lN = std::log(N);

            N -= std::fma(N, lN, - std::fma(p, N, -q)) / (lN + 1.f - p);
        }
    }

    if(std::isnan(N) || std::isinf(N))
        return -1;
    return std::ceil(N);
}

template<class Type>
__host__ inline int EstN(float sigma_range, float epsilon)
{
    return sigma_range < 5.f ? 800 : sigma_range > 70 ? 10 : getN<Type>(sigma_range, epsilon);
}

__host__ inline int getKernelSize(const double& sigma)
{
    return static_cast<int>(std::ceil(4.f * sigma + 0.5f));
}


// Constant memory used but all the kernels.
__constant__ int d_rows;
__constant__ int d_cols;

// Constant memory variables used by the calculation
// and updates kernels.
__constant__ float d_minus_half = -0.5f;
__constant__ float d_sigma_range;
__constant__ float d_inv_sigma_range;
__constant__ float d_mid_range;
__constant__ float d_neg_norm_mid_range;
__constant__ float d_sqrt_n;
__constant__ float d_inv_sqrt_n;

// Constant memory variables used by the filtering operations.
// Note: 65 is an arbitrary number, which represents the maximum
// kernel size to be stored in constant memory. Filters with
// a kernel size higher than 65 will be placed in global memory.
__constant__ int d_kernel_size_h;
__constant__ int d_kernel_size_v;
__constant__ int d_half_kernel_size_h;
__constant__ int d_half_kernel_size_v;
__constant__ float d_kernel_horz[65];
__constant__ float d_kernel_vert[65];
__constant__ int d_pointer_offsets_horz[65];
__constant__ int d_pointer_offsets_vert[65];







///////////////////////////////////////////
/// DEVICE FUNCTIONS
///////////////////////////////////////////

// H = (src - mid_range) / sigma -> (src x (1 / sigma)) - (mid_range / sigma)
// F = exp(-0.5 x H x H)
template<class SrcType, class WrkType>
__device__ __forceinline__ void _compute_H_and_F(const SrcType& src, WrkType& H, WrkType& F)
{
    if constexpr (std::is_fundamental<SrcType>())
    {
        H = __fmaf_rn(static_cast<float>(src), d_inv_sigma_range, d_neg_norm_mid_range);
        F = __expf(__fmul_rn(d_minus_half, __fmul_rn(H, H)));
    }
    else if constexpr (vectorTraits<SrcType>::channels == 1)
    {
        H.x = __fmaf_rn(static_cast<float>(src.x), d_inv_sigma_range, d_neg_norm_mid_range);
        F.x = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.x, H.x)));
    }
    else if constexpr (vectorTraits<SrcType>::channels == 2)
    {
        H.x = __fmaf_rn(static_cast<float>(src.x), d_inv_sigma_range, d_neg_norm_mid_range);
        H.y = __fmaf_rn(static_cast<float>(src.y), d_inv_sigma_range, d_neg_norm_mid_range);

        F.x = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.x, H.x)));
        F.y = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.y, H.y)));
    }
    else if constexpr (vectorTraits<SrcType>::channels == 3)
    {
        H.x = __fmaf_rn(static_cast<float>(src.x), d_inv_sigma_range, d_neg_norm_mid_range);
        H.y = __fmaf_rn(static_cast<float>(src.y), d_inv_sigma_range, d_neg_norm_mid_range);
        H.z = __fmaf_rn(static_cast<float>(src.z), d_inv_sigma_range, d_neg_norm_mid_range);

        F.x = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.x, H.x)));
        F.y = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.y, H.y)));
        F.z = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.z, H.z)));
    }
    else
    {
        H.x = __fmaf_rn(static_cast<float>(src.x), d_inv_sigma_range, d_neg_norm_mid_range);
        H.y = __fmaf_rn(static_cast<float>(src.y), d_inv_sigma_range, d_neg_norm_mid_range);
        H.z = __fmaf_rn(static_cast<float>(src.z), d_inv_sigma_range, d_neg_norm_mid_range);
        H.w = __fmaf_rn(static_cast<float>(src.w), d_inv_sigma_range, d_neg_norm_mid_range);

        F.x = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.x, H.x)));
        F.y = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.y, H.y)));
        F.z = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.z, H.z)));
        F.w = __expf(__fmul_rn(d_minus_half, __fmul_rn(H.w, H.w)));
    }
}

// Q += G x FBar
template<class Type>
__device__ __forceinline__ Type _update_Q(const Type& Q, const Type& G, const Type& FBar)
{
    Type dst;

    if constexpr (std::is_fundamental<Type>())
    {
        dst = __fmaf_rn(G, FBar, Q);
    }
    else if constexpr (vectorTraits<Type>::channels == 1)
    {
        dst.x = __fmaf_rn(G.x, FBar.x, Q.x);
    }
    else if constexpr (vectorTraits<Type>::channels == 2)
    {
        dst.x = __fmaf_rn(G.x, FBar.x, Q.x);
        dst.y = __fmaf_rn(G.y, FBar.y, Q.y);
    }
    else if constexpr (vectorTraits<Type>::channels == 3)
    {
        dst.x = __fmaf_rn(G.x, FBar.x, Q.x);
        dst.y = __fmaf_rn(G.y, FBar.y, Q.y);
        dst.z = __fmaf_rn(G.z, FBar.z, Q.z);
    }
    else
    {
        dst.x = __fmaf_rn(G.x, FBar.x, Q.x);
        dst.y = __fmaf_rn(G.y, FBar.y, Q.y);
        dst.z = __fmaf_rn(G.z, FBar.z, Q.z);
        dst.w = __fmaf_rn(G.w, FBar.w, Q.w);
    }

    return dst;
}


// F = H x F x inv_sqrt_n
template<class Type>
__device__ __forceinline__ Type _update_F(const Type& F, const Type& H)
{
    Type dst;

    if constexpr (std::is_fundamental<Type>())
    {
        dst = __fmul_rn(H, __fmul_rn(F, d_inv_sqrt_n));
    }
    else if constexpr( vectorTraits<Type>::channels == 1)
    {
        dst.x = __fmul_rn(H.x, __fmul_rn(F.x, d_inv_sqrt_n));
    }
    else if constexpr( vectorTraits<Type>::channels == 2)
    {
        dst.x = __fmul_rn(H.x, __fmul_rn(F.x, d_inv_sqrt_n));
        dst.y = __fmul_rn(H.y, __fmul_rn(F.y, d_inv_sqrt_n));
    }
    else if constexpr( vectorTraits<Type>::channels == 3)
    {
        dst.x = __fmul_rn(H.x, __fmul_rn(F.x, d_inv_sqrt_n));
        dst.y = __fmul_rn(H.y, __fmul_rn(F.y, d_inv_sqrt_n));
        dst.z = __fmul_rn(H.z, __fmul_rn(F.z, d_inv_sqrt_n));
    }
    else
    {
        dst.x = __fmul_rn(H.x, __fmul_rn(F.x, d_inv_sqrt_n));
        dst.y = __fmul_rn(H.y, __fmul_rn(F.y, d_inv_sqrt_n));
        dst.z = __fmul_rn(H.z, __fmul_rn(F.z, d_inv_sqrt_n));
        dst.w = __fmul_rn(H.w, __fmul_rn(F.w, d_inv_sqrt_n));
    }

    return dst;
}

// P += G x Fbar x sqrt_n
template<class Type>
__device__ __forceinline__ Type _update_P(const Type& P, const Type& G, const Type& Fbar)
{
    Type dst;

    if constexpr (std::is_fundamental<Type>())
    {
        dst = __fmaf_rn(G, __fmul_rn(Fbar, d_sqrt_n), P);
    }
    else if constexpr (vectorTraits<Type>::channels == 1)
    {
        dst.x = __fmaf_rn(G.x, __fmul_rn(Fbar.x, d_sqrt_n), P.x);
    }
    else if constexpr (vectorTraits<Type>::channels == 2)
    {
        dst.x = __fmaf_rn(G.x, __fmul_rn(Fbar.x, d_sqrt_n), P.x);
        dst.y = __fmaf_rn(G.y, __fmul_rn(Fbar.y, d_sqrt_n), P.y);
    }
    else if constexpr (vectorTraits<Type>::channels == 3)
    {
        dst.x = __fmaf_rn(G.x, __fmul_rn(Fbar.x, d_sqrt_n), P.x);
        dst.y = __fmaf_rn(G.y, __fmul_rn(Fbar.y, d_sqrt_n), P.y);
        dst.z = __fmaf_rn(G.z, __fmul_rn(Fbar.z, d_sqrt_n), P.z);
    }
    else
    {
        dst.x = __fmaf_rn(G.x, __fmul_rn(Fbar.x, d_sqrt_n), P.x);
        dst.y = __fmaf_rn(G.y, __fmul_rn(Fbar.y, d_sqrt_n), P.y);
        dst.z = __fmaf_rn(G.z, __fmul_rn(Fbar.z, d_sqrt_n), P.z);
        dst.w = __fmaf_rn(G.w, __fmul_rn(Fbar.w, d_sqrt_n), P.w);
    }

    return dst;
}

// G += H x G x inv_sqrt_n
template<class Type>
__device__ __forceinline__ Type _update_G(const Type& G, const Type& H)
{

    Type dst;

    if constexpr (std::is_fundamental<Type>())
    {
        dst = __fmaf_rn(H, __fmul_rn(G, d_inv_sqrt_n), G);
    }
    else if constexpr (vectorTraits<Type>::channels == 1)
    {
        dst.x = __fmaf_rn(H.x, __fmul_rn(G.x, d_inv_sqrt_n), G.x);
    }
    else if constexpr (vectorTraits<Type>::channels == 2)
    {
        dst.x = __fmaf_rn(H.x, __fmul_rn(G.x, d_inv_sqrt_n), G.x);
        dst.y = __fmaf_rn(H.y, __fmul_rn(G.y, d_inv_sqrt_n), G.y);
    }
    else if constexpr (vectorTraits<Type>::channels == 3)
    {
        dst.x = __fmaf_rn(H.x, __fmul_rn(G.x, d_inv_sqrt_n), G.x);
        dst.y = __fmaf_rn(H.y, __fmul_rn(G.y, d_inv_sqrt_n), G.y);
        dst.z = __fmaf_rn(H.z, __fmul_rn(G.z, d_inv_sqrt_n), G.z);
    }
    else
    {
        dst.x = __fmaf_rn(H.x, __fmul_rn(G.x, d_inv_sqrt_n), G.x);
        dst.y = __fmaf_rn(H.y, __fmul_rn(G.y, d_inv_sqrt_n), G.y);
        dst.z = __fmaf_rn(H.z, __fmul_rn(G.z, d_inv_sqrt_n), G.z);
        dst.w = __fmaf_rn(H.w, __fmul_rn(G.w, d_inv_sqrt_n), G.w);
    }

    return dst;
}

 //g = midRange + sigma_range x (P / Q);
template<class Type>
__device__ __forceinline__ Type _compute_g(const Type& P, const Type& Q)
{
    Type dst;

    if constexpr (std::is_fundamental<Type>())
    {
        dst = __fmaf_rn(__fdiv_rn(P, Q), d_sigma_range, d_mid_range);
    }
    else if constexpr(vectorTraits<Type>::channels == 1)
    {
        dst.x = __fmaf_rn(__fdiv_rn(P.x, Q.x), d_sigma_range, d_mid_range);
    }
    else if constexpr(vectorTraits<Type>::channels == 2)
    {
        dst.x = __fmaf_rn(__fdiv_rn(P.x, Q.x), d_sigma_range, d_mid_range);
        dst.y = __fmaf_rn(__fdiv_rn(P.y, Q.y), d_sigma_range, d_mid_range);
    }
    else if constexpr(vectorTraits<Type>::channels == 3)
    {
        dst.x = __fmaf_rn(__fdiv_rn(P.x, Q.x), d_sigma_range, d_mid_range);
        dst.y = __fmaf_rn(__fdiv_rn(P.y, Q.y), d_sigma_range, d_mid_range);
        dst.z = __fmaf_rn(__fdiv_rn(P.z, Q.z), d_sigma_range, d_mid_range);
    }
    else
    {
        dst.x = __fmaf_rn(__fdiv_rn(P.x, Q.x), d_sigma_range, d_mid_range);
        dst.y = __fmaf_rn(__fdiv_rn(P.y, Q.y), d_sigma_range, d_mid_range);
        dst.z = __fmaf_rn(__fdiv_rn(P.z, Q.z), d_sigma_range, d_mid_range);
        dst.w = __fmaf_rn(__fdiv_rn(P.w, Q.w), d_sigma_range, d_mid_range);
    }

    return dst;
}

// Return either 0.f or a vector type initialized on zero.
template<class Type>
__device__ __forceinline__ Type _set_to_zero()
{
    static_assert(std::is_same<typename vectorTraits<Type>::lane_type, float>(), "This function only accept single precision floating point data types.");

    if constexpr (std::is_fundamental<Type>())
    {
        return 0.f;
    }
    else if constexpr (vectorTraits<Type>::channels == 1)
    {
        return make_float1(0.f);
    }
    else if constexpr (vectorTraits<Type>::channels == 2)
    {
        return make_float2(0.f, 0.f);
    }
    else if constexpr (vectorTraits<Type>::channels == 3)
    {
        return make_float3(0.f, 0.f, 0.f);
    }
    else
    {
        return make_float4(0.f, 0.f, 0.f, 0.f);
    }
}

// sum += w * current.
template<class Type>
__device__ __forceinline__ void _update_weighted_sum(Type& sum, const Type& current, const float& weight)
{
    if constexpr (std::is_fundamental<Type>())
    {
        sum = __fmaf_rn(current, weight, sum);
    }
    else if constexpr (vectorTraits<Type>::channels == 1)
    {
        sum.x = __fmaf_rn(current.x, weight, sum.x);
    }
    else if constexpr (vectorTraits<Type>::channels == 2)
    {
        sum.x = __fmaf_rn(current.x, weight, sum.x);
        sum.y = __fmaf_rn(current.y, weight, sum.y);
    }
    else if constexpr (vectorTraits<Type>::channels == 3)
    {
        sum.x = __fmaf_rn(current.x, weight, sum.x);
        sum.y = __fmaf_rn(current.y, weight, sum.y);
        sum.z = __fmaf_rn(current.z, weight, sum.z);
    }
    else
    {
        sum.x = __fmaf_rn(current.x, weight, sum.x);
        sum.y = __fmaf_rn(current.y, weight, sum.y);
        sum.z = __fmaf_rn(current.z, weight, sum.z);
        sum.w = __fmaf_rn(current.w, weight, sum.w);
    }
}

// do a type casting, does not considere the saturation.
template<class DstType, class SrcType>
__device__ __forceinline__ DstType _cvt_to_(const SrcType& src)
{
    using destination_lane_type = typename vectorTraits<DstType>::lane_type;

    if constexpr (std::is_same<SrcType, DstType>())
    {
        return src;
    }
    else
    {
        DstType ret;

        if constexpr (std::is_fundamental<DstType>())
        {
            ret = static_cast<destination_lane_type>(src);
        }
        else if constexpr (vectorTraits<SrcType>::channels == 1)
        {
            ret.x = static_cast<destination_lane_type>(src.x);
        }
        else if constexpr (vectorTraits<SrcType>::channels == 2)
        {
            ret.x = static_cast<destination_lane_type>(src.x);
            ret.y = static_cast<destination_lane_type>(src.y);
        }
        else if constexpr (vectorTraits<SrcType>::channels == 3)
        {
            ret.x = static_cast<destination_lane_type>(src.x);
            ret.y = static_cast<destination_lane_type>(src.y);
            ret.z = static_cast<destination_lane_type>(src.z);
        }
        else
        {
            ret.x = static_cast<destination_lane_type>(src.x);
            ret.y = static_cast<destination_lane_type>(src.y);
            ret.z = static_cast<destination_lane_type>(src.z);
            ret.w = static_cast<destination_lane_type>(src.w);
        }


        return ret;
    }
}

///////////////////////////////////////////
/// GLOBAL FUNCTIONS (KERNELS)
///////////////////////////////////////////

// H = (src - mid_range) / sigma -> (src x (1 / sigma)) - (mid_range / sigma)
// F = exp(-0.5 x H x H)
template<class SrcType, class WrkType>
__global__ void k_compute_H_and_F(const PtrStep<SrcType> _src, PtrStep<WrkType> _H, PtrStep<WrkType> _F)
{
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    _compute_H_and_F(_src.at(y, x), _H.at(y, x), _F.at(y, x));

}

// G <- 1
// P <- Q <- 0
template<class Type>
__global__ void k_set_G_P_Q(PtrStep<Type> G, PtrStep<Type> P, PtrStep<Type> Q)
{
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    if constexpr (std::is_fundamental<Type>())
    {
        G.at(y, x) = 1.f;
        P.at(y, x) = Q.at(y, x) = 0.f;
    }
    else if constexpr (vectorTraits<Type>::channels == 1)
    {
        G.at(y, x) = make_float1(1.f);
        P.at(y, x) = Q.at(y, x) = make_float1(1.f);
    }
    else if constexpr (vectorTraits<Type>::channels == 2)
    {
        G.at(y, x) = make_float2(1.f, 1.f);
        P.at(y, x) = Q.at(y, x) = make_float2(0.f, 0.f);
    }
    else if constexpr (vectorTraits<Type>::channels == 3)
    {
        G.at(y, x) = make_float3(1.f, 1.f, 1.f);
        P.at(y, x) = Q.at(y, x) = make_float3(0.f, 0.f, 0.f);
    }
    else
    {
        G.at(y, x) = make_float4(0.f, 0.f, 0.f, 0.f);
        P.at(y, x) = Q.at(y, x) = make_float4(1.f, 1.f, 1.f, 1.f);
    }
}

// Q += G x FBar
template<class Type>
__global__ void k_update_Q(PtrStep<Type> Q, const PtrStep<Type> G, const PtrStep<Type> FBar)
{
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    Q.at(y, x) = _update_Q(Q.at(y,x), G.at(y,x), FBar.at(y, x));
}

// F = H x F x inv_sqrt_n
template<class Type>
__global__ void k_update_F(PtrStep<Type> F, const PtrStep<Type> H)
{
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    F.at(y, x) = _update_F(F.at(y, x), H.at(y,x));
}

// P += G x Fbar x sqrt_n
template<class Type>
__global__ void k_update_P(PtrStep<Type> P, const PtrStep<Type> G, const PtrStep<Type> FBar)
{
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    P.at(y, x) = _update_P(P.at(y,x), G.at(y,x), FBar.at(y, x));
}

// G += H x G x inv_sqrt_n
template<class Type>
__global__ void k_update_G(PtrStep<Type> G, const PtrStep<Type> H)
{
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    G.at(y, x) = _update_G(G.at(y,x), H.at(y,x));
}

//g = midRange + sigma_range x (P / Q);
template<class WrkType, class DstType>
__global__ void k_compute_g(PtrStep<DstType> g, const PtrStep<WrkType> P, const PtrStep<WrkType> Q)
{
    using destination_type = typename vectorTraits<DstType>::lane_type;

    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    WrkType tmp = _compute_g(P.at(y,x), Q.at(y,x));
    DstType gi;

    if constexpr (std::is_fundamental<DstType>())
    {
        gi = static_cast<destination_type>(tmp);
    }
    else if constexpr (vectorTraits<DstType>::channels == 1)
    {
        gi.x = static_cast<destination_type>(tmp.x);
    }
    else if constexpr (vectorTraits<DstType>::channels == 2)
    {
        gi.x = static_cast<destination_type>(tmp.x);
        gi.y = static_cast<destination_type>(tmp.y);
    }
    else if constexpr (vectorTraits<DstType>::channels == 3)
    {
        gi.x = static_cast<destination_type>(tmp.x);
        gi.y = static_cast<destination_type>(tmp.y);
        gi.z = static_cast<destination_type>(tmp.z);
    }
    else
    {
        gi.x = static_cast<destination_type>(tmp.x);
        gi.y = static_cast<destination_type>(tmp.y);
        gi.z = static_cast<destination_type>(tmp.z);
        gi.w = static_cast<destination_type>(tmp.w);
    }

    g.at(y, x) = gi;
}

// Cast types from a type to another without regards for
// saturation, and do an horizontal padding.
template<class SrcType, class WrkType>
__global__ void k_cvt(const PtrStep<SrcType> src, PtrStep<WrkType> dst)
{
    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    WrkType tmp = _cvt_to_<WrkType>(src.at(y, x) );

    dst.at(y, x + d_half_kernel_size_h) = tmp;

    if(x < d_half_kernel_size_h)
        dst.at(y, d_half_kernel_size_h - x - 1) = tmp;

    if(x >= (d_cols - d_half_kernel_size_h - 1))
    {
        int delta = d_cols - x;

        dst.at(y, d_half_kernel_size_h + d_cols + delta - 1) = tmp;
    }
}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the constant memory,
// and that output has a padding.
template<class Type>
__global__ void k_horizontal_filtering_with_symbols(const PtrStep<Type> tmp_horz, PtrStep<Type> tmp_vert)
{
    using working_type = std::conditional_t<std::is_fundamental<Type>::value, float, make_vector_type_t<float, vectorTraits<Type>::channels> >;
    using const_working_pointer = const working_type*;

    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;

    working_type tmp = _set_to_zero<working_type>();

    const_working_pointer current_address_src = tmp_horz.ptr(y, x + d_half_kernel_size_h);

    for(int i=0; i<d_kernel_size_h; ++i)
    {
        float weight = d_kernel_horz[i];
        working_type current = *(current_address_src + d_pointer_offsets_horz[i]);

        if constexpr(std::is_fundamental<Type>())
        {
            tmp = __fmaf_rn(current, weight, tmp);
        }
        else if constexpr(vectorTraits<Type>::channels == 1)
        {
            tmp.x = __fmaf_rn(current.x, weight, tmp.x);
        }
        else if constexpr(vectorTraits<Type>::channels == 2)
        {
            tmp.x = __fmaf_rn(current.x, weight, tmp.x);
            tmp.y = __fmaf_rn(current.y, weight, tmp.y);
        }
        else if constexpr(vectorTraits<Type>::channels == 3)
        {
            tmp.x = __fmaf_rn(current.x, weight, tmp.x);
            tmp.y = __fmaf_rn(current.y, weight, tmp.y);
            tmp.z = __fmaf_rn(current.z, weight, tmp.z);
        }
        else if constexpr(vectorTraits<Type>::channels == 4)
        {
            tmp.x = __fmaf_rn(current.x, weight, tmp.x);
            tmp.y = __fmaf_rn(current.y, weight, tmp.y);
            tmp.z = __fmaf_rn(current.z, weight, tmp.z);
            tmp.w = __fmaf_rn(current.w, weight, tmp.w);
        }
    }

    tmp_vert.at(y + d_half_kernel_size_v, x) = tmp;

    if(y < d_half_kernel_size_v)
        tmp_vert.at(d_half_kernel_size_v - y, x) = tmp;

    if(y >= d_rows - d_half_kernel_size_v - 1)
    {
        int delta = d_rows - y;

        tmp_vert.at(d_half_kernel_size_v + d_rows + delta - 1, x) = tmp;
    }
}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the global memory,
// and that output has a padding.
template<class Type>
__global__ void k_horizontal_filtering_without_symbols(const PtrStep<Type> tmp_horz, const float* __restrict__ kernel_horz,  const int* __restrict__ offset_horz, PtrStep<Type> tmp_vert)
{
    using working_type = std::conditional_t<std::is_fundamental<Type>::value, float, make_vector_type_t<float, vectorTraits<Type>::channels> >;
    using const_working_pointer = const working_type*;


    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;


    working_type tmp = _set_to_zero<working_type>();

    const_working_pointer current_address_src = tmp_horz.ptr(y, x + d_half_kernel_size_h);

    const int* __restrict__ it_offset = offset_horz;
    const float* __restrict__ it_weights = kernel_horz;

    for(int i=0; i<d_kernel_size_h; ++i, ++it_offset, ++it_weights)
    {
        float weight = *it_weights;
        working_type current = *(current_address_src + *it_offset);

        _update_weighted_sum(tmp, current, weight);
    }

    tmp_vert.at(y + d_half_kernel_size_v, x) = tmp;

    if(y < d_half_kernel_size_v)
        tmp_vert.at(d_half_kernel_size_v - y, x) = tmp;

    if(y >= d_rows - d_half_kernel_size_v - 1)
    {
        int delta = d_rows - y;

        tmp_vert.at(d_half_kernel_size_v + d_rows + delta - 1, x) = tmp;
    }
}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the constant memory.
template<class WrkType, class DstType>
__global__ void k_vertical_filtering_with_symbols(const PtrStep<WrkType> tmp_vert, PtrStep<DstType> dst)
{
    using working_type = WrkType;
    using const_working_pointer = const WrkType*;

    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;


    working_type tmp = _set_to_zero<working_type>();

    const_working_pointer current_address_src = tmp_vert.ptr(y + d_half_kernel_size_v, x);

    for(int i=0; i<d_kernel_size_v; ++i)
    {
        float weight = d_kernel_vert[i];
        working_type current = *reinterpret_cast<const_working_pointer>(reinterpret_cast<const unsigned char*>(current_address_src) + d_pointer_offsets_vert[i]);

        _update_weighted_sum(tmp, current, weight);
    }

    dst.at(y, x) = _cvt_to_<DstType>(tmp);

}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the global memory.
template<class WrkType, class DstType>
__global__ void k_vertical_filtering_without_symbols(const PtrStep<WrkType> tmp_vert, const float* __restrict__ kernel,  const int* __restrict__ offsets, PtrStep<DstType> dst)
{
    using working_type = WrkType;
    using const_working_pointer = const WrkType*;

    const int y = blockDim.y * blockIdx.y + threadIdx.y;
    const int x = blockDim.x * blockIdx.x + threadIdx.x;

    if(y >= d_rows || x >= d_cols)
        return;


    working_type tmp = _set_to_zero<working_type>();

    const_working_pointer current_address_src = tmp_vert.ptr(y + d_half_kernel_size_v, x);

    const float* __restrict__ it_kernel = kernel;
    const int* __restrict__ it_offsets = offsets;

    for(int i=0; i<d_kernel_size_v; ++i, ++it_kernel, ++it_offsets)
    {
        float weight = *it_kernel;

        working_type current = *reinterpret_cast<const_working_pointer>(reinterpret_cast<const unsigned char*>(current_address_src) + *it_offsets);

        _update_weighted_sum(tmp, current, weight);
    }

    dst.at(y, x) = _cvt_to_<DstType>(tmp);
}

///////////////////////////////////////////
/// HOST FUNCTIONS (CALLERS)
///////////////////////////////////////////

// H = (src - mid_range) / sigma -> (src x (1 / sigma)) - (mid_range / sigma)
// F = exp(-0.5 x H x H)
template<class SrcType, class WrkType>
__host__ void compute_H_and_F(dim3& grid, dim3& blocks, const PtrStep<SrcType> _src, PtrStep<WrkType> _H, PtrStep<WrkType> _F, safe_stream& stream)
{


    // Execute the kernel.
    check_error_cuda(cudaFuncSetCacheConfig(k_compute_H_and_F<SrcType, WrkType>, cudaFuncCachePreferL1));
    k_compute_H_and_F<SrcType, WrkType><<<grid, blocks, 0, stream>>>(_src, _H, _F);
    check_error_cuda(cudaGetLastError());
}

// G <- 1
// P <- Q <- 0
template<class Type>
__host__ void set_G_P_Q(dim3& grid, dim3& blocks, PtrStep<Type> G, PtrStep<Type> P, PtrStep<Type> Q, safe_stream& stream)
{
    // Execute the kernel.
    check_error_cuda(cudaFuncSetCacheConfig(k_set_G_P_Q<Type>, cudaFuncCachePreferL1));
    k_set_G_P_Q<Type><<<grid, blocks, 0, stream>>>(G, P, Q);
    check_error_cuda(cudaGetLastError());
}

// Q += G x FBar
template<class Type>
__host__ void update_Q(dim3& grid, dim3& blocks, PtrStep<Type> Q, const PtrStep<Type> G, const PtrStep<Type> FBar, safe_stream& stream)
{
    // Execute the kernel.
    check_error_cuda(cudaFuncSetCacheConfig(k_update_Q<Type>, cudaFuncCachePreferL1));
    k_update_Q<Type><<<grid, blocks, 0, stream>>>(Q, G, FBar);
    check_error_cuda(cudaGetLastError());
}

// F = H x F x inv_sqrt_n
template<class Type>
__host__ void update_F(dim3& grid, dim3& blocks, PtrStep<Type> F, const PtrStep<Type> H, safe_stream& stream)
{
    // Execute the kernel.
    check_error_cuda(cudaFuncSetCacheConfig(k_update_F<Type>, cudaFuncCachePreferL1));
    k_update_F<Type><<<grid, blocks, 0, stream>>>(F, H);
    check_error_cuda(cudaGetLastError());
}

// P += G x Fbar x sqrt_n
template<class Type>
__host__ void update_P(dim3& grid, dim3& blocks, PtrStep<Type> P, const PtrStep<Type> G, const PtrStep<Type> FBar, safe_stream& stream)
{
    // Execute the kernel.
    check_error_cuda(cudaFuncSetCacheConfig(k_update_P<Type>, cudaFuncCachePreferL1));
    k_update_P<Type><<<grid, blocks, 0, stream>>>(P, G, FBar);
    check_error_cuda(cudaGetLastError());
}

// G += H x G x inv_sqrt_n
template<class Type>
__host__ void update_G(dim3& grid, dim3& blocks, PtrStep<Type> G, const PtrStep<Type> H, safe_stream& stream)
{
    // Execute the kernel.
    check_error_cuda(cudaFuncSetCacheConfig(k_update_G<Type>, cudaFuncCachePreferL1));
    k_update_G<Type><<<grid, blocks, 0, stream>>>(G, H);
    check_error_cuda(cudaGetLastError());
}

//g = midRange + sigma_range x (P / Q);
template<class WrkType, class DstType>
__host__ void update_g(dim3& grid, dim3& blocks, PtrStep<DstType> g, const PtrStep<WrkType> P, const PtrStep<WrkType> Q, safe_stream& stream)
{
    // Execute the kernel.
    check_error_cuda(cudaFuncSetCacheConfig(k_compute_g<WrkType, DstType>, cudaFuncCachePreferL1));
    k_compute_g<WrkType, DstType><<<grid, blocks, 0, stream>>>(g, P, Q);
    check_error_cuda(cudaGetLastError());
}

// Cast types from a type to another without regards for
// saturation, and do an horizontal padding.
template<class SrcType, class WrkType>
__host__ __forceinline__ void apply_type_conversion(dim3& grid, dim3& blocks, const PtrStep<SrcType> src, PtrStep<WrkType> dst, safe_stream& stream)
{
    check_error_cuda(cudaFuncSetCacheConfig(k_cvt<SrcType, WrkType>, cudaFuncCachePreferL1));
    k_cvt<<<grid, blocks, 0, stream>>>(src, dst);
    check_error_cuda(cudaGetLastError());
}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the constant memory,
// and that output has a padding.
template<class Type>
__host__ __forceinline__ void apply_horizontal_filter_with_symbols(dim3& grid, dim3& blocks, const PtrStep<Type> horz, PtrStep<Type> vert, safe_stream& stream)
{
    check_error_cuda(cudaFuncSetCacheConfig(k_horizontal_filtering_with_symbols<Type>, cudaFuncCachePreferL1));
    k_horizontal_filtering_with_symbols<<<grid, blocks, 0, stream>>>(horz, vert);
    check_error_cuda(cudaGetLastError());
}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the global memory,
// and that output has a padding.
template<class Type>
__host__ __forceinline__ void apply_horizontal_filter_without_symbols(dim3& grid, dim3& blocks, const PtrStep<Type> horz, const float* __restrict__ kernel, const int* __restrict__ offsets, PtrStep<Type> vert, safe_stream& stream)
{
    check_error_cuda(cudaFuncSetCacheConfig(k_horizontal_filtering_without_symbols<Type>, cudaFuncCachePreferL1));
    k_horizontal_filtering_without_symbols<<<grid, blocks, 0, stream>>>(horz, kernel, offsets, vert);
    check_error_cuda(cudaGetLastError());
}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the constant memory.
template<class WrkType, class DstType>
__host__ __forceinline__ void apply_vertical_filter_with_symbols(dim3& grid, dim3& blocks, const PtrStep<WrkType> vert, PtrStep<DstType> dst, safe_stream& stream)
{
    check_error_cuda(cudaFuncSetCacheConfig(k_vertical_filtering_with_symbols<WrkType, DstType>, cudaFuncCachePreferL1));
    k_vertical_filtering_with_symbols<<<grid, blocks, 0, stream>>>(vert, dst);
    check_error_cuda(cudaGetLastError());
}

// Compute the horizontal filtering (weighted sum),
// assuming that the kernel is in the global memory.
template<class WrkType, class DstType>
__host__ __forceinline__ void apply_vertical_filter_without_symbols(dim3& grid, dim3& blocks, const PtrStep<WrkType> vert, const float* __restrict__ kernel, const int* __restrict__ offsets, PtrStep<DstType> dst, safe_stream& stream)
{
    check_error_cuda(cudaFuncSetCacheConfig(k_vertical_filtering_without_symbols<WrkType, DstType>, cudaFuncCachePreferL1));
    k_vertical_filtering_without_symbols<<<grid, blocks, 0, stream>>>(vert, kernel, offsets, dst);
    check_error_cuda(cudaGetLastError());
}


///////////////////////////



///
/// \brief The cudaFilter_ class
/// The role of this class is to be a separable filter,
/// i.e. it is to be a apply two 1D filter to a 2D image.
/// Why a class? Simply, in the Gaussian Polynomial Approximation
/// of the Bilateral Filter, some matrices need to be filter several
/// time with the same kernel. Using a class allow to initialize the
/// constant memory variable or their global counter-part depending
/// the size of the kernels, only once.
template<class Type>
class cudaFilter_
{
public:

    using source_value_type = Type;
    using const_source_pointer = const Type*;
    using source_lane_type = typename vectorTraits<Type>::lane_type;

    using working_type = std::conditional_t<std::is_fundamental<source_value_type>::value, float, make_vector_type_t<float, vectorTraits<Type>::channels> >;
    using working_lane_type = typename vectorTraits<working_type>::lane_type;
    using working_pointer = working_type*;
    using const_working_pointer = const working_type*;

    using PtrStepW = PtrStep<working_type>;
    using PtrStepS = PtrStep<Type>;

    ///
    /// \brief cudaFilter_::~cudaFilter_ : destructor
    /// Deallocate all the memory allocated (if allocated).
    ///
    __host__ ~cudaFilter_();

    ///
    /// \brief cudaFilter_::apply : apply the kernels on the source image, and write destination image.
    /// \param src : image to process.
    /// \param dst : image to fill with the results of the processing.
    /// \note It is safe to provide the same object for input and output.
    ///
    __host__ void apply(const PtrStepS src, PtrStepS dst);

    ///
    /// \brief create
    /// initialize the filter attributes.
    /// \param rows : number of rows of the matrix to process.
    /// \param cols : number of columns of the matrix to process.
    /// \param kernel_size_h : size of the horizontal kernel.
    /// \param kernel_size_v : size of the vertical kernel.
    /// \param kernel_horz : horizonal kernel.
    /// \param kernel_vert : vertical kernel.
    /// \param stream : stream to use.
    /// \return a unique pointer object of the current class.
    ///
    static __host__ __forceinline__ std::unique_ptr<cudaFilter_<Type> > create(const int& rows, const int& cols, const int& kernel_size_h, const int& kernel_size_v, const float* kernel_horz, const float* kernel_vert, safe_stream& stream)
    {
        return std::unique_ptr<cudaFilter_<Type> >( new cudaFilter_<Type>(rows, cols, kernel_size_h, kernel_size_v, kernel_horz, kernel_vert, stream) );
    }

private:

    ///
    /// \brief cudaFilter_::cudaFilter_ : default contructor
    /// initalize all the attributes to a default value.
    ///
    __host__ cudaFilter_();


    ///
    /// \brief cudaFilter_::cudaFilter_ : parametric constructor
    /// initialize the filter attributes.
    /// \param rows : number of rows of the matrix to process.
    /// \param cols : number of columns of the matrix to process.
    /// \param kernel_size_h : size of the horizontal kernel.
    /// \param kernel_size_v : size of the vertical kernel.
    /// \param kernel_horz : horizonal kernel.
    /// \param kernel_vert : vertical kernel.
    /// \param stream : stream to use.
    ///
    __host__ cudaFilter_(const int& rows,
                         const int& cols,
                         const int& kernel_size_h,
                         const int& kernel_size_v,
                         const float* kernel_horz,
                         const float* kernel_vert,
                         safe_stream& stream);



    ///
    /// \brief cudaFilter_::init : initialization method.
    /// initialize the filter attributes.
    /// \param rows : number of rows of the matrix to process.
    /// \param cols : number of columns of the matrix to process.
    /// \param kernel_size_h : size of the horizontal kernel.
    /// \param kernel_size_v : size of the vertical kernel.
    /// \param kernel_horz : horizonal kernel.
    /// \param kernel_vert : vertical kernel.
    /// \param _stream : stream to use.
    ///
    __host__ void init(const int& rows,
                       const int& cols,
                       const int& kernel_size_h,
                       const int& kernel_size_v,
                       const float* kernel_horz,
                       const float* kernel_vert,
                       safe_stream& _stream);

    dim3 blocks, grid;

    int* device_offsets_h;
    int* device_offsets_v;

    float* device_kernel_h;
    bool own_kernel_h;

    float* device_kernel_v;
    bool own_kernel_v;

    unsigned char* device_tmp_h;
    size_t step_tmp_h;

    unsigned char* device_tmp_v;
    size_t step_tmp_v;

    bool use_symbols_h;
    bool use_symbols_v;

    safe_stream* stream;
};

///
/// \brief cudaFilter_::cudaFilter_ : default contructor
/// initalize all the attributes to a default value.
///
template<class Type>
cudaFilter_<Type>::cudaFilter_():
    blocks(32, 8),
    grid(),
    device_offsets_h(nullptr),
    device_offsets_v(nullptr),
    device_kernel_h(nullptr),
    own_kernel_h(false),
    device_kernel_v(nullptr),
    own_kernel_v(false),
    device_tmp_h(nullptr),
    step_tmp_h(0),
    device_tmp_v(nullptr),
    step_tmp_v(0),
    use_symbols_h(false),
    use_symbols_v(false)
{}

///
/// \brief cudaFilter_::cudaFilter_ : parametric constructor
/// initialize the filter attributes.
/// \param rows : number of rows of the matrix to process.
/// \param cols : number of columns of the matrix to process.
/// \param kernel_size_h : size of the horizontal kernel.
/// \param kernel_size_v : size of the vertical kernel.
/// \param kernel_horz : horizonal kernel.
/// \param kernel_vert : vertical kernel.
/// \param stream : stream to use.
///
template<class Type>
cudaFilter_<Type>::cudaFilter_(const int& rows, const int& cols, const int& kernel_size_h, const int& kernel_size_v, const float* kernel_horz, const float* kernel_vert, safe_stream& stream):
    cudaFilter_()
{
    this->init(rows, cols, kernel_size_h, kernel_size_v, kernel_horz, kernel_vert, stream);
}

///
/// \brief cudaFilter_::~cudaFilter_ : destructor
/// Deallocate all the memory allocated (if allocated).
///
template<class Type>
cudaFilter_<Type>::~cudaFilter_()
{
    if(this->device_offsets_h)
        check_error_cuda(cudaFree(this->device_offsets_h));

    if(this->device_offsets_v)
        check_error_cuda(cudaFree(this->device_offsets_v));

    if(this->device_kernel_h && this->own_kernel_h)
        check_error_cuda(cudaFree(this->device_kernel_h));

    if(this->device_kernel_v && this->own_kernel_v)
        check_error_cuda(cudaFree(this->device_kernel_v));

    if(this->device_tmp_h)
        check_error_cuda(cudaFree(this->device_tmp_h));

    if(this->device_tmp_v)
        check_error_cuda(cudaFree(this->device_tmp_v));
}

///
/// \brief cudaFilter_::init : initialization method.
/// initialize the filter attributes.
/// \param rows : number of rows of the matrix to process.
/// \param cols : number of columns of the matrix to process.
/// \param kernel_size_h : size of the horizontal kernel.
/// \param kernel_size_v : size of the vertical kernel.
/// \param kernel_horz : horizonal kernel.
/// \param kernel_vert : vertical kernel.
/// \param _stream : stream to use.
///
template<class Type>
void cudaFilter_<Type>::init(const int& rows,
                                      const int& cols,
                                      const int& kernel_size_h,
                                      const int& kernel_size_v,
                                      const float* kernel_horz,
                                      const float* kernel_vert,
                                      safe_stream& _stream)
{
    this->stream = std::addressof(_stream);
    this->grid = dim3(div_up(cols, this->blocks.x), div_up(rows, this->blocks.y));

    int half_kernel_size_h = kernel_size_h >> 1;
    int half_kernel_size_v = kernel_size_v >> 1;

    // Step 1) Allocate the two temporary buffers.

    check_error_cuda(cudaMallocPitch(&this->device_tmp_h, &this->step_tmp_h, (cols + kernel_size_h - 1) * sizeof(working_type), rows));
    check_error_cuda(cudaMallocPitch(&this->device_tmp_v, &this->step_tmp_v, cols * sizeof(working_type), rows + kernel_size_v - 1));


    // Step 2) Update the constant memory hyper parameters

    check_error_cuda(cudaMemcpyToSymbol(d_rows, &rows, sizeof(int)));
    check_error_cuda(cudaMemcpyToSymbol(d_cols, &cols, sizeof(int)));
    check_error_cuda(cudaMemcpyToSymbol(d_kernel_size_h, &kernel_size_h, sizeof(int)));
    check_error_cuda(cudaMemcpyToSymbol(d_kernel_size_v, &kernel_size_v, sizeof(int)));
    check_error_cuda(cudaMemcpyToSymbol(d_half_kernel_size_h, &half_kernel_size_h, sizeof(int)));
    check_error_cuda(cudaMemcpyToSymbol(d_half_kernel_size_v, &half_kernel_size_v, sizeof(int)));

    // Step 3) Compute the pointer offsets for the kernel sizes.

    std::vector<int> host_offsets_horz;
    std::vector<int> host_offsets_vert;

    host_offsets_horz.reserve(kernel_size_h);
    host_offsets_vert.reserve(kernel_size_v);

    host_offsets_horz.resize(host_offsets_horz.capacity());

    std::iota(host_offsets_horz.begin(), host_offsets_horz.end(), -half_kernel_size_h);

    for(int i=-half_kernel_size_v; i<=half_kernel_size_v; ++i)
        host_offsets_vert.push_back(i * this->step_tmp_v);

    // Step 4) Upload the offsets into either constant of global memory.

    this->use_symbols_h = kernel_size_h < 65;
    this->use_symbols_v = kernel_size_v < 65;


    if(this->use_symbols_h && this->use_symbols_v)
    {
        check_error_cuda(cudaMemcpyToSymbol(d_pointer_offsets_horz, host_offsets_horz.data(), kernel_size_h * sizeof(int)));
        check_error_cuda(cudaMemcpyToSymbol(d_pointer_offsets_vert, host_offsets_vert.data(), kernel_size_v * sizeof(int)));
    }
    else if (this->use_symbols_h && !this->use_symbols_v)
    {
        check_error_cuda(cudaMemcpyToSymbol(d_pointer_offsets_horz, host_offsets_horz.data(), kernel_size_h * sizeof(int)));

        check_error_cuda(cudaMalloc(&this->device_offsets_v, host_offsets_vert.size() * sizeof(int)));
        check_error_cuda(cudaMemcpy(this->device_offsets_v, host_offsets_vert.data(), host_offsets_vert.size() * sizeof(int), cudaMemcpyHostToDevice));
    }
    else if (!this->use_symbols_h && this->use_symbols_v)
    {
        check_error_cuda(cudaMalloc(&this->device_offsets_h, host_offsets_horz.size() * sizeof(int)));
        check_error_cuda(cudaMemcpy(this->device_offsets_h, host_offsets_horz.data(), host_offsets_horz.size() * sizeof(int), cudaMemcpyHostToDevice));

        check_error_cuda(cudaMemcpyToSymbol(d_pointer_offsets_vert, host_offsets_vert.data(), kernel_size_v * sizeof(int)));
    }
    else
    {
        check_error_cuda(cudaMalloc(&this->device_offsets_h, host_offsets_horz.size() * sizeof(int)));
        check_error_cuda(cudaMemcpy(this->device_offsets_h, host_offsets_horz.data(), host_offsets_horz.size() * sizeof(int), cudaMemcpyHostToDevice));

        check_error_cuda(cudaMalloc(&this->device_offsets_v, host_offsets_vert.size() * sizeof(int)));
        check_error_cuda(cudaMemcpy(this->device_offsets_v, host_offsets_vert.data(), host_offsets_vert.size() * sizeof(int), cudaMemcpyHostToDevice));
    }

    // Step 5) Upload the kernels into either constant of global memory.

    if(!this->use_symbols_h)
    {
        if(!isDevicePointer(kernel_horz))
        {
            check_error_cuda(cudaMalloc(&this->device_kernel_h, kernel_size_h * sizeof(float)));
            check_error_cuda(cudaMemcpy(this->device_kernel_h, kernel_horz, kernel_size_h * sizeof(float), cudaMemcpyHostToDevice));
            this->own_kernel_h = true;
        }
        else
        {
            this->device_kernel_h = const_cast<float*>(kernel_horz);
            this->own_kernel_h = false;
        }
    }
    else
    {
        check_error_cuda(cudaMemcpyToSymbol(d_kernel_horz, kernel_horz, kernel_size_h * sizeof(float)));
    }


    if(!this->use_symbols_v)
    {
        if(!isDevicePointer(kernel_vert))
        {
            check_error_cuda(cudaMalloc(&this->device_kernel_v, kernel_size_v * sizeof(float)));
            check_error_cuda(cudaMemcpy(this->device_kernel_v, kernel_vert, kernel_size_v * sizeof(float), cudaMemcpyHostToDevice));
            this->own_kernel_v = true;
        }
        else
        {
            this->device_kernel_v = const_cast<float*>(kernel_vert);
            this->own_kernel_v = false;
        }
    }
    else
    {
        check_error_cuda(cudaMemcpyToSymbol(d_kernel_vert, kernel_vert, kernel_size_v * sizeof(float)));
    }
}

///
/// \brief cudaFilter_::apply : apply the kernels on the source image, and write destination image.
/// \param src : image to process.
/// \param dst : image to fill with the results of the processing.
/// \note It is safe to provide the same object for input and output.
///
template<class Type>
void cudaFilter_<Type>::apply(const PtrStepS src, PtrStepS dst)
{

    PtrStepW tmp_horz(reinterpret_cast<working_pointer>(this->device_tmp_h), this->step_tmp_h);
    PtrStepW tmp_vert(reinterpret_cast<working_pointer>(this->device_tmp_v), this->step_tmp_v);

    // Step 1) Pad Horizontaly
    apply_type_conversion(this->grid, this->blocks, src, tmp_horz, *this->stream);

    // Step 2) Horizontal Filtering
    if(this->use_symbols_h)
    {
        apply_horizontal_filter_with_symbols(this->grid, this->blocks, tmp_horz, tmp_vert, *this->stream);
    }
    else
    {
        apply_horizontal_filter_without_symbols(this->grid, this->blocks, tmp_horz, this->device_kernel_h, reinterpret_cast<int*>(this->device_offsets_h), tmp_vert, *this->stream);
    }
    check_error_cuda(cudaGetLastError());

    // Step 3) Vertical Filtering
    if(this->use_symbols_v)
    {
        apply_vertical_filter_with_symbols(this->grid, this->blocks, tmp_vert, dst, *this->stream);
    }
    else
    {
        apply_vertical_filter_without_symbols(this->grid, this->blocks, tmp_vert, this->device_kernel_v, reinterpret_cast<int*>(this->device_offsets_v) , dst, *this->stream);
    }
    check_error_cuda(cudaGetLastError());

}

///////////////////////////

///
/// \brief The ScopeBuffer2D_t class
/// This class is a buffer class,
/// which is designed be used
/// in the scope it is declared.
///
template<class T>
struct ScopeBuffer2D_t
{
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;

    ///
    /// \brief ScopeBuffer2D_t::ScopeBuffer2D_t : default constructor
    /// initializate the attributes of the class to a proper value.
    ///
    __host__ ScopeBuffer2D_t();

    ///
    /// \brief ScopeBuffer2D_t::ScopeBuffer2D_t : parametric construct
    /// Usefull to get create an object from a externaly allocated
    /// block of memroy. If the ownership is set to true, the current
    /// object wil deallocate the memory.
    /// \param _data : address of the external memory to consider.
    /// \param _step : number of byte to pass from a line to another.
    /// \param _rows : number of rows.
    /// \param _cols : number of columns.
    /// \param _own : ownership.
    ///
    __host__ ScopeBuffer2D_t(pointer _data, const size_t& _step, const int& _rows, const int& _cols, const bool& _own);

    ScopeBuffer2D_t(const ScopeBuffer2D_t&) = delete;
    ScopeBuffer2D_t(ScopeBuffer2D_t&&) = default;

    ///
    /// \brief ScopeBuffer2D_t::~ScopeBuffer2D_t : destructor
    /// Will deallocate the memory if memory was allocated,
    /// or if the ownership of a external pointer was given.
    /// Do not attempt to deallocate otherwise.
    ///
    __host__ ~ScopeBuffer2D_t();

    ScopeBuffer2D_t& operator=(const ScopeBuffer2D_t&) = delete;
    ScopeBuffer2D_t& operator=(ScopeBuffer2D_t&&) = default;

    ///
    /// \brief ScopeBuffer2D_t::release
    /// Will deallocate the memory if memory was allocated,
    /// or if the ownership of a external pointer was given.
    /// Do not attempt to deallocate otherwise.
    ///
    __host__ void release();

    ///
    /// \brief ScopeBuffer2D_t::create
    /// Allocate an amount of aligned memory at least
    /// represent a matrix of _rows rows and _cols columns.
    /// \param _rows : number of rows.
    /// \param _cols : number of columns.
    ///
    __host__ void create(const int &_rows, const int &_cols);

    ///
    /// \brief ScopeBuffer2D_t::copyFrom
    /// Copy data to the current object.
    /// An assumption is made that the provided
    /// pointer has the same size as the object one.
    /// \note The provided pointer can be on the
    /// host or on the device.
    /// \param psrc : address of the first element to copy.
    /// \param step : number of bytes to pass from a row to the next.
    ///
    __host__ void copyFrom(const_pointer psrc, const size_t& step);

    ///
    /// \brief ScopeBuffer2D_t::copyTo
    /// Copy to current object to the provided address.
    /// An assumption is made that the provided
    /// pointer has the same size as the object one.
    /// \note The provided pointer can be on the
    /// host or on the device.
    /// \param pdst : address of the first element to copy to.
    /// \param step : number of bytes to pass from a row to the next.
    ///
    __host__ void copyTo(pointer pdst, const size_t& step) const;

    ///
    /// \brief ScopeBuffer2D_t::ptr : accessor
    /// \param y : The index of the row is used to obtain the address of the first element.
    /// \return address of the first element of the row.
    ///
    __host__ pointer ptr(const int& y=0);

    ///
    /// \brief ScopeBuffer2D_t::ptr : accessor
    /// \param y : The index of the row is used to obtain the address of the first element.
    /// \return address of the first element of the row.
    ///
    __host__ const_pointer ptr(const int& y=0) const;

    ///
    /// \brief ScopeBuffer2D_t::ptr : accessor
    /// Returns a pointer to the element at the specified row and column.
    /// \param y The index of the row.
    /// \param x The index of the column.
    /// \return A pointer to the element at the specified position.
    ///
    __host__ pointer ptr(const int& y, const int& x);

    ///
    /// \brief ScopeBuffer2D_t::ptr : accessor
    /// Returns a pointer to the element at the specified row and column.
    /// \param y The index of the row.
    /// \param x The index of the column.
    /// \return A pointer to the element at the specified position.
    ///
    __host__ const_pointer ptr(const int& y, const int& x) const;

    ///
    /// \brief ScopeBuffer2D_t::operator PtrStep
    /// Implicit conversion operator to PtrStep.
    ///
    __host__ operator PtrStep<value_type>();

    unsigned char* data;
    std::size_t step;

    int rows, cols;
    bool own;

};

///
/// \brief ScopeBuffer2D_t::ScopeBuffer2D_t : default constructor
/// initializate the attributes of the class to a proper value.
///
template<class Type>
ScopeBuffer2D_t<Type>::ScopeBuffer2D_t():
    data(nullptr),
    step(0),
    rows(0),
    cols(0),
    own(false)
{}

///
/// \brief ScopeBuffer2D_t::ScopeBuffer2D_t : parametric construct
/// Usefull to get create an object from a externaly allocated
/// block of memroy. If the ownership is set to true, the current
/// object wil deallocate the memory.
/// \param _data : address of the external memory to consider.
/// \param _step : number of byte to pass from a line to another.
/// \param _rows : number of rows.
/// \param _cols : number of columns.
/// \param _own : ownership.
///
template<class Type>
ScopeBuffer2D_t<Type>::ScopeBuffer2D_t(pointer _data, const size_t& _step, const int& _rows, const int& _cols, const bool& _own):
    data(reinterpret_cast<unsigned char*>(_data)),
    step(_step),
    rows(_rows),
    cols(_cols),
    own(_own)
{}


///
/// \brief ScopeBuffer2D_t::~ScopeBuffer2D_t : destructor
/// Will deallocate the memory if memory was allocated,
/// or if the ownership of a external pointer was given.
/// Do not attempt to deallocate otherwise.
///
template<class Type>
ScopeBuffer2D_t<Type>::~ScopeBuffer2D_t()
{
    this->release();
}

///
/// \brief ScopeBuffer2D_t::release
/// Will deallocate the memory if memory was allocated,
/// or if the ownership of a external pointer was given.
/// Do not attempt to deallocate otherwise.
///
template<class Type>
void ScopeBuffer2D_t<Type>::release()
{
    if(this->data && this->own)
    {
        check_error_cuda(cudaFree(this->data));
        this->rows = this->cols = 0;
        this->step = 0;
        this->own = false;
        this->data = nullptr;
    }
}

///
/// \brief ScopeBuffer2D_t::create
/// Allocate an amount of aligned memory at least
/// represent a matrix of _rows rows and _cols columns.
/// \param _rows : number of rows.
/// \param _cols : number of columns.
///
template<class Type>
void ScopeBuffer2D_t<Type>::create(const int &_rows, const int &_cols)
{
    if((_rows != this->rows) || (_cols != this->cols))
    {
        this->rows = _rows;
        this->cols = _cols;
        this->own = true;
        check_error_cuda(cudaMallocPitch(std::addressof(this->data),std::addressof(this->step), this->cols * sizeof(value_type), this->rows));
    }
}

///
/// \brief ScopeBuffer2D_t::copyFrom
/// Copy data to the current object.
/// An assumption is made that the provided
/// pointer has the same size as the object one.
/// \note The provided pointer can be on the
/// host or on the device.
/// \param psrc : address of the first element to copy.
/// \param step : number of bytes to pass from a row to the next.
///
template<class Type>
void ScopeBuffer2D_t<Type>::copyFrom(const_pointer psrc, const size_t& step)
{
    if(this->data)
    {
        check_error_cuda(cudaMemcpy2D(this->data, this->step, psrc, step, this->cols * sizeof(value_type), this->rows, isDevicePointer(psrc) ? cudaMemcpyDeviceToDevice : cudaMemcpyHostToDevice));
    }
}

///
/// \brief ScopeBuffer2D_t::copyTo
/// Copy to current object to the provided address.
/// An assumption is made that the provided
/// pointer has the same size as the object one.
/// \note The provided pointer can be on the
/// host or on the device.
/// \param pdst : address of the first element to copy to.
/// \param step : number of bytes to pass from a row to the next.
///
template<class Type>
void ScopeBuffer2D_t<Type>::copyTo(pointer pdst, const size_t& step) const
{
    if(this->data)
    {
        check_error_cuda(cudaMemcpy2D(pdst, step, this->data, this->step, this->cols * sizeof(value_type), this->rows, isDevicePointer(pdst) ? cudaMemcpyDeviceToDevice : cudaMemcpyDeviceToHost));
    }
}

///
/// \brief ScopeBuffer2D_t::ptr : accessor
/// \param y : The index of the row is used to obtain the address of the first element.
/// \return address of the first element of the row.
///
template<class Type>
typename ScopeBuffer2D_t<Type>::pointer ScopeBuffer2D_t<Type>::ptr(const int& y)
{
    return reinterpret_cast<pointer>(this->data + y * this->step);
}

///
/// \brief ScopeBuffer2D_t::ptr : accessor
/// \param y : The index of the row is used to obtain the address of the first element.
/// \return address of the first element of the row.
///
template<class Type>
typename ScopeBuffer2D_t<Type>::const_pointer ScopeBuffer2D_t<Type>::ptr(const int& y) const
{
    return reinterpret_cast<const_pointer>(this->data + y * this->step);
}


///
/// \brief ScopeBuffer2D_t::ptr : accessor
/// Returns a pointer to the element at the specified row and column.
/// \param y The index of the row.
/// \param x The index of the column.
/// \return A pointer to the element at the specified position.
///
template<class Type>
typename ScopeBuffer2D_t<Type>::pointer ScopeBuffer2D_t<Type>::ptr(const int& y, const int& x)
{
    return this->ptr(y) + x;
}

///
/// \brief ScopeBuffer2D_t::ptr : accessor
/// Returns a pointer to the element at the specified row and column.
/// \param y The index of the row.
/// \param x The index of the column.
/// \return A pointer to the element at the specified position.
///
template<class Type>
typename ScopeBuffer2D_t<Type>::const_pointer ScopeBuffer2D_t<Type>::ptr(const int& y, const int& x) const
{
    return this->ptr(y) + x;
}

///
/// \brief ScopeBuffer2D_t::operator PtrStep
/// Implicit conversion operator to PtrStep.
///
template<class Type>
ScopeBuffer2D_t<Type>::operator PtrStep<value_type>()
{
    return PtrStep<value_type>(this->ptr(), this->step);
}


/// Modified from OpenCV. Used for its accuracy.
///
/// \brief get_gaussian_kernel_bit_exact
/// \param sigma : gaussian kernel standard deviation.
/// \param n : size of the kernel.
/// \param result : separable kernel values, on the host memory.
///
static void get_gaussian_kernel_bit_exact(const double sigma, const int& n, std::vector<double>& result)
{
    constexpr const static double sd_0_15 = 0.15;
    constexpr const static double sd_0_35 = 0.32;
    constexpr const static double sd_minus_0_125 = -0.125;

#ifdef FP_FAST_FMA
    double sigmaX = sigma > 0 ? double(sigma) : std::fma(double(n), sd_0_15, sd_0_35);
#else
    double sigmaX = sigma > 0 ? sigma : (static_cast<double>(n) * sd_0_15 + sd_0_35);
#endif
    double scale2X = sd_minus_0_125/(sigmaX*sigmaX);

    int n2_ = (n - 1) >> 1;

    std::vector<double> values;

    values.reserve(n2_ + 1);
    values.resize(values.capacity());

    double sum = 0.;
    for (int i = 0, x = 1 - n; i < n2_; ++i, x+=2)
    {
        double t = exp(static_cast<double>(x * x) * scale2X);
        values[i] = t;
        sum += t;
    }

#ifdef FP_FAST_FMA
    sum = std::fma(sum, 2., 1.);
#else
    sum = sum * 2. + 1.;
#endif

    if (!(n & 1) )
    {
        sum += 1.;
    }

    // normalize: sum(k[i]) = 1
    double mul1 = 1. / sum;

    result.reserve(n);
    result.resize(result.capacity());

    for (int i = 0, j = n - 1; i < n2_; ++i, --j )
    {
        double t = values[i] * mul1;
        result[i] = result[j] = t;
    }

    result[n2_] = 1. * mul1;

    if (!(n & 1))
        result[n2_ + 1] = result[n2_];

}



///
/// \brief generate_gaussian_kernel : generate a gaussian kernel
/// adjusted using the provided coefficients.
/// \param kernel_size : size of the kernel.
/// \param sigma : gaussian kernel standard deviation.
/// \param kernel : filter coefficients.
///
__host__ void generate_gaussian_kernel(const int& kernel_size, const double& sigma, std::vector<float>& kernel)
{
    std::vector<double> tmp;

    get_gaussian_kernel_bit_exact(sigma, kernel_size, tmp);

    kernel.reserve(tmp.size());
    kernel.resize(kernel.capacity());

    std::transform(tmp.begin(), tmp.end(), kernel.begin(), [](const double& a)->float{return static_cast<float>(a);});
}

///
/// \brief generate_box_kernel : generate a box filter, using the provided coefficient.
/// \param kernel_size : size of the kernel.
/// \param kernel : filter coefficients.
///
__host__ void generate_box_kernel(const int& kernel_size, std::vector<float>& kernel)
{
    kernel.reserve(kernel_size);
    kernel.resize(kernel.capacity());

    std::fill(kernel.begin(), kernel.end(), 1.f / static_cast<float>(kernel_size));
}


///
/// \brief cudaBilateralFilterGPA_ : template function to compute the Gaussian Polynomial Approximation Bilateral filter
/// \param psrc : address of the first element of the source memory.
/// \param nsrc : number of step to pass from a row to the next in the source memory.
/// \param rows : number of rows.
/// \param cols : number of columns
/// \param sigma_range: variance of the range kernel.
/// \param sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag : what operation to process apply on the image.
/// \param pdst : address of the first element of the destination memory.
/// \param ndst : number of step to pass from a row to the next in the destination memory.
/// \param eps: aka precision, maximum difference between the result of the current and the previous iteration.
/// \param stream : stream to use.
/// \note if no stream or the default stream are provided the function will use two streams.
///
template<class Type>
void cudaBilateralFilterGPA_(const Type* const psrc, const int& nsrc, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, Type* const pdst, const int& ndst, const double& eps,const safe_stream& stream)
{
    using source_type = Type;
    using destination_type = source_type;
    using work_type = std::conditional_t<std::is_fundamental<Type>::value, float, make_vector_type_t<float, vectorTraits<Type>::channels> >;

    using PtrStepS = PtrStep<source_type>;
    using PtrStepD = PtrStepS;
    using PtrStepW = PtrStep<work_type>;

    using ScopeBuffer_t = ScopeBuffer2D_t<work_type>;

    using source_lane_type = typename vectorTraits<source_type>::lane_type;
    using work_lane_type = typename vectorTraits<work_type>::lane_type;

    // Safe stream

    safe_stream stream1, stream2;

    safe_event event1, event2;

    bool do_synchronize = false;

    if(!stream)
    {
        stream1.create();
        stream2.create();

        event1.create();
        event2.create();

        do_synchronize = true;
    }


    // Prepare the grid.
    dim3 blocks(32,8);
    dim3 grid(div_up(cols, blocks.x), div_up(rows, blocks.y) );

    ScopeBuffer_t H_, F_,  G_, P_, Q_, Fbar_;

    PtrStepW H, F, G, P, Q, Fbar, g;
    PtrStepS src(const_cast<Type*>(psrc), nsrc), dst(pdst, ndst);

    int N = EstN<Type>(sigma_range, eps);
    float inv_sqrt_n, sqrt_n;

    float midRange;

    if constexpr (std::is_floating_point<source_lane_type>())
    {
        midRange = -0.5f;
    }
    else if constexpr (std::is_signed<source_lane_type>())
    {
        midRange = 0.f;
    }
    else
    {
        midRange = static_cast<float>((static_cast<int>(std::numeric_limits<source_lane_type>::max()) + 1) >> 1);
    }

    H_.create(rows, cols);
    F_.create(rows, cols);
    G_.create(rows, cols);
    P_.create(rows, cols);
    Q_.create(rows, cols);
    Fbar_.create(rows, cols);

    H = PtrStepW(H_.ptr(), H_.step);
    F = PtrStepW(F_.ptr(), F_.step);
    G = PtrStepW(G_.ptr(), G_.step);
    P = PtrStepW(P_.ptr(), P_.step);
    Q = PtrStepW(Q_.ptr(), Q_.step);
    Fbar = PtrStepW(Fbar_.ptr(), Fbar_.step);

    std::vector<float> Hs;

    int kernel_size(0);
//    int half_kernel_size(0);

    if(flag == GPA_FLAG::GAUSSIAN)
    {
        kernel_size = getKernelSize( sigma_spatial_or_box_width );

        generate_gaussian_kernel(kernel_size, sigma_spatial_or_box_width, Hs);
    }
    else
    {
        kernel_size =  2 * sigma_spatial_or_box_width + 1;

        generate_box_kernel(kernel_size, Hs);
    }


    // Compute H and F


    float inv_sigma_range = 1.f / sigma_range;
    float neg_norm_mid_range = - midRange / sigma_range;

    check_error_cuda(cudaMemcpyToSymbol(d_rows              , &rows              , sizeof(int  )));
    check_error_cuda(cudaMemcpyToSymbol(d_cols              , &cols              , sizeof(int  )));
    check_error_cuda(cudaMemcpyToSymbol(d_inv_sigma_range   , &inv_sigma_range   , sizeof(float)));
    check_error_cuda(cudaMemcpyToSymbol(d_neg_norm_mid_range, &neg_norm_mid_range, sizeof(float)));
    check_error_cuda(cudaMemcpyToSymbol(d_sigma_range       , &sigma_range       , sizeof(float)));
    check_error_cuda(cudaMemcpyToSymbol(d_mid_range         , &midRange          , sizeof(float)));


    // H = (src - midRange) x (1. / sigma_range)
    // F = exp(-0.5 x (H / sigma_range)^2)
    compute_H_and_F(grid, blocks, src, H, F, stream1);


    // G <- 1.f
    // P <- 0.f
    // Q <- 0.f
    set_G_P_Q(grid, blocks, G, P, Q, stream2);


    // Replicate F borders.

    auto filter = cudaFilter_<work_type>::create(rows, cols, kernel_size, kernel_size, Hs.data(), Hs.data(), stream1);

    filter->apply(F, Fbar);


    for(int i=0; i<N;i++)
    {
        sqrt_n = std::sqrt(static_cast<float>(i) + 1.f);
        inv_sqrt_n = 1.f / sqrt_n;

        check_error_cuda(cudaMemcpyToSymbol(d_sqrt_n    , &sqrt_n    , sizeof(float) ));
        check_error_cuda(cudaMemcpyToSymbol(d_inv_sqrt_n, &inv_sqrt_n, sizeof(float) ));

        // In the following lines the multiplications are elementwise.

        if(do_synchronize)
        {
            // Q += G x Fbar
            update_Q(grid, blocks, Q, G, Fbar, stream1);
            event1.record(stream1);

            // F = H x F x inv_sqrt_n
            update_F(grid, blocks, F, H, stream2);
            event2.record(stream2);

            stream1.waitEvent(event1);
            stream1.waitEvent(event2);
        }
        else
        {
            // Q += G x Fbar
            update_Q(grid, blocks, Q, G, Fbar, stream1);

            // F = H x F x inv_sqrt_n
            update_F(grid, blocks, F, H, stream2);
        }


        // sepFilter2D(F, Fbar, CV_32F, Hs.t(), Hs);
        filter->apply(F, Fbar);

        // P += G x Fbar x sqrt_n
        update_P(grid, blocks, P, G, Fbar, stream1);

        // G += H x G x inv_sqrt_n
        update_G(grid, blocks, G, H, stream1);
    }

    //Type g = midRange + sigma_range x (P / Q);
    update_g(grid, blocks, dst, P, Q, stream1);
}

////
//#define SPEC_CUDA_BF_GPA(type)\
//    template void cudaBilateralFilterGPA_<type>(const type* const, const int&, const int&, const int&, const float&, const float&, const GPA_FLAG&, type* const, const int&, const double&, const safe_stream&);

//SPEC_CUDA_BF_GPA(uchar)
//SPEC_CUDA_BF_GPA(uchar1)
//SPEC_CUDA_BF_GPA(uchar2)
//SPEC_CUDA_BF_GPA(uchar3)
//SPEC_CUDA_BF_GPA(uchar4)

//SPEC_CUDA_BF_GPA(ushort)
//SPEC_CUDA_BF_GPA(ushort1)
//SPEC_CUDA_BF_GPA(ushort2)
//SPEC_CUDA_BF_GPA(ushort3)
//SPEC_CUDA_BF_GPA(ushort4)

//SPEC_CUDA_BF_GPA(short)
//SPEC_CUDA_BF_GPA(short1)
//SPEC_CUDA_BF_GPA(short2)
//SPEC_CUDA_BF_GPA(short3)
//SPEC_CUDA_BF_GPA(short4)

//SPEC_CUDA_BF_GPA(float)
//SPEC_CUDA_BF_GPA(float1)
//SPEC_CUDA_BF_GPA(float2)
//SPEC_CUDA_BF_GPA(float3)
//SPEC_CUDA_BF_GPA(float4)



template<class VectorType, class LaneType>
void gaussianApproximationBilateralFilter_Ctx(const LaneType* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, LaneType* const pdst, const int& nDstStep, const double& eps, const safe_stream& stream)
{
    // If both the source and the destination pointer are on the device, let run directly.
    if(isDevicePointer(psrc) && isDevicePointer(pdst))
    {
        //void (const Type* const psrc, const int& nsrc, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, Type* const pdst, const int& ndst, const double& eps, safe_stream& stream)
        cudaBilateralFilterGPA_(reinterpret_cast<const VectorType*>(psrc), nSrcStep, rows, cols, sigma_range, sigma_spatial_or_box_width, flag, reinterpret_cast<VectorType*>(pdst), nDstStep, eps, stream);
    }
    else
    {
        // If at lease one between the
        // source or destination pointer
        // is not on the device.

        ScopeBuffer2D_t<VectorType> tmp_src, tmp_dst;

        bool own_dst(false);

        if(isDevicePointer(psrc))
        {
            // If source is on the device let simply cast the pointer to the temporaty one.
            // The variable "own_src" is set to false which ensure that, not attempt of
            // memory deallocation will be made.
            tmp_src = std::move(ScopeBuffer2D_t(const_cast<VectorType*>(reinterpret_cast<const VectorType*>(psrc)), nSrcStep, rows, cols, false));
        }
        else
        {
            // Otherwise a temporary buffer on the device is allocated, and the data are copied
            // into it. The variable "own_src" is set to true, which ensure that the memory,
            // will be deallocated.
            tmp_src.create(rows, cols);
            tmp_src.copyFrom(reinterpret_cast<const VectorType*>(psrc), nSrcStep);
        }

        if(isDevicePointer(pdst))
        {
            tmp_dst = std::move(ScopeBuffer2D_t<VectorType>(reinterpret_cast<VectorType*>(pdst), nDstStep, rows, cols, false));
        }
        else
        {
            tmp_dst.create(rows, cols);
            own_dst = true;
        }

        cudaBilateralFilterGPA_(tmp_src.ptr(),
                                static_cast<int>(tmp_src.step),
                                rows,
                                cols,
                                sigma_range,
                                sigma_spatial_or_box_width,
                                flag,
                                tmp_dst.ptr(),
                                static_cast<int>(tmp_dst.step),
                                eps,
                                stream);

        // If the destination pointer was on the host the memory, a copy from device to host is required before ending.
        if(!own_dst)
        {
            tmp_dst.copyTo(reinterpret_cast<VectorType*>(pdst), nDstStep);
        }
    }

}

//#define SPEC_CALLING_GPA_FUN_(fname, vname)\
//    template void gaussianApproximationBilateralFilter_Ctx<vname, fname>(const fname* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, fname* const pdst, const int& nDstStep, const double& eps, const safe_stream& stream);

//#define SPEC_CALLING_GPA_FUN_CN_(fname, vname)\
//    SPEC_CALLING_GPA_FUN_(fname, fname) \
//    SPEC_CALLING_GPA_FUN_(fname, vname ## 1) \
//    SPEC_CALLING_GPA_FUN_(fname, vname ## 2) \
//    SPEC_CALLING_GPA_FUN_(fname, vname ## 3) \
//    SPEC_CALLING_GPA_FUN_(fname, vname ## 4)

//#define SPEC_CALLING_GPA_FUN\
//    SPEC_CALLING_GPA_FUN_CN_(unsigned char, uchar)\
//    SPEC_CALLING_GPA_FUN_CN_(unsigned short, ushort)\
//    SPEC_CALLING_GPA_FUN_CN_(short, short)\
//    SPEC_CALLING_GPA_FUN_CN_(float, float)



} // anonymous

#define IMPL_GPA_SPECS_(prefix, cn, ftype, vtype)\
    void gaussianApproximationBilateralFilter_ ## prefix ## _C ## cn ##_Ctx(const ftype* const psrc, const int& nSrcStep, const int& rows, const int& cols, const float& sigma_range, const float& sigma_spatial_or_box_width, const GPA_FLAG& flag, ftype* const pdst, const int& nDstStep, const double& eps, cudaStream_t stream)\
{\
    gaussianApproximationBilateralFilter_Ctx<vtype> (psrc, nSrcStep, rows, cols, sigma_range, sigma_spatial_or_box_width, flag, pdst, nDstStep, eps, stream);\
}

#define IMPL_GPA_SPECS_CN_(prefix, ftype, vtype)\
    IMPL_GPA_SPECS_(prefix, 1, ftype, ftype)\
    IMPL_GPA_SPECS_(prefix, 2, ftype, vtype ## 2)\
    IMPL_GPA_SPECS_(prefix, 3, ftype, vtype ## 3)\
    IMPL_GPA_SPECS_(prefix, 4, ftype, vtype ## 4)

#define IMP_GPA_SPECS\
    IMPL_GPA_SPECS_CN_(8u , unsigned char, uchar)\
    IMPL_GPA_SPECS_CN_(16u, unsigned short, ushort)\
    IMPL_GPA_SPECS_CN_(16s, short, short)\
    IMPL_GPA_SPECS_CN_(32f, float, float)

IMP_GPA_SPECS





} // cas
