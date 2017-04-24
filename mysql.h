#ifndef MYSQL_H_
#define MYSQL_H_

#include <mysql/mysql.h>

int mysql_init(MYSQL *&conn)
{
	char server [] = "localhost", user [] = "root", password [] = "Wjw7132131",
	     database [] = "server";

	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
	{
		printf("%s\n", mysql_error(conn));
		return -1;
	}
	
	return 0;
}

int mysql_select(MYSQL *conn, const char *szSelect, char* szResult) 
{
	MYSQL_RES *res;
	MYSQL_FIELD *fields;
	MYSQL_ROW row;

	if (mysql_query(conn, szSelect))
	{
		printf("%s\n", mysql_error(conn));
		return -1;
	}

	res = mysql_use_result(conn);
	int nFields = mysql_num_fields(res);

	char szTempBuffer[32] = {0};
	fields = mysql_fetch_fields(res);
	for (int i = 0; i < nFields; i++)
	{
		sprintf(szTempBuffer, "%s ", fields[i].name);
		strcat(szResult, szTempBuffer);
	}

	while ((row = mysql_fetch_row(res)) != NULL)
	{
		for (int i = 0; i < nFields; i++)
		{
			sprintf(szTempBuffer, "%s ", row[i]);
			strcat(szResult, szTempBuffer);
		}
	}

	mysql_free_result(res);

	return 0;
}

int mysql_noselect(MYSQL *conn, const char* szNoSelect, char* szResult) 
{
	if (mysql_query(conn, szNoSelect))
	{
		printf("%s\n", mysql_error(conn));
		return -1;
	}

	int nAffectedRow =  mysql_affected_rows(conn);
	sprintf(szResult, "%d rows affected\n", nAffectedRow);

	return 0;
}

#endif