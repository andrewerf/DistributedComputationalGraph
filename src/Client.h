#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_CLIENT_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_CLIENT_H

#include <rpc/client.h>


class Client
{
public:
    explicit Client(const std::string &managerRpcHost);

    void speak();

private:
    rpc::client rpcClient;
};


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_CLIENT_H
