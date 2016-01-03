#include "StdAfx.h"
#include "globalobjectManager.h"

globalobjectManager::globalobjectManager(void)
: m_startDraw(false)
{
}

globalobjectManager::~globalobjectManager(void)
{
}


globalobjectManager* globalobjectManager::instance()
{
	static globalobjectManager theInstance;

	return &theInstance;
}




void globalobjectManager::enableStartDraw(bool enabled)
{
	m_startDraw = enabled;
}

bool globalobjectManager::enabledStartDraw(void)
{
	return m_startDraw;
}
