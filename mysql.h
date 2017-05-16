#ifndef MYSQL_H_
#define MYSQL_H_

#include <mysql/mysql.h>

#include <vector>
using namespace std;

#include "protocol.h"

class CMysql
{
private:
	MYSQL*  m_pMysql;
	static CMysql *m_pCMysql;
	static const int m_nSqlLen = 256;

private:
	int mysql_FreeResult();
	
public:
	CMysql();
	~CMysql();
	static CMysql* GetInstance();
	unsigned long long mysql_GetUserID(const char* szUserName, const char* szPassWord);
	unsigned long long mysql_GetUserID(const char* szUserName);
	int mysql_AddOnlineUsers(int nSocketID, unsigned long long ullUserID, unsigned long long ullMacID);
	int mysql_DeleteOnlineUsers(int nSocketID);
	int mysql_SelectOnlineUsers(int nSocketID);
	int mysql_SelectOnlineUsers(unsigned long long ullUserID);
	int mysql_SelectUserList(vector<UserNet*>& UserList);
	int mysql_SelectUserIDAndSocketID(const char* szUserName, unsigned long long& ullUserID, int& nSocketID);
};

#endif