// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/op/op.hpp"

namespace ov {
namespace op {
namespace v1 {
class OPENVINO_API Reverse : public Op {
public:
    OPENVINO_OP("Reverse", "opset1", op::Op, 1);
    BWDCMP_RTTI_DECLARATION;

    enum class Mode { INDEX, MASK };

    Reverse() = default;
    /// \brief Constructs a reverse operation.
    ///
    /// \param data The input tensor, some of whose axes are to be reversed.
    /// \param reversed_axes The axes to reverse in a form of a set of indices or
    /// boolean mask.
    /// \param mode The way reversed_axes should be interpreted - a set or a mask.
    Reverse(const Output<Node>& data, const Output<Node>& reversed_axes, const std::string& mode);

    Reverse(const Output<Node>& data, const Output<Node>& reversed_axes, const Mode mode);

    bool visit_attributes(AttributeVisitor& visitor) override;
    void validate_and_infer_types() override;

    std::shared_ptr<Node> clone_with_new_inputs(const OutputVector& new_args) const override;

    /// \return The second input data interpretation mode.
    Mode get_mode() const {
        return m_mode;
    }
    void set_mode(const Mode mode) {
        m_mode = mode;
    }
    OPENVINO_SUPPRESS_DEPRECATED_START
    bool evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const override;
    OPENVINO_SUPPRESS_DEPRECATED_END
    bool has_evaluate() const override;

protected:
    Mode mode_from_string(const std::string& mode) const;

    /// \brief Indicates how the values from the second input should be interpreted.
    ///
    /// The second input can contain a set of indices pointing to axes in the data
    /// tensor shape.
    /// Alternatively it can contain a boolean mask that indicates which axes should be
    /// reversed.
    Mode m_mode;

private:
    bool evaluate_reverse(const HostTensorVector& outputs, const HostTensorVector& inputs) const;
};
}  // namespace v1
}  // namespace op

OPENVINO_API
std::ostream& operator<<(std::ostream& s, const op::v1::Reverse::Mode& type);

template <>
class OPENVINO_API AttributeAdapter<op::v1::Reverse::Mode> : public EnumAttributeAdapterBase<op::v1::Reverse::Mode> {
public:
    AttributeAdapter(op::v1::Reverse::Mode& value) : EnumAttributeAdapterBase<op::v1::Reverse::Mode>(value) {}

    OPENVINO_RTTI("AttributeAdapter<ov::op::v1::Reverse::Mode>");
    BWDCMP_RTTI_DECLARATION;
};

}  // namespace ov
