#include "Client.h"
#include "common.h"
#include "Node.h"
#include "Operations.h"

#include <rpc/rpc_error.h>

#include <plog/Log.h>


Client::Client(const std::string &managerRpcHost):
    rpcClient(extractHostname(managerRpcHost), extractRpcPort(managerRpcHost))
{}

void Client::speak()
{
    auto graphId = rpcClient.call("addGraphUnique").as<TID>();
    Node node{
        0, graphId, {}, 0
    };

    rpcClient.call("addNode", node);

    node.id = 1;
    node.inputs.push_back(0);
    rpcClient.call("addNode", node);

    Eigen::Tensor<float, 1> tensor(1);
    tensor(0) = 15;

    auto encodedTensor = encodeTensor(std::move(tensor));

    rpcClient.call("setInputValue", graphId, 0, encodedTensor.first, encodedTensor.second);


}