#pragma once


//~ todo : 让Timer可以对时间进行缩放等
/**	\class Timer
	\brief 时间管理工具
	\par
		<br>.游戏中所有用到时间的地方都需统一从此取得
		<br>.提供整体运行时间和两帧间隔时间
		<br>.内部保存原始数据(毫秒)和浮点数据,前者为了减少浮点误差,后者提供给游戏使用
*/
class Timer
{
protected:
	//--内部状态量
	DWORD	m_dwLastTime;	//上一次Update得到的系统时间
	DWORD	m_dwCurrentTime;	//本次Update得到的系统时间
	//--------------------------
	DWORD	m_dwElapse;	//程序从启动到现在的总时间,毫秒
	DWORD	m_dwDelta;	//两次Update之间的间隔时间,毫秒

	float	m_fElapse;	//秒
	float	m_fDelta;	//秒
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

