#include "Operations.h"

#include <iomanip>


TensorWithMeta sumOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    return reduceOp(std::move(encodedTensors), std::plus<>());
}

TensorWithMeta prodOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    return reduceOp(std::move(encodedTensors), std::multiplies<>());
}

static constexpr auto powOpImpl =
    [] <typename T1, int rank1, typename T2>
    (Eigen::Tensor<T1, rank1> &&a, Eigen::Tensor<T2, 1> &&b) -> Eigen::Tensor<T1, rank1>
    {return a.pow(b(0));};
TensorWithMeta powOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 2)
        throw std::runtime_error("Bad inputs count");

    return reduceOp(std::move(encodedTensors), powOpImpl);
}

TensorWithMeta sqrOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 1)
        throw std::runtime_error("Bad inputs count");

    auto t = decodeTensor(std::move(encodedTensors[0]));

    return encodeTensor(magicVisit([] <typename T, int rank> (Eigen::Tensor<T, rank> &&a) -> Eigen::Tensor<T, rank> {return a.pow(2);}, std::move(t)));
}