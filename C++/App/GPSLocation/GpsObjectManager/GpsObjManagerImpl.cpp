#include "GpsObjManagerImpl.h"

GpsObjManagerImpl::GpsObjManagerImpl(void)
{
}

GpsObjManagerImpl::~GpsObjManagerImpl(void)
{

	if (!m_GpsObjMap.empty())
	{
		for (MapIDObj_it i=m_GpsObjMap.begin(); i!=m_GpsObjMap.end(); /*i++*/)  
		{  
			MapIDObj_it tmp = i;
			i++;  
			m_GpsObjMap.erase(tmp);
		}  
	}
}

bool GpsObjManagerImpl::AddGpsObj(long Id)
{
	int IdNum = m_GpsObjMap.count(Id);
	if (IdNum>0)
	{
		return false;
	}
    GpsObj* lpNewGPS = new GpsObj;
    
    m_GpsObjMap.insert(pair<long,GpsObj*>(Id,lpNewGPS));
    //m_GpsObjMap(Id) = lpNewGPS;
	return true;
}

bool GpsObjManagerImpl::DelGpsObj(long Id)
{
	MapIDObj_it l_it = m_GpsObjMap.find(Id);
	if(l_it==m_GpsObjMap.end())
	{
		cout<<"we do not find 112"<<endl;
		return false;
	}
	else
	{
		GpsObj* lpDelGPS = l_it->second;
		delete lpDelGPS;
		m_GpsObjMap.erase(l_it);  //delete ;
	}
	return true;
}

GpsObj* GpsObjManagerImpl::getGpsObject(long Id)
{
	MapIDObj_it l_it = m_GpsObjMap.find(Id);
	if(l_it==m_GpsObjMap.end())
	{
		cout<<"we do not find 112"<<endl;
		return NULL;
	}
	else
	{
		GpsObj* lpDelGPS = l_it->second;
		return lpDelGPS;
	}
	return NULL;
			
	
}
