#pragma once

#include "GpsObjManagerImpl.h"
#include "GpsObj.h"
class GpsObjManagerImpl;
class GpsObj;

class GpsObjectManager
{
//public:
//	GpsObjectManager(void);
//	~GpsObjectManager(void);

private:
	// Singleton implementation, constructors are private
	GpsObjectManager(void);
	virtual ~GpsObjectManager(void);
	GpsObjectManager(GpsObjectManager const&);
	GpsObjectManager const& operator=(GpsObjectManager const&);
public:
	static GpsObjectManager* instance(); // return pointer to singleton object

private:
	GpsObjManagerImpl* m_pImpl;
public:
	bool AddGpsObj(long Id);
	bool DelGpsObj(long Id);
	GpsObj* getGpsObject(long Id);
	bool initialise(void);
};
