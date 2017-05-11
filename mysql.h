#ifndef MYSQL_H_
#define MYSQL_H_

#include <mysql/mysql.h>

class CMysql
{
private:
	MYSQL*  m_pMysql;
	static CMysql *m_pCMysql;
	static const int m_nSqlLen = 256;
	
public:
	CMysql();
	~CMysql();
	static CMysql* GetInstance();
	unsigned long long mysql_GetUserID(const char* szUserName, const char* szPassWord);
};

#endif