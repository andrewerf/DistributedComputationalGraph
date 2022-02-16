#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONS_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONS_H

#include "TensorCodecs.h"


TensorWithMeta sumOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta minusOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta prodOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta powOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta sqrOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta sumAxisOp(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta sumAll(std::vector<TensorWithMeta> &&encodedTensors);
TensorWithMeta meanAll(std::vector<TensorWithMeta> &&encodedTensors);


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

TensorWithMeta reduceInputsOp(std::vector<TensorWithMeta> &&encodedTensors, const auto &bfunc)
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
