// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "behavior/ov_plugin/caching_tests.hpp"
#include <ngraph_ops/nms_ie_internal.hpp>
#include <ngraph_ops/nms_static_shape_ie.hpp>

using namespace ov::test::behavior;
using namespace ngraph;

namespace {
    static const std::vector<ngraph::element::Type> precisionsCPU = {
            ngraph::element::f32,
            ngraph::element::f16,
            ngraph::element::i32,
            ngraph::element::i64,
            ngraph::element::i8,
            ngraph::element::u8,
            ngraph::element::i16,
            ngraph::element::u16,
    };

    static const std::vector<std::size_t> batchSizesCPU = {
            1, 2
    };

    static const std::vector<ngraph::element::Type> precisionsCPUInternal = {
            ngraph::element::f32
    };

    static const std::vector<std::size_t> batchSizesCPUInternal = {
            1
    };

    static std::shared_ptr<ngraph::Function> simple_function_non_max_supression_internal(ngraph::element::Type, size_t) {
        auto boxes = std::make_shared<ov::op::v0::Parameter>(element::f32, Shape{1, 1000, 4});
        auto scores = std::make_shared<ov::op::v0::Parameter>(element::f32, Shape{1, 1, 1000});
        auto max_output_boxes_per_class = ov::op::v0::Constant::create(element::i32, Shape{1}, {10});
        auto iou_threshold = ov::op::v0::Constant::create(element::f32, Shape{1}, {0.75});
        auto score_threshold = ov::op::v0::Constant::create(element::f32, Shape{1}, {0.7});
        auto nms = std::make_shared<op::internal::NonMaxSuppressionIEInternal>(boxes, scores, max_output_boxes_per_class,
                iou_threshold, score_threshold, 0, true, element::i32);
        auto res = std::make_shared<ov::op::v0::Result>(nms);
        auto func = std::make_shared<Function>(NodeVector{nms}, ParameterVector{boxes, scores});
        return func;
    }

    static std::shared_ptr<ngraph::Function> simple_function_matrix_nms_internal(ngraph::element::Type, size_t) {
        auto boxes = std::make_shared<ov::op::v0::Parameter>(element::f32, Shape{1, 1000, 4});
        auto scores = std::make_shared<ov::op::v0::Parameter>(element::f32, Shape{1, 1, 1000});
        ov::op::v8::MatrixNms::Attributes attr;
        // convert_precision does not support internal op 'NmsStaticShapeIE'
        attr.output_type = element::i32;
        auto nms = std::make_shared<op::internal::NmsStaticShapeIE<ov::op::v8::MatrixNms>>(boxes, scores, attr);
        auto res = std::make_shared<ov::op::v0::Result>(nms);
        auto func = std::make_shared<Function>(NodeVector{nms}, ParameterVector{boxes, scores});
        return func;
    }

    static std::shared_ptr<ngraph::Function> simple_function_multiclass_nms_internal(ngraph::element::Type, size_t) {
        auto boxes = std::make_shared<ov::op::v0::Parameter>(element::f32, Shape{1, 1000, 4});
        auto scores = std::make_shared<ov::op::v0::Parameter>(element::f32, Shape{1, 1, 1000});
        ov::op::v8::MulticlassNms::Attributes attr;
        attr.output_type = element::i32;
        auto nms = std::make_shared<op::internal::NmsStaticShapeIE<ov::op::v8::MulticlassNms>>(boxes, scores, attr);
        auto res = std::make_shared<ov::op::v0::Result>(nms);
        auto func = std::make_shared<Function>(NodeVector{nms}, ParameterVector{boxes, scores});
        return func;
    }

    static std::vector<ovModelWithName> internal_functions_cpu() {
        std::vector<ovModelWithName> funcs = {
            ovModelWithName { simple_function_non_max_supression_internal, "NonMaxSuppressionIEInternal"},
            ovModelWithName { simple_function_matrix_nms_internal, "NmsStaticShapeIE_MatrixNms"},
            ovModelWithName { simple_function_multiclass_nms_internal, "NmsStaticShapeIE_MulticlassNms"},
        };
        return funcs;
    }

    INSTANTIATE_TEST_SUITE_P(smoke_CachingSupportCase_CPU, CompileModelCacheTestBase,
                            ::testing::Combine(
                                    ::testing::ValuesIn(CompileModelCacheTestBase::getStandardFunctions()),
                                    ::testing::ValuesIn(precisionsCPU),
                                    ::testing::ValuesIn(batchSizesCPU),
                                    ::testing::Values(CommonTestUtils::DEVICE_CPU)),
                            CompileModelCacheTestBase::getTestCaseName);

    INSTANTIATE_TEST_SUITE_P(smoke_CachingSupportCase_CPU_Internal, CompileModelCacheTestBase,
                            ::testing::Combine(
                                    ::testing::ValuesIn(internal_functions_cpu()),
                                    ::testing::ValuesIn(precisionsCPUInternal),
                                    ::testing::ValuesIn(batchSizesCPUInternal),
                                    ::testing::Values(CommonTestUtils::DEVICE_CPU)),
                            CompileModelCacheTestBase::getTestCaseName);
} // namespace
