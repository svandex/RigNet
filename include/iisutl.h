#ifndef __IISUTL_H__
#define __IISUTL_H__
#include "config.h"

namespace TV
{
namespace Utility
{
static std::string GetServerVariable(const char *vName, IHttpContext *pHttpContext)
{
    PCSTR _pcstr;
    DWORD _dword;
    if (pHttpContext->GetServerVariable(vName, &_pcstr, &_dword) == S_OK)
    {
        return std::string(_pcstr);
    }
    else
    {
        return "";
    }
};

template <typename T>
void ReadEntity(IHttpContext *pHttpContext, std::vector<T> &buf)
{
    DWORD _dword;
    auto cLength = GetServerVariable("CONTENT_LENGTH", pHttpContext);
    if (std::is_signed<T>::value)
    {
        buf.resize(std::atoi(cLength.c_str()) + 1);
    }
    else
    {
        buf.resize(std::atoi(cLength.c_str()));
    }
    pHttpContext->GetRequest()->ReadEntityBody(buf.data(), std::atoi(cLength.c_str()), FALSE, &_dword);
}

/*
This class is used to parse multidata
*/
class MultiDataParser
{
public:
	MultiDataParser() = delete;
	MultiDataParser(std::vector<char> &, std::string);

	//extract parameter
    std::string metadata();

    //store element
    bool store(size_t index);

    //whether parsing successfully
    bool constructed(){
        return m_isConstruted;
    }

private:
    //if construtor succeed
	bool m_isConstruted = false;
	//http body
	std::vector<char> m_body;
	//boundary string
	std::string m_boundary;

	//positions for boundaries
	std::vector<uint64_t> m_boundaries;
	//positions vector for \r\n
	std::vector<uint64_t> m_rnPositions;

	//positions vector for element which is after \r\n\r\n
	std::vector<uint64_t> m_elePositions;
	std::vector<uint64_t> m_elePositionEnds;
};

}; // namespace Utility
} // namespace TV

#endif