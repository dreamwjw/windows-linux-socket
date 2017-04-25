#ifndef MYSQL_H_
#define MYSQL_H_

#include <mysql/mysql.h>

class CMysql
{
private:
	MYSQL*  m_pMysql;
	static CMysql *m_pCMysql;
	
public:
	CMysql();
	~CMysql();
	static CMysql* GetInstance();
	int mysql_GetUserID(const char* szUserName, const char* szPassWord);
};

#endif