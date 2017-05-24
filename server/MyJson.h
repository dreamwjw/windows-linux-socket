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

public:
	//remember use "cJason_free" to free the result of this function
	char* SetUserListJasonData(const vector<UserNet*> vecUserList);
};

