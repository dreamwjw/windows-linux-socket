#include "MyJson.h"

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
		cJSON *pUserID = cJSON_CreateObject();
		cJSON_AddNumberToObject(pUserID, "UserID", static_cast<double>(vecUserList[i]->ullUserID));
		cJSON_AddItemToArray(pArray, pUserID);

		cJSON *pIsOnline = cJSON_CreateObject();
		cJSON_AddBoolToObject(pIsOnline, "IsOnline", vecUserList[i]->bIsOnline);
		cJSON_AddItemToArray(pArray, pIsOnline);
	}

	cJSON_AddItemToObject(pRoot, "UserList", pArray);

	char *szRet = cJSON_Print(pRoot);
	cJSON_Delete(pRoot);
	return szRet;
}