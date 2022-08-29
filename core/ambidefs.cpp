
#include "config.h"

#include "ambidefs.h"

#include "alnumbers.h"


namespace {

using AmbiChannelFloatArray = std::array<float,MaxAmbiChannels>;

constexpr auto inv_sqrt2f = static_cast<float>(1.0/al::numbers::sqrt2);


constexpr std::array<std::array<float,4>,8> FirstOrderDecoder{{
    {{ 1.250000000e-01f,  1.250000000e-01f,  1.250000000e-01f,  1.250000000e-01f, }},
    {{ 1.250000000e-01f,  1.250000000e-01f,  1.250000000e-01f, -1.250000000e-01f, }},
    {{ 1.250000000e-01f, -1.250000000e-01f,  1.250000000e-01f,  1.250000000e-01f, }},
    {{ 1.250000000e-01f, -1.250000000e-01f,  1.250000000e-01f, -1.250000000e-01f, }},
    {{ 1.250000000e-01f,  1.250000000e-01f, -1.250000000e-01f,  1.250000000e-01f, }},
    {{ 1.250000000e-01f,  1.250000000e-01f, -1.250000000e-01f, -1.250000000e-01f, }},
    {{ 1.250000000e-01f, -1.250000000e-01f, -1.250000000e-01f,  1.250000000e-01f, }},
    {{ 1.250000000e-01f, -1.250000000e-01f, -1.250000000e-01f, -1.250000000e-01f, }},
}};
constexpr std::array<AmbiChannelFloatArray,8> FirstOrderEncoder{{
    CalcAmbiCoeffs( 0.57735026919f,  0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f,  0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f,  0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f,  0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f, -0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f, -0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f, -0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f, -0.57735026919f, -0.57735026919f),
}};
static_assert(FirstOrderDecoder.size() == FirstOrderEncoder.size(), "First-order mismatch");

/* This calculates a first-order "upsampler" matrix. It combines a first-order
 * decoder matrix with a max-order encoder matrix, creating a matrix that
 * behaves as if the B-Format input signal is first decoded to a speaker array
 * at first-order, then those speaker feeds are encoded to a higher-order
 * signal. While not perfect, this should accurately encode a lower-order
 * signal into a higher-order signal.
 */
auto CalcFirstOrderUp()
{
    std::array<AmbiChannelFloatArray,4> res{};

    for(size_t i{0};i < FirstOrderDecoder[0].size();++i)
    {
        for(size_t j{0};j < FirstOrderEncoder[0].size();++j)
        {
            double sum{0.0};
            for(size_t k{0};k < FirstOrderDecoder.size();++k)
                sum += double{FirstOrderDecoder[k][i]} * FirstOrderEncoder[k][j];
            res[i][j] = static_cast<float>(sum);
        }
    }

    return res;
}


constexpr std::array<std::array<float,4>,4> FirstOrder2DDecoder{{
    {{ 2.500000000e-01f,  2.041241452e-01f, 0.0f,  2.041241452e-01f, }},
    {{ 2.500000000e-01f,  2.041241452e-01f, 0.0f, -2.041241452e-01f, }},
    {{ 2.500000000e-01f, -2.041241452e-01f, 0.0f,  2.041241452e-01f, }},
    {{ 2.500000000e-01f, -2.041241452e-01f, 0.0f, -2.041241452e-01f, }},
}};
constexpr std::array<AmbiChannelFloatArray,4> FirstOrder2DEncoder{{
    CalcAmbiCoeffs( inv_sqrt2f, 0.0f,  inv_sqrt2f),
    CalcAmbiCoeffs( inv_sqrt2f, 0.0f, -inv_sqrt2f),
    CalcAmbiCoeffs(-inv_sqrt2f, 0.0f,  inv_sqrt2f),
    CalcAmbiCoeffs(-inv_sqrt2f, 0.0f, -inv_sqrt2f),
}};
static_assert(FirstOrder2DDecoder.size() == FirstOrder2DEncoder.size(), "First-order 2D mismatch");

/* This calculates a 2D first-order "upsampler" matrix. Same as the first-order
 * matrix, just using a more optimized speaker array for horizontal-only
 * content.
 */
auto CalcFirstOrder2DUp()
{
    std::array<AmbiChannelFloatArray,4> res{};

    for(size_t i{0};i < FirstOrder2DDecoder[0].size();++i)
    {
        for(size_t j{0};j < FirstOrder2DEncoder[0].size();++j)
        {
            double sum{0.0};
            for(size_t k{0};k < FirstOrder2DDecoder.size();++k)
                sum += double{FirstOrder2DDecoder[k][i]} * FirstOrder2DEncoder[k][j];
            res[i][j] = static_cast<float>(sum);
        }
    }

    return res;
}


constexpr std::array<std::array<float,9>,14> SecondOrderDecoder{{
    {{ 7.142857143e-02f,  0.000000000e+00f,  0.000000000e+00f,  1.237179148e-01f,  0.000000000e+00f,  0.000000000e+00f, -7.453559925e-02f,  0.000000000e+00f,  1.290994449e-01f, }},
    {{ 7.142857143e-02f,  0.000000000e+00f,  0.000000000e+00f, -1.237179148e-01f,  0.000000000e+00f,  0.000000000e+00f, -7.453559925e-02f,  0.000000000e+00f,  1.290994449e-01f, }},
    {{ 7.142857143e-02f,  1.237179148e-01f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f, -7.453559925e-02f,  0.000000000e+00f, -1.290994449e-01f, }},
    {{ 7.142857143e-02f, -1.237179148e-01f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f, -7.453559925e-02f,  0.000000000e+00f, -1.290994449e-01f, }},
    {{ 7.142857143e-02f,  0.000000000e+00f,  1.237179148e-01f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f,  1.490711985e-01f,  0.000000000e+00f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f,  0.000000000e+00f, -1.237179148e-01f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f,  1.490711985e-01f,  0.000000000e+00f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f,  7.142857143e-02f,  7.142857143e-02f,  7.142857143e-02f,  9.682458366e-02f,  9.682458366e-02f,  0.000000000e+00f,  9.682458366e-02f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f,  7.142857143e-02f,  7.142857143e-02f, -7.142857143e-02f, -9.682458366e-02f,  9.682458366e-02f,  0.000000000e+00f, -9.682458366e-02f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f, -7.142857143e-02f,  7.142857143e-02f,  7.142857143e-02f, -9.682458366e-02f, -9.682458366e-02f,  0.000000000e+00f,  9.682458366e-02f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f, -7.142857143e-02f,  7.142857143e-02f, -7.142857143e-02f,  9.682458366e-02f, -9.682458366e-02f,  0.000000000e+00f, -9.682458366e-02f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f,  7.142857143e-02f, -7.142857143e-02f,  7.142857143e-02f,  9.682458366e-02f, -9.682458366e-02f,  0.000000000e+00f, -9.682458366e-02f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f,  7.142857143e-02f, -7.142857143e-02f, -7.142857143e-02f, -9.682458366e-02f, -9.682458366e-02f,  0.000000000e+00f,  9.682458366e-02f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f, -7.142857143e-02f, -7.142857143e-02f,  7.142857143e-02f, -9.682458366e-02f,  9.682458366e-02f,  0.000000000e+00f, -9.682458366e-02f,  0.000000000e+00f, }},
    {{ 7.142857143e-02f, -7.142857143e-02f, -7.142857143e-02f, -7.142857143e-02f,  9.682458366e-02f,  9.682458366e-02f,  0.000000000e+00f,  9.682458366e-02f,  0.000000000e+00f, }},
}};
constexpr std::array<AmbiChannelFloatArray,14> SecondOrderEncoder{{
    CalcAmbiCoeffs( 0.00000000000f,  0.00000000000f,  1.00000000000f),
    CalcAmbiCoeffs( 0.00000000000f,  0.00000000000f, -1.00000000000f),
    CalcAmbiCoeffs( 1.00000000000f,  0.00000000000f,  0.00000000000f),
    CalcAmbiCoeffs(-1.00000000000f,  0.00000000000f,  0.00000000000f),
    CalcAmbiCoeffs( 0.00000000000f,  1.00000000000f,  0.00000000000f),
    CalcAmbiCoeffs( 0.00000000000f, -1.00000000000f,  0.00000000000f),
    CalcAmbiCoeffs( 0.57735026919f,  0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f,  0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f,  0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f,  0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f, -0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f, -0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f, -0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f, -0.57735026919f, -0.57735026919f),
}};
static_assert(SecondOrderDecoder.size() == SecondOrderEncoder.size(), "Second-order mismatch");

/* This calculates a second-order "upsampler" matrix. Same as the first-order
 * matrix, just using a slightly more dense speaker array suitable for second-
 * order content.
 */
auto CalcSecondOrderUp()
{
    std::array<AmbiChannelFloatArray,9> res{};

    for(size_t i{0};i < SecondOrderDecoder[0].size();++i)
    {
        for(size_t j{0};j < SecondOrderEncoder[0].size();++j)
        {
            double sum{0.0};
            for(size_t k{0};k < SecondOrderDecoder.size();++k)
                sum += double{SecondOrderDecoder[k][i]} * SecondOrderEncoder[k][j];
            res[i][j] = static_cast<float>(sum);
        }
    }

    return res;
}


constexpr std::array<std::array<float,9>,6> SecondOrder2DDecoder{{
    {{ 1.666666667e-01f, -9.622504486e-02f, 0.0f,  1.666666667e-01f, -1.490711985e-01f, 0.0f, 0.0f, 0.0f,  8.606629658e-02f, }},
    {{ 1.666666667e-01f, -1.924500897e-01f, 0.0f,  0.000000000e+00f,  0.000000000e+00f, 0.0f, 0.0f, 0.0f, -1.721325932e-01f, }},
    {{ 1.666666667e-01f, -9.622504486e-02f, 0.0f, -1.666666667e-01f,  1.490711985e-01f, 0.0f, 0.0f, 0.0f,  8.606629658e-02f, }},
    {{ 1.666666667e-01f,  9.622504486e-02f, 0.0f, -1.666666667e-01f, -1.490711985e-01f, 0.0f, 0.0f, 0.0f,  8.606629658e-02f, }},
    {{ 1.666666667e-01f,  1.924500897e-01f, 0.0f,  0.000000000e+00f,  0.000000000e+00f, 0.0f, 0.0f, 0.0f, -1.721325932e-01f, }},
    {{ 1.666666667e-01f,  9.622504486e-02f, 0.0f,  1.666666667e-01f,  1.490711985e-01f, 0.0f, 0.0f, 0.0f,  8.606629658e-02f, }},
}};
constexpr std::array<AmbiChannelFloatArray,6> SecondOrder2DEncoder{{
    CalcAmbiCoeffs(-0.50000000000f, 0.00000000000f,  0.86602540379f),
    CalcAmbiCoeffs(-1.00000000000f, 0.00000000000f,  0.00000000000f),
    CalcAmbiCoeffs(-0.50000000000f, 0.00000000000f, -0.86602540379f),
    CalcAmbiCoeffs( 0.50000000000f, 0.00000000000f, -0.86602540379f),
    CalcAmbiCoeffs( 1.00000000000f, 0.00000000000f,  0.00000000000f),
    CalcAmbiCoeffs( 0.50000000000f, 0.00000000000f,  0.86602540379f),
}};
static_assert(SecondOrder2DDecoder.size() == SecondOrder2DEncoder.size(),
    "Second-order 2D mismatch");

/* This calculates a 2D second-order "upsampler" matrix. Same as the second-
 * order matrix, just using a more optimized speaker array for horizontal-only
 * content.
 */
auto CalcSecondOrder2DUp()
{
    std::array<AmbiChannelFloatArray,9> res{};

    for(size_t i{0};i < SecondOrder2DDecoder[0].size();++i)
    {
        for(size_t j{0};j < SecondOrder2DEncoder[0].size();++j)
        {
            double sum{0.0};
            for(size_t k{0};k < SecondOrder2DDecoder.size();++k)
                sum += double{SecondOrder2DDecoder[k][i]} * SecondOrder2DEncoder[k][j];
            res[i][j] = static_cast<float>(sum);
        }
    }

    return res;
}


constexpr std::array<std::array<float,16>,20> ThirdOrderDecoder{{
    {{ 5.000000000e-02f,  3.090169944e-02f,  8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f,  6.454972244e-02f,  9.045084972e-02f,  0.000000000e+00f, -1.232790000e-02f, -1.256118221e-01f,  0.000000000e+00f,  1.126112056e-01f,  7.944389175e-02f,  0.000000000e+00f,  2.421151497e-02f,  0.000000000e+00f, }},
    {{ 5.000000000e-02f, -3.090169944e-02f,  8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f, -6.454972244e-02f,  9.045084972e-02f,  0.000000000e+00f, -1.232790000e-02f,  1.256118221e-01f,  0.000000000e+00f, -1.126112056e-01f,  7.944389175e-02f,  0.000000000e+00f,  2.421151497e-02f,  0.000000000e+00f, }},
    {{ 5.000000000e-02f,  3.090169944e-02f, -8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f, -6.454972244e-02f,  9.045084972e-02f,  0.000000000e+00f, -1.232790000e-02f, -1.256118221e-01f,  0.000000000e+00f,  1.126112056e-01f, -7.944389175e-02f,  0.000000000e+00f, -2.421151497e-02f,  0.000000000e+00f, }},
    {{ 5.000000000e-02f, -3.090169944e-02f, -8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f,  6.454972244e-02f,  9.045084972e-02f,  0.000000000e+00f, -1.232790000e-02f,  1.256118221e-01f,  0.000000000e+00f, -1.126112056e-01f, -7.944389175e-02f,  0.000000000e+00f, -2.421151497e-02f,  0.000000000e+00f, }},
    {{ 5.000000000e-02f,  8.090169944e-02f,  0.000000000e+00f,  3.090169944e-02f,  6.454972244e-02f,  0.000000000e+00f, -5.590169944e-02f,  0.000000000e+00f, -7.216878365e-02f, -7.763237543e-02f,  0.000000000e+00f, -2.950836627e-02f,  0.000000000e+00f, -1.497759251e-01f,  0.000000000e+00f, -7.763237543e-02f, }},
    {{ 5.000000000e-02f,  8.090169944e-02f,  0.000000000e+00f, -3.090169944e-02f, -6.454972244e-02f,  0.000000000e+00f, -5.590169944e-02f,  0.000000000e+00f, -7.216878365e-02f, -7.763237543e-02f,  0.000000000e+00f, -2.950836627e-02f,  0.000000000e+00f,  1.497759251e-01f,  0.000000000e+00f,  7.763237543e-02f, }},
    {{ 5.000000000e-02f, -8.090169944e-02f,  0.000000000e+00f,  3.090169944e-02f, -6.454972244e-02f,  0.000000000e+00f, -5.590169944e-02f,  0.000000000e+00f, -7.216878365e-02f,  7.763237543e-02f,  0.000000000e+00f,  2.950836627e-02f,  0.000000000e+00f, -1.497759251e-01f,  0.000000000e+00f, -7.763237543e-02f, }},
    {{ 5.000000000e-02f, -8.090169944e-02f,  0.000000000e+00f, -3.090169944e-02f,  6.454972244e-02f,  0.000000000e+00f, -5.590169944e-02f,  0.000000000e+00f, -7.216878365e-02f,  7.763237543e-02f,  0.000000000e+00f,  2.950836627e-02f,  0.000000000e+00f,  1.497759251e-01f,  0.000000000e+00f,  7.763237543e-02f, }},
    {{ 5.000000000e-02f,  0.000000000e+00f,  3.090169944e-02f,  8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f, -3.454915028e-02f,  6.454972244e-02f,  8.449668365e-02f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f,  3.034486645e-02f, -6.779013272e-02f,  1.659481923e-01f,  4.797944664e-02f, }},
    {{ 5.000000000e-02f,  0.000000000e+00f,  3.090169944e-02f, -8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f, -3.454915028e-02f, -6.454972244e-02f,  8.449668365e-02f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f,  3.034486645e-02f,  6.779013272e-02f,  1.659481923e-01f, -4.797944664e-02f, }},
    {{ 5.000000000e-02f,  0.000000000e+00f, -3.090169944e-02f,  8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f, -3.454915028e-02f, -6.454972244e-02f,  8.449668365e-02f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f, -3.034486645e-02f, -6.779013272e-02f, -1.659481923e-01f,  4.797944664e-02f, }},
    {{ 5.000000000e-02f,  0.000000000e+00f, -3.090169944e-02f, -8.090169944e-02f,  0.000000000e+00f,  0.000000000e+00f, -3.454915028e-02f,  6.454972244e-02f,  8.449668365e-02f,  0.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f, -3.034486645e-02f,  6.779013272e-02f, -1.659481923e-01f, -4.797944664e-02f, }},
    {{ 5.000000000e-02f,  5.000000000e-02f,  5.000000000e-02f,  5.000000000e-02f,  6.454972244e-02f,  6.454972244e-02f,  0.000000000e+00f,  6.454972244e-02f,  0.000000000e+00f,  1.016220987e-01f,  6.338656910e-02f, -1.092600649e-02f, -7.364853795e-02f,  1.011266756e-01f, -7.086833869e-02f, -1.482646439e-02f, }},
    {{ 5.000000000e-02f,  5.000000000e-02f,  5.000000000e-02f, -5.000000000e-02f, -6.454972244e-02f,  6.454972244e-02f,  0.000000000e+00f, -6.454972244e-02f,  0.000000000e+00f,  1.016220987e-01f, -6.338656910e-02f, -1.092600649e-02f, -7.364853795e-02f, -1.011266756e-01f, -7.086833869e-02f,  1.482646439e-02f, }},
    {{ 5.000000000e-02f, -5.000000000e-02f,  5.000000000e-02f,  5.000000000e-02f, -6.454972244e-02f, -6.454972244e-02f,  0.000000000e+00f,  6.454972244e-02f,  0.000000000e+00f, -1.016220987e-01f, -6.338656910e-02f,  1.092600649e-02f, -7.364853795e-02f,  1.011266756e-01f, -7.086833869e-02f, -1.482646439e-02f, }},
    {{ 5.000000000e-02f, -5.000000000e-02f,  5.000000000e-02f, -5.000000000e-02f,  6.454972244e-02f, -6.454972244e-02f,  0.000000000e+00f, -6.454972244e-02f,  0.000000000e+00f, -1.016220987e-01f,  6.338656910e-02f,  1.092600649e-02f, -7.364853795e-02f, -1.011266756e-01f, -7.086833869e-02f,  1.482646439e-02f, }},
    {{ 5.000000000e-02f,  5.000000000e-02f, -5.000000000e-02f,  5.000000000e-02f,  6.454972244e-02f, -6.454972244e-02f,  0.000000000e+00f, -6.454972244e-02f,  0.000000000e+00f,  1.016220987e-01f, -6.338656910e-02f, -1.092600649e-02f,  7.364853795e-02f,  1.011266756e-01f,  7.086833869e-02f, -1.482646439e-02f, }},
    {{ 5.000000000e-02f,  5.000000000e-02f, -5.000000000e-02f, -5.000000000e-02f, -6.454972244e-02f, -6.454972244e-02f,  0.000000000e+00f,  6.454972244e-02f,  0.000000000e+00f,  1.016220987e-01f,  6.338656910e-02f, -1.092600649e-02f,  7.364853795e-02f, -1.011266756e-01f,  7.086833869e-02f,  1.482646439e-02f, }},
    {{ 5.000000000e-02f, -5.000000000e-02f, -5.000000000e-02f,  5.000000000e-02f, -6.454972244e-02f,  6.454972244e-02f,  0.000000000e+00f, -6.454972244e-02f,  0.000000000e+00f, -1.016220987e-01f,  6.338656910e-02f,  1.092600649e-02f,  7.364853795e-02f,  1.011266756e-01f,  7.086833869e-02f, -1.482646439e-02f, }},
    {{ 5.000000000e-02f, -5.000000000e-02f, -5.000000000e-02f, -5.000000000e-02f,  6.454972244e-02f,  6.454972244e-02f,  0.000000000e+00f,  6.454972244e-02f,  0.000000000e+00f, -1.016220987e-01f, -6.338656910e-02f,  1.092600649e-02f,  7.364853795e-02f, -1.011266756e-01f,  7.086833869e-02f,  1.482646439e-02f, }},
}};
constexpr std::array<AmbiChannelFloatArray,20> ThirdOrderEncoder{{
    CalcAmbiCoeffs( 0.35682208976f,  0.93417235897f,  0.00000000000f),
    CalcAmbiCoeffs(-0.35682208976f,  0.93417235897f,  0.00000000000f),
    CalcAmbiCoeffs( 0.35682208976f, -0.93417235897f,  0.00000000000f),
    CalcAmbiCoeffs(-0.35682208976f, -0.93417235897f,  0.00000000000f),
    CalcAmbiCoeffs( 0.93417235897f,  0.00000000000f,  0.35682208976f),
    CalcAmbiCoeffs( 0.93417235897f,  0.00000000000f, -0.35682208976f),
    CalcAmbiCoeffs(-0.93417235897f,  0.00000000000f,  0.35682208976f),
    CalcAmbiCoeffs(-0.93417235897f,  0.00000000000f, -0.35682208976f),
    CalcAmbiCoeffs( 0.00000000000f,  0.35682208976f,  0.93417235897f),
    CalcAmbiCoeffs( 0.00000000000f,  0.35682208976f, -0.93417235897f),
    CalcAmbiCoeffs( 0.00000000000f, -0.35682208976f,  0.93417235897f),
    CalcAmbiCoeffs( 0.00000000000f, -0.35682208976f, -0.93417235897f),
    CalcAmbiCoeffs( 0.57735026919f,  0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f,  0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f,  0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f,  0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f, -0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs( 0.57735026919f, -0.57735026919f, -0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f, -0.57735026919f,  0.57735026919f),
    CalcAmbiCoeffs(-0.57735026919f, -0.57735026919f, -0.57735026919f),
}};
static_assert(ThirdOrderDecoder.size() == ThirdOrderEncoder.size(), "Third-order mismatch");

/* This calculates a third-order "upsampler" matrix. Same as the first-order
 * matrix, just using a more dense speaker array suitable for third-order
 * content.
 */
auto CalcThirdOrderUp()
{
    std::array<AmbiChannelFloatArray,16> res{};

    for(size_t i{0};i < ThirdOrderDecoder[0].size();++i)
    {
        for(size_t j{0};j < ThirdOrderEncoder[0].size();++j)
        {
            double sum{0.0};
            for(size_t k{0};k < ThirdOrderDecoder.size();++k)
                sum += double{ThirdOrderDecoder[k][i]} * ThirdOrderEncoder[k][j];
            res[i][j] = static_cast<float>(sum);
        }
    }

    return res;
}


constexpr std::array<std::array<float,16>,8> ThirdOrder2DDecoder{{
    {{ 1.250000000e-01f, -5.523559567e-02f, 0.0f,  1.333505242e-01f, -9.128709292e-02f, 0.0f, 0.0f, 0.0f,  9.128709292e-02f, -1.104247249e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  4.573941867e-02f, }},
    {{ 1.250000000e-01f, -1.333505242e-01f, 0.0f,  5.523559567e-02f, -9.128709292e-02f, 0.0f, 0.0f, 0.0f, -9.128709292e-02f,  4.573941867e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.104247249e-01f, }},
    {{ 1.250000000e-01f, -1.333505242e-01f, 0.0f, -5.523559567e-02f,  9.128709292e-02f, 0.0f, 0.0f, 0.0f, -9.128709292e-02f,  4.573941867e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  1.104247249e-01f, }},
    {{ 1.250000000e-01f, -5.523559567e-02f, 0.0f, -1.333505242e-01f,  9.128709292e-02f, 0.0f, 0.0f, 0.0f,  9.128709292e-02f, -1.104247249e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -4.573941867e-02f, }},
    {{ 1.250000000e-01f,  5.523559567e-02f, 0.0f, -1.333505242e-01f, -9.128709292e-02f, 0.0f, 0.0f, 0.0f,  9.128709292e-02f,  1.104247249e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -4.573941867e-02f, }},
    {{ 1.250000000e-01f,  1.333505242e-01f, 0.0f, -5.523559567e-02f, -9.128709292e-02f, 0.0f, 0.0f, 0.0f, -9.128709292e-02f, -4.573941867e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  1.104247249e-01f, }},
    {{ 1.250000000e-01f,  1.333505242e-01f, 0.0f,  5.523559567e-02f,  9.128709292e-02f, 0.0f, 0.0f, 0.0f, -9.128709292e-02f, -4.573941867e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.104247249e-01f, }},
    {{ 1.250000000e-01f,  5.523559567e-02f, 0.0f,  1.333505242e-01f,  9.128709292e-02f, 0.0f, 0.0f, 0.0f,  9.128709292e-02f,  1.104247249e-01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  4.573941867e-02f, }},
}};
constexpr std::array<AmbiChannelFloatArray,8> ThirdOrder2DEncoder{{
    CalcAmbiCoeffs(-0.38268343237f,  0.00000000000f,  0.92387953251f),
    CalcAmbiCoeffs(-0.92387953251f,  0.00000000000f,  0.38268343237f),
    CalcAmbiCoeffs(-0.92387953251f,  0.00000000000f, -0.38268343237f),
    CalcAmbiCoeffs(-0.38268343237f,  0.00000000000f, -0.92387953251f),
    CalcAmbiCoeffs( 0.38268343237f,  0.00000000000f, -0.92387953251f),
    CalcAmbiCoeffs( 0.92387953251f,  0.00000000000f, -0.38268343237f),
    CalcAmbiCoeffs( 0.92387953251f,  0.00000000000f,  0.38268343237f),
    CalcAmbiCoeffs( 0.38268343237f,  0.00000000000f,  0.92387953251f),
}};
static_assert(ThirdOrder2DDecoder.size() == ThirdOrder2DEncoder.size(), "Third-order 2D mismatch");

/* This calculates a 2D third-order "upsampler" matrix. Same as the third-order
 * matrix, just using a more optimized speaker array for horizontal-only
 * content.
 */
auto CalcThirdOrder2DUp()
{
    std::array<AmbiChannelFloatArray,16> res{};

    for(size_t i{0};i < ThirdOrder2DDecoder[0].size();++i)
    {
        for(size_t j{0};j < ThirdOrder2DEncoder[0].size();++j)
        {
            double sum{0.0};
            for(size_t k{0};k < ThirdOrder2DDecoder.size();++k)
                sum += double{ThirdOrder2DDecoder[k][i]} * ThirdOrder2DEncoder[k][j];
            res[i][j] = static_cast<float>(sum);
        }
    }

    return res;
}


constexpr std::array<std::array<float,25>,10> FourthOrder2DDecoder{{
    {{ 1.000000000e-01f,  3.568220898e-02f, 0.0f,  1.098185471e-01f,  6.070619982e-02f, 0.0f, 0.0f, 0.0f,  8.355491589e-02f,  7.735682057e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  5.620301997e-02f,  8.573754253e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  2.785781628e-02f, }},
    {{ 1.000000000e-01f,  9.341723590e-02f, 0.0f,  6.787159473e-02f,  9.822469464e-02f, 0.0f, 0.0f, 0.0f, -3.191513794e-02f,  2.954767620e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -9.093839659e-02f, -5.298871540e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -7.293270986e-02f, }},
    {{ 1.000000000e-01f,  1.154700538e-01f, 0.0f,  0.000000000e+00f,  0.000000000e+00f, 0.0f, 0.0f, 0.0f, -1.032795559e-01f, -9.561828875e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.000000000e+00f,  0.000000000e+00f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  9.014978717e-02f, }},
    {{ 1.000000000e-01f,  9.341723590e-02f, 0.0f, -6.787159473e-02f, -9.822469464e-02f, 0.0f, 0.0f, 0.0f, -3.191513794e-02f,  2.954767620e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  9.093839659e-02f,  5.298871540e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -7.293270986e-02f, }},
    {{ 1.000000000e-01f,  3.568220898e-02f, 0.0f, -1.098185471e-01f, -6.070619982e-02f, 0.0f, 0.0f, 0.0f,  8.355491589e-02f,  7.735682057e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -5.620301997e-02f, -8.573754253e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  2.785781628e-02f, }},
    {{ 1.000000000e-01f, -3.568220898e-02f, 0.0f, -1.098185471e-01f,  6.070619982e-02f, 0.0f, 0.0f, 0.0f,  8.355491589e-02f, -7.735682057e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -5.620301997e-02f,  8.573754253e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  2.785781628e-02f, }},
    {{ 1.000000000e-01f, -9.341723590e-02f, 0.0f, -6.787159473e-02f,  9.822469464e-02f, 0.0f, 0.0f, 0.0f, -3.191513794e-02f, -2.954767620e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  9.093839659e-02f, -5.298871540e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -7.293270986e-02f, }},
    {{ 1.000000000e-01f, -1.154700538e-01f, 0.0f,  0.000000000e+00f,  0.000000000e+00f, 0.0f, 0.0f, 0.0f, -1.032795559e-01f,  9.561828875e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.000000000e+00f,  0.000000000e+00f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  9.014978717e-02f, }},
    {{ 1.000000000e-01f, -9.341723590e-02f, 0.0f,  6.787159473e-02f, -9.822469464e-02f, 0.0f, 0.0f, 0.0f, -3.191513794e-02f, -2.954767620e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -9.093839659e-02f,  5.298871540e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -7.293270986e-02f, }},
    {{ 1.000000000e-01f, -3.568220898e-02f, 0.0f,  1.098185471e-01f, -6.070619982e-02f, 0.0f, 0.0f, 0.0f,  8.355491589e-02f, -7.735682057e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  5.620301997e-02f, -8.573754253e-02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  2.785781628e-02f, }},
}};
constexpr std::array<AmbiChannelFloatArray,10> FourthOrder2DEncoder{{
    CalcAmbiCoeffs( 3.090169944e-01f,  0.000000000e+00f,  9.510565163e-01f),
    CalcAmbiCoeffs( 8.090169944e-01f,  0.000000000e+00f,  5.877852523e-01f),
    CalcAmbiCoeffs( 1.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f),
    CalcAmbiCoeffs( 8.090169944e-01f,  0.000000000e+00f, -5.877852523e-01f),
    CalcAmbiCoeffs( 3.090169944e-01f,  0.000000000e+00f, -9.510565163e-01f),
    CalcAmbiCoeffs(-3.090169944e-01f,  0.000000000e+00f, -9.510565163e-01f),
    CalcAmbiCoeffs(-8.090169944e-01f,  0.000000000e+00f, -5.877852523e-01f),
    CalcAmbiCoeffs(-1.000000000e+00f,  0.000000000e+00f,  0.000000000e+00f),
    CalcAmbiCoeffs(-8.090169944e-01f,  0.000000000e+00f,  5.877852523e-01f),
    CalcAmbiCoeffs(-3.090169944e-01f,  0.000000000e+00f,  9.510565163e-01f),
}};
static_assert(FourthOrder2DDecoder.size() == FourthOrder2DEncoder.size(), "Fourth-order 2D mismatch");

/* This calculates a 2D fourth-order "upsampler" matrix. There is no 3D fourth-
 * order upsampler since fourth-order is the max order we'll be supporting for
 * the foreseeable future. This is only necessary for mixing horizontal-only
 * fourth-order content to 3D.
 */
auto CalcFourthOrder2DUp()
{
    std::array<AmbiChannelFloatArray,25> res{};

    for(size_t i{0};i < FourthOrder2DDecoder[0].size();++i)
    {
        for(size_t j{0};j < FourthOrder2DEncoder[0].size();++j)
        {
            double sum{0.0};
            for(size_t k{0};k < FourthOrder2DDecoder.size();++k)
                sum += double{FourthOrder2DDecoder[k][i]} * FourthOrder2DEncoder[k][j];
            res[i][j] = static_cast<float>(sum);
        }
    }

    return res;
}

} // namespace

const std::array<AmbiChannelFloatArray,4> AmbiScale::FirstOrderUp{CalcFirstOrderUp()};
const std::array<AmbiChannelFloatArray,4> AmbiScale::FirstOrder2DUp{CalcFirstOrder2DUp()};
const std::array<AmbiChannelFloatArray,9> AmbiScale::SecondOrderUp{CalcSecondOrderUp()};
const std::array<AmbiChannelFloatArray,9> AmbiScale::SecondOrder2DUp{CalcSecondOrder2DUp()};
const std::array<AmbiChannelFloatArray,16> AmbiScale::ThirdOrderUp{CalcThirdOrderUp()};
const std::array<AmbiChannelFloatArray,16> AmbiScale::ThirdOrder2DUp{CalcThirdOrder2DUp()};
const std::array<AmbiChannelFloatArray,25> AmbiScale::FourthOrder2DUp{CalcFourthOrder2DUp()};

const std::array<float,MaxAmbiOrder+1> AmbiScale::DecoderHFScale1O{{
    2.000000000e+00f, 1.154700538e+00f
}};
const std::array<float,MaxAmbiOrder+1> AmbiScale::DecoderHFScale1O2D{{
    1.414213562e+00f, 1.000000000e+00f
}};
const std::array<float,MaxAmbiOrder+1> AmbiScale::DecoderHFScale2O{{
    1.972026594e+00f, 1.527525232e+00f, 7.888106377e-01f
}};
const std::array<float,MaxAmbiOrder+1> AmbiScale::DecoderHFScale2O2D{{
    1.414213562e+00f, 1.224744871e+00f, 7.071067812e-01f
}};
const std::array<float,MaxAmbiOrder+1> AmbiScale::DecoderHFScale3O{{
    1.865086714e+00f, 1.606093894e+00f, 1.142055301e+00f, 5.683795528e-01f
}};
const std::array<float,MaxAmbiOrder+1> AmbiScale::DecoderHFScale3O2D{{
    1.414213562e+00f, 1.306562965e+00f, 1.000000000e+00f, 5.411961001e-01f
}};
