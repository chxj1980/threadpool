#ifndef _PROC_DEF_H_
#define _PROC_DEF_H_

//get/setº¯Êýºê
#define TPGETSET(ObjName, ObjValue)				 \
	ObjName ObjValue;								  \
	public:												\
	inline ObjName Get##ObjValue(){ return ObjValue; }   \
	inline void Set##ObjValue(ObjName obj){ ObjValue = obj; }		\
	private:

#endif	//_PROC_DEF_H_