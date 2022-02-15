#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONSMANAGER_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONSMANAGER_H

#include "Operations.h"


using Operation = TensorWithMeta(*)(std::vector<TensorWithMeta> &&);

class OperationsManager
{
public:
    static OperationsManager* getInstance();

    std::string getOperationName(TID opId) const;
    TID getOperationId(const std::string &opName) const;

    Operation findOperation(TID opId) const;
    Operation findOperation(const std::string &opName) const;

private:
    OperationsManager();
    static inline OperationsManager* instance = nullptr;

    std::vector<std::pair<std::string, Operation>> operations;
    std::unordered_map<std::string, TID> operationsNames;
};


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_OPERATIONSMANAGER_H
