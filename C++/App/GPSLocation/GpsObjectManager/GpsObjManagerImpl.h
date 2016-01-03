#pragma once

#include "GpsObjectManager.h"
#include "GpsObj.h"
#include <map>
#include <iostream>
using namespace std;


class GpsObjManagerImpl
{
	friend class GpsObjectManager;

protected:
	GpsObjManagerImpl(void);
	virtual ~GpsObjManagerImpl(void);
private:
	typedef map<long ,GpsObj* > MapIDObj;
	typedef MapIDObj::iterator MapIDObj_it;
	MapIDObj m_GpsObjMap;

public:
	bool AddGpsObj(long Id);
	bool DelGpsObj(long Id);
	GpsObj* getGpsObject(long Id);
};
