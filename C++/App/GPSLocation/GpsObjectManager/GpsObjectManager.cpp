#include "GpsObjectManager.h"


GpsObjectManager::GpsObjectManager(void)
{
	m_pImpl = new GpsObjManagerImpl();
}

GpsObjectManager::~GpsObjectManager(void)
{
	if (m_pImpl)
		delete m_pImpl;
}


GpsObjectManager* GpsObjectManager::instance()
{
	static GpsObjectManager theInstance;

	return &theInstance;
}