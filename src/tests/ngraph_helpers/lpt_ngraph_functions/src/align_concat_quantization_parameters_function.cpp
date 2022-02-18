// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "lpt_ngraph_functions/align_concat_quantization_parameters_function.hpp"

#include <ngraph/opsets/opset1.hpp>
#include <ngraph_ops/type_relaxed.hpp>

#include "low_precision/network_helper.hpp"
#include "lpt_ngraph_functions/common/builders.hpp"
#include "ngraph_functions/subgraph_builders.hpp"

namespace ngraph {
namespace builder {
namespace subgraph {

std::shared_ptr<ngraph::Function> AlignConcatQuantizationParametersFunction::getOriginal(
    const ngraph::element::Type precision,
    const ngraph::element::Type inputPrecision,
    const ngraph::Shape& inputShape,
    const bool addFQ,
    const std::string additionalLayer,
    const ngraph::builder::subgraph::DequantizationOperations& dequantizationBefore) {
    const auto input1 = std::make_shared<ngraph::opset1::Parameter>(inputPrecision, ngraph::Shape(inputShape));
    std::shared_ptr<ngraph::Node> parent1 = input1;
    {
        parent1 = ngraph::builder::makeFakeQuantize(input1, precision, 256, {}, { -1.28 }, { 1.27 }, { -1.28 }, { 1.27 });
        parent1->set_friendly_name("fakeQuantizeOnActivations1");

        parent1 = std::make_shared<ngraph::opset1::AvgPool>(
            parent1,
            Strides{ 1, 1 },
            Shape{ 1, 1 },
            Shape{ 0, 0 },
            Shape{ 2, 2 },
            true,
            op::RoundingType::FLOOR);
        parent1->set_friendly_name("avgPool1");

        if (additionalLayer == "maxpool") {
            parent1 = std::make_shared<ngraph::opset1::MaxPool>(
                parent1,
                Strides{ 1, 1 },
                Shape{ 1, 1 },
                Shape{ 0, 0 },
                Shape{ 2, 2 },
                op::RoundingType::FLOOR);
            parent1->set_friendly_name("maxPool1");
        }

        if (addFQ) {
            parent1 = ngraph::builder::makeFakeQuantize(parent1, precision, 256, {}, { 0 }, { 255 }, { 0 }, { 255 });
            parent1->set_friendly_name("lastFakeQuantize1");
        }
    }

    const auto input2 = std::make_shared<ngraph::opset1::Parameter>(inputPrecision, ngraph::Shape(inputShape));
    std::shared_ptr<ngraph::Node> parent2 = input2;
    {
        parent2 = ngraph::builder::makeFakeQuantize(input1, precision, 256, {}, { -1.28f / 2.f }, { 1.27f / 2.f }, { -1.28f / 2.f }, { 1.27f / 2.f });
        parent2->set_friendly_name("fakeQuantizeOnActivations2");

        parent2 = std::make_shared<ngraph::opset1::AvgPool>(
            parent2,
            Strides{ 1, 1 },
            Shape{ 1, 1 },
            Shape{ 0, 0 },
            Shape{ 2, 2 },
            true,
            op::RoundingType::FLOOR);
        parent2->set_friendly_name("avgPool2");

        if (additionalLayer == "maxpool") {
            parent2 = std::make_shared<ngraph::opset1::MaxPool>(
                parent2,
                Strides{ 1, 1 },
                Shape{ 1, 1 },
                Shape{ 0, 0 },
                Shape{ 2, 2 },
                op::RoundingType::FLOOR);
            parent2->set_friendly_name("maxPool2");
        }

        if (addFQ) {
            parent2 = ngraph::builder::makeFakeQuantize(parent1, precision, 256, {}, { 0 }, { 255 }, { 0 }, { 255 });
            parent2->set_friendly_name("lastFakeQuantize2");
        }
    }
    auto parent = std::dynamic_pointer_cast<ngraph::Node>(std::make_shared<opset1::Concat>(ngraph::OutputVector{ parent1, parent2 }, 1));
    parent->set_friendly_name("concat");

    {
        const size_t outputChannels = 9ul;
        const size_t inputChannels = 6ul;
        const auto shape = Shape{ outputChannels, inputChannels, 1, 1 };
        const auto fakeQuantizeOnWeights = ngraph::builder::makeFakeQuantize(
            std::make_shared<opset1::Constant>(element::f32, shape, std::vector<float>(1.f, ngraph::shape_size(shape))),
            precision,
            255,
            {outputChannels, 1, 1, 1},
            std::vector<float>(outputChannels, -1.27f),
            std::vector<float>(outputChannels, 1.27f),
            std::vector<float>(outputChannels, -1.27f),
            std::vector<float>(outputChannels, 1.27f));
        fakeQuantizeOnWeights->set_friendly_name("fakeQuantizeOnWeights");

        parent = std::make_shared<ngraph::opset1::Convolution>(
            ngraph::op::TemporaryReplaceOutputType(parent, precision).get(),
            ngraph::op::TemporaryReplaceOutputType(fakeQuantizeOnWeights, precision).get(),
            ngraph::Strides{ 1, 1 },
            ngraph::CoordinateDiff{ 0, 0 },
            ngraph::CoordinateDiff{ 0, 0 },
            ngraph::Strides{ 1, 1 });

        parent->set_friendly_name("convolution");
    }

    parent->set_friendly_name("output");

    ngraph::ResultVector results{ std::make_shared<ngraph::opset1::Result>(parent) };
    return std::make_shared<ngraph::Function>(results, ngraph::ParameterVector{ input1, input2 }, "AlignConcatQuantizationParameters");
}

std::shared_ptr<ngraph::Function> AlignConcatQuantizationParametersFunction::getReference(
    const ngraph::element::Type precision,
    const ngraph::element::Type inputPrecision,
    const ngraph::Shape& inputShape,
    const bool addFQ,
    const std::string additionalLayer,
    const ngraph::builder::subgraph::DequantizationOperations& dequantizationBefore,
    const ngraph::element::Type precisionAfterOperation,
    const ngraph::builder::subgraph::DequantizationOperations& dequantizationAfter) {
    const auto input1 = std::make_shared<ngraph::opset1::Parameter>(inputPrecision, ngraph::Shape(inputShape));
    std::shared_ptr<ngraph::Node> parent1 = input1;
    {
        FakeQuantizeOnData onData = { 256, {}, { -1.28f }, { 1.27f }, { 0.f }, { 255.f }, ngraph::element::u8};
        parent1 = makeFakeQuantizeTypeRelaxed(input1, element::f32, onData);
        ngraph::pass::low_precision::NetworkHelper::setOutDataPrecisionForTypeRelaxed(parent1, element::u8);
        parent1->set_friendly_name("fakeQuantizeOnActivations1");

        parent1 = std::make_shared<ngraph::opset1::AvgPool>(
            parent1,
            Strides{ 1, 1 },
            Shape{ 1, 1 },
            Shape{ 0, 0 },
            Shape{ 2, 2 },
            true,
            op::RoundingType::FLOOR);
        parent1->set_friendly_name("avgPool1");

        if (additionalLayer == "maxpool") {
            parent1 = std::make_shared<ngraph::opset1::MaxPool>(
                parent1,
                Strides{ 1, 1 },
                Shape{ 1, 1 },
                Shape{ 0, 0 },
                Shape{ 2, 2 },
                op::RoundingType::FLOOR);
            parent1->set_friendly_name("maxPool1");
        }

        if (addFQ) {
            parent1 = ngraph::builder::makeFakeQuantize(parent1, precision, 256, {}, { 0 }, { 255 }, { 0 }, { 255 });
            parent1->set_friendly_name("lastFakeQuantize1");
        }
    }

    const auto input2 = std::make_shared<ngraph::opset1::Parameter>(inputPrecision, ngraph::Shape(inputShape));
    std::shared_ptr<ngraph::Node> parent2 = input2;
    {
        FakeQuantizeOnData onData = { 256, {}, { -0.64f }, { 0.635f }, { 64.f }, { 192.f }, element::u8};
        parent2 = makeFakeQuantizeTypeRelaxed(input2, element::f32, onData);
        ngraph::pass::low_precision::NetworkHelper::setOutDataPrecisionForTypeRelaxed(parent2, element::u8);
        parent2->set_friendly_name("fakeQuantizeOnActivations2");

        parent2 = std::make_shared<ngraph::opset1::AvgPool>(
            parent2,
            Strides{ 1, 1 },
            Shape{ 1, 1 },
            Shape{ 0, 0 },
            Shape{ 2, 2 },
            true,
            op::RoundingType::FLOOR);
        parent2->set_friendly_name("avgPool2");

        if (additionalLayer == "maxpool") {
            parent2 = std::make_shared<ngraph::opset1::MaxPool>(
                parent2,
                Strides{ 1, 1 },
                Shape{ 1, 1 },
                Shape{ 0, 0 },
                Shape{ 2, 2 },
                op::RoundingType::FLOOR);
            parent2->set_friendly_name("maxPool2");
        }

        if (addFQ) {
            parent2 = ngraph::builder::makeFakeQuantize(parent1, precision, 256, {}, { 0 }, { 255 }, { 0 }, { 255 });
            parent2->set_friendly_name("lastFakeQuantize2");
        }
    }
    auto parent = std::dynamic_pointer_cast<ngraph::Node>(std::make_shared<opset1::Concat>(ngraph::OutputVector{ parent1, parent2 }, 1));
    parent->set_friendly_name("concat");

    if (!dequantizationBefore.empty()) {
        parent = makeDequantization(parent, dequantizationBefore);
    }

    {
        const size_t outputChannels = 9ul;
        const size_t inputChannels = 6ul;
        const auto shape = Shape{ outputChannels, inputChannels, 1, 1 };
        const auto onWeights = std::make_shared<opset1::Constant>(
            element::i8,
            shape,
            std::vector<size_t>(outputChannels * inputChannels, 127));

        parent = std::make_shared<ngraph::opset1::Convolution>(
            ngraph::op::TemporaryReplaceOutputType(parent, precision).get(),
            ngraph::op::TemporaryReplaceOutputType(onWeights, precision).get(),
            ngraph::Strides{ 1, 1 },
            ngraph::CoordinateDiff{ 0, 0 },
            ngraph::CoordinateDiff{ 0, 0 },
            ngraph::Strides{ 1, 1 });

        parent->set_friendly_name("convolution");
    }

    if (!dequantizationAfter.empty()) {
        parent = makeDequantization(parent, dequantizationAfter);
    }

    parent->set_friendly_name("output");

    ngraph::ResultVector results{ std::make_shared<ngraph::opset1::Result>(parent) };
    return std::make_shared<ngraph::Function>(results, ngraph::ParameterVector{ input1, input2 }, "AlignConcatQuantizationParameters");
}

}  // namespace subgraph
}  // namespace builder
}  // namespace ngraph
