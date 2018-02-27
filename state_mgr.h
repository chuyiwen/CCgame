
/*******************************************************************************

Copyright 2010 by tiankong Interactive Game Co., Ltd.
All rights reserved.

This software is the confidential and proprietary information of
tiankong Interactive Game Co., Ltd. ('Confidential Information'). You shall
not disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered into with
tiankong Interactive  Co., Ltd.

*******************************************************************************/
//状态位操作

#pragma once
//-----------------------------------------------------------------------------
// 类声明
//-----------------------------------------------------------------------------
template<class T, typename E>
class State
{
public:
	State();
	
	VOID	InitState(T tState);

	BOOL	IsInState(E eState) const;
	BOOL	IsInStateAll(T tState) const;
	BOOL	IsInStateAny(T tState) const;

	T		GetState() const;
	VOID	SetState(E eState);
	VOID	UnsetState(E eState);

private:
	T		m_tState;
};


//-----------------------------------------------------------------------------
// 内联实现
//-----------------------------------------------------------------------------
template<class T, typename E>
inline State<T, E>::State()
{
	ZeroMemory(this, sizeof(*this));
}

template<class T, typename E>
inline VOID State<T, E>::InitState(T tState)
{
	m_tState = tState;
}

template<class T, typename E>
inline T State<T, E>::GetState() const
{
	return m_tState;
}

template<class T, typename E>
inline BOOL State<T, E>::IsInState(E eState) const
{
	return (BOOL)((m_tState & (T)eState) > 0);
}

template<class T, typename E>
inline BOOL State<T, E>::IsInStateAll(T tState) const
{
	return (BOOL)((m_tState & tState) == tState);
}

template<class T, typename E>
inline BOOL State<T, E>::IsInStateAny(T tState) const
{
	return (BOOL)((m_tState & tState) > 0);
}

template<class T, typename E>
inline VOID State<T, E>::SetState(E eState)
{
	m_tState |= eState;
}

template<class T, typename E>
inline VOID State<T, E>::UnsetState(E eState)
{
	if(IsInState(eState))
	{
		m_tState ^= eState;
	}
}