#include "SimensPLC.h"

SimensPLC::SimensPLC(const char *remAddr, int rack, int slot)
{
    client = new TS7Client();
    int result = client->ConnectTo(remAddr, rack, slot);
    std::cout<<result;
    if (!client->Connected())
    {
        std::cout << "Err! " << CliErrorText(client->LastError()) << std::endl;
    }
}
