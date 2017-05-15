#include <stdio.h>

#include "mysql.h"

CMysql* CMysql::m_pCMysql = NULL;

CMysql::CMysql()
{
	char server [] = "localhost", user [] = "root", password [] = "Wjw7132131",
	     database [] = "server";

	m_pMysql = mysql_init(NULL);
	if (!mysql_real_connect(m_pMysql, server, user, password, database, 0, NULL, 0))
	{
		printf("%s\n", mysql_error(m_pMysql));
		exit(1);
	}
}

CMysql::~CMysql()
{
	mysql_close(m_pMysql);
	
	if (m_pCMysql != NULL)
	{
		delete m_pCMysql;
		m_pCMysql = NULL;
	}
}

CMysql* CMysql::GetInstance()
{
	if (m_pCMysql == NULL) m_pCMysql = new CMysql();
	
	return m_pCMysql;
}

unsigned long long CMysql::mysql_GetUserID(const char* szUserName, const char* szPassWord)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	char szSelect[m_nSqlLen] = {0};
	sprintf(szSelect, "select id from users where username = '%s' and password = '%s'", szUserName, szPassWord);
	printf("%s\n", szSelect);
	if (mysql_query(m_pMysql, szSelect))
	{
		printf("%s\n", mysql_error(m_pMysql));
		return -1;
	}
	
	res = mysql_use_result(m_pMysql);
	row = mysql_fetch_row(res);
	
	unsigned long long nRet = 0;
	if (row != NULL)
	{
		nRet = strtoull(row[0], NULL, 10);
	}
	
	mysql_free_result(res);
	
	return nRet;
}

int CMysql::mysql_AddOnlineUsers(int nSocketID, unsigned long long ullUserID)
{
	if(!mysql_SelectOnlineUsers(ullUserID)) return -1;

	char szInsert[m_nSqlLen] = {0};
	sprintf(szInsert, "insert online_users(user_id, socket_id) value(%llu, %d)", ullUserID, nSocketID);
	printf("%s\n", szInsert);
	if (mysql_query(m_pMysql, szInsert))
	{
		printf("%s\n", mysql_error(m_pMysql));
		return -2;
	}

	int nAffectedRow =  mysql_affected_rows(m_pMysql);
	printf("%d rows affected\n", nAffectedRow);

	return nAffectedRow;
}

int CMysql::mysql_DeleteOnlineUsers(int nSocketID)
{
	char szDelete[m_nSqlLen] = {0};
	sprintf(szDelete, "delete from online_users where socket_id = %d", nSocketID);
	printf("%s\n", szDelete);
	if (mysql_query(m_pMysql, szDelete))
	{
		printf("%s\n", mysql_error(m_pMysql));
		return -1;
	}

	int nAffectedRow =  mysql_affected_rows(m_pMysql);
	printf("%d rows affected\n", nAffectedRow);

	return nAffectedRow;
}

int CMysql::mysql_SelectOnlineUsers(int nSocketID)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	char szSelect[m_nSqlLen] = {0};
	sprintf(szSelect, "select from online_users where socket_id = %d", nSocketID);
	printf("%s\n", szSelect);
	if (mysql_query(m_pMysql, szSelect))
	{
		printf("%s\n", mysql_error(m_pMysql));
		return -1;
	}

	res = mysql_use_result(m_pMysql);
	int nRow = mysql_num_rows(res);

	return nRow;
}

int CMysql::mysql_SelectOnlineUsers(unsigned long long ullUserID)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	char szSelect[m_nSqlLen] = {0};
	sprintf(szSelect, "select from online_users where user_id = %llu", ullUserID);
	printf("%s\n", szSelect);
	if (mysql_query(m_pMysql, szSelect))
	{
		printf("%s\n", mysql_error(m_pMysql));
		return -1;
	}

	res = mysql_use_result(m_pMysql);
	int nRow = mysql_num_rows(res);

	return nRow;
}

int CMysql::mysql_SelectUserList(vector<UserNet*>& UserList)
{
	MYSQL_RES *res;
	MYSQL_ROW row;

	char szSelect[m_nSqlLen] = {0};
	sprintf(szSelect, "select * from users");
	printf("%s\n", szSelect);
	if (mysql_query(m_pMysql, szSelect))
	{
		printf("%s\n", mysql_error(m_pMysql));
		return -1;
	}

	res = mysql_use_result(m_pMysql);
	while((row = mysql_fetch_row(res)) != NULL)
	{
		UserNet* pUserNet = new UserNet;
		unsigned long long ullUserID = strtoull(row[0], NULL, 10);
		pUserNet->ullUserID = ullUserID;
		if(mysql_SelectOnlineUsers(ullUserID) >= 1)
		{
			pUserNet->bIsOnline = true;
		}
		else
		{
			pUserNet->bIsOnline = false;
		}
		UserList.push_back(pUserNet);
	}

	return 0;
}
//int mysql_select(MYSQL *conn, const char *szSelect, char* szResult) 
//{
//	MYSQL_RES *res;
//	MYSQL_FIELD *fields;
//	MYSQL_ROW row;
//
//	if (mysql_query(conn, szSelect))
//	{
//		printf("%s\n", mysql_error(conn));
//		return -1;
//	}
//
//	res = mysql_use_result(conn);
//	int nFields = mysql_num_fields(res);
//
//	char szTempBuffer[32] = {0};
//	fields = mysql_fetch_fields(res);
//	for (int i = 0; i < nFields; i++)
//	{
//		sprintf(szTempBuffer, "%s ", fields[i].name);
//		strcat(szResult, szTempBuffer);
//	}
//
//	while ((row = mysql_fetch_row(res)) != NULL)
//	{
//		for (int i = 0; i < nFields; i++)
//		{
//			sprintf(szTempBuffer, "%s ", row[i]);
//			strcat(szResult, szTempBuffer);
//		}
//	}
//
//	mysql_free_result(res);
//
//	return 0;
//}
//
//int mysql_noselect(MYSQL *conn, const char* szNoSelect, char* szResult) 
//{
//	if (mysql_query(conn, szNoSelect))
//	{
//		printf("%s\n", mysql_error(conn));
//		return -1;
//	}
//
//	int nAffectedRow =  mysql_affected_rows(conn);
//	sprintf(szResult, "%d rows affected\n", nAffectedRow);
//
//	return 0;
//}