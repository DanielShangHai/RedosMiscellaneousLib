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
bool GpsObjectManager::AddGpsObj(long Id)
{
	return m_pImpl->AddGpsObj(Id);
}

bool GpsObjectManager::DelGpsObj(long Id)
{
	return m_pImpl->DelGpsObj(Id);
}

GpsObj* GpsObjectManager::getGpsObject(long Id)
{
	return m_pImpl->getGpsObject(Id);
}

bool GpsObjectManager::initialise(void)
{
	return true;
}
