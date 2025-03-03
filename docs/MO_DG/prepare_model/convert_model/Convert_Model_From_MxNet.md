# Converting an MXNet* Model {#openvino_docs_MO_DG_prepare_model_convert_model_Convert_Model_From_MxNet}

A summary of the steps for optimizing and deploying a model that was trained with the MXNet\* framework:

1. [Configure the Model Optimizer](../../Deep_Learning_Model_Optimizer_DevGuide.md) for MXNet* (MXNet was used to train your model)
2. [Convert a MXNet model](#ConvertMxNet) to produce an optimized [Intermediate Representation (IR)](../../IR_and_opsets.md) of the model based on the trained network topology, weights, and biases values
3. Test the model in the Intermediate Representation format using the [OpenVINO™ Runtime](../../../OV_Runtime_UG/openvino_intro.md) in the target environment via provided [OpenVINO Samples](../../../OV_Runtime_UG/Samples_Overview.md)
4. [Integrate](../../../OV_Runtime_UG/Samples_Overview.md) the [OpenVINO™ Runtime](../../../OV_Runtime_UG/openvino_intro.md) in your application to deploy the model in the target environment

## Convert an MXNet* Model <a name="ConvertMxNet"></a>

To convert an MXNet\* model, run Model Optimizer with a path to the input model `.params` file and to an output directory where you have write permissions:

```sh
 mo --input_model model-file-0000.params --output_dir <OUTPUT_MODEL_DIR>
```

Two groups of parameters are available to convert your model:

* Framework-agnostic parameters are used to convert a model trained with any supported framework. For details, see the General Conversion Parameters section on the [Converting a Model to Intermediate Representation (IR)](Converting_Model.md) page.
* [MXNet-specific parameters](#mxnet_specific_conversion_params) are used to convert only MXNet models.


### Using MXNet\*-Specific Conversion Parameters <a name="mxnet_specific_conversion_params"></a>
The following list provides the MXNet\*-specific parameters.

```
MXNet-specific parameters:
  --input_symbol <SYMBOL_FILE_NAME>
            Symbol file (for example, "model-symbol.json") that contains a topology structure and layer attributes
  --nd_prefix_name <ND_PREFIX_NAME>
            Prefix name for args.nd and argx.nd files
  --pretrained_model_name <PRETRAINED_MODEL_NAME>
            Name of a pre-trained MXNet model without extension and epoch
            number. This model will be merged with args.nd and argx.nd
            files
  --save_params_from_nd
            Enable saving built parameters file from .nd files
  --legacy_mxnet_model
            Enable MXNet loader to make a model compatible with the latest MXNet version.
            Use only if your model was trained with MXNet version lower than 1.0.0
  --enable_ssd_gluoncv
            Enable transformation for converting the gluoncv ssd topologies.
            Use only if your topology is one of ssd gluoncv topologies
```

> **NOTE**: By default, the Model Optimizer does not use the MXNet loader, as it transforms the topology to another format, which is compatible with the latest
> version of MXNet, but it is required for models trained with lower version of MXNet. If your model was trained with MXNet version lower than 1.0.0, specify the
> `--legacy_mxnet_model` key to enable the MXNet loader. However, the loader does not support models with custom layers. In this case, you must manually
> recompile MXNet with custom layers and install it to your environment.

## Custom Layer Definition

Internally, when you run the Model Optimizer, it loads the model, goes through the topology, and tries to find each layer type in a list of known layers. Custom layers are layers that are not included in the list of known layers. If your topology contains any layers that are not in this list of known layers, the Model Optimizer classifies them as custom.

## Supported MXNet\* Layers
Refer to [Supported Framework Layers ](../Supported_Frameworks_Layers.md) for the list of supported standard layers.

## Frequently Asked Questions (FAQ)

The Model Optimizer provides explanatory messages if it is unable to run to completion due to issues like typographical errors, incorrectly used options, or other issues. The message describes the potential cause of the problem and gives a link to the [Model Optimizer FAQ](../Model_Optimizer_FAQ.md). The FAQ has instructions on how to resolve most issues. The FAQ also includes links to relevant sections in the Model Optimizer Developer Guide to help you understand what went wrong.

## Summary

In this document, you learned:

* Basic information about how the Model Optimizer works with MXNet\* models
* Which MXNet\* models are supported
* How to convert a trained MXNet\* model using the Model Optimizer with both framework-agnostic and MXNet-specific command-line options

## See Also
[Model Conversion Tutorials](Convert_Model_Tutorials.md)
