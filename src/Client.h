#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_CLIENT_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_CLIENT_H

#include "OperationsManager.h"

#include <rpc/client.h>


class Client
{
public:
    explicit Client(const std::string &managerRpcHost);

    void sendNode(const Node &node);

    void setInputValue(TID graphId, TID nodeId, const VarTensor &input);

    VarTensor getOutputValue(TID graphId, TID nodeId);

    std::unordered_map<TID, VarTensor>
        processGraph(const std::unordered_map<TID, VarTensor> &inputValues,
                     std::istream &graphStream,
                     const std::vector<TID> &outputs);

private:
    rpc::client rpcClient;
    OperationsManager *operationsManager;
};


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_CLIENT_H
