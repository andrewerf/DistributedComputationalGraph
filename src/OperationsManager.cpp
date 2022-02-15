#include "OperationsManager.h"


OperationsManager::OperationsManager()
{
    operations = {
        {"sumOp", sumOp},
        {"prodOp", prodOp},
        {"powOp", powOp},
        {"sqrOp", sqrOp}
    };

    for(size_t i = 0; i < operations.size(); ++i)
        operationsNames.insert({operations[i].first, i});
}

OperationsManager *OperationsManager::getInstance()
{
    if(instance == nullptr)
        instance = new OperationsManager;
    return instance;
}

std::string OperationsManager::getOperationName(TID opId) const
{
    return operations.at(opId).first;
}

TID OperationsManager::getOperationId(const std::string &opName) const
{
    return operationsNames.at(opName);
}

Operation OperationsManager::findOperation(TID opId) const
{
    return operations.at(opId).second;
}

Operation OperationsManager::findOperation(const std::string &opName) const
{
    return operations.at(getOperationId(opName)).second;
}