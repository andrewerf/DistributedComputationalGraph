#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONS_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONS_H

#include <variant>

#include <unsupported/Eigen/CXX11/Tensor>

#include "Node.h"



using TensorWithMeta = std::pair<std::string, MetaData>;

using VarTensor =
        std::variant<Eigen::Tensor<int32_t, 1>, Eigen::Tensor<float, 1>,
                     Eigen::Tensor<int32_t, 2>, Eigen::Tensor<float, 2>,
                     Eigen::Tensor<int32_t, 3>, Eigen::Tensor<float, 3>,
                     Eigen::Tensor<int32_t, 4>, Eigen::Tensor<float, 4>>;


VarTensor decodeTensor(TensorWithMeta &&tensorWithMeta);

TensorWithMeta encodeTensor(VarTensor &&tensor);


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

TensorWithMeta sumOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta prodOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta powOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta sqrOp(std::vector<TensorWithMeta> &&encodedTensors);


template <typename ...TArgs>
VarTensor magicVisit(const auto &func, TArgs&& ...tensors)
{
    return std::visit(overloaded {
        [&func] (auto&& ...args) -> decltype(VarTensor(func(std::move(args)...)))
        {
            return func(std::move(args)...);
        },
        [](...) -> VarTensor
        {
            throw std::runtime_error("Bad args");
        }
    }, std::forward<TArgs>(tensors)...);
}

TensorWithMeta reduceOp(std::vector<TensorWithMeta> &&encodedTensors, const auto &bfunc)
{
    std::vector<VarTensor> tensors;
    tensors.reserve(encodedTensors.size());
    std::transform(std::make_move_iterator(encodedTensors.begin()),
                   std::make_move_iterator(encodedTensors.end()),
                   std::back_inserter(tensors),
                   decodeTensor);

    VarTensor result = std::move(tensors[0]);
    for(size_t i = 1; i < tensors.size(); ++i)
    {
        result = magicVisit(bfunc, std::move(result), std::move(tensors[i]));
    }

    return encodeTensor(std::move(result));
}


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONS_H
