#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H

#include <cstdint>
#include <vector>

#include <rpc/msgpack.hpp>


namespace msgpack = clmdep_msgpack;
using TID = std::uint32_t;

enum class DataType
{
    int32, float32
};
MSGPACK_ADD_ENUM(DataType);

struct Node
{
    TID id;
    TID graphId;
    std::vector<TID> inputs;
    TID opId;

    MSGPACK_DEFINE_ARRAY(id, graphId, inputs, opId);
};

struct MetaData
{
    DataType dataType;
    int rank;

    MSGPACK_DEFINE_ARRAY(dataType, rank);
};


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H
