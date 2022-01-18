// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
//  pwl_design.cpp : simple activation function designer
//

#include <vector>
#include <iostream>
#include <limits>
#include <cstdint>
#include <algorithm>

#ifdef _NO_MKL_
#include <cmath>
#include "backend/make_pwl.hpp"

#define SCOPY(num, in, inci, out, inco) for (int i_ = 0; i_ < *(num); i_++) *(out + i_ * *(inco)) = *(in + i_ * *(inci));
#define SSCAL(num, scale, inout, inco)  for (int i_ = 0; i_ < *(num); i_++) *(inout + i_ * *(inco)) = *(scale) * *(inout + i_ * *(inco));
#define TANH(num, in, out) for (int i_ = 0; i_ < num; i_++) *(out+i_) = tanh(*(in+i_))
#else
#include <mkl.h>
#define SCOPY(num, in, incx, out, incy) scopy(num, in, incx, out, incy)
#define SSCAL(num, scale, inout, incx) sscal(num, scale, inout, incx)
#define TANH(num, in, out) vsTanh(num, in, out)
#endif

#include "pwl.h"
#include "gna_plugin_log.hpp"
#include "gna_slope_scale.h"
#include "round_float_define.hpp"

inline double neglog(const double x) { return(-1.0*log(x)); }
inline double neghalflog(const double x) { return(-0.5*log(x)); }
inline double first_deriv_neglog(const double x) { return(-1.0 / x); }
inline double first_deriv_neghalflog(const double x) { return(-0.5 / x); }
double softsign(const double x) { return(x / (1.0 + fabs(x))); }
double first_deriv_softsign(const double x) { return(1.0 / ((1.0 + fabs(x)) * (1.0 + fabs(x)))); }
double relu(const double x) { if (x < 0) { return(0.0); } else { return(x); } }
double leaky_relu(const double x) { if (x < 0.0) { return(LEAKYRELU_SLOPE*x); } else { return(x); } }
double clipping(const double x, const double lbound, const double ubound) { return((x < lbound)?lbound:((x > ubound)?ubound:x)); }

template <typename T1, typename T2>
double pivot_search(std::vector<pwl_t>& result,
                    T1 f,
                    T2 first_deriv_f,
                    const uint32_t N,
                    const double alpha_0,
                    const double alpha_N,
                    const double threshold,
                    const bool negative,
                    size_t iter_num) {
    std::vector<std::vector<double>> t(N + 1);
    std::vector<std::vector<double>> alpha(N + 1);
    std::vector<std::vector<double>> epsilon(N + 1);
    std::vector<std::vector<double>> d(N + 1);
    bool same_epsilon = false;
    double Delta;
    double epsilon_final = 0.0;
    double max_epsilon = 0.0;
    double max_epsilon_prev;
    double min_epsilon;
    double sgn = (negative) ? -1.0 : 1.0;
    int j;

    if (threshold < 0) {
        return epsilon_final;
    }
    // Figure 4:  Box #1
    j = 0;
    Delta = 1.0;

    for (int i = 0; i < N; i++) {
        t[i].push_back(alpha_0 + (static_cast<double>((i + 1)) / static_cast<double>((N + 1))) * (alpha_N - alpha_0));
    }

    while (true) {
        // Figure 4:  Box #2
        alpha[0].resize(j + 1);
        alpha[0][j] = alpha_0;
        for (int i = 1; i < N; i++) {
            alpha[i].resize(j + 1);
            alpha[i][j] = (f(t[i - 1][j]) - f(t[i][j]) + first_deriv_f(t[i][j]) * t[i][j] - first_deriv_f(t[i - 1][j]) * t[i - 1][j])
                / (first_deriv_f(t[i][j]) - first_deriv_f(t[i - 1][j]));
        }
        alpha[N].resize(j + 1);
        alpha[N][j] = alpha_N;

        // Figure 4:  Box #3
        for (int i = 0; i < N; i++) {
            epsilon[i].resize(j + 1);
            epsilon[i][j] = sgn * (first_deriv_f(t[i][j]) * (alpha[i][j] - t[i][j]) + f(t[i][j]) - f(alpha[i][j]));
        }
        epsilon[N].resize(j + 1);
        epsilon[N][j] = sgn * (first_deriv_f(t[N - 1][j]) * (alpha[N][j] - t[N - 1][j]) + f(t[N - 1][j]) - f(alpha[N][j]));

        // Figure 4:  Test for completion
        max_epsilon_prev = max_epsilon;
        max_epsilon = fabs(epsilon[0][j]);
        min_epsilon = fabs(epsilon[0][j]);
        for (int i = 1; i < N + 1; i++) {
            if (fabs(epsilon[i][j]) > max_epsilon) max_epsilon = fabs(epsilon[i][j]);
            if (fabs(epsilon[i][j]) < min_epsilon) min_epsilon = fabs(epsilon[i][j]);
        }
        if ((j == iter_num) || (max_epsilon - min_epsilon < threshold * min_epsilon)) {
            pwl_t value;
            result.resize(0);
            epsilon_final = (max_epsilon + min_epsilon) / 4.0;  // Andrzej's modification
            for (int i = 0; i < N; i++) {
                double val, val_next;
                value.t = t[i][j];
                value.alpha = alpha[i][j];
                val = sgn * first_deriv_f(value.t) * (value.alpha - value.t) + sgn * f(value.t) - epsilon_final;
                val_next = sgn * first_deriv_f(value.t) * (alpha[i + 1][j] - value.t) + sgn * f(value.t) - epsilon_final;
                value.beta = val;
                value.m = (val_next - val) / (alpha[i + 1][j] - value.alpha);
                value.b = (val - value.m * value.alpha);
                result.push_back(value);
            }
            value.t = value.m = value.b = 0.0;
            value.alpha = alpha[N][j];
            value.beta = sgn * first_deriv_f(t[N - 1][j]) * (alpha[N][j] - t[N - 1][j]) + sgn * f(t[N - 1][j]) - epsilon_final;
            result.push_back(value);
            if (j == iter_num) {
                THROW_GNA_EXCEPTION << "Failed to converge in pivot_search!";
            }
            return(epsilon_final);
        }

        if (j > 0) {
            if (max_epsilon > max_epsilon_prev) {
                j = j - 1;
                Delta = Delta / 2;
            } else if (max_epsilon == max_epsilon_prev) {
                if (!same_epsilon) {
                    same_epsilon = true;
                } else {
                    j = j - 1;
                    Delta = Delta / 2;
                    same_epsilon = false;
                }
            }
        }

        // Figure 4:  Box #4
        for (int i = 0; i < N; i++) {
            d[i].resize(j + 1);
            d[i][j] = Delta * (epsilon[i + 1][j] - epsilon[i][j]) /
                ((epsilon[i + 1][j] / (alpha[i + 1][j] - t[i][j])) + (epsilon[i][j] / (t[i][j] - alpha[i][j])));
        }

        // Figure 4:  Box #5
        for (int i = 0; i < N; i++) {
            t[i].resize(j + 2);
            t[i][j + 1] = t[i][j] + d[i][j];
        }
        t[N].resize(j + 2);

        j = j + 1;
    }
}

double pivot_search(std::vector<pwl_t>& result, double(*f)(const double),
                    double(*first_deriv_f)(const double),
                    const uint32_t N,
                    const double alpha_0,
                    const double alpha_N,
                    const double threshold,
                    const bool negative,
                    size_t iter_num) {
    double epsilon_final = 0.0;

    if (f == nullptr ||
        first_deriv_f == nullptr ||
        threshold < 0) {
        return epsilon_final;
    }

    auto fun = [&f](double x) -> double { return f(x); };
    auto first_deriv = [&first_deriv_f](double x) -> double { return first_deriv_f(x); };
    return pivot_search(result, fun, first_deriv, N, alpha_0, alpha_N, threshold, negative, iter_num);
}

double calculate_error_pct(const DnnActivation& activation_type,
                            const double l_bound,
                            const double u_bound,
                            const double offset,
                            const int samples) {
    double delta = (u_bound - l_bound) / (samples + 1);
    double min_val = 0.0;
    double max_val = 0.0;

    if ( delta < 0 ) {
        return 0.0;
    }

    switch (activation_type) {
        case kActNegLog:
            min_val = max_val = neglog(l_bound);
            break;
        case kActNegHalfLog:
            min_val = max_val = neghalflog(l_bound);
            break;
        case kActSoftSign:
            min_val = max_val = softsign(l_bound);
            break;
        default:
            break;
    }

    for (int i = 0; i < samples; i++) {
        double arg = l_bound + i * delta;
        double val = 0.0;
        switch (activation_type) {
            case kActSoftSign:
                val = softsign(arg);
                break;
            case kActNegLog:
                val = neglog(arg);
                break;
            case kActNegHalfLog:
                val = neghalflog(arg);
                break;
            default:
                break;
        }
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }

    return(100.0 * fabs(offset) / (max_val - min_val));
}

inline bool split_search(const DnnActivation& activation_type,
                    const double l_bound,
                    const double u_bound) {
    bool is_split = false;
    if (l_bound > u_bound) {
        return is_split;
    }
    double break_bound = 0;
    switch (activation_type) {
        case kActSoftSign:
            is_split = ((l_bound < break_bound) && (u_bound > break_bound));
            break;
        default:
            is_split = false;
    }
    return is_split;
}

inline std::vector<pwl_t> negative_pwl(const std::vector<pwl_t>& pwl) {
    std::vector<pwl_t> new_pwl;
    new_pwl = pwl;
    for (uint32_t i = 0; i < pwl.size(); i++) {
        new_pwl[i].m = -pwl[i].m;
        new_pwl[i].b = -pwl[i].b;
        new_pwl[i].beta = -pwl[i].beta;
    }

    return(new_pwl);
}

std::vector<pwl_t> pwl_search(const DnnActivation& activation_type,
                                const double l_bound,
                                const double u_bound,
                                const double threshold,
                                const double allowed_err_pct,
                                const int samples,
                                double& err_pct) {
    std::vector<pwl_t> pwl;
    double err = 0.0;
    int n_segments = 1;

    if (l_bound > u_bound ||
        threshold < 0) {
        return pwl;
    }

    if (split_search(activation_type, l_bound, u_bound)) {
        std::vector<pwl_t> pwl2;
        double err_pct1 = 0.0, err_pct2 = 0.0;
        double break_bound = 0;

        pwl = pwl_search(activation_type, l_bound, break_bound, threshold, allowed_err_pct, samples, err_pct1);
        pwl = negative_pwl(pwl);
        pwl2 = pwl_search(activation_type, break_bound, u_bound, threshold, allowed_err_pct, samples, err_pct2);

        // merge
        pwl.pop_back();  // remove final alpha and beta from first half
        pwl.insert(pwl.end(), pwl2.begin(), pwl2.end());  // concatenate the two halves
        err_pct = (err_pct1 + err_pct2) / 2;  // this is not quite correct but should give an indication
    } else {
        bool negative = false;
        switch (activation_type) {
            case kActSoftSign:
                if (u_bound == 0) negative = true;  // make left half convex
                err = pivot_search(pwl, softsign, first_deriv_softsign, n_segments, l_bound, u_bound,
                    threshold, negative, PWL_MAX_ITERATIONS_DEFAULT);
                break;
            case kActNegLog:
                negative = true;  // make function convex
                err = pivot_search(pwl, neglog, first_deriv_neglog, n_segments, l_bound, u_bound,
                    threshold, negative, PWL_MAX_ITERATIONS_LOG);
                pwl = negative_pwl(pwl);
                break;
            case kActNegHalfLog:
                negative = true;  // make function convex
                err = pivot_search(pwl, neghalflog, first_deriv_neghalflog, n_segments, l_bound, u_bound,
                    threshold, negative, PWL_MAX_ITERATIONS_LOG);
                pwl = negative_pwl(pwl);
                break;
            default:
                break;
        }
        err_pct = calculate_error_pct(activation_type, l_bound, u_bound, err, samples);

        while ((n_segments < PWL_MAX_NUM_SEGMENTS) && (allowed_err_pct < err_pct)) {
            n_segments += 1;
            switch (activation_type) {
                case kActSoftSign:
                    err = pivot_search(pwl, softsign, first_deriv_softsign, n_segments, l_bound, u_bound,
                        threshold, negative, PWL_MAX_ITERATIONS_DEFAULT);
                        break;
                case kActNegLog:
                    err = pivot_search(pwl, neglog, first_deriv_neglog, n_segments, l_bound, u_bound,
                        threshold, negative, PWL_MAX_ITERATIONS_LOG);
                    pwl = negative_pwl(pwl);
                    break;
                case kActNegHalfLog:
                    err = pivot_search(pwl, neghalflog, first_deriv_neghalflog, n_segments, l_bound, u_bound,
                        threshold, negative, PWL_MAX_ITERATIONS_LOG);
                    pwl = negative_pwl(pwl);
                    break;
                default:
                    break;
            }
            err_pct = calculate_error_pct(activation_type, l_bound, u_bound, err, samples);
        }

        if (n_segments >= PWL_MAX_NUM_SEGMENTS) {
            THROW_GNA_EXCEPTION << "Failed to converge in pwl_search!";
        }
    }
    return(pwl);
}


void PwlDesignOpt(const DnnActivation& activation_type,
                    std::vector<gna_pwl_segment_t> &ptr_segment,
                    const float scale_in,
                    const float scale_out,
                    const float pwlMaxErrorPercent,
                    const bool low_precision,
                    const std::shared_ptr<ngraph::Node>& node) {
    std::vector<pwl_t> pwl;
    double err_pct = 0.0;
    auto minInputStats = 0.0f;
    auto maxInputStats = 0.0f;
    if (activation_type.srcFQParams.set) {
        minInputStats = std::min(*activation_type.srcFQParams.input_low, *activation_type.srcFQParams.input_high) * 1.25f;
        maxInputStats = std::max(*activation_type.srcFQParams.input_low, *activation_type.srcFQParams.input_high) * 1.25f;
    }
    switch (activation_type) {
        case kActPwl: {
            auto minInput = 0;
            auto maxInput = 0;
            make_gna_pwl(node, minInput, maxInput, scale_in, scale_out, low_precision, ptr_segment);
            break;
        }
        case kActRelu:
            make_gna_pwl(activation_type, pwl, -1.0, 1.0, scale_in, scale_out, low_precision, ptr_segment);
            break;
        case kActLeakyRelu:
            make_gna_pwl(activation_type, pwl, -1.0, 1.0, scale_in, scale_out, low_precision, ptr_segment);
            break;
        case kActIdentity:
        case kActFakeQuantize:
            make_gna_pwl(activation_type, pwl, -1.0, 1.0, scale_in, scale_out, low_precision, ptr_segment);
            break;
        case kActKaldiLstmClipping:
            make_gna_pwl(activation_type, pwl, activation_type.args.clamp.low, activation_type.args.clamp.high,
                         scale_in, scale_out, low_precision, ptr_segment);
            break;
        case kActNegLog: {
            double x_min = (1 + ~XBASEMASK) / scale_in;
            double x_max = ((static_cast<double>(INT32_MAX) / scale_in) < LOG_DOMAIN) ? (static_cast<double>(INT32_MAX) / scale_in) : LOG_DOMAIN;
            pwl = pwl_search(activation_type, x_min, x_max, PWL_DESIGN_THRESHOLD, pwlMaxErrorPercent, PWL_DESIGN_SAMPLES, err_pct);
            make_gna_pwl(activation_type, pwl, x_min, x_max, scale_in, scale_out, low_precision, ptr_segment);
            break;
        }
        case kActNegHalfLog: {
            double x_min = (1 + ~XBASEMASK) / scale_in;
            double x_max = ((static_cast<double>(INT32_MAX) / scale_in) < LOG_DOMAIN) ? (static_cast<double>(INT32_MAX) / scale_in) : LOG_DOMAIN;
            pwl = pwl_search(activation_type, x_min, x_max, PWL_DESIGN_THRESHOLD, pwlMaxErrorPercent, PWL_DESIGN_SAMPLES, err_pct);
            make_gna_pwl(activation_type, pwl, x_min, x_max, scale_in, scale_out, low_precision, ptr_segment);
            break;
        }
        case kActSign:
            make_gna_pwl(activation_type, pwl, -1.0, 1.0, scale_in, scale_out, low_precision, ptr_segment);
            break;
        case kActAbs:
            make_gna_pwl(activation_type, pwl, -1.0, 1.0, scale_in, scale_out, low_precision, ptr_segment);
            break;
        default:
            break;
    }
}

void PwlDesign(const DnnActivation& activation_type,
                 gna_pwl_segment_t *ptr_segment,
                 const uint32_t num_segments,
                 const float scale_in,
                 const float scale_out,
                 const bool low_precision) {
    switch (activation_type) {
        case kActSoftSign:
            {
                gnalog() << "=========================== SoftSign Segments===========================\n";
                uint32_t num_segment_size = 0;
                int32_t offset = 0;
                ptr_segment[0].xBase = static_cast<int32_t>(INT32_MIN & XBASEMASK);  // zero out the 2 lsb
                num_segment_size = static_cast<int32_t>(SOFTSIGN_DOMAIN * scale_in / ((num_segments - 2) / 2) + 0.5);
                offset = -static_cast<int32_t>(num_segment_size * (num_segments - 2) / 2);
                for (uint32_t i = 1; i < num_segments; i++) {
                    ptr_segment[i].xBase = static_cast<int32_t>(offset & XBASEMASK);  // zero out the 2 lsb
                    offset += num_segment_size;
                }
                for (uint32_t i = 0; i < num_segments; i++) {
                    int32_t xbase = static_cast<int32_t>(ptr_segment[i].xBase & XBASEMASK);
                    int32_t xbasenext = (i < num_segments - 1) ? static_cast<int32_t>(ptr_segment[i + 1].xBase & XBASEMASK) : INT32_MAX;
                    float floatarg = static_cast<float>(xbase / (2 * scale_in));
                    float floatargnext = static_cast<float>(xbasenext / (2 * scale_in));
                    float floatval, floatvalnext, slope;
                    floatval = softsign(floatarg);
                    floatvalnext = softsign(floatargnext);
                    slope = scale_out * (floatvalnext - floatval) / static_cast<float>(xbasenext - xbase);
                    {
                        // find best scale factor
                        uint64_t slope_scale;
                        uint32_t slope_scale_index;
                        for (slope_scale_index = 3; slope_scale_index > 0; slope_scale_index--) {
                            slope_scale = static_cast<uint64_t>(1) << (8 * (1 + slope_scale_index));
                            if (((slope * slope_scale) <= 32767.0) && ((slope * slope_scale) >= -32768.0))
                                break;
                        }
                        slope_scale = static_cast<uint64_t>(1) << (8 * (1 + slope_scale_index));
                        ptr_segment[i].slope = FLOAT_TO_INT16(slope * slope_scale);
                        ptr_segment[i].xBase = ptr_segment[i].xBase | slope_scale_index;
                    }
                    ptr_segment[i].yBase = FLOAT_TO_INT16(floatval * scale_out);
                    gnalog() << (static_cast<int32_t>((ptr_segment[i].xBase & XBASEMASK)) / scale_out)
                        << " "
                        << (static_cast<float>((ptr_segment[i].yBase)) / scale_out)
                        << " "
                        << (slope / scale_out)
                        << "\n";
                }
            }
            break;
        case kActRelu:
            THROW_GNA_EXCEPTION << "Rectilinear activation function design not yet implemented!";
        case kActIdentity:
        case kActKaldiLstmClipping:  // clipping of IDENTITY is more aggressive than Kaldi
            {
                float slope = 0.0;
                int64_t x_lower_limit = static_cast<int64_t>((INT16_MIN / scale_out) * scale_in - 0.5);
                int64_t x_upper_limit = static_cast<int64_t>((INT16_MAX / scale_out) * scale_in + 0.5);
                int16_t y_lower_limit = INT16_MIN;
                int16_t y_upper_limit = INT16_MAX;
                if (activation_type == kActKaldiLstmClipping)
                    gnalog() << "=========================== Clipping Segments ===========================\n";
                else
                    gnalog() << "=========================== Identity Segments ===========================\n";
                if (x_lower_limit < INT32_MIN) {
                    std::cerr << "Warning:  saturation in PwlDesign! " << x_lower_limit  << " < INT32_MIN"<< std::endl;
                    x_lower_limit = INT32_MIN;
                    y_lower_limit = static_cast<int16_t>((scale_out / scale_in)*static_cast<float>(INT32_MIN) - 0.5);
                }
                if (x_upper_limit > INT32_MAX) {
                    std::cerr << "Warning:  saturation in PwlDesign! " << x_upper_limit  << " > INT32_MAX"<< std::endl;
                    x_upper_limit = INT32_MAX;
                    y_upper_limit = static_cast<int16_t>((scale_out / scale_in)*static_cast<float>(INT32_MAX) + 0.5);
                }
                slope =
                    static_cast<float>(static_cast<uint64_t>(y_upper_limit) - static_cast<uint64_t>(y_lower_limit)) /
                                               static_cast<float>(static_cast<uint64_t>(x_upper_limit) - static_cast<uint64_t>(x_lower_limit));
                ptr_segment[0].xBase = static_cast<int32_t>(INT32_MIN & XBASEMASK);  // zero out the 2 lsb
                ptr_segment[0].yBase = y_lower_limit;
                ptr_segment[0].slope = 0;

                gnalog() << ptr_segment[0].xBase / scale_in
                    << " " << ptr_segment[0].yBase / scale_out
                    << " " << 0
                    << "\n";

                ptr_segment[1].xBase = static_cast<int32_t>(x_lower_limit & XBASEMASK);
                ptr_segment[1].yBase = y_lower_limit;
                {
                    // find best scale factor
                    uint64_t slope_scale = 0;
                    uint32_t slope_scale_index = 0;
                    for (slope_scale_index = 3; slope_scale_index > 0; slope_scale_index--) {
                        slope_scale = static_cast<uint64_t>(1) << (8 * (1 + slope_scale_index));
                        if (((slope * slope_scale) <= std::numeric_limits<int16_t>::max()) &&
                                    ((slope * slope_scale) >= std::numeric_limits<int16_t>::min()))
                            break;
                    }
                    slope_scale = static_cast<uint64_t>(1) << (8 * (1 + slope_scale_index));
                    ptr_segment[1].slope = FLOAT_TO_INT16(slope * slope_scale);
                    ptr_segment[1].xBase = ptr_segment[1].xBase | slope_scale_index;
                }
                ptr_segment[2].xBase = static_cast<int32_t>(x_upper_limit & XBASEMASK);
                ptr_segment[2].yBase = y_upper_limit;
                ptr_segment[2].slope = 0;
            }
            break;
        default:
            fprintf(stderr, "Activation function design for %s not yet implemented!\n", intel_dnn_activation_name[activation_type]);
            throw -1;
    }
}

void PwlApply32(intel_dnn_component_t *component, uint32_t num_subset_size) {
    if (component->orientation_in == kDnnInterleavedOrientation) {  // subsets only supported in interleaved orientation
        PwlApply32(component, 0, num_subset_size - 1, 0, component->num_columns_in - 1);
    } else {
        PwlApply32(component, 0, component->num_rows_in - 1, 0, component->num_columns_in - 1);
    }
}

void PwlApply32(intel_dnn_component_t *component,
                uint32_t num_row_start,
                uint32_t num_row_end,
                uint32_t num_col_start,
                uint32_t num_col_end) {
    intel_piecewiselinear_t *transform = reinterpret_cast<intel_piecewiselinear_t *>(&component->op.pwl);
    float *ptr_in = reinterpret_cast<float *>(component->ptr_inputs);
    float *ptr_out = reinterpret_cast<float *>(component->ptr_outputs);
    uint32_t num_columns = component->num_columns_in;
    switch (transform->func_id.type) {
        case kActSigmoid:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = 0.5 * (1.0 + tanh(0.5 * ptr_in[i * num_columns + j]));
                }
            }
            break;
        case kActTanh:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = tanh(ptr_in[i * num_columns + j]);
                }
            }
            break;
        case kActSoftSign:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = ptr_in[i * num_columns + j] / (1.0 + fabs(ptr_in[i * num_columns + j]));
                }
            }
            break;
        case kActRelu:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] =
                        (ptr_in[i * num_columns + j] < 0.0f) ?
                            ptr_in[i * num_columns + j] * transform->func_id.args.lrelu.negative_slope :
                            ptr_in[i * num_columns + j];
                }
            }
            break;
        case kActIdentity:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = ptr_in[i * num_columns + j];
                }
            }
            break;
        case kActKaldiLstmClipping: {
            float upper_limit = component->op.pwl.func_id.args.clamp.high;
            float lower_limit = component->op.pwl.func_id.args.clamp.low;
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    float val = ptr_in[i * num_columns + j];
                    if (val > upper_limit) {
                        ptr_out[i * num_columns + j] = upper_limit;
                    } else if (val < lower_limit) {
                        ptr_out[i * num_columns + j] = lower_limit;
                    } else {
                        ptr_out[i * num_columns + j] = val;
                    }
                }
            }
            break;
        }
        case kActExp:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = exp(ptr_in[i * num_columns + j]);
                }
            }
            break;
        case kActLog:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = log(ptr_in[i * num_columns + j]);
                }
            }
            break;
        case kActAbs:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = fabs(ptr_in[i * num_columns + j]);
                }
            }
            break;
        case kActSign:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = (ptr_in[i * num_columns + j] == 0) ? 0.0 : ((ptr_in[i * num_columns + j] > 0) ? 1.0 : -1.0);
                }
            }
            break;
        case kActNegLog:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = -1.0 * log(ptr_in[i * num_columns + j]);
                }
            }
            break;
        case kActNegHalfLog:
            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    ptr_out[i * num_columns + j] = -0.5 * log(ptr_in[i * num_columns + j]);
                }
            }
            break;
        case kActPow: {
                float exponent = transform->func_id.args.pow.exponent;
                float scale = transform->func_id.args.pow.scale;
                float offset = transform->func_id.args.pow.offset;
                for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                    for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                        ptr_out[i * num_columns + j] = pow(offset + scale * ptr_in[i * num_columns + j], exponent);
                    }
                }
            }
            break;
        case kActFakeQuantize: {
            double levels  = transform->func_id.fqParams.levels;

            for (uint32_t i = num_row_start; i <= num_row_end; i++) {
                auto inputChannel  = transform->func_id.fqParams.inputPerChannel ? i : 0;
                auto outputChannel = transform->func_id.fqParams.outputPerChannel ? i : 0;

                double input_low   = transform->func_id.fqParams.input_low[inputChannel];
                double input_high  = transform->func_id.fqParams.input_high[inputChannel];
                double output_low  = transform->func_id.fqParams.output_low[outputChannel];
                double output_high = transform->func_id.fqParams.output_high[outputChannel];

                for (uint32_t j = num_col_start; j <= num_col_end; j++) {
                    auto offset = i * num_columns + j;
                    auto x = ptr_in[offset];

                    if (x <= std::min(input_low, input_high)) {
                        ptr_out[offset] = output_low;
                    } else if (x > std::max(input_low, input_high)) {
                        ptr_out[offset] = output_high;
                    } else {
                        ptr_out[offset] = nearbyint((x - input_low) / (input_high - input_low) * (levels - 1)) /
                            (levels - 1) * (output_high - output_low) + output_low;
                    }
                }
            }
            break;
        }
        case kActCustom:
        default:
            THROW_GNA_EXCEPTION << component->original_layer_name << ", Unknown piecewise linear function type: " << transform->func_id.type;
    }
}
