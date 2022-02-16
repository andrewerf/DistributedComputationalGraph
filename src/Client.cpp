#include "Client.h"
#include "common.h"
#include "Node.h"
#include "TensorCodecs.h"
#include <nlohmann/json.hpp>

#include <rpc/rpc_error.h>

#include <plog/Log.h>


Client::Client(const std::string &managerRpcHost):
    rpcClient(extractHostname(managerRpcHost), extractRpcPort(managerRpcHost)),
    operationsManager(OperationsManager::getInstance())
{}

void Client::sendNode(const Node &node)
{
    rpcClient.call("addNode", node);
}

void Client::setInputValue(TID graphId, TID nodeId, const VarTensor &input)
{
    VarTensor inputCopy = input;
    auto encodedTensor = encodeTensor(std::move(inputCopy));
    rpcClient.call("setInputValue", graphId, nodeId, encodedTensor.first, encodedTensor.second);
}

VarTensor Client::getOutputValue(TID graphId, TID nodeId)
{
    auto meta = rpcClient.call("getMetaData", graphId, nodeId).as<MetaData>();
    auto encodedTensor = rpcClient.call("getData", graphId, nodeId).as<std::string>();
    auto ret = decodeTensor(std::make_pair(std::move(encodedTensor), meta));
    return ret;
}

std::unordered_map<TID, VarTensor>
        Client::processGraph(const std::unordered_map<TID, VarTensor> &inputValues,
                             std::istream &graphStream,
                             const std::vector<TID> &outputs)
{
    auto graphId = rpcClient.call("addGraphUnique").as<TID>();

    auto json = nlohmann::json::parse(graphStream);
    std::vector<Node> nodes;
    for(const auto &jsonNode : json)
    {
        auto nodeId = jsonNode.find("id");
        auto inputs = jsonNode.find("inputs");
        auto opName = jsonNode.find("op");

        if(nodeId == jsonNode.end())
            throw std::runtime_error("Each node must specify id");

        Node node;
        nodeId->get_to(node.id);
        node.graphId = graphId;

        if(inputs == jsonNode.end() && !inputValues.contains(node.id))
            throw std::runtime_error("Node should either be input or has inputs specified");

        if(inputs != jsonNode.end())
            inputs->get_to(node.inputs);
        if(opName != jsonNode.end())
            node.opId = operationsManager->getOperationId(opName->get<std::string>());

        nodes.push_back(node);
    }

    for(const auto &node : nodes)
        sendNode(node);

    for(const auto &[nodeId, input] : inputValues)
        setInputValue(graphId, nodeId, input);


    std::unordered_map<TID, VarTensor> result;
    for(TID outId : outputs)
    {
        while(!rpcClient.call("isComputed", graphId, outId).as<bool>())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        result.emplace(outId, getOutputValue(graphId, outId));
    }

    return result;
}