#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_TENSORCODECS_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_TENSORCODECS_H

#include "Node.h"

#include <variant>

#include <unsupported/Eigen/CXX11/Tensor>


using TensorWithMeta = std::pair<std::string, MetaData>;

using VarTensor =
    std::variant<Eigen::Tensor<int32_t, 1>, Eigen::Tensor<float, 1>,
                 Eigen::Tensor<int32_t, 2>, Eigen::Tensor<float, 2>,
                 Eigen::Tensor<int32_t, 3>, Eigen::Tensor<float, 3>,
                 Eigen::Tensor<int32_t, 4>, Eigen::Tensor<float, 4>>;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

VarTensor decodeTensor(TensorWithMeta &&tensorWithMeta);

TensorWithMeta encodeTensor(VarTensor &&tensor);

#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_TENSORCODECS_H
