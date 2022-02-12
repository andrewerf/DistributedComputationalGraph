#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H

#include <cstdint>
#include <vector>

#include <rpc/msgpack.hpp>


using TID = std::uint32_t;
using TData = clmdep_msgpack::object_bin;

struct Node
{
    TID id;
    TID graphId;
    std::vector<TID> outputs;
    TID opId;

    MSGPACK_DEFINE_ARRAY(id, graphId, outputs, opId);
};


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H
