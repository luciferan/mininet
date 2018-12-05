#include "util_String.h"

//
std::string FormatA(const char *pFormat, ...)
{
	char szBuffer[eString::BUFFER_MAX + 1];
	memset(szBuffer, 0, sizeof(szBuffer));

	va_list args;
	va_start(args, pFormat);
	_vsnprintf_s(szBuffer, eString::BUFFER_MAX, eString::BUFFER_MAX, pFormat, args);
	va_end(args);

	return std::string(szBuffer);
}

std::wstring FormatW(const WCHAR *pFormat, ...)
{
	WCHAR wcsBuffer[eString::BUFFER_MAX + 1];
	memset(wcsBuffer, 0, sizeof(wcsBuffer));

	va_list args;
	va_start(args, pFormat);
	_vsnwprintf_s(wcsBuffer, eString::BUFFER_MAX, eString::BUFFER_MAX, pFormat, args);
	va_end(args);

	return std::wstring(wcsBuffer);
}

void TokenizeA(string &str, vector<string> &tokens, string delimiter)
{
	auto lastPos = str.find_first_not_of(delimiter, 0);
	auto pos = str.find_first_of(delimiter, lastPos);

	while( string::npos != pos || string::npos != lastPos )
	{
		tokens.push_back(str.substr(lastPos, pos-lastPos));

		lastPos = str.find_first_not_of(delimiter, pos);
		pos = str.find_first_of(delimiter, lastPos);
	}
}

void TokenizeW(wstring &str, vector<wstring> &tokens, wstring delimiter)
{
	auto lastPos = str.find_first_not_of(delimiter, 0);
	auto pos = str.find_first_of(delimiter, lastPos);

	while( wstring::npos != pos || wstring::npos != lastPos )
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		lastPos = str.find_first_not_of(delimiter, pos);
		pos = str.find_first_of(delimiter, lastPos);
	}
}

//
using namespace _json;

void json_parser_simple::node::set(string str, int dept)
{
	this->dept = dept;

	std::transform(str.begin(), str.end(), str.begin(), [](int c){if( '"' == c ) return ' '; return (char)::tolower(c);});

	auto str_len = str.length();

	auto key_s = str.find_first_not_of(_json::delimiter_space, 0);
	auto key_e = str.find_first_of(_json::delimiter_key, key_s);
	auto val_s = str.find_first_not_of(_json::delimiter_key, key_e);
	auto val_e = str.find_last_not_of(_json::delimiter_space);

	bool novalue = false;
	if( key_e == val_e )
		novalue = true;

	key_e = str.find_last_not_of(_json::delimiter_key, key_e);
	key_e = str.find_last_not_of(_json::delimiter_space, key_e);
	if( string::npos == key_s || string::npos == key_e )
		return;
	key = str.substr(key_s, key_e-key_s+1);

	val_s = str.find_first_not_of(_json::delimiter_space, val_s);
	if( string::npos == val_s || string::npos == val_e )
		return;
	value = str.substr(val_s, val_e-val_s+1);
}

json_parser_simple::node* json_parser_simple::node::find(string findkey)
{
	if( 0 >= findkey.length() )
		return nullptr;
	if( 0 >= node_list.size() )
		return nullptr;

	for( json_parser_simple::node *pCurrent : node_list )
	{
		if( pCurrent->checkKey(findkey) )
		{
			return pCurrent;
		}
	}

	return nullptr;
}

size_t json_parser_simple::parse()
{
	auto json_data_len = json_data.length();

	//
	int node_count = 0;
	{
		string::size_type pos = 0;
		
		do 
		{
			pos = json_data.find_first_of(delimiter_key, pos+1);
			node_count += 1;
		} while ( string::npos != pos );

		node_list.reserve(node_count-1);
	}

	//
	{
		int dept = 1;

		auto pos = json_data.find_first_not_of(delimiter_family_start, 0);
		auto node_end = json_data.find_first_of(delimiter_node, pos);

		auto family_start = json_data.find_first_of(delimiter_family_start, pos);
		auto family_end = string::npos;

		node *pCurrent = &node_first;

		while( 0 <= dept && string::npos != pos  )
		{
			if( family_start < node_end )
			{
				// 신규 패밀리 노드. 차일드 발견
				node *pData = new node;
				pData->set(json_data.substr(pos, family_start - pos), dept);

				//
				node_list.push_back(pData);
				pCurrent->addChild(pData);
				
				dept += 1;
				pCurrent = pData;

				//
				pos = json_data.find_first_not_of(delimiter_family_start, family_start);
				node_end = json_data.find_first_of(delimiter_node, pos);
				family_start = json_data.find_first_of(delimiter_family_start, pos);
				family_end = json_data.find_first_of(delimiter_family_end, pos);
			}
			else if( family_end < node_end )
			{
				// 패밀리 노드 종료.
				node *pData = new node;
				pData->set(json_data.substr(pos, family_end-pos), dept);
				
				//
				node_list.push_back(pData);
				pCurrent->addChild(pData);

				// 예외. 한번에 여러 세대의 패밀리가 종료
				node_end = json_data.find_first_of(delimiter_node, family_end);
				while( 1 <= dept && family_end < node_end )
				{
					dept -= 1;
					pCurrent = pCurrent->getParent();
					family_end = json_data.find_first_of(delimiter_family_end, family_end+1);
				}

				node_end = json_data.find_first_of(delimiter_node, pos);

				pos = json_data.find_first_not_of(delimiter_node, node_end);
				node_end = json_data.find_first_of(delimiter_node, pos);
				family_start = json_data.find_first_of(delimiter_family_start, pos);
				family_end = json_data.find_first_of(delimiter_family_end, pos);
			}
			else
			{
				node *pData = new node;
				pData->set(json_data.substr(pos, node_end-pos), dept);

				//
				node_list.push_back(pData);
				pCurrent->addChild(pData);

				//
				pos = json_data.find_first_not_of(delimiter_node, node_end);
				node_end = json_data.find_first_of(delimiter_node, pos);
			}
		}
	}

	//
	return node_list.size();
}

void json_parser_simple::_test()
{
	{
		string msg = "{""value0"":{""value2"":{""result"":""000""}}, ""value1"":""001""}";

		//
		json_parser_simple json(msg);
		auto data = json.getFirstNode();
	}

	return;
}