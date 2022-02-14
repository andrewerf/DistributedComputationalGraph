#include "TensorCodecs.h"


template <typename T>
constexpr DataType to_dt()
{
    if constexpr (std::is_same_v<T, int32_t>)
        return DataType::int32;
    else if constexpr (std::is_same_v<T, float>)
        return DataType::float32;
    else
        throw std::logic_error("Trying to convert wrong type");
}


// Tensor <-> msgpack converters
template <typename T, int rank>
struct msgpack::adaptor::convert<Eigen::Tensor<T, rank>> {
    msgpack::object const& operator()(msgpack::object const &object, Eigen::Tensor<T, rank> &tensor) const
    {
        auto sizes = object.via.array.ptr[0].as<std::array<typename Eigen::Tensor<T, rank>::Index, rank>>();
        auto data = object.via.array.ptr[1].as<std::vector<T>>();
        tensor = Eigen::TensorMap<Eigen::Tensor<T, rank>>(data.data(), sizes);
        return object;
    }
};

template <typename T, int rank>
struct msgpack::adaptor::pack<Eigen::Tensor<T, rank>> {
    template <typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream> &out, Eigen::Tensor<T, rank> const &tensor) const
    {
        out.pack_array(2);
        out.pack(static_cast<std::array<typename Eigen::Tensor<T, rank>::Index, rank>>(tensor.dimensions()));
        out.pack(std::vector<T>(tensor.data(), tensor.data() + tensor.size()));
        return out;
    }
};

template <typename T, int rank>
VarTensor decodeTensorTR(std::string &&encodedTensor)
{
    return msgpack::unpack(encodedTensor.c_str(), encodedTensor.size()).as<Eigen::Tensor<T, rank>>();
}

template <int rank>
VarTensor decodeTensorR(TensorWithMeta &&tensorWithMeta)
{
    switch (tensorWithMeta.second.dataType)
    {
        case DataType::int32:
            return decodeTensorTR<int32_t, rank>(std::move(tensorWithMeta.first));
        case DataType::float32:
            return decodeTensorTR<float, rank>(std::move(tensorWithMeta.first));
        default:
            throw std::runtime_error("Wrong DataType");
    }
}

VarTensor decodeTensor(TensorWithMeta &&tensorWithMeta)
{
    switch (tensorWithMeta.second.rank)
    {
        case 1:
            return decodeTensorR<1>(std::move(tensorWithMeta));
        case 2:
            return decodeTensorR<2>(std::move(tensorWithMeta));
        case 3:
            return decodeTensorR<3>(std::move(tensorWithMeta));
        case 4:
            return decodeTensorR<4>(std::move(tensorWithMeta));
        default:
            throw std::runtime_error("Wrong tensor rank");
    }
}

TensorWithMeta encodeTensor(VarTensor &&tensor)
{
    MetaData md;
    std::string encodedTensor = std::visit(overloaded {
            [&md] <typename T, int rank> (Eigen::Tensor<T, rank> &&t)
            {
                msgpack::sbuffer buffer;
                msgpack::pack(buffer, t);
                md.rank = rank;
                md.dataType = to_dt<T>();
                return std::string(buffer.data(), buffer.size());
            },
            [](auto t) -> std::string
            {
                throw std::logic_error("Trying to encode bad tensor");
            }
    }, std::move(tensor));

    return std::make_pair(std::move(encodedTensor), md);
}
