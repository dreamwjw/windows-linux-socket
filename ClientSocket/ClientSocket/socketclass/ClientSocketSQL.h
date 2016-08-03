#ifndef CLIENTSOCKETSQL_H_
#define CLIENTSOCKETSQL_H_

#include "MySocketClass.h"
#include <vector>
using std::vector;
using std::string;

class CClientSocketSQL : public CMySocketClass
{
public:
	bool Select(const char *strSelect, vector<vector<string>> &vecResult);
	bool NoSelect(const char *strSQL, char *strResult);
	//because update,insert,delete all have the same code, so make it to NoSelect function
	//bool Update(const char *strUpdate, char *strResult);
	//bool Insert(const char *strInsert, char *strResult);
	//bool Delete(const char *strDelete, char *strResult);
};

#endif