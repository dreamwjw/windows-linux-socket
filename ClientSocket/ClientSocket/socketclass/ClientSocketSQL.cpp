#include "ClientSocketSQL.h"

bool CClientSocketSQL::Select(const char *strSelect, vector<vector<string>> &vecResult)
{
	PRINTF("%s\n", strSelect);

	if(!Send(strSelect)) 
	{
		PRINTF("Select Failed");
		return false;
	}

	char strRecv[BUFFER_SIZE] = {0};
	if(!Recv(strRecv))
	{
		PRINTF("Select Failed");
		return false;
	}

	string strTemp;
	vector<string> vecTemp;
	int pos = 0;
	for (unsigned int i = 0; i < strlen(strRecv); i++)
	{
		if(strRecv[i] == ' ')
		{
			vecTemp.push_back(strTemp);
			strTemp.clear();
		}
		else if(strRecv[i] == '\n')
		{
			vecResult.push_back(vecTemp);
			vecTemp.clear();
		}
		else strTemp.push_back(strRecv[i]);
	}

	PRINTF("Select Success!\n");
	return true;
}

//bool CClientSocketSQL::Update(const char *strUpdate, char *strResult)
//{
//	if(!Send(strUpdate)) 
//	{
//		PRINTF("Update Failed!\n");
//		return false;
//	}
//
//	char strRecv[BUFFER_SIZE] = {0};
//	if(!Recv(strRecv))
//	{
//		PRINTF("Update Failed!\n");
//		return false;
//	}
//	strcpy_s(strResult, BUFFER_SIZE, strRecv);
//
//	return true;
//}
//
//bool CClientSocketSQL::Insert(const char *strInsert, char *strResult)
//{
//	if(!Send(strInsert)) 
//	{
//		PRINTF("Update Failed!\n");
//		return false;
//	}
//
//	char strRecv[BUFFER_SIZE] = {0};
//	if(!Recv(strRecv))
//	{
//		PRINTF("Update Failed!\n");
//		return false;
//	}
//	strcpy_s(strResult, BUFFER_SIZE, strRecv);
//
//	return true;
//}
//
//bool CClientSocketSQL::Delete(const char *strDelete, char *strResult)
//{
//	if(!Send(strDelete)) 
//	{
//		PRINTF("Update Failed!\n");
//		return false;
//	}
//
//	char strRecv[BUFFER_SIZE] = {0};
//	if(!Recv(strRecv))
//	{
//		PRINTF("Update Failed!\n");
//		return false;
//	}
//	strcpy_s(strResult, BUFFER_SIZE, strRecv);
//
//	return true;
//}

bool CClientSocketSQL::NoSelect(const char *strSQL, char *strResult)
{
	PRINTF("%s\n", strSQL);

	if(!Send(strSQL)) 
	{
		PRINTF("NoSelect Failed!\n");
		return false;
	}

	char strRecv[BUFFER_SIZE] = {0};
	if(!Recv(strRecv))
	{
		PRINTF("NoSelect Failed!\n");
		return false;
	}
	strcpy_s(strResult, BUFFER_SIZE, strRecv);

	return true;
}