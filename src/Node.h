#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H

#include <cstdint>
#include <vector>

#include <rpc/msgpack.hpp>


using TID = std::uint32_t;

struct Node
{
    TID id;
    TID graphId;
    std::vector<TID> inputs;
    TID opId;

    MSGPACK_DEFINE_ARRAY(id, graphId, inputs, opId);
};


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_NODE_H
