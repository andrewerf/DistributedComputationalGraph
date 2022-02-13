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


struct Operation
{
    virtual TensorWithMeta call(std::vector<TensorWithMeta> &&encodedTensors) const = 0;
};

struct SumOp : public Operation
{
    TensorWithMeta call(std::vector<TensorWithMeta> &&encodedTensors) const override;
};

struct SqrOp : public Operation
{
    TensorWithMeta call(std::vector<TensorWithMeta> &&encodedTensors) const override;
};



#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONS_H
