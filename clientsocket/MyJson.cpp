#include "MyJson.h"
#include "Common.h"

CMyJson g_MyJson;

extern CCommon g_Common;

CMyJson::CMyJson(void)
{
}

CMyJson::~CMyJson(void)
{
}

int CMyJson::GetItemInt(cJSON *pcJSON, const char* szName)
{
	cJSON *pValue = cJSON_GetObjectItem(pcJSON, szName);
	if(pValue != NULL) return pValue->valueint;
	else return -1;
}

double CMyJson::GetItemDouble(cJSON *pcJSON, const char* szName)
{
	cJSON *pValue = cJSON_GetObjectItem(pcJSON, szName);
	if(pValue != NULL) return pValue->valuedouble;
	else return -1;
}

string CMyJson::GetItemString(cJSON *pcJSON, const char* szName)
{
	cJSON *pValue = cJSON_GetObjectItem(pcJSON, szName);
	if(pValue != NULL) return pValue->valuestring;
	else return "";
}

int CMyJson::GetItemBool(cJSON *pcJSON, const char* szName)//0:true,-1:false
{
	cJSON *pValue = cJSON_GetObjectItem(pcJSON, szName);
	if(pValue != NULL) return pValue->type==cJSON_True?0:-1;
	else return -1;
}

int CMyJson::GetUserListJasonData(char* szData, vector<UserNet*>& vecUserList)
{
	cJSON *pRoot = cJSON_Parse(szData);
	if(pRoot == NULL) 
	{
		g_Common.MyOutputDebugString("Jason Parse Failed\n");
		return -1;
	}

	cJSON *pUserList = cJSON_GetObjectItem(pRoot, "UserList");
	if(pUserList != NULL)
	{
		int nArraySize = cJSON_GetArraySize(pUserList);
		for(int i = 0; i < nArraySize;i++)
		{
			UserNet* pUserNet = new UserNet;

			cJSON *pArray = cJSON_GetArrayItem(pUserList, i);

			if(pArray == NULL) continue;

			memcpy(pUserNet->szUserName, GetItemString(pArray, "UserName").c_str(), 32);
			pUserNet->bIsOnline = (GetItemBool(pArray, "IsOnline") == 0)?true:false;

			vecUserList.push_back(pUserNet);
		}
	}

	cJSON_Delete(pRoot);

	return 0;
}
