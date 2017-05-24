#ifndef STRUCT_H_
#define STRUCT_H_

typedef struct tagClientSocketStruct
{
	int m_nClientSokcet;
	time_t m_LastTimer;
	tagClientSocketStruct()
	{
		m_nClientSokcet = 0;
		m_LastTimer = 0;
	}
	tagClientSocketStruct(int nClientSokcet, time_t LastTimer)
	{
		m_nClientSokcet = nClientSokcet;
		m_LastTimer = LastTimer;
	}
}ClientSocketStruct;

#endif