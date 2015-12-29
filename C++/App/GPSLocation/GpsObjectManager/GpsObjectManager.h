#pragma once

#include "GpsObjManagerImpl.h"

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
};
