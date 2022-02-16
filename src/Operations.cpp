#include "Operations.h"

#include <iomanip>


static constexpr auto sumOpImpl =
    [] <typename T, int rank>
    (Eigen::Tensor<T, rank> &&a, Eigen::Tensor<T, rank> &&b) -> Eigen::Tensor<T, rank>
    {return a + b;};
TensorWithMeta sumOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    return reduceInputsOp(std::move(encodedTensors), sumOpImpl);
}

static constexpr auto minusOpImpl =
    [] <typename T, int rank>
    (Eigen::Tensor<T, rank> &&a, Eigen::Tensor<T, rank> &&b) -> Eigen::Tensor<T, rank>
    {return a - b;};
TensorWithMeta minusOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 2)
        throw std::runtime_error("Bad inputs count");

    return encodeTensor(magicVisit(minusOpImpl, decodeTensor(std::move(encodedTensors[0])), decodeTensor(std::move(encodedTensors[1]))));
}


static constexpr auto prodOpImpl =
    [] <typename T, int rank>
    (Eigen::Tensor<T, rank> &&a, Eigen::Tensor<T, rank> &&b) -> Eigen::Tensor<T, rank>
    {return a * b;};
TensorWithMeta prodOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    return reduceInputsOp(std::move(encodedTensors), prodOpImpl);
}

static constexpr auto powOpImpl =
    [] <typename T1, int rank1, typename T2>
    (Eigen::Tensor<T1, rank1> &&a, Eigen::Tensor<T2, 1> &&b) -> Eigen::Tensor<T1, rank1>
    {return a.pow(b(0));};
TensorWithMeta powOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 2)
        throw std::runtime_error("Bad inputs count");

    return reduceInputsOp(std::move(encodedTensors), powOpImpl);
}

TensorWithMeta sqrOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 1)
        throw std::runtime_error("Bad inputs count");

    auto t = decodeTensor(std::move(encodedTensors[0]));

    return encodeTensor(magicVisit([] <typename T, int rank> (Eigen::Tensor<T, rank> &&a) -> Eigen::Tensor<T, rank> {return a.pow(2);}, std::move(t)));
}

TensorWithMeta sumAxisOp(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 2)
        throw std::runtime_error("Bad inputs count");

    auto a = decodeTensor(std::move(encodedTensors[0]));
    auto ax = decodeTensor(std::move(encodedTensors[1]));

    if(!std::holds_alternative<Eigen::Tensor<int32_t, 1>>(ax))
        throw std::runtime_error("Bad tensor shape");

    return encodeTensor(magicVisit(
        [] <typename T1, typename T2, int rank> requires (rank >= 1) (Eigen::Tensor<T1, rank> &&a, Eigen::Tensor<T2, 1> &&axis) -> Eigen::Tensor<T1, rank - 1>
        {
            std::array<int32_t, 1> dims = {axis(0)};
            return a.sum(dims);
        },
        std::move(a), std::move(ax)
    ));
}

static constexpr auto sumAllImpl =
    [] <typename T, int rank>
    (Eigen::Tensor<T, rank> &&a) -> Eigen::Tensor<T, 0>
    {return a.sum();};
TensorWithMeta sumAll(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 1)
        throw std::runtime_error("Bad inputs count");

    return encodeTensor(magicVisit(sumAllImpl, decodeTensor(std::move(encodedTensors[0]))));
}

static constexpr auto meanAllImpl =
    [] <typename T, int rank>
    (Eigen::Tensor<T, rank> &&a) -> Eigen::Tensor<T, 0>
    {return a.mean();};
TensorWithMeta meanAll(std::vector<TensorWithMeta> &&encodedTensors)
{
    if(encodedTensors.size() != 1)
        throw std::runtime_error("Bad inputs count");

    return encodeTensor(magicVisit(meanAllImpl, decodeTensor(std::move(encodedTensors[0]))));
}
