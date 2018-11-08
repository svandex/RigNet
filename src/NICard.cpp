#include "NICard.h"

int NICard::m_init_num = 0;
ViSession *NICard::defaultRM = new ViSession();

NICard::NICard(const char *remAddr)
{
    if (NICard::m_init_num==0)
    {
        run(tv::MakeRigFunctor_s(viOpenDefaultRM), NICard::defaultRM);
    }

    std::string res = "TCPIP0::" + std::string(remAddr) + "::SOCKET";
    //std::string res = "ASRL0::" + std::string(remAddr) + "::INSTR";
    //std::string res = "TCPIP::" + std::string(remAddr) + "::INSTR";

    run(tv::MakeRigFunctor_s(viOpen), *NICard::defaultRM, res.c_str(), VI_NULL, VI_NULL, &m_instr);
    NICard::m_init_num += 1;
}
