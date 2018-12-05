#pragma once
#ifndef __STRING_H__
#define __STRING_H__

//
#include <windows.h>

#include <algorithm>
#include <string>
#include <vector>
#include <list>

//
using namespace std;

enum eString
{
	BUFFER_MAX = 1024*10,
};

//
string FormatA(const char *pFormat, ...);
wstring FormatW(const WCHAR *pFormat, ...);

void TokenizeA(string &str, vector<string> &tokens, string delimiter);
void TokenizeW(wstring &str, vector<wstring> &tokens, wstring delimiter);

//
namespace _json
{
const string delimiter_family_start = "{";
const string delimiter_family_end = "}";
const string delimiter_node = ",";
const string delimiter_key = ":";
const string delimiter_space = " ";

class json_parser_simple
{
public:
	struct node
	{
		int dept = 0;

		//string str = {};
		string key = {};
		string value = {};

		node *pParent = nullptr;
		vector<node*> node_list = {};

		//
		node() {}
		~node() { pParent = nullptr; node_list.clear(); }

		//void set(string str, int dept) { this->str = str; this->dept = dept; }
		void set(string str, int dept);
		void addChild(node *pData) { pData->pParent = this; node_list.push_back(pData); }
		node* getParent() { return pParent; }

		node* find(string findkey);
		bool checkKey(string key) { return ( 0 == this->key.compare(key) ? true : false ); }

		string getKey() { return key; }
		string getValue() { return value; }
	};
	
public:
	string json_data = {};

	vector<node*> node_list = {};
	node node_first;

	//
public:
	json_parser_simple() {};
	virtual ~json_parser_simple()
	{
		for( node *p : node_list )
			delete p;

		node_list.clear();
		node_first.node_list.clear();
	};

	json_parser_simple(string data) { json_data = data; parse(); }
	size_t setData(string data) { json_data = data; return json_data.length(); }
	size_t parse();

	node* getFirstNode() { return &node_first; }

	//
	void _test();
}; // class json_parser_simple
}; // namespace _json

//
#endif //__STRING_H__