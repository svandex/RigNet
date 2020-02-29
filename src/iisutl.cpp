#include "config.h"

TV::Utility::MultiDataParser::MultiDataParser(std::vector<char>& _httpbody,std::string _boundary){
    m_body = _httpbody;
    m_boundary = _boundary;

    m_boundaries.push_back(0);

    //check /r/n
    for (auto ci = _httpbody.begin(); ci != _httpbody.end(); ci++)
    {
        if (*ci == '\r' && *(ci + 1) == '\n')
        {
            m_rnPositions.push_back(std::distance(_httpbody.begin(), ci));
        }
    }

    //check element positions
    for (auto ci = m_rnPositions.begin(); ci != m_rnPositions.end(); ci++)
    {
        //at least 3 /r/n to assure one element
        if (ci == m_rnPositions.end() - 1)
        {
            break;
        }
        
        //double /r/n
        if (*(ci + 1) - *ci == 2)
        {
            m_elePositions.push_back(*(ci + 1) + 2);
            m_elePositionEnds.push_back(*(ci + 2) - 1);
            m_boundaries.push_back(*(ci + 2));
        }
    }

    //In certain circumstance IIS may truncate characters
    //Check if the last /r/n is at the position of httpbody.size()-1
    if (m_rnPositions.size() == 0 || m_elePositions.size() == 0 || m_rnPositions.back() != _httpbody.size() - 1)
    {
        m_isConstruted = false;
    }
    else
    {
        m_isConstruted = true;
    }
}

std::string TV::Utility::MultiDataParser::metadata(){
    if(!m_isConstruted){
        return std::string("construction failed");
    }

    rapidjson::Document _metadata;
    _metadata.SetObject();
    rapidjson::Document::AllocatorType &_dallocator=_metadata.GetAllocator();

    rapidjson::Value _result(rapidjson::kArrayType);

    //loop each element
    for (size_t index = 0; index < m_elePositions.size(); index++)
    {
        rapidjson::Value one_element(rapidjson::kObjectType);
        for (size_t rindex = 0; rindex < m_rnPositions.size(); rindex++)
        {
            //within boudaries and before each element
            if (m_rnPositions[rindex] > m_boundaries[index] && m_rnPositions[rindex] < m_elePositions[index])
            {
				std::vector<char> temp;
				temp.clear();

                //from right after boundary to right before element start position
                temp.assign(m_body.cbegin() + m_rnPositions[rindex] + 2, m_body.cbegin() + m_elePositions[index]);
                if (temp.size() == 0)
				{
					continue;
				}
                
                //store each element's metadata into json object
				std::cmatch cm;
				if (std::regex_search(temp.data(), cm, std::regex("filename=\".+\"")))
				{
					if (cm.size() > 0)
					{
						//store image
                        auto _tmp=cm[0].str().substr(10);
                        one_element.AddMember("filename", rapidjson::Value(_tmp.substr(0, _tmp.size() - 1).c_str(), _dallocator), _dallocator);
                    }
                }

                if (std::regex_search(temp.data(), cm, std::regex("name=\"[^\"]+")))
                {
					if (cm.size() > 0)
					{
						//store image
                        one_element.AddMember("name", rapidjson::Value(cm[0].str().substr(6).c_str(), _dallocator), _dallocator);
                    }
                }

                //if exist Content-Type then the element should be stored
                if (std::regex_search(temp.data(), cm, std::regex("Content-Type")))
                {
					if (cm.size() > 0)
					{
						//store image
                        one_element.AddMember("storable", rapidjson::Value(true), _dallocator);
                    }
                }

                //start and end positions
                one_element.AddMember("start", rapidjson::Value(m_elePositions[index]), _dallocator);
                one_element.AddMember("end", rapidjson::Value(m_elePositionEnds[index]), _dallocator);
            }
        }
        _result.PushBack(one_element.Move(), _dallocator);
    }

    _metadata.AddMember("metadata", _result.Move(), _dallocator);

    //json object to string
    rapidjson::StringBuffer _s;
    rapidjson::Writer<rapidjson::StringBuffer> _w(_s);
    _metadata.Accept(_w);

    return std::string(_s.GetString());
}

bool TV::Utility::MultiDataParser::store(size_t index){
	auto envValue = Svandex::tools::GetEnvVariable(TV_PROJECT_NAME);
	auto file_uuid = Svandex::tools::GetUUID();
	if (envValue.size() > 0)
	{
		auto saved_path = std::string(envValue[0]) + "\\img\\" + file_uuid + ".jpg";

		/* store body to a file*/
		std::fstream uploadFile(saved_path, std::ios::binary | std::ios::out);
		if (uploadFile.is_open())
		{
			uploadFile.write(m_body.data() + m_elePositions[index], m_elePositionEnds[index] - m_elePositions[index]);
			uploadFile.flush();
			uploadFile.close();
		}
		else
		{
            return false;
		}
	}
	else
	{
        return false;
	}
	return true;
}