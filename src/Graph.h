#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_GRAPH_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_GRAPH_H

#include "Node.h"


class GraphAlreadyExists : public std::exception
{
public:
    const char* what() const noexcept override
    { return "Graph with such id already exists"; }
};

class NodeAlreadyExists : public std::exception
{
public:
    const char* what() const noexcept override
    { return "Node with such id already exists"; }
};

class GraphDoesNotExist : public std::exception
{
public:
    explicit GraphDoesNotExist(TID id):
        graphId(id),
        msg("Graph with id = " + std::to_string(id) + " doesn't exist")
    {}

    const char* what() const noexcept override
    { return msg.c_str(); }

    TID getId() const noexcept
    { return graphId; }

private:
    TID graphId;
    std::string msg;
};

class NodeDoesNotExist : public std::exception
{
public:
    explicit NodeDoesNotExist(TID id):
        nodeId(id),
        msg("Node with id = " + std::to_string(id) + " doesn't exist")
    {}

    const char* what() const noexcept override
    { return msg.c_str(); }

    TID getId() const noexcept
    { return nodeId; }

private:
    TID nodeId;
    std::string msg;
};



class Graph
{
public:
    explicit Graph(TID id): graphId(id) {}

    TID id() const
    { return graphId; }

    template <typename T>
    void addNode(T &&node);

    inline void setReady(TID nodeId);

    bool isReachable(TID nodeId) const
    { return reachableNodes.contains(nodeId); }

    bool isReady(TID nodeId) const
    { return readyNodes.contains(nodeId); }

    auto begin() const
    { return reachableNodes.begin(); }
    auto end() const
    { return reachableNodes.end(); }

    const Node& at(TID id) const
    { return nodes.at(id); }

private:
    TID graphId;
    std::unordered_map<TID, Node> nodes;
    std::unordered_map<TID, size_t> nodesInputs, nodesInputsReady;
    std::unordered_set<TID> reachableNodes, readyNodes;
};


template<typename T>
void Graph::addNode(T &&node)
{
    if(nodes.contains(node.id))
        throw NodeAlreadyExists();
    TID id = node.id;

    for(TID outId : node.outputs)
        nodesInputs[outId] += 1;

    nodesInputs.insert({id, 0});
    nodes.insert({id, std::forward<T>(node)});
}

void Graph::setReady(TID nodeId)
{
    auto nodeIt = nodes.find(nodeId);
    if(nodeIt == nodes.end())
        throw NodeDoesNotExist(nodeId);

    readyNodes.insert(nodeId);
    for(TID outId : nodeIt->second.outputs)
    {
        auto &rcount = nodesInputsReady.at(outId);
        if(++rcount == nodesInputs.at(outId))
            reachableNodes.insert(outId);
    }
}


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_GRAPH_H
