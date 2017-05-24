#include "MyJson.h"

CMyJson g_MyJson;

CMyJson::CMyJson(void)
{
}

CMyJson::~CMyJson(void)
{
}

char* CMyJson::SetUserListJasonData(const vector<UserNet*> vecUserList)
{
	cJSON *pRoot = cJSON_CreateObject();

	cJSON *pArray = cJSON_CreateArray();
	int nCount = vecUserList.size();
	for(int i = 0; i < nCount; i++)
	{
		cJSON *pUserNet = cJSON_CreateObject();

		cJSON_AddStringToObject(pUserNet, "UserName", (char*)vecUserList[i]->szUserName);
		cJSON_AddBoolToObject(pUserNet, "IsOnline", vecUserList[i]->bIsOnline);

		cJSON_AddItemToArray(pArray, pUserNet);
	}

	cJSON_AddItemToObject(pRoot, "UserList", pArray);

	char *szRet = cJSON_Print(pRoot);
	cJSON_Delete(pRoot);
	return szRet;
}