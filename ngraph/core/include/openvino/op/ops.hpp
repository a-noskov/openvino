// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

// All OpenVINO Operation Headers

#pragma once

#include "openvino/op/abs.hpp"
#include "openvino/op/acos.hpp"
#include "openvino/op/acosh.hpp"
#include "openvino/op/adaptive_avg_pool.hpp"
#include "openvino/op/adaptive_max_pool.hpp"
#include "openvino/op/add.hpp"
#include "openvino/op/asin.hpp"
#include "openvino/op/asinh.hpp"
#include "openvino/op/assign.hpp"
#include "openvino/op/atan.hpp"
#include "openvino/op/atanh.hpp"
#include "openvino/op/avg_pool.hpp"
#include "openvino/op/batch_norm.hpp"
#include "openvino/op/batch_to_space.hpp"
#include "openvino/op/binary_convolution.hpp"
#include "openvino/op/broadcast.hpp"
#include "openvino/op/bucketize.hpp"
#include "openvino/op/ceiling.hpp"
#include "openvino/op/clamp.hpp"
#include "openvino/op/concat.hpp"
#include "openvino/op/constant.hpp"
#include "openvino/op/convert.hpp"
#include "openvino/op/convert_like.hpp"
#include "openvino/op/convolution.hpp"
#include "openvino/op/cos.hpp"
#include "openvino/op/cosh.hpp"
#include "openvino/op/ctc_greedy_decoder.hpp"
#include "openvino/op/ctc_greedy_decoder_seq_len.hpp"
#include "openvino/op/ctc_loss.hpp"
#include "openvino/op/cum_sum.hpp"
#include "openvino/op/deformable_convolution.hpp"
#include "openvino/op/deformable_psroi_pooling.hpp"
#include "openvino/op/depth_to_space.hpp"
#include "openvino/op/detection_output.hpp"
#include "openvino/op/dft.hpp"
#include "openvino/op/divide.hpp"
#include "openvino/op/einsum.hpp"
#include "openvino/op/elu.hpp"
#include "openvino/op/embedding_segments_sum.hpp"
#include "openvino/op/embeddingbag_offsets_sum.hpp"
#include "openvino/op/embeddingbag_packedsum.hpp"
#include "openvino/op/equal.hpp"
#include "openvino/op/erf.hpp"
#include "openvino/op/exp.hpp"
#include "openvino/op/experimental_detectron_detection_output.hpp"
#include "openvino/op/experimental_detectron_generate_proposals.hpp"
#include "openvino/op/experimental_detectron_prior_grid_generator.hpp"
#include "openvino/op/experimental_detectron_roi_feature.hpp"
#include "openvino/op/experimental_detectron_topkrois.hpp"
#include "openvino/op/extractimagepatches.hpp"
#include "openvino/op/fake_quantize.hpp"
#include "openvino/op/floor.hpp"
#include "openvino/op/floor_mod.hpp"
#include "openvino/op/gather.hpp"
#include "openvino/op/gather_elements.hpp"
#include "openvino/op/gather_nd.hpp"
#include "openvino/op/gather_tree.hpp"
#include "openvino/op/gelu.hpp"
#include "openvino/op/greater.hpp"
#include "openvino/op/greater_eq.hpp"
#include "openvino/op/grn.hpp"
#include "openvino/op/group_conv.hpp"
#include "openvino/op/gru_cell.hpp"
#include "openvino/op/gru_sequence.hpp"
#include "openvino/op/hard_sigmoid.hpp"
#include "openvino/op/hsigmoid.hpp"
#include "openvino/op/hswish.hpp"
#include "openvino/op/i420_to_bgr.hpp"
#include "openvino/op/i420_to_rgb.hpp"
#include "openvino/op/idft.hpp"
#include "openvino/op/if.hpp"
#include "openvino/op/interpolate.hpp"
#include "openvino/op/less.hpp"
#include "openvino/op/less_eq.hpp"
#include "openvino/op/log.hpp"
#include "openvino/op/log_softmax.hpp"
#include "openvino/op/logical_and.hpp"
#include "openvino/op/logical_not.hpp"
#include "openvino/op/logical_or.hpp"
#include "openvino/op/logical_xor.hpp"
#include "openvino/op/loop.hpp"
#include "openvino/op/lrn.hpp"
#include "openvino/op/lstm_cell.hpp"
#include "openvino/op/lstm_sequence.hpp"
#include "openvino/op/matmul.hpp"
#include "openvino/op/matrix_nms.hpp"
#include "openvino/op/max_pool.hpp"
#include "openvino/op/maximum.hpp"
#include "openvino/op/minimum.hpp"
#include "openvino/op/mish.hpp"
#include "openvino/op/mod.hpp"
#include "openvino/op/multiclass_nms.hpp"
#include "openvino/op/multiply.hpp"
#include "openvino/op/mvn.hpp"
#include "openvino/op/negative.hpp"
#include "openvino/op/non_max_suppression.hpp"
#include "openvino/op/non_zero.hpp"
#include "openvino/op/normalize_l2.hpp"
#include "openvino/op/not_equal.hpp"
#include "openvino/op/nv12_to_bgr.hpp"
#include "openvino/op/nv12_to_rgb.hpp"
#include "openvino/op/one_hot.hpp"
#include "openvino/op/pad.hpp"
#include "openvino/op/parameter.hpp"
#include "openvino/op/power.hpp"
#include "openvino/op/prelu.hpp"
#include "openvino/op/prior_box.hpp"
#include "openvino/op/prior_box_clustered.hpp"
#include "openvino/op/proposal.hpp"
#include "openvino/op/psroi_pooling.hpp"
#include "openvino/op/random_uniform.hpp"
#include "openvino/op/range.hpp"
#include "openvino/op/read_value.hpp"
#include "openvino/op/reduce_l1.hpp"
#include "openvino/op/reduce_l2.hpp"
#include "openvino/op/reduce_logical_and.hpp"
#include "openvino/op/reduce_logical_or.hpp"
#include "openvino/op/reduce_max.hpp"
#include "openvino/op/reduce_mean.hpp"
#include "openvino/op/reduce_min.hpp"
#include "openvino/op/reduce_prod.hpp"
#include "openvino/op/reduce_sum.hpp"
#include "openvino/op/region_yolo.hpp"
#include "openvino/op/relu.hpp"
#include "openvino/op/reorg_yolo.hpp"
#include "openvino/op/reshape.hpp"
#include "openvino/op/result.hpp"
#include "openvino/op/reverse.hpp"
#include "openvino/op/reverse_sequence.hpp"
#include "openvino/op/rnn_cell.hpp"
#include "openvino/op/rnn_sequence.hpp"
#include "openvino/op/roi_align.hpp"
#include "openvino/op/roi_pooling.hpp"
#include "openvino/op/roll.hpp"
#include "openvino/op/round.hpp"
#include "openvino/op/scatter_elements_update.hpp"
#include "openvino/op/scatter_nd_update.hpp"
#include "openvino/op/scatter_update.hpp"
#include "openvino/op/select.hpp"
#include "openvino/op/selu.hpp"
#include "openvino/op/shape_of.hpp"
#include "openvino/op/shuffle_channels.hpp"
#include "openvino/op/sigmoid.hpp"
#include "openvino/op/sign.hpp"
#include "openvino/op/sin.hpp"
#include "openvino/op/sinh.hpp"
#include "openvino/op/slice.hpp"
#include "openvino/op/softmax.hpp"
#include "openvino/op/softplus.hpp"
#include "openvino/op/space_to_batch.hpp"
#include "openvino/op/space_to_depth.hpp"
#include "openvino/op/split.hpp"
#include "openvino/op/sqrt.hpp"
#include "openvino/op/squared_difference.hpp"
#include "openvino/op/squeeze.hpp"
#include "openvino/op/strided_slice.hpp"
#include "openvino/op/subtract.hpp"
#include "openvino/op/swish.hpp"
#include "openvino/op/tan.hpp"
#include "openvino/op/tanh.hpp"
#include "openvino/op/tensor_iterator.hpp"
#include "openvino/op/tile.hpp"
#include "openvino/op/topk.hpp"
#include "openvino/op/transpose.hpp"
#include "openvino/op/unsqueeze.hpp"
#include "openvino/op/util/attr_types.hpp"
#include "openvino/op/util/op_types.hpp"
#include "openvino/op/variadic_split.hpp"
#include "openvino/op/xor.hpp"
