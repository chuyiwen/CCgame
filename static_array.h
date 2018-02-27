/*******************************************************************************

	Copyright 2010 by Shengshi Interactive Game Co., Ltd.
	All rights reserved.
	
	This software is the confidential and proprietary information of
	Shengshi Interactive Game Co., Ltd. ('Confidential Information'). You shall
	not disclose such Confidential Information and shall use it only in
	accordance with the terms of the license agreement you entered into with
	Shengshi Interactive  Co., Ltd.

*******************************************************************************/

/**
 *	@file		static_array
 *	@author		mwh
 *	@date		2012/02/08	initial
 *	@version	0.0.1.0
 *	@brief		¾²Ì¬Êý¾Ý
*/
#ifndef STATIIC_ARRAY_H
#define STATIIC_ARRAY_H

 template
 <
 	typename T
 >
 struct DefaultValue
 {
 	static T Default;
 };
 
 template<typename T> T DefaultValue<T>::Default;

template
<
	int C,
	typename T
>
struct StaticArraySP
{
public:
	/** set all item = DefaultValue<T>::Default */
	void Initialize( ){
		number = 0;
		for(int n = 0; n < C; ++n){
			array[n] = DefaultValue<T>::Default;
		}
	}

	/** find; idx, more effective */
	int Find(const T& t, unsigned idx = 0) const {
		for(int n = idx; n < number; ++n){
			if(array[n] == t) return n;
		}
		return -1;
	}

	template<typename Pred> 
	int Find(Pred& Prd, unsigned idx = 0) const{
		for(int n = idx; n < number; ++n){
			if(Prd(&array[n])) return n;
		}

		return -1;
	}

	/** is exist */
	bool IsExist(const T& t) const {
		return Find(t) != -1;
	}

	template<typename Pred> 
	bool IsExist(Pred& Prd) const {
		return Find(Prd) != -1;
	}

	/** add */
	bool Add(const T& t) {
		if(number >= C) 
			return false;

		array[number] = t; 
		++number; return true;
	}

	/** remove */
	bool Remove(const T& t){
		int index = Find(t);
		return RemoveIndex(index);
	}

	template<typename Pred> 
	bool Remove(Pred& Prd){
		int index = Find(Prd);
		return RemoveIndex(index);
	}


	/** fast remove */
	bool FastRemove(const T& t){
		int index = Find(t);
		return FastRemoveIndex(index);
	}

	template<typename Pred> 
	bool FastRemove(Pred& Prd){
		int index = Find(Prd);
		return FastRemoveIndex(index);
	}

	/** remove by index */
	bool RemoveIndex(unsigned index){
		/** inner index is int, outer index is unsigned*/
		if(index >= (unsigned)number)
			return false;

		int n = index + 1;
		for(; n < number; ++n){
			array[n-1] = array[n];
		}
		array[n - 1] = DefaultValue<T>::Default;
		--number; return true;
	}

	/** fast remove by index */
	bool FastRemoveIndex(unsigned index){
		/** inner index is int, outer index is unsigned*/
		if(index >= (unsigned)number)
			return false;

		--number;
		array[index] = array[number];
		array[number] = DefaultValue<T>::Default;
		return true;
	}


	/** remove all items = 't', return the count of 't' */
	int RemoveAll(const T& t)
	{
		int idx = 0, lost = 0;
		for(idx = Find(t, idx); idx != -1; 
			(idx = Find(t, idx))){
				RemoveIndex(idx); ++lost;
		}

		return lost;
	}

	template<typename Pred> 
	int RemoveAll(Pred& Prd)
	{
		int idx = 0, lost = 0;
		for(idx = Find(Prd, idx); idx != -1; 
			(idx = Find(Prd, idx))){
				RemoveIndex(idx); ++lost;
		}

		return lost;
	}


	/** remove all items = 't', return the count of 't' */
	int FastRemoveAll(const T& t)
	{
		int idx = 0, lost = 0;
		for(idx = Find(t, idx); idx != -1; 
			(idx = Find(t, idx))){
				FastRemoveIndex(idx); ++lost;
		}

		return lost;
	}

	template<typename Pred> 
	int FastRemoveAll(Pred& Prd)
	{
		int idx = 0, lost = 0;
		for(idx = Find(Prd, idx); idx != -1; 
			(idx = Find(Prd, idx))){
				FastRemoveIndex(idx); ++lost;
		}

		return lost;
	}

	void Resize(INT size)
	{
		if(size <= C)
			number = size;
	}

	/** get: inner index is int, outer index is unsigned
	*	get: return the value's copy or the DefaultValue<T>::Default 
	*/ 
	T Get(const unsigned idx) const {
		return idx < (unsigned)number ? array[idx] : DefaultValue<T>::Default;
	}
	T Get(const unsigned idx){
		return idx < (unsigned)number ? array[idx] : DefaultValue<T>::Default;
	}


	/**	GetPtr: return the value's pointer or NULL*/
	const T* GetPtr(const unsigned idx) const {
		return idx < (unsigned)number ? &array[idx] : NULL;
	}

	T* GetPtr(const unsigned idx){
		return idx < (unsigned)number ? &array[idx] : NULL;
	}

	/** GetRef: WARNING: !!!!!! side-effect !!!!!!!!
	*	return the reference of the value or any item's reference
	*/
	const T& GetRef(const unsigned idx) const {
		return array[idx%C];
	}

	T& GetRef(const unsigned idx){
		return array[idx%C];
	}

	void Clear( ){ number = 0; memset(array, 0, sizeof(array)); }

	int GetSize( ) const { return number; }
	int GetFree( ) const { return C - number; }
public:
	StaticArraySP(){
		number = 0;
		memset(array, 0, sizeof(array));
	}
private:
	int number;
	T array[C];
};

#endif /** STATIIC_ARRAY_H */