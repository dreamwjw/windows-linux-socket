#pragma once

#include <vector>
using namespace std;

#include "protocol.h"
#include "cJSON.h"

class CMyJson
{
public:
	CMyJson(void);
	~CMyJson(void);

private:
	int GetItemInt(cJSON *pcJSON, const char* szName);
	double GetItemDouble(cJSON *pcJSON, const char* szName);
	string GetItemString(cJSON *pcJSON, const char* szName);
	int GetItemBool(cJSON *pcJSON, const char* szName);//0:true,-1:false

public:
	int GetUserListJasonData(char* szData, vector<UserNet*>& vecUserList);
};

