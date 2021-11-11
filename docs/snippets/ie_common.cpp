// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <ie_core.hpp>

int main() {
    //! [ie:create_core]
    InferenceEngine::Core core;
    //! [ie:create_core]

    //! [ie:read_model]
    InferenceEngine::CNNNetwork network = core.ReadNetwork("model.xml");
    //! [ie:read_model]

    //! [ie:get_inputs_outputs]
    InferenceEngine::InputsDataMap inputs = network.getInputsInfo();
    InferenceEngine::OutputsDataMap outputs = network.getOutputsInfo();
    //! [ie:get_inputs_outputs]

    //! [ie:compile_model]
    InferenceEngine::ExecutableNetwork exec_network = core.LoadNetwork(network, "CPU");
    //! [ie:compile_model]

    //! [ie:create_infer_request]
    InferenceEngine::InferRequest infer_request = exec_network.CreateInferRequest();
    //! [ie:create_infer_request]

    //! [ie:get_input_tensor]
    InferenceEngine::Blob::Ptr input_blob1 = infer_request.GetBlob(inputs.begin()->first);
    // fill first blob
    InferenceEngine::SizeVector dims1 = input_blob1->getTensorDesc().getDims();
    InferenceEngine::MemoryBlob::Ptr minput1 = InferenceEngine::as<InferenceEngine::MemoryBlob>(input_blob1);
    if (minput1) {
        // locked memory holder should be alive all time while access to its
        // buffer happens
        auto minputHolder = minput1->wmap();
        // Original I64 precision was converted to I32
        auto data = minputHolder.as<InferenceEngine::PrecisionTrait<InferenceEngine::Precision::I32>::value_type*>();
        // Fill data ...
    }
    InferenceEngine::Blob::Ptr input_blob2 = infer_request.GetBlob("data2");
    // fill first blob
    InferenceEngine::MemoryBlob::Ptr minput2 = InferenceEngine::as<InferenceEngine::MemoryBlob>(input_blob2);
    if (minput2) {
        // locked memory holder should be alive all time while access to its
        // buffer happens
        auto minputHolder = minput2->wmap();
        // Original I64 precision was converted to I32
        auto data = minputHolder.as<InferenceEngine::PrecisionTrait<InferenceEngine::Precision::I32>::value_type*>();
        // Fill data ...
    }
    //! [ie:get_input_tensor]

    //! [ie:inference]
    infer_request.Infer();
    //! [ie:inference]

    //! [ie:get_output_tensor]
    InferenceEngine::Blob::Ptr output_blob = infer_request.GetBlob(outputs.begin()->first);
    InferenceEngine::MemoryBlob::Ptr moutput = InferenceEngine::as<InferenceEngine::MemoryBlob>(output_blob);
    if (moutput) {
        // locked memory holder should be alive all time while access to its
        // buffer happens
        auto minputHolder = moutput->rmap();
        // Original I64 precision was converted to I32
        auto data =
            minputHolder.as<const InferenceEngine::PrecisionTrait<InferenceEngine::Precision::I32>::value_type*>();
        // process output data
    }
    //! [ie:get_output_tensor]
    return 0;
}
