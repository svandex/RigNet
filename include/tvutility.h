#ifndef __TVUTILITY_H__
#define __TVUTILITY_H__
#include "precomp.h"

namespace TV
{
class Utility
{
public:
    std::string GetServerVariable(const char *vName, IHttpContext *pHttpContext)
    {
        if (pHttpContext->GetServerVariable(vName, &m_tempcsTR, &m_tempDWORD) == S_OK)
        {
            return std::string(m_tempcsTR);
        }
        else
        {
            return "";
        }
    };

    template <typename T>
    void ReadEntity(IHttpContext *pHttpContext, std::vector<T> &buf);

private:
    PCSTR m_tempcsTR;
    DWORD m_tempDWORD;
};
} // namespace TV

template <typename T>
void TV::Utility::ReadEntity(IHttpContext *pHttpContext, std::vector<T> &buf)
{
	auto cLength = GetServerVariable("CONTENT_LENGTH", pHttpContext);
	if (std::is_signed<T>::value)
	{
		buf.resize(std::atoi(cLength.c_str()) + 1);
	}
	else
	{
		buf.resize(std::atoi(cLength.c_str()));
	}
	pHttpContext->GetRequest()->ReadEntityBody(buf.data(), std::atoi(cLength.c_str()), FALSE, &m_tempDWORD);
}
#endif