#pragma once


//~ todo : ��Timer���Զ�ʱ��������ŵ�
/**	\class Timer
	\brief ʱ�������
	\par
		<br>.��Ϸ�������õ�ʱ��ĵط�����ͳһ�Ӵ�ȡ��
		<br>.�ṩ��������ʱ�����֡���ʱ��
		<br>.�ڲ�����ԭʼ����(����)�͸�������,ǰ��Ϊ�˼��ٸ������,�����ṩ����Ϸʹ��
*/
class Timer
{
protected:
	//--�ڲ�״̬��
	DWORD	m_dwLastTime;	//��һ��Update�õ���ϵͳʱ��
	DWORD	m_dwCurrentTime;	//����Update�õ���ϵͳʱ��
	//--------------------------
	DWORD	m_dwElapse;	//��������������ڵ���ʱ��,����
	DWORD	m_dwDelta;	//����Update֮��ļ��ʱ��,����

	float	m_fElapse;	//��
	float	m_fDelta;	//��
public:

	void Init();
	
	void Update();

	void SetElapseTime(DWORD time)	
	{	m_dwElapse=time;
		m_fElapse=m_dwElapse*0.001f;
	}

	DWORD GetDeltaDW()	{ return m_dwDelta;}
	float GetDelta()	{ return m_fDelta; }

	DWORD GetElapseDW()	{ return m_dwElapse; }
	float GetElapse()	{ return m_fElapse; }
			
	Timer(void);
	~Timer(void);
};

