// VIDEOJET TECHNOLOGIES, INC.
//
// (C) Copyright 2015 Videojet Technologies,Inc.  All
// Rights Reserved.  An unpublished work of Videojet Technologies,
// Inc.  The software and documentation contained
// herein are copyrighted works which include confidential
// information and trade secrets proprietary to Videojet Technologies,
// Inc. and shall not be copied, duplicated,
// disclosed or used, in whole or part, except pursuant to
// the License Agreement or as otherwise expressly approved by
// Videojet Technologies, Inc.
//
// Description :  Event log manager

#include "stdafx.h"
#include <boost/bind.hpp>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>

#include <information_manager/InformationManager.h>
#include <parameter/LogEventParameter.h>
#include <globaldefs/GlobalDefs.h>
#include <logging/vjlogging.h>
#include <parameter/ReferenceParameter.h>
#include <alarm_manager/AlarmManager.h>
#include <eventlogging/EventLogDeclarations.h>
#include <common_functions/XpSqlDbToCsvFile.h>
#include <common_functions/XpSqliteDbVersion.h>
#include <eventlogging/EventLogManager.h>
#include <MachineStatus/MachineStatus.h>
#include "DateTimeHandlerDefault.h"
#include "EventLogReader.h"
#include <eventlogging/EventLogTypes.h>


#ifdef VJ_LOG_CATEGORY_NAME
#undef VJ_LOG_CATEGORY_NAME
#define VJ_LOG_CATEGORY_NAME "EventLogManager"
#endif

namespace im  = xplatform::information_manager;
namespace gd  = xplatform::global_defs;
namespace xel = xplatform::event_logging;
namespace xc  = xplatform::core_component;
namespace xal = xplatform::alarms;

xel::EventLogManagerPtr xel::EventLogManager::m_Instance = NULL;

const int kEntriesPerOnePageOfEventLogs = 20; // The number of entries of the Event Logs to be displayed at once

// TODO Change database schema so that categories are not fixed: instead of one column per category, have one column for all categories, 
// containing a comma-separated list of category names.
namespace
{
    const int kInterval           = 60000;    // Interval to update database records
    const int kCategoryFlagCount  = 6;        // Numbers of Flags
    const int kMaxEventAgeMonths  = 18;       // Events older than this are purged from the database
    const int kMsPerMinute        = 60000;    // Total number of milliseconds in a Minute
    const int kSecondsPerDay      = 86400;    // Total number of seconds in a day
    const int kMsPerDay           = 86400000; // Total number of milliseconds in a day
    const QString kEventLogManagerThreadName = QStringLiteral("EventLogManager");
    // Table column constants
    const QString kTableIndex               = QStringLiteral("Id");
    const QString kSId                      = QStringLiteral("SId");
    const QString kType                     = QStringLiteral("Type");
    const QString kAlarm_Id                 = QStringLiteral("Alarm_Id");
    const QString kEvent_Id                 = QStringLiteral("Event_Id");
    const QString kStart_Time               = QStringLiteral("Start_Time");
    const QString kEnd_Time                 = QStringLiteral("End_Time");
    const QString kEvent_Description        = QStringLiteral("Event_Description");
    const QString kEvent_UserCleared        = QStringLiteral("User_Event_Cleared");
    const QString kEvent_UserOccurred       = QStringLiteral("User_Event_Occurred");
    const QString kEvent_Var_Data           = QStringLiteral("Event_Var_Data");
    const QString kProductionFlag           = QStringLiteral("Production_Flag");
    const QString kEventFlag                = QStringLiteral("Event_Flag");
    const QString kServiceFlag              = QStringLiteral("Service_Flag");
    const QString kSystemFlag               = QStringLiteral("System_Flag");
    const QString kSecurityFlag             = QStringLiteral("Security_Flag");
    const QString kPrinterAvailabilityFlag  = QStringLiteral("Printer_Availability_Flag");
    const QString kProxy1AvailabilityFlag   = QStringLiteral("Operational_Availability_Proxy1_Flag");
    const QString kProxy2AvailabilityFlag   = QStringLiteral("Operational_Availability_Proxy2_Flag");

    // OEE Available Log Data Table column constants
    const QString kDate                     = QStringLiteral("Date");
    const QString kPrinter_Down_Time        = QStringLiteral("Printer_Down_Time");
    const QString kPrinter_Total_Time       = QStringLiteral("Printer_Total_Time");
    const QString kProxy1_Total_Time        = QStringLiteral("Proxy1_Total_Time");
    const QString kProxy1_Down_Time         = QStringLiteral("Proxy1_Down_Time");
    const QString kProxy2_Total_Time        = QStringLiteral("Proxy2_Total_Time");
    const QString kProxy2_Down_Time         = QStringLiteral("Proxy2_Down_Time");

    const QString kPrinterTotalTime         = QStringLiteral("Printer Total Time");
    const QString kPrinterReadyTotalTime    = QStringLiteral("Printer Ready Total Time");

    const QVector<QString> kEventLogColumnNames = QVector<QString>() << kTableIndex
                                                                   << kType
                                                                   << kSId
                                                                   << kProductionFlag
                                                                   << kEventFlag
                                                                   << kServiceFlag
                                                                   << kSystemFlag
                                                                   << kSecurityFlag
                                                                   << kPrinterAvailabilityFlag
                                                                   << kProxy1AvailabilityFlag
                                                                   << kProxy2AvailabilityFlag
                                                                   << kStart_Time
                                                                   << kEnd_Time
                                                                   << kEvent_UserOccurred
                                                                   << kEvent_UserCleared
                                                                   << kEvent_Var_Data;

    const QVector<QString> kOeeLogColumnNames = QVector<QString>()   << kTableIndex
                                                                   << kDate
                                                                   << kPrinter_Down_Time
                                                                   << kPrinter_Total_Time
                                                                   << kProxy1_Down_Time
                                                                   << kProxy1_Total_Time
                                                                   << kProxy2_Down_Time
                                                                   << kProxy2_Total_Time;

     const QVector<QString> kEventTableColumnNames = QVector<QString>()   << kSId
                                                                   << kEvent_Id
                                                                   << kEvent_Description;

     const QVector<QString> kDownTimeAlarmListColumnNames = QVector<QString>() << kTableIndex
                                                                               << kAlarm_Id
                                                                               << kProxy2AvailabilityFlag;

    WString formatWString(WString wStringToFormat)
    {
        // convert to QString from WString because it's easier to manipulate
        QString wStringAsQString = fromWString(wStringToFormat);
        QString allLowerCaps = wStringAsQString.toLower();

        int offset = 0;  // To adjust for the increased length of the
                         // QString each time ' ' is inserted

        // Insert a space ' ' character before each capital character in the QString
        // NOTE : Ignore the first character of the QString
        for (int i = 1; i < allLowerCaps.length(); i++)
        {
            // if the characters being compared are same, move on to the next character
            if (wStringAsQString[i + offset] == allLowerCaps[i])
            {
                continue;
            }
            else
            {
                // the character in wStringAsQString is Capital, Insert a space here
                wStringAsQString.insert(i + offset, ' ');
                offset++;
            }
        }
        // convert QString back to WString after manipulation
        return fromQString(wStringAsQString);
    }





    QString getAdditionalInformation(QMap<im::Parameter, im::ReferenceParameter> &infomap, im::Parameter node)
    {
        QString info;

        if (infomap.find(node) != infomap.end())
        {
            im::ReferenceParameter ref_param = infomap[node];
            if (ref_param)
            {
                im::Parameter additionalInfo;
                ref_param.GetValue(additionalInfo);
                VJ_ASSERT(additionalInfo);

                WString paramType = additionalInfo.GetParameterType();

                if (gd::xml_Bool == paramType)
                {
                    info = ((additionalInfo.toQString() == "true") ? " On" : " Off");
                }
                else
                {
                    info = QString(" - ").append(additionalInfo.toQString());
                }
            }
        }
        return info;
    }
}

xel::EventLogManager::EventLogRec::EventLogRec
(
    int                      eventType,
    int                      eventId,
    QString                  eventDescription,
    QString                  start_time,
    QString                  end_time,
    QMap<QString, int>       flagValueMap, 
    QString varText
)
    : m_eventType(eventType)
    , m_eventId(eventId)
    , m_eventDescription(eventDescription)
    , m_start_time(start_time)
    , m_end_time(end_time)
    , m_categoryFlagValues(flagValueMap)
    , m_varText(varText)
    , m_node()
    , m_eventIsOpen(false)
{
}

xel::EventLogManager::EventLogRec::EventLogRec
(
                    int                 eventType,
                    QString             eventDescription,
                    QString             start_time,
                    QString             end_time,
                    xplatform::information_manager::Parameter node,
                    bool                eventIsOpen
)
    : m_eventType(eventType)
    , m_eventId(-1)
    , m_eventDescription(eventDescription)
    , m_start_time(start_time)
    , m_end_time(end_time)
    , m_categoryFlagValues()
    , m_varText()
    , m_node(node)
    , m_eventIsOpen(eventIsOpen)
{
}

xel::EventLogManager::EventLogManager()
  : m_db(),
    m_logged_in_user(),
    m_language(im::InformationManager::GetParameter(gd::xpath_SelectedLocale)),
    m_country(im::InformationManager::GetParameter(gd::xpath_SelectedCountry)),
    m_openEvents(),
    m_openAlarmEventsLst(),
    m_timer_id(0),
    m_locale(),
    m_flagsMap(),
    m_AvailabilityEventCnt(0),
    m_Proxy1AvailabilityEventCnt(0),
    m_Proxy2AvailabilityEventCnt(0),
    m_pDailyTimer(NULL),
    m_CurrentDay(0),
    m_pIdtHandler(new DateTimeHandlerDefault()),
    m_BackupDbCreated(false),
    m_throttling(false),
    m_maxItemsToWrite(0),
    m_machine_status(),
    m_AvailibilityProxy2EventID(-1)
{
    qRegisterMetaType<QSharedPointer<IDateTimeHandler> >("QSharedPointer<IDateTimeHandler>");
    qRegisterMetaType<QSharedPointer<EventLogRec> >("QSharedPointer<EventLogRec>");
    qRegisterMetaType<QSharedPointer<EventLogParameterRec> >("QSharedPointer<EventLogParameterRec>");
    qRegisterMetaType<xplatform::information_manager::Parameter>("xplatform::information_manager::Parameter");

    qRegisterMetaType<QList<Flags> >("QList<Flags>");
    qRegisterMetaType<QList<EventTypes> >("QList<EventTypes>");
    qRegisterMetaType<xplatform::event_logging::EventLogDataLstPtr>("xplatform::event_logging::EventLogDataLstPtr");

    UpdateLocale();
    // connect to this qt signal from the application, so that we can do some clean ups.
    VJ_VERIFY(QObject::connect(qApp, &QCoreApplication::aboutToQuit, this,
                               &xel::EventLogManager::OnAppAboutToQuit));

    VJ_ASSERT(m_language);
    VJ_ASSERT(m_country);

    m_flagsMap.insert(Production, kProductionFlag);
    m_flagsMap.insert(Event, kEventFlag);
    m_flagsMap.insert(Service, kServiceFlag);
    m_flagsMap.insert(System, kSystemFlag);
    m_flagsMap.insert(Security, kSecurityFlag);
    m_flagsMap.insert(Printer_Availability, kPrinterAvailabilityFlag);
    m_flagsMap.insert(Operational_Availability_Proxy1, kProxy1AvailabilityFlag);
    m_flagsMap.insert(Operational_Availability_Proxy2, kProxy2AvailabilityFlag);

    m_TimeFrameList << eThirtyDays << eNinetyDays << eCurrentMonth << eLastMonth << eLastSecondMonth
                    << eLastThirdMonth << eLastFourthMonth << eLastFifthMonth << eLastSixthMonth;

    // Read all the event logs context
    ReadTranslationContexts();

    // Initialize system event name and id's from IM values
    UpdateSystemEventNamesAndIDs();
    m_OnInsertLogItemsMapPtr = xel::TargetEventLogItemPtrMapPtr (new xel::TargetEventLogItemPtrMap());
    m_OnUpdateLogItemsMapPtr = xel::TargetEventLogItemPtrMapPtr (new xel::TargetEventLogItemPtrMap());
    m_OnDeleteLogItemsMapPtr = xel::TargetEventLogItemPtrMapPtr (new xel::TargetEventLogItemPtrMap());
}

xel::EventLogManagerPtr xel::EventLogManager::GetInstance()
{
    if (m_Instance == NULL)
    {
        m_Instance = new EventLogManager();
    }
    return m_Instance;
}

bool xel::EventLogManager::AddEventConfiguration(const QString &file)
{
    event_lock lock(m_mutex);
    bool outcome = false;
    xel::TargetEventLogItemPtrMapPtr localMapPtr;
    EventLogItemPtrLst eventlst;
    EventLogReader reader(file);
    outcome = reader.readEventList(eventlst);
    if(outcome)
    {
        EventLogItemPtrLst::iterator itr;
        EventLogItemPtrLst::iterator end = eventlst.end();
        for (itr = eventlst.begin(); itr != end; ++itr)
        {
            EventLogItemPtr itemptr = (*itr);
            if(itemptr->GetType().compare("OnParameterInsert")==0)
            {
                localMapPtr = m_OnInsertLogItemsMapPtr;
            }
            else if(itemptr->GetType().compare("OnParameterUpdate")==0)
            {
                localMapPtr = m_OnUpdateLogItemsMapPtr;
            }
            else if(itemptr->GetType().compare("OnParameterDelete")==0)
            {
                localMapPtr = m_OnDeleteLogItemsMapPtr;
            }
            else
            {
                VJ_ERROR("[EventLogManager] not supported event type " << itemptr->GetType().toStdString());
                outcome = false;
                continue;
            }
            outcome = !(localMapPtr->contains(itemptr->GetTarget()));
            if(outcome)
            {
               localMapPtr->insert(itemptr->GetTarget(),itemptr);
            }
            else
            {
                VJ_ERROR("[EventLogManager] duplicate event target " << itemptr->GetTarget().toStdString());
            }
        }
    }
    ReadandAddTranslationContexts(eventlst);
    return  outcome;
}

int xel::EventLogManager::GetNumOfEventConfiguration(const WString& xpath)
{
    event_lock lock(m_mutex);
    int inumber = 0;
    if (xpath == gd::xpath_OnUpdateLogItems)
    {
        inumber = m_OnUpdateLogItemsMapPtr->size();
    }
    else if (xpath == gd::xpath_OnInsertLogItems)
    {
        inumber = m_OnInsertLogItemsMapPtr->size();
    }
    else if (xpath == gd::xpath_OnDeleteLogItems)
    {
        inumber = m_OnDeleteLogItemsMapPtr->size();
    }
    return inumber;
}



// Worker thread
void xel::EventLogManager::InitDailyTimer()
{
    m_CurrentDay = m_pIdtHandler->getCurrentDateTimeUtc().date().day();
    m_pDailyTimer = new QTimer(this);

    // low accuracy timer
    m_pDailyTimer->setTimerType(Qt::VeryCoarseTimer);
    VJ_VERIFY(
        QObject::connect(m_pDailyTimer, &QTimer::timeout, this, &xel::EventLogManager::OnNewDay));

    m_pDailyTimer->start(kMsPerDay);
}

// Worker thread
void xel::EventLogManager::OnNewDay()
{
    if (NULL != m_pDailyTimer)
    {
        m_pDailyTimer->stop();

        // check if the day has really has been changed (may not due to inaccuracies)
        int day = m_pIdtHandler->getCurrentDateTimeUtc().date().day();
        if (day != m_CurrentDay)
        {
            RestoreOEEAvailabilityData();
            UpdateOEENodesForLastThirtyDay();
            // restart the timer to cache the next day change
            m_pDailyTimer->start(kMsPerDay);
        }
        else
        {
            // restart the timer now every minute to cache the day change
            m_pDailyTimer->start(kMsPerMinute);
        }
    }
    else
    {
        VJ_ASSERT(!" m_pDailyTimer Timer is not initialized ");
    }
}

// Worker thread
void xel::EventLogManager::OnSystemRestart()
{
    bool success = true;

    success &= ResetOnPowerCycle();

    if(success)
    {
        BuildStartDownTimeAlarmList();
        ProcessDowntimeEndConditon(xal::eDowntimeEndConditionPowerCycle, QVariant(QString()));

        RaiseInternalEvent(m_kSystemStartupEventID, m_kSystemStartupEventName);
        if (m_AvailabilityEventCnt == 0)
        {
            RaiseInternalEvent(m_kPrinterUpEventID, m_kPrinterUpEventName);
        }

        RaiseInternalEvent(m_kProxy1SystemStartupEventID, m_kProxy1SystemStartupEventName);
        if (m_Proxy1AvailabilityEventCnt == 0)
        {
            RaiseInternalEvent(m_kProxy1PrinterUpEventID, m_kProxy1PrinterUpEventName);
        }
        if (m_Proxy2AvailabilityEventCnt == 0)
        {
            RaiseInternalEvent(m_kProxy2PrinterUpEventID, m_kProxy2PrinterUpEventName);
        }
    }

    // NOTE: (J.R) For debugging purposes
    // This will shutdown the application right after all the components have completed the PostInitialisation stage
    // qApp->quit ();

}

// Worker thread
void xel::EventLogManager::BuildStartDownTimeAlarmList()
{
    event_lock lock(m_mutex);

    QString queryString = "SELECT Alarm_Id, Operational_Availability_Proxy2_Flag FROM DOWNTIME_ALARM_LIST";

    QSqlQuery query(m_db);

    if (!query.exec(queryString))
    {
        VJ_ERROR("SQL query error " << queryString.toStdString());
        return;
    }
    xal::AlarmPtrLst       alarmList = xal::AlarmManager::GetAvailableAlarms();
    QMap<int, xal::AlarmPtr> alarmMap;
    
    foreach (xal::AlarmPtr alarmPtr, alarmList)
    {
      alarmMap.insert(alarmPtr->GetId(), alarmPtr);
    }
    //
    // Build the list of alarms whose downtime has not ended.
    // These alarms may or may not be cleard.
    // 
    while (query.next())
    {
        int alarm_id =  query.value(kAlarm_Id).toInt();

        if (alarmMap.find(alarm_id) != alarmMap.end())
        {
            xal::AlarmPtr alarmPtr = alarmMap[alarm_id];
            m_DowntimeStartAlarmList.prepend(*alarmPtr);
            
            QMap<QString, int> categoryFlagValues;

            GetCategoryFlagInfo(alarmPtr->GetCategories(), categoryFlagValues);

            int availability       = categoryFlagValues.value(kPrinterAvailability);
            int proxy1availability = categoryFlagValues.value(kOperational_Availability_Proxy1);
            int proxy2availability = query.value(kProxy2AvailabilityFlag).toInt();
           
            // populate downtime counters
            m_AvailabilityEventCnt       += availability;
            m_Proxy1AvailabilityEventCnt += proxy1availability;
            m_Proxy2AvailabilityEventCnt += proxy2availability;
        }
        else
        {
            VJ_ERROR("Event database is corrupted, Alarm Id " << alarm_id << " not found"); 
            VJ_ASSERT(false);
        }
    }
    VJ_INFO("Printer Availibity counter: " << m_AvailabilityEventCnt 
              << "  Proxy1 Availibity counter: " << m_Proxy1AvailabilityEventCnt
              << "  Proxy2 Availibity counter: " << m_Proxy2AvailabilityEventCnt);

    // now build list of open events
    queryString = QString("SELECT Event_Id, Start_Time, Event_Description \
                           FROM EVENT_LOG NATURAL JOIN EVENT_TABLE \
                           WHERE End_Time is NULL");
    if (!query.exec(queryString))
    {
        VJ_ERROR("SQL query error " << queryString.toStdString());
        return;
    }
    while (query.next())
    {
        int     eventId        = query.value(kEvent_Id).toInt();
        QString startTime      = query.value(kStart_Time).toString();
        QString description    = query.value(kEvent_Description).toString();

        m_openEvents.insert(eventId, startTime);
        VJ_INFO("Event Id " << eventId << " " << description.toStdString() << " still open ");
    }
}

// Main UI thread
bool xel::EventLogManager::SubscribeForParameterUpdate()
{
    event_lock lock(m_mutex);
    bool success = true;

    im::LogEventParameter proxy2 = im::InformationManager::GetParameter(gd::xpath_AvailabilityProxy2);
    im::Int32Parameter eventIDParam = proxy2.GetParameter(gd::xml_EventID);
    
    success &= eventIDParam.GetValue(m_AvailibilityProxy2EventID);
    
    // Subscribe for notification on Language or Country update
    success &= RegisterForLocaleUpdate();

    // Subscribe for Custom Events to get notification whenever any event is raised or cleared.
    success &= RegisterForEventNotification(gd::xpath_PlatformEvents);
    success &= RegisterForEventNotification(gd::xpath_TechnologyEvents);

    // Subscribe for all Parameter change events
    success &= RegisterForParameterChangeEvents(gd::xpath_OnUpdateLogItems);
    success &= RegisterForParameterChangeEvents(gd::xpath_OnInsertLogItems);
    success &= RegisterForParameterChangeEvents(gd::xpath_OnDeleteLogItems);

    // Subscribe to all alarm updates
    success &= RegisterForAlarmManagerUpdates();
    
    // Subscribe for MachineState set value
    success &= RegisterForParameterMachineStatusChanged();
    return success;
}

// Main UI thread
bool xel::EventLogManager::DumpEventLogDataToCsvFile(const QString &csvFile)
{
    return DumpTableToCsvFile(csvFile, kEventLogTableName + " NATURAL JOIN " + kEventTableName);
}

// Main UI thread
bool xel::EventLogManager::DumpAvailabilityDataToCsvFile(const QString &csvFile)
{
    event_lock lock(m_mutex);
    
    WriteEventLogRecords();
    xplatform::common_functions::XpSqlDbToCsvFile mySqlDbToCsvFile(csvFile, &m_db,
                                                                   kOEEAvailLogTableName);
    return (mySqlDbToCsvFile.Dump());
}

// Main UI thread
bool xel::EventLogManager::RegisterForAlarmManagerUpdates()
{
    bool okRaise = xal::AlarmManager::SubscribeAllAndBypassSuppression(
        this, QStringLiteral("OnAlarmRaised"), xal::eAlarmRaised);
    if (!okRaise)
    {
        VJ_ERROR("FAILED to register for AlarmRaised notifications");
    }

    bool okClear = xal::AlarmManager::SubscribeAllAndBypassSuppression(
        this, QStringLiteral("OnAlarmCleared"), xal::eAlarmCleared);
    if (!okClear)
    {
        VJ_ERROR("FAILED to register for AlarmCleared notifications");
    }

    // No need to subscribe to AlarmModified notifications, since the modifiable alarm properties
    // (clearable, description) are not logged.

    return okRaise && okClear;
}

// Main UI thread
bool xel::EventLogManager::RegisterForLocaleUpdate()
{
    bool success = true;

    if (m_language.IsNull() || m_country.IsNull())
    {
        return false;
    }

    // Subscribe for update on language and country parameters in IM
    NotificationDisconnector languageNotificationDisconnector;
    success &= m_language.RegisterOnUpdate(im::Parameter::GenerateUniqueNotificationTag(),
                                           boost::bind(&xel::EventLogManager::UpdateLocale, this),
                                           &languageNotificationDisconnector);
    m_disconnectors.push_back(languageNotificationDisconnector);

    NotificationDisconnector countryNotificationDisconnector;
    success &= m_country.RegisterOnUpdate(im::Parameter::GenerateUniqueNotificationTag(),
                                          boost::bind(&xel::EventLogManager::UpdateLocale, this),
                                          &countryNotificationDisconnector);
    m_disconnectors.push_back(countryNotificationDisconnector);

    return success;
}

// Main UI thread
bool xel::EventLogManager::RegisterForEventNotification(const WString & xpath)
{
    bool success = true;

    im::Parameter events = im::InformationManager::GetParameter(xpath);
    if (events.IsNull())
    {
        return true;
    }

    im::ParameterList eventsList = events.Children();
    if (eventsList.IsEmpty())
    {
        // this is a valid result: no events of this type have been configured
        return true;
    }

    im::ParameterList::iterator itr;
    im::ParameterList::iterator end = eventsList.end();

    // Now subscribe for all events notification
    for (itr = eventsList.begin(); itr != end; ++itr)
    {
        NotificationDisconnector event_trigger_disconnector, event_clear_disconnector;

        im::Parameter event = (*itr);
        if (event.IsNull())
        {
            return false;
        }

        // OnSet Registration is to get notification whenever the event is raised using RaiseEvent()
        success &= event.RegisterOnSet(im::Parameter::GenerateUniqueNotificationTag(),
                                       boost::bind(&xel::EventLogManager::OnEventTrigger, this, _1),
                                       &event_trigger_disconnector);

        // OnUpdate Registration is to get notification whenever the event is cleared using
        // CloseEvent()
        success &= event.RegisterOnUpdate(
            im::Parameter::GenerateUniqueNotificationTag(),
            boost::bind(&xel::EventLogManager::OnClearEvent, this, _1), &event_clear_disconnector);

        m_disconnectors.push_back(event_trigger_disconnector);
        m_disconnectors.push_back(event_clear_disconnector);
    }

    return success;
}

// Main UI thread
bool xel::EventLogManager::RegisterForParameterChangeEvents(const WString &xpath )
{
    event_lock lock(m_mutex);
    QMap<xplatform::information_manager::Parameter, QString>* targetPathMap;
    bool success = true;

    TargetEventLogItemPtrMapPtr  eventlogitemsptr;
    if (xpath == gd::xpath_OnUpdateLogItems)
    {
        eventlogitemsptr = m_OnUpdateLogItemsMapPtr;
        targetPathMap = &m_OnUpdateTargetPathMap;
    }
    else if (xpath == gd::xpath_OnInsertLogItems)
    {
        eventlogitemsptr = m_OnInsertLogItemsMapPtr;
        targetPathMap = &m_OnInsertTargetPathMap;
    }
    else if (xpath == gd::xpath_OnDeleteLogItems)
    {
        eventlogitemsptr = m_OnDeleteLogItemsMapPtr;
        targetPathMap = &m_OnDeleteTargetPathMap;
    }

    if(eventlogitemsptr->isEmpty())
    {
          // valid result: no events of this type have been configured
          return true;
    }

    xel::TargetEventLogItemPtrMap::const_iterator itr;
    xel::TargetEventLogItemPtrMap::const_iterator end = eventlogitemsptr->constEnd();

    // Now iterate through all items and subscribe for notification
    for (itr = eventlogitemsptr->constBegin(); itr != end; ++itr)
    {
        NotificationDisconnector disconnector;

        EventLogItemPtr item = itr.value();
        if (item.isNull())
        {
            success &= false;
            continue;
        }

        im::Parameter targetParam = im::InformationManager::GetParameter((item->GetTarget()));
        if (targetParam)
        {
            targetPathMap->insert(targetParam,itr.key());
            if (item->GetType() == QStringLiteral("OnParameterUpdate"))
            {
                // OnUpdate Registration is to get notification whenever parameter value changes
                success &= targetParam.RegisterOnUpdate(
                im::Parameter::GenerateUniqueNotificationTag(),
                boost::bind(&xel::EventLogManager::OnParameterUpdate, this, _1),
                &disconnector);
            }
            else if (item->GetType() == QStringLiteral("OnParameterInsert"))
            {
                // OnInsertChild Registration is to get notification whenever new parameter gets
                // inserted under targetParam
                success &= targetParam.RegisterOnInsertChild(
                    im::Parameter::GenerateUniqueNotificationTag(),
                    boost::bind(&xel::EventLogManager::OnParameterInsert, this, _1),
                    &disconnector);
            }
            else if (item->GetType() == QStringLiteral("OnParameterDelete"))
            {
                // OnPostDeleteChild Registration is to get notification whenever any child of
                // targetParam gets deleted from IM
                success &= targetParam.RegisterOnPostDeleteChild(
                    im::Parameter::GenerateUniqueNotificationTag(),
                    boost::bind(&xel::EventLogManager::OnParameterDelete, this, _1),
                    &disconnector);
            }
            m_disconnectors.push_back(disconnector);
        }
    }

    return success;
}

// Main UI thread
bool xel::EventLogManager::RegisterForParameterMachineStatusChanged()
{
    bool success = true;

    m_machine_status = im::InformationManager::GetParameter(gd::xpath_MachineState);
    
    if (m_machine_status.IsNull())
    {
        return false;
    }
    
    im::Parameter machineState = im::InformationManager::GetParameter(gd::xpath_MachineOverallState);

    if (machineState.IsNull())
    {
        return false;
    }

    NotificationDisconnector disconnector;
    
    success &= machineState.RegisterOnUpdate(
                        im::Parameter::GenerateUniqueNotificationTag(),
                        boost::bind(&xel::EventLogManager::OnParameterMachineStatusChanged, this, _1),
                        &disconnector);
    m_disconnectors.push_back(disconnector);
    return success;
}

// Main and worker thread
void xel::EventLogManager::EndDownTime(const xal::Alarm &downTimeStartAlarm)
{
    event_lock lockdata(m_mutex);
    QMap<QString, int> categoryFlagValues;

    GetCategoryFlagInfo(downTimeStartAlarm.GetCategories(), categoryFlagValues);

    int availability       = categoryFlagValues.value(kPrinterAvailability);
    int proxy1availability = categoryFlagValues.value(kOperational_Availability_Proxy1);

    if (1 == availability)
    {
        VJ_ASSERT(m_AvailabilityEventCnt >= 1);
        m_AvailabilityEventCnt--;
        if (m_AvailabilityEventCnt == 0)
        {
            RaiseInternalEvent(m_kPrinterUpEventID, m_kPrinterUpEventName, true);
            // Clear Down event if we do not have any availability events open..
            ClearEvent(m_kPrinterDownEventID);
        }
    }

    if (1 == proxy1availability)
    {
        VJ_ASSERT(m_Proxy1AvailabilityEventCnt >= 1);
        m_Proxy1AvailabilityEventCnt--;
        if (m_Proxy1AvailabilityEventCnt == 0)
        {
            RaiseInternalEvent(m_kProxy1PrinterUpEventID, m_kProxy1PrinterUpEventName, true);
            // Clear Down event if we do not have any proxy1 availability events open..
            ClearEvent(m_kProxy1PrinterDownEventID);
        }

        if (m_Proxy2AvailabilityEventCnt > 0)
        {
            m_Proxy2AvailabilityEventCnt--;
            if (m_Proxy2AvailabilityEventCnt == 0)
            {
                RaiseInternalEvent(m_kProxy2PrinterUpEventID, m_kProxy2PrinterUpEventName, true);
                // Clear Down event if we do not have any proxy2 availability events open..
                ClearEvent(m_kProxy2PrinterDownEventID);
            }
        }
    }

    QString   preparedQuery = QString("DELETE FROM DOWNTIME_ALARM_LIST WHERE Alarm_Id = %1")
                                     .arg(downTimeStartAlarm.GetId());
    QSqlQuery deleteQuery(m_db);
   
    if (!deleteQuery.exec(preparedQuery))
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedQuery.toStdString());
        VJ_ERROR("SQL Error: " << deleteQuery.lastError().text().toStdString());
    }
}

// Main and worker thread
void xel::EventLogManager::ProcessDowntimeEndConditon(xal::DowntimeEndConditionType type, QVariant Id)
{
    event_lock lock(m_mutex);

    QList<xal::Alarm>::iterator it   = m_DowntimeStartAlarmList.begin();
    
    for(; it != m_DowntimeStartAlarmList.end(); )
    {
        xal::Alarm downTimeStartAlarm                         = *it;
        xal::ExplicitDowntimeEndConditionLst endDownTimeConds = (*it).GetExplicitDowntimeEndConditionList();
        
        bool endDownTime = false;
        if (endDownTimeConds.isEmpty())
        {
           if (type == xal::eDowntimeEndConditionPowerCycle)
           {
               endDownTime = true;
           }
           else if (type == xal::eDowntimeEndConditionAlarmCleared)
           {
               // it has to be same alarm
               if (downTimeStartAlarm.GetId() == Id)
               {
                   endDownTime = true;
               }
           }
        }
        else
        {
            // explicit downtime end conditions, any alarm with matching
            // downtime end conditions will be selected.
            foreach(const xal::DowntimeEndCondition edcond, endDownTimeConds)
            {
                endDownTime = (edcond.GetType() == type) 
                                && (edcond.GetId() == Id || type == xal::eDowntimeEndConditionPowerCycle);
                if (endDownTime)
                {
                    break;
                }
            }
        }

        if (endDownTime)
        {
            EndDownTime(downTimeStartAlarm);
            it = m_DowntimeStartAlarmList.erase(it);
        }
        else
        {
            it++;
        }
    }
}

// Worker thread - asynchronus callback from Alarm Manager
void xel::EventLogManager::OnAlarmRaised( xal::Alarm alarm )
{
    event_lock lock(m_mutex);

    // Event start time
    QString startTime = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);

    // Get alarm Id (event Id)
    int eventId = alarm.GetId();
    
    if (m_throttling)
    {
        VJ_INFO("In throttle mode, discarding alarm raised signal for id "
                 << eventId);
        return;
    }

    // Get alarm name (event name)
    QString alarmName = alarm.GetName();

    // Get alarm variable texts
    QStringList varTexts = alarm.GetVariableTextStringList();
    QString varTextsJoined = varTexts.join("|");

    // Check if Variable text is empty or not
    if ("" != varTextsJoined)
    {
        foreach (const OpenAlarmEvents &openAlarmEvents, m_openAlarmEventsLst)
        {
            if ((openAlarmEvents.eventId == eventId) && (openAlarmEvents.varTexts == varTexts))
            {
                // Check if event is already open.
                // If yes, no need to log again in database
                VJ_TRACE("Event was already open: ID = " << alarm.GetId());
                return;  // E A R L Y   R E T U R N
            }
        }

        OpenAlarmEvents openAlarmEvents;
        openAlarmEvents.eventId = eventId;
        openAlarmEvents.varTexts = varTexts;
        openAlarmEvents.startTime = startTime;

        // Make an entry into open events list based on ID and varTexts
        m_openAlarmEventsLst.append(openAlarmEvents);
    }
    else
    {
        if (m_openEvents.find(eventId) != m_openEvents.end())
        {
            // Check if event is already open.
            // If yes, no need to log again in database
            VJ_TRACE("Event was already open: ID = " << alarm.GetId());
            return;  // E A R L Y   R E T U R N
        }
        else
        {
            // Make an entry into open events map based on ID
            m_openEvents.insert(eventId, startTime);
        }
    }

    // Get event type
    int eventType = Alarm;

    // Categories
    QMap<QString, int> categoryFlagValues;
    GetCategoryFlagInfo(alarm.GetCategories(), categoryFlagValues);

    // Now insert record into database
    if (varTexts.count() > 0)
    {
        event_lock lockrec(m_eventlogrecs_mutex);
        m_eventlogrecs.push_back(EventLogRec(eventType, eventId, alarmName, startTime, "", categoryFlagValues,
                     varTexts.join("|")));
    }
    else
    {
        event_lock lockrec(m_eventlogrecs_mutex);
        m_eventlogrecs.push_back(EventLogRec(eventType, eventId, alarmName, startTime, "", categoryFlagValues));
    }

    int printerAvailability = categoryFlagValues.value(kPrinterAvailability);
    int proxy1Availability = categoryFlagValues.value(kOperational_Availability_Proxy1);
    int proxy2Availability = categoryFlagValues.value(kOperational_Availability_Proxy2);
    
    if (printerAvailability || proxy1Availability || proxy2Availability)
    {
        m_DowntimeStartAlarmList.prepend(alarm);
        // insert into database
        QString preparedInsertQuery = 
            QString("INSERT INTO DOWNTIME_ALARM_LIST(Alarm_Id, Operational_Availability_Proxy2_Flag)VALUES(%1,%2)")
                   .arg(eventId)
                   .arg(proxy2Availability);

        QSqlQuery query(m_db);

        if (!query.exec(preparedInsertQuery))
        {
            VJ_ERROR("SQL query: " << preparedInsertQuery.toStdString() << "failed");
        }
    }
    if (1 == printerAvailability)
    {
        emit logEvent(AvailabilityEvents);
        if (m_AvailabilityEventCnt == 0)
        {
            // Raise printer down if not already existing
            RaiseInternalEvent(m_kPrinterDownEventID, m_kPrinterDownEventName);
        }
        m_AvailabilityEventCnt++;
    }
    if (1 == proxy1Availability)
    {
        emit logEvent(Proxy1OperationalAvailabilityEvents);
        if (m_Proxy1AvailabilityEventCnt == 0)
        {
            RaiseInternalEvent(m_kProxy1PrinterDownEventID, m_kProxy1PrinterDownEventName);
        }
        m_Proxy1AvailabilityEventCnt++;
    }
    if (1 == proxy2Availability)
    {
        emit logEvent(Proxy2OperationalAvailabilityEvents);
        if (m_Proxy2AvailabilityEventCnt == 0)
        {
            RaiseInternalEvent(m_kProxy2PrinterDownEventID, m_kProxy2PrinterDownEventName);
        }
        m_Proxy2AvailabilityEventCnt++;
    }
    else if (Warning == eventType || Alarm == eventType)
    {
        emit logEvent(AlarmEvents);
    }
    else
    {
        emit logEvent(AllEvents);
    }
}

// Worker thread - asynchronus call from Alaram Manager
void xel::EventLogManager::OnAlarmCleared( xal::Alarm alarm )
{
    {
        event_lock lockdata(m_mutex);
        if (m_throttling)
        {
            VJ_INFO("In throttle mode, discarding alarm raised signal for id "
                    << alarm.GetId());
            return;
        }
    }

    ClearAlarm(alarm);
    ProcessDowntimeEndConditon(xal::eDowntimeEndConditionAlarmCleared, QVariant(alarm.GetId()));
}

// Main and worker thread
void xel::EventLogManager::GetCategoryFlagInfo( xal::AlarmPropertyLstPtr categoryList, QMap<QString, int> &flagValueMap )
{
    // First initialize all to 0 (i.e. false)
    flagValueMap.insert(kProduction, 0);
    flagValueMap.insert(kEvent, 0);
    flagValueMap.insert(kService, 0);
    flagValueMap.insert(kSystem, 0);
    flagValueMap.insert(kSecurity, 0);
    flagValueMap.insert(kPrinterAvailability, 0);
    flagValueMap.insert(kOperational_Availability_Proxy1, 0);
    flagValueMap.insert(kOperational_Availability_Proxy2, 0);

    if (!categoryList)
    {
        return;  // E A R L Y   R E T U R N
    }

    foreach (xal::AlarmProperty category, *categoryList)
    {
        flagValueMap[category] = 1;
        if 
        (
            category == kOperational_Availability_Proxy1 
            &&
            m_openEvents.find(m_AvailibilityProxy2EventID) != m_openEvents.end()
        )
        {
            flagValueMap[kOperational_Availability_Proxy2] = 1;
        }
    }
}

// Main and worker thread
// This overload is still used for events other than Alarms
void xel::EventLogManager::GetCategoryFlagInfo(im::Parameter node, QMap<QString, int> &flagValueMap)
{
    // First initialize all to 0 (i.e. false)
    flagValueMap.insert(kProduction, 0);
    flagValueMap.insert(kEvent, 0);
    flagValueMap.insert(kService, 0);
    flagValueMap.insert(kSystem, 0);
    flagValueMap.insert(kSecurity, 0);
    flagValueMap.insert(kPrinterAvailability, 0);
    flagValueMap.insert(kOperational_Availability_Proxy1, 0);
    flagValueMap.insert(kOperational_Availability_Proxy2, 0);

    im::ParameterList categoryList = node.Children();
    if (categoryList.IsEmpty())
    {
        return;
    }

    im::ParameterList::iterator itr;
    im::ParameterList::iterator end = categoryList.end();

    for (itr = categoryList.begin(); itr != end; ++itr)
    {
        im::StringParameter category = (*itr);
        if (category)
        {
            WString value = gd::wstrNull;
            category.GetValue(value);
            if (gd::wstrNull != value)
            {
                flagValueMap.insert(fromWString(value), 1);
                if 
                (
                    fromWString(value) == kOperational_Availability_Proxy1
                    &&
                    m_openEvents.find(m_AvailibilityProxy2EventID) != m_openEvents.end()
                )
                {
                    flagValueMap.insert(kOperational_Availability_Proxy2, 1);
                }
            }
        }
    }
}

// Main and Worker thread
bool xel::EventLogManager::InsertRecord(int eventType, int eventId, QString eventDescription, QString start_time,
                                        QString end_time, const QMap<QString, int> &flagValueMap, QString varText)
{
    StoreRowCounts();

    event_lock lock(m_mutex);
    bool retVal = false;
    if (!m_db.isOpen())
    {
        VJ_ERROR("Event database is not ready yet!");
        return retVal;
    }

    if (m_logged_in_user.IsNull())
    {
        m_logged_in_user = im::InformationManager::GetParameter(gd::xpath_LoggedinName);
    }

    WString event_raiser = gd::wstrNull;
    if (m_logged_in_user)
    {
        m_logged_in_user.GetValue(event_raiser);
    }
    //
    // Insert the Event_Id and Event_Description if not present in
    // EVENT_TABLE table
    //
    int     eventRefId = 0;
    QString preparedInsertQuery;

    QString   selectQuery = QString("SELECT DISTINCT SId, Event_Id, Event_Description from EVENT_TABLE\
                                             WHERE((Event_Id = %1) AND (Event_Description = '%2'))")
                                           .arg(eventId)
                                           .arg(eventDescription);
    QSqlQuery query(m_db);

    if (!query.exec(selectQuery))
    {
        VJ_TRACE("SQL query: " << selectQuery.toStdString() << " failed ");
        return false;
    }

    if (query.next())
    {
        //
        // Event_Id and Event_Description present.
        //
        eventRefId =  query.value(kSId).toInt();
    }
    else
    {
        //
        // Event_Id and Event_Description is not present
        //
        preparedInsertQuery = QString("INSERT INTO EVENT_TABLE(Event_Id, Event_Description)\
                                       VALUES(%1,'%2')").arg(eventId).arg(eventDescription);
        if (!query.exec(preparedInsertQuery))
        {
            VJ_TRACE("SQL query: " << preparedInsertQuery.toStdString() << "failed");
            return false;
        }
        if (!query.exec(selectQuery))
        {
          VJ_TRACE("SQL query: " << selectQuery.toStdString() << " failed ");
          return false;
        }

        if (query.next())
        {
           eventRefId =  query.value(kSId).toInt();
        }
        else
        {
          VJ_TRACE("Failed to move to next record");
        }
    }

    // Now Prepare query with fetched data
    // if duration value is NULL or Empty, it means event is closed already.
    if (end_time.isEmpty() || end_time.isNull())
    {
        if (varText.isEmpty() || varText.isNull())
            preparedInsertQuery =
                QString(
                    "INSERT INTO EVENT_LOG(Type, SId, Production_Flag,\
                                              Event_Flag, Service_Flag, System_Flag, Security_Flag, Printer_Availability_Flag,\
                                              Operational_Availability_Proxy1_Flag, Operational_Availability_Proxy2_Flag, Start_Time,\
                                              User_Event_Occurred) VALUES(%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,'%11','%12')")
                    .arg(eventType)
                    .arg(eventRefId)
                    .arg(flagValueMap.value(kProduction))
                    .arg(flagValueMap.value(kEvent))
                    .arg(flagValueMap.value(kService))
                    .arg(flagValueMap.value(kSystem))
                    .arg(flagValueMap.value(kSecurity))
                    .arg(flagValueMap.value(kPrinterAvailability))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy1))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy2))
                    .arg(start_time)
                    .arg(fromWString(event_raiser));
        else
            preparedInsertQuery =
                QString(
                    "INSERT INTO EVENT_LOG(Type, SId, Production_Flag,\
                                              Event_Flag, Service_Flag, System_Flag, Security_Flag, Printer_Availability_Flag,\
                                              Operational_Availability_Proxy1_Flag, Operational_Availability_Proxy2_Flag, Start_Time,\
                                              User_Event_Occurred, Event_Var_Data) VALUES(%1,%2, %3,%4,%5,%6,%7,%8,%9,%10, '%11','%12','%13')")
                    .arg(eventType)
                    .arg(eventRefId)
                    .arg(flagValueMap.value(kProduction))
                    .arg(flagValueMap.value(kEvent))
                    .arg(flagValueMap.value(kService))
                    .arg(flagValueMap.value(kSystem))
                    .arg(flagValueMap.value(kSecurity))
                    .arg(flagValueMap.value(kPrinterAvailability))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy1))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy2))
                    .arg(start_time)
                    .arg(fromWString(event_raiser))
                    .arg(varText);
    }
    else
    {
        if (varText.isEmpty() || varText.isNull())
            preparedInsertQuery =
                QString(
                    "INSERT INTO EVENT_LOG(Type, SId, Production_Flag,\
                                                  Event_Flag, Service_Flag, System_Flag, Security_Flag, Printer_Availability_Flag,\
                                                  Operational_Availability_Proxy1_Flag, Operational_Availability_Proxy2_Flag, Start_Time,\
                                                  End_Time, User_Event_Occurred) VALUES(%1,%2, %3,%4,%5,%6,%7,%8,%9,%10, '%11','%12','%13')")
                    .arg(eventType)
                    .arg(eventRefId)
                    .arg(flagValueMap.value(kProduction))
                    .arg(flagValueMap.value(kEvent))
                    .arg(flagValueMap.value(kService))
                    .arg(flagValueMap.value(kSystem))
                    .arg(flagValueMap.value(kSecurity))
                    .arg(flagValueMap.value(kPrinterAvailability))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy1))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy2))
                    .arg(start_time)
                    .arg(end_time)
                    .arg(fromWString(event_raiser));
        else
            preparedInsertQuery =
                QString(
                    "INSERT INTO EVENT_LOG(Type, SId, Production_Flag,\
                                              Event_Flag, Service_Flag, System_Flag, Security_Flag, Printer_Availability_Flag,\
                                              Operational_Availability_Proxy1_Flag, Operational_Availability_Proxy2_Flag, Start_Time,\
                                              End_Time, User_Event_Occurred, Event_Var_Data) VALUES(%1,%2, %3,%4,%5,%6,%7,%8,%9,%10, '%11','%12','%13','%14')")
                    .arg(eventType)
                    .arg(eventRefId)
                    .arg(flagValueMap.value(kProduction))
                    .arg(flagValueMap.value(kEvent))
                    .arg(flagValueMap.value(kService))
                    .arg(flagValueMap.value(kSystem))
                    .arg(flagValueMap.value(kSecurity))
                    .arg(flagValueMap.value(kPrinterAvailability))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy1))
                    .arg(flagValueMap.value(kOperational_Availability_Proxy2))
                    .arg(start_time)
                    .arg(end_time)
                    .arg(fromWString(event_raiser))
                    .arg(varText);
    }

    //            VJ_TRACE("SQL query: " << preparedInsertQuery.toStdString());
    
    retVal = query.exec(preparedInsertQuery);
    if (!retVal)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedInsertQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
    }
    return retVal;
}

// Worker Thread
void xel::EventLogManager::OnEventTriggerAction( QSharedPointer<EventLogRec> rec)
{
    event_lock lock(m_mutex);
    
    int eventId;
    im::LogEventParameter event = static_cast<im::LogEventParameter>(rec->m_node);
    im::Int32Parameter eventIDParam = event.GetParameter(gd::xml_EventID);
    eventIDParam.GetValue(eventId);

    if (m_openEvents.find(eventId) != m_openEvents.end())
    {
        return;
    }
    rec->m_eventId = eventId;
    
    //VJ_INFO("Event Raised " << eventId << "at " << rec->m_start_time.toStdString());
    
    if (rec->m_eventIsOpen)
    {
        m_openEvents.insert(eventId, rec->m_start_time);
    }
    
    emit onEventTrigger(rec->m_node);
    im::StringParameter categoriesParam = event.GetParameter(gd::xml_Categories);
    VJ_ASSERT(categoriesParam);

    GetCategoryFlagInfo(categoriesParam, rec->m_categoryFlagValues);

    ProcessDowntimeEndConditon(xal::eDowntimeEndConditionEventRaised, QVariant(eventId));

    {
        event_lock logrec(m_eventlogrecs_mutex);
        // Now insert record into database
        m_eventlogrecs.push_back(*rec);
    }

    // Check printer availability flag to add internal "Printer Down" event
    int printerAvailability = rec->m_categoryFlagValues.value(kPrinterAvailability);
    // Check printer availability flag to add internal "Printer Down" event for Proxy1
    int proxy1Availability = rec->m_categoryFlagValues.value(kOperational_Availability_Proxy1);
    // Check printer availability flag to add internal "Printer Down" event for Proxy2
    int proxy2Availability = rec->m_categoryFlagValues.value(kOperational_Availability_Proxy2);

    if (eventId == m_AvailibilityProxy2EventID)
    {
        RaiseInternalEvent(m_kProxy2SystemStartupEventID, m_kProxy2SystemStartupEventName);
    }

    if (1 == printerAvailability)
    {
        if (m_AvailabilityEventCnt == 0)
        {
            // Raise printer down if not already existing
            RaiseInternalEvent(m_kPrinterDownEventID, m_kPrinterDownEventName);
        }
        m_AvailabilityEventCnt++;
    }

    if (1 == proxy1Availability)
    {
        if (m_Proxy1AvailabilityEventCnt == 0)
        {
            // Raise proxy1 printer down if not already existing
            RaiseInternalEvent(m_kProxy1PrinterDownEventID, m_kProxy1PrinterDownEventName);
        }
        m_Proxy1AvailabilityEventCnt++;
    }

    if (1 == proxy2Availability)
    {
        if (m_Proxy2AvailabilityEventCnt == 0)
        {
            // Raise proxy2 printer down if not already existing
            RaiseInternalEvent(m_kProxy2PrinterDownEventID, m_kProxy2PrinterDownEventName);
        }
        m_Proxy2AvailabilityEventCnt++;
    }

    emit updateDatabase();
}

// Main UI thread
void xel::EventLogManager::OnEventTrigger( im::Parameter node)
{
    event_lock lock(m_mutex);
    
    if (m_throttling)
    {
        VJ_INFO("In throttle mode - discarding event");
        return;
    }

    im::LogEventParameter event = static_cast<im::LogEventParameter>(node);

    if (event.IsNull())
    {
        return;
    }

    // Get all required data of this parameter to insert into database

    // Get Class
    im::LogEventClass eventType;
    event.GetClass(eventType);

    // Get start time and date
    DateTimeUtc start_time;
    event.GetStartTime(start_time);
    QString startTime = start_time.GetValue().toString(Qt::ISODate);

    QString endTime;
    // Get Duration
    int duration;
    event.GetDuration(duration);

    // Get Event Name
    WString eventName;
    event.GetName(eventName);

    // format and store the node name as event name if the eventName defaults to empty string
    // after call to event.GetName()
    if (0 == eventName.compare(L""))
    {
        eventName = formatWString(node.GetName());
    }

    bool isEventOpen;
    event.IsOpen(isEventOpen);

    if (!isEventOpen)
    {
        if (duration > 0)
        {
            endTime = start_time.GetValue().addSecs(duration).toString(Qt::ISODate);
        }
        else
        {
            endTime = startTime;
        }
    }

    WString additionalInformation;
    event.GetAdditionalInformation(additionalInformation);

    if (gd::wstrNull != additionalInformation)
    {
        eventName.append(L" - ").append(additionalInformation);
    }

    QSharedPointer<EventLogRec> rec(new EventLogRec
                                        (
                                            eventType,
                                            fromWString(eventName),
                                            startTime,
                                            endTime,
                                            node,
                                            isEventOpen
                                        )
                                    );
    emit eventRaised(rec);
}

// Main UI thread - used for clearing an event
void xel::EventLogManager::ClearEvent( int eventId )
{
    event_lock lock(m_mutex);

    WriteEventLogRecords();

    QString endTime = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);
    VJ_TRACE("Clearing event: " << eventId << " - " << endTime.toStdString());

    if (m_logged_in_user.IsNull())
    {
        m_logged_in_user = im::InformationManager::GetParameter(gd::xpath_LoggedinName);
    }

    WString user = gd::wstrNull;
    if (m_logged_in_user)
    {
        m_logged_in_user.GetValue(user);
    }

    WriteEventLogRecords();
    
    if (m_openEvents.find(eventId) != m_openEvents.end())
    {
        // Update the database with User logged-in information at the time of clearing the event.
         QString preparedQuery = QString(
                                    "UPDATE EVENT_LOG \
                                        SET User_Event_Cleared='%1', End_Time='%4' \
                                     WHERE EXISTS ( \
                                        SELECT *  \
                                           FROM EVENT_TABLE \
                                          WHERE EVENT_TABLE.SId = EVENT_LOG.SId AND Event_Id=%2 AND Start_Time='%3'\
                                     )")
                                    .arg(fromWString(user))
                                    .arg(eventId)
                                    .arg(m_openEvents.value(eventId))
                                    .arg(endTime);

        // Update Record
        QSqlQuery updateQuery(m_db);
        bool ok = updateQuery.exec(preparedQuery);
        if (!ok)
        {
            VJ_ERROR("FAILED to execute SQL query: " << preparedQuery.toStdString());
            VJ_ERROR("SQL Error: " << updateQuery.lastError().text().toStdString());
        }

        // Event is cleared, remove it from the map
        m_openEvents.remove(eventId);

        emit clearEvent();
    }
}

// Worker thread - used for clearing an alarm
void xel::EventLogManager::ClearAlarm( xal::Alarm alarm )
{
    event_lock lock(m_mutex);
    
    WriteEventLogRecords();
    // Get Event ID(Alarm ID)
    int eventId = alarm.GetId();

    // Get Alarm Variable Text
    QStringList variableTexts = alarm.GetVariableTextStringList();
    QString variableTextJoined = variableTexts.join("|");

    QString endTime = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);
    VJ_TRACE("Clearing alarm: " << eventId << " - " << endTime.toStdString());

    if (m_logged_in_user.IsNull())
    {
        m_logged_in_user = im::InformationManager::GetParameter(gd::xpath_LoggedinName);
    }

    WString user = gd::wstrNull;
    if (m_logged_in_user)
    {
        m_logged_in_user.GetValue(user);
    }

    QString preparedQuery;
    if ("" != variableTextJoined)
    {
        QString startTime;
        int index = 0;
        foreach (const OpenAlarmEvents &openAlarmEvents, m_openAlarmEventsLst)
        {
            if ((openAlarmEvents.eventId == eventId) && (openAlarmEvents.varTexts == variableTexts))
            {
                // Read the cleared Event  start time
                startTime = openAlarmEvents.startTime;

                // Remove the cleared Event form open events list
                m_openAlarmEventsLst.removeAt(index);
                break;
            }
            index++;
        }

        // Update the database with User logged-in information at the time of clearing the event.
        preparedQuery = QString(
                                "UPDATE EVENT_LOG \
                                    SET User_Event_Cleared='%1', End_Time='%2' \
                                 WHERE EXISTS ( \
                                    SELECT *  \
                                       FROM EVENT_TABLE \
                                      WHERE EVENT_TABLE.SId = EVENT_LOG.SId AND Event_Var_Data='%3' AND Start_Time='%4'\
                                 )")
                                .arg(fromWString(user))
                                .arg(endTime)
                                .arg(variableTextJoined)
                                .arg(startTime);
    }
    else
    {
        // Update the database with User logged-in information at the time of clearing the event.
        preparedQuery = QString(
                                "UPDATE EVENT_LOG \
                                    SET User_Event_Cleared='%1', End_Time='%4' \
                                 WHERE EXISTS ( \
                                    SELECT *  \
                                       FROM EVENT_TABLE \
                                      WHERE EVENT_TABLE.SId = EVENT_LOG.SId AND Event_Id=%2 AND Start_Time='%3'\
                                 )")
                                .arg(fromWString(user))
                                .arg(eventId)
                                .arg(m_openEvents.value(eventId))
                                .arg(endTime);

        // Event is cleared, remove it from the map
        m_openEvents.remove(eventId);
    }

    // Update Record
    QSqlQuery updateQuery(m_db);
    bool ok = updateQuery.exec(preparedQuery);
    if (!ok)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedQuery.toStdString());
        VJ_ERROR("SQL Error: " << updateQuery.lastError().text().toStdString());
    }

    emit clearEvent();
}

// Main thread
void xel::EventLogManager::OnClearEvent( im::Parameter node)
{
    event_lock lock(m_mutex);

    if (m_throttling)
    {
        VJ_INFO("In throttle mode - discarding clear event");
        return;
    }
    emit performClearEvent(node);
}

// Worker thread
void xel::EventLogManager::OnClearEventAction(const xplatform::information_manager::Parameter &node)
{
    im::LogEventParameter avail_event = static_cast<im::LogEventParameter>(node);

    if (avail_event)
    {
        im::Int32Parameter eventIDParam = avail_event.GetParameter(gd::xml_EventID);
        int eventId;

        eventIDParam.GetValue(eventId);
        bool evnetIsRaised = (m_openEvents.find(eventId) != m_openEvents.end());

        ClearEvent(eventId);

        if (!evnetIsRaised)
        {
            return;
        }
        event_lock lockdata(m_mutex);
        QMap<QString, int> categoryFlagValues;
        
        im::StringParameter categories = avail_event.GetParameter(gd::xml_Categories);
        
        GetCategoryFlagInfo(categories, categoryFlagValues);

        int availability       = categoryFlagValues.value(kPrinterAvailability);
        int proxy1availability = categoryFlagValues.value(kOperational_Availability_Proxy1);

        if (eventId == m_AvailibilityProxy2EventID)
        {
            RaiseInternalEvent(m_kProxy2SystemShutdownEventID, m_kProxy2SystemShutdownEventName);
        }
        if (1 == availability)
        {
            VJ_ASSERT(m_AvailabilityEventCnt >= 1);
            m_AvailabilityEventCnt--;
            if (m_AvailabilityEventCnt == 0)
            {
                RaiseInternalEvent(m_kPrinterUpEventID, m_kPrinterUpEventName, true);
                // Clear Down event if we do not have any availability events open..
                ClearEvent(m_kPrinterDownEventID);
            }
        }

        if (1 == proxy1availability)
        {
            VJ_ASSERT(m_Proxy1AvailabilityEventCnt >= 1);
            m_Proxy1AvailabilityEventCnt--;
            if (m_Proxy1AvailabilityEventCnt == 0)
            {
                RaiseInternalEvent(m_kProxy1PrinterUpEventID, m_kProxy1PrinterUpEventName, true);
                // Clear Down event if we do not have any proxy1 availability events open..
                ClearEvent(m_kProxy1PrinterDownEventID);
            }

            if (m_Proxy2AvailabilityEventCnt > 0)
            {
                m_Proxy2AvailabilityEventCnt--;
                if (m_Proxy2AvailabilityEventCnt == 0)
                {
                    RaiseInternalEvent(m_kProxy2PrinterUpEventID, m_kProxy2PrinterUpEventName, true);
                    // Clear Down event if we do not have any proxy2 availability events open..
                    ClearEvent(m_kProxy2PrinterDownEventID);
                }
            }
        }
    }
}

void xel::EventLogManager::GetLogDataAndInsertAction(QSharedPointer<EventLogParameterRec> param)
{
    VJ_ASSERT(!param->m_logEventItemsPtr->isEmpty());
    // Now look for the item and fetch the required data to log
    QString keystr = param->m_targetpath;//fromWString(param->m_node.GetXPath());
    EventLogItemPtr item = param->m_logEventItemsPtr->value(keystr);
    VJ_ASSERT(!item.isNull());
    emit onEventTrigger(param->m_node);
    if (!item.isNull())
    {
    // Get Event ID
    int32 eventId = item->GetEventID();
    // Get Event Type
    int32 eventType = UserAction;
    // Get Event Name/Description
    QString eventDescription = item->GetEventName();

    // Fetch any additional information
    //if (!param->m_additionalInfo.isEmpty())
    //{
    //    eventDescription.append(param->m_additionalInfo);
    //}

    QString startTime = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);
    QString endTime = startTime;

    QMap<QString, int> categoryFlagValues;
    GetCategoryFlagInfo(item->GetCategories(),categoryFlagValues);

    EventLogRec rec(eventType, eventId, eventDescription, startTime, endTime,
                         categoryFlagValues);

    event_lock logrec(m_eventlogrecs_mutex);

    m_eventlogrecs.push_back(rec);
    }
    emit logEvent(false);
}

// Main UI thread
void xel::EventLogManager::OnParameterUpdate( im::Parameter node )
{
    QString info = getAdditionalInformation(m_additonalInfo, node);
    QString targetPath = m_OnUpdateTargetPathMap[node];
    QSharedPointer<EventLogParameterRec> paramInfo( new EventLogParameterRec(m_OnUpdateLogItemsMapPtr, info,targetPath, node));
    emit parameterChanged(paramInfo);
}

// Main UI thread
void xel::EventLogManager::OnParameterInsert( im::Parameter node )
{
    QString info = getAdditionalInformation(m_additonalInfo, node);
    QString targetPath = m_OnInsertTargetPathMap[node];
    QSharedPointer<EventLogParameterRec> paramInfo( new EventLogParameterRec(m_OnInsertLogItemsMapPtr, info, targetPath,node));
    emit parameterChanged(paramInfo);
}

// Worker thread
void xel::EventLogManager::OnParameterDelete( im::Parameter node )
{
    QString info = getAdditionalInformation(m_additonalInfo, node);
    QString targetPath = m_OnDeleteTargetPathMap[node];
    QSharedPointer<EventLogParameterRec> paramInfo( new EventLogParameterRec(m_OnDeleteLogItemsMapPtr, info,targetPath, node));
    emit parameterChanged(paramInfo);
}

// Worker thread
void xel::EventLogManager::OnParameterMachineStatusChangedAction(QString  str)
{
    ProcessDowntimeEndConditon(xal::eDowntimeEndConditionMachineStatusChanged, QVariant(str));
}

// Main UI thread
void xel::EventLogManager::OnParameterMachineStatusChanged( im::Parameter node )
{
    QString    strVal = fromWString(node.ToString());

    emit machineStatusChange(strVal);
}

// both Main and worker thread
void xel::EventLogManager::UpdateLocale()
{
    event_lock lock(m_mutex);
    m_locale = QLocale(QLocale::English);

    int32 language = 0;
    int32 country = 0;

    bool success = m_language.GetValue(language);
    m_country.GetValue(country);

    if (success)
    {
        m_locale = QLocale((QLocale::Language)language, (QLocale::Country)country);
    }
}

// Main UI
QString xel::EventLogManager::GetLocalizedDate(const QDate &date) const
{
    event_lock lock(m_mutex);
    return (m_locale.toString(date, QLocale::ShortFormat));
}

// Main UI
QString xel::EventLogManager::GetLocalizedTime(const QTime &time) const
{
    event_lock lock(m_mutex);
    return (m_locale.toString(time, QLocale::ShortFormat));
}

// Wroker thread
void  xel::EventLogManager::GetEventLogsForPrinterLogsAction
(
    const QList<Flags>      &eventFlags, 
    const QList<EventTypes> &eventTypes, 
    bool                    isHijraCalender,
    int                     offset,
    int                     limit
)
{
    //
    // If no event selection criteria provided, then
    // return no data will be provided
    //
    if (eventFlags.empty() && eventTypes.empty())
    {
        VJ_INFO("No event selection criteria provided, returning an empty list");
        return;
    }

    event_lock lock(m_mutex);

     // Get date exactly 18 months earlier from now
    QString date18MonthsEarlier =
        m_pIdtHandler->getCurrentDateTimeUtc().addMonths(-kMaxEventAgeMonths).toString(Qt::ISODate);
    QList<EventLogData> eventLogDataList;
    QString preparedSelectQuery;
    QList<QString> queryFormatter = PopulateQuery(eventFlags, eventTypes);
    int size = queryFormatter.size();
    QString flagsQuery;
    QString eventTypeQuery;

    if (2 == size)
    {
        flagsQuery = queryFormatter[0];
        eventTypeQuery = queryFormatter[1];
    }

    if (eventFlags.empty())
    {
        preparedSelectQuery =
            QString(
                "SELECT DISTINCT Type, Event_Id, Event_Description, Start_Time, End_Time, User_Event_Cleared, Event_Var_Data, \
                                              cast(  (strftime('%s',End_Time) - strftime('%s',Start_Time) ) AS real  ) AS Duration \
                                              FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1')) >= 0) \
                                              AND ( %2 ) LIMIT %3 OFFSET %4")
                .arg(date18MonthsEarlier)
                .arg(eventTypeQuery)
                .arg(limit)
                .arg(offset);
    }
    else if (eventTypes.empty())
    {
        preparedSelectQuery =
            QString(
                "SELECT DISTINCT Type, Event_Id, Event_Description, Start_Time, End_Time, User_Event_Cleared, Event_Var_Data, \
                                              cast(  (strftime('%s',End_Time) - strftime('%s',Start_Time) ) AS real  ) AS Duration \
                                              FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1') ) >= 0) \
                                              AND ( %2 ) LIMIT %3 OFFSET %4")
                .arg(date18MonthsEarlier)
                .arg(flagsQuery)
                .arg(limit)
                .arg(offset);
    }
    else
    {
        preparedSelectQuery =
            QString(
                "SELECT DISTINCT Type, Event_Id, Event_Description, Start_Time, End_Time, User_Event_Cleared, Event_Var_Data, \
                                              cast(  (strftime('%s',End_Time) - strftime('%s',Start_Time) ) AS real  ) AS Duration \
                                              FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1') ) >= 0) \
                                              AND (( %2 ) OR ( %3 )) LIMIT %4 OFFSET %5")
                .arg(date18MonthsEarlier)
                .arg(flagsQuery)
                .arg(eventTypeQuery)
                .arg(limit)
                .arg(offset);
    }
    QList<xel::EventLogData> data;
    data = GetEventLogs(preparedSelectQuery, eventLogDataList, isHijraCalender);
    
    xplatform::event_logging::EventLogDataLstPtr dataList(new  QList<xel::EventLogData>(data));
    
    emit printerLogData(dataList, isHijraCalender, offset, limit);
}

// Public API, Main UI
void xel::EventLogManager::GetEventLogsForPrinterLogsAsync
(
    const QList<Flags>      &eventFlags, 
    const QList<EventTypes> &eventTypes, 
    bool                    isHijraCalender,
    int                     offset,
    int                     limit
)
{
    emit getPrinterLogs(eventFlags, eventTypes, isHijraCalender, offset, limit);
}

// Public API, Main UI
QList<xel::EventLogData> xel::EventLogManager::GetEventLogsForPrinterLogs(
    const QList<Flags> &eventFlags, const QList<EventTypes> &eventTypes /* = QList<EventTypes>() */,
    bool isHijraCalender, int offset, int limit)

{
    event_lock lock(m_mutex);
    //
    // If no event selection criteria provided, then
    // return an empty event log data.
    //
    if (eventFlags.empty() && eventTypes.empty())
    {
        VJ_INFO("No event selection criteria provided, returning an empty list");
        return QList<EventLogData>();
    }

    // Get date exactly 18 months earlier from now
    QString date18MonthsEarlier =
        m_pIdtHandler->getCurrentDateTimeUtc().addMonths(-kMaxEventAgeMonths).toString(Qt::ISODate);
    QList<EventLogData> eventLogDataList;
    QString preparedSelectQuery;
    QList<QString> queryFormatter = PopulateQuery(eventFlags, eventTypes);
    int size = queryFormatter.size();
    QString flagsQuery;
    QString eventTypeQuery;

    if (2 == size)
    {
        flagsQuery = queryFormatter[0];
        eventTypeQuery = queryFormatter[1];
    }

    if (eventFlags.empty())
    {
        preparedSelectQuery =
            QString(
                "SELECT DISTINCT Type, Event_Id, Event_Description, Start_Time, End_Time, User_Event_Cleared, Event_Var_Data, \
                                              cast(  (strftime('%s',End_Time) - strftime('%s',Start_Time) ) AS real  ) AS Duration \
                                              FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1')) >= 0) \
                                              AND ( %2 ) LIMIT %3 OFFSET %4")
                .arg(date18MonthsEarlier)
                .arg(eventTypeQuery)
                .arg(limit)
                .arg(offset);
    }
    else if (eventTypes.empty())
    {
        preparedSelectQuery =
            QString(
                "SELECT DISTINCT Type, Event_Id, Event_Description, Start_Time, End_Time, User_Event_Cleared, Event_Var_Data, \
                                              cast(  (strftime('%s',End_Time) - strftime('%s',Start_Time) ) AS real  ) AS Duration \
                                              FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1') ) >= 0) \
                                              AND ( %2 ) LIMIT %3 OFFSET %4")
                .arg(date18MonthsEarlier)
                .arg(flagsQuery)
                .arg(limit)
                .arg(offset);
    }
    else
    {
        preparedSelectQuery =
            QString(
                "SELECT DISTINCT Type, Event_Id, Event_Description, Start_Time, End_Time, User_Event_Cleared, Event_Var_Data, \
                                              cast(  (strftime('%s',End_Time) - strftime('%s',Start_Time) ) AS real  ) AS Duration \
                                              FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1') ) >= 0) \
                                              AND (( %2 ) OR ( %3 )) LIMIT %4 OFFSET %5")
                .arg(date18MonthsEarlier)
                .arg(flagsQuery)
                .arg(eventTypeQuery)
                .arg(limit)
                .arg(offset);
    }

    return GetEventLogs(preparedSelectQuery, eventLogDataList, isHijraCalender);
}

// Main UI and worker thread
int xel::EventLogManager::GetEventLogsSizeForPrinterLogs(
    const QList<Flags> &eventFlags, const QList<EventTypes> &eventTypes /* = QList<EventTypes>() */)

{
    event_lock lock(m_mutex);
    //
    // If no event selection criteria provided, then
    // return an empty event log data.
    //
    if (eventFlags.empty() && eventTypes.empty())
    {
        VJ_INFO("No event selection criteria provided, returning an empty list");
        return 0;
    }

    // Get date exactly 18 months earlier from now
    QString date18MonthsEarlier =
        m_pIdtHandler->getCurrentDateTimeUtc().addMonths(-kMaxEventAgeMonths).toString(Qt::ISODate);
    QList<EventLogData> eventLogDataList;
    QString preparedSelectQuery;
    QList<QString> queryFormatter = PopulateQuery(eventFlags, eventTypes);
    int size = queryFormatter.size();
    QString flagsQuery;
    QString eventTypeQuery;

    if (2 == size)
    {
        flagsQuery = queryFormatter[0];
        eventTypeQuery = queryFormatter[1];
    }

    if (eventFlags.empty())
    {
        preparedSelectQuery =
        QString(
            "SELECT COUNT( * ) AS Total FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1') ) >= 0) \
                                                AND ( %2 ) ")
            .arg(date18MonthsEarlier)
            .arg(eventTypeQuery);
    }
    else if (eventTypes.empty())
    {
        preparedSelectQuery =
        QString(
            "SELECT COUNT( * ) AS Total FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1') ) >= 0) \
                                                AND ( %2 ) ")
            .arg(date18MonthsEarlier)
            .arg(flagsQuery);
    }
    else
    {
        preparedSelectQuery =
            QString(
                "SELECT COUNT( * ) AS Total FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (julianday(date(Start_Time)) - julianday(date('%1') ) >= 0) \
                                                 AND (( %2 ) OR ( %3 )) ")
                .arg(date18MonthsEarlier)
                .arg(flagsQuery)
                .arg(eventTypeQuery);
    }

    return GetEventLogSize(preparedSelectQuery);
}

QList<xel::AvailabilityLogData> xel::EventLogManager::GetAvailabilityLogs()
{
    event_lock lock(m_mutex);

    WriteEventLogRecords();
    CalculateOEEAvailabity();
    QList<AvailabilityLogData> availLogDataLst;
    AvailabilityLogData currentLogData;

    QList<OEEAvailableData>::iterator iter;
    for (iter = m_AvailabilityLogDataList.begin(); iter != m_AvailabilityLogDataList.end(); ++iter)
    {
        // only add it to the model if there are data present
        if ((*iter).timeFrame != eUndefined)
        {
            currentLogData.timeframe = GetTimeFrameName((*iter).timeFrame);

            currentLogData.printerAvailability =
                QString::number((*iter).printerAvailability, 'f', 1) + "%";
            currentLogData.opAvProxy1 = QString::number((*iter).opAvProxy1, 'f', 1) + "%";
            currentLogData.opAvProxy2 = QString::number((*iter).opAvProxy2, 'f', 1) + "%";

            availLogDataLst << AvailabilityLogData(currentLogData);
        }
    }

    return availLogDataLst;
}

// Main UI thread
QList<xel::EventStatItem> xel::EventLogManager::GetEventStatItems()
{
    event_lock lock(m_mutex);
    return m_EventStatItemList;
}

// Main UI thread
QList<xel::EventOccurenceItem> xel::EventLogManager::GetEventOccurenceItems()
{
    event_lock lock(m_mutex);
    return m_EventOccurences;
}

// Main UI and Worker thread
void xel::EventLogManager::UpdateOEENodesForLastThirtyDay()
{
    QList<xel::AvailabilityLogData> availabilityLogs = GetAvailabilityLogs();
    QList<xel::AvailabilityLogData>::iterator iter;
    for (iter= availabilityLogs.begin(); iter != availabilityLogs.end(); ++iter)
    {
        if( iter->timeframe == GetTimeFrameName(xel::eThirtyDays))
        {
            im::StringParameter printerAvailability =
                    im::InformationManager::GetParameter(gd::xpath_PrinterAvailability);
            VJ_ASSERT(printerAvailability);

            printerAvailability.SetValue(iter->printerAvailability);

            im::StringParameter operationalAvailabilityProxy1 =
                    im::InformationManager::GetParameter(gd::xpath_OperationalAvailabilityProxy1);
            VJ_ASSERT(operationalAvailabilityProxy1);

            operationalAvailabilityProxy1.SetValue(iter->opAvProxy1);

            im::StringParameter operationalAvailabilityProxy2 =
                    im::InformationManager::GetParameter(gd::xpath_OperationalAvailabilityProxy2);
            VJ_ASSERT(operationalAvailabilityProxy2);

            operationalAvailabilityProxy2.SetValue(iter->opAvProxy2);
        }
    }
}

QString xel::EventLogManager::GetTimeFrameName(OEETimeFrame aTimeFrame)
{
    event_lock lock(m_mutex);
    QString timeFrameName;
    switch (aTimeFrame)
    {
        case eThirtyDays:
            timeFrameName = qApp->translate("PerformanceDataApp", "Last 30 Days");
            break;
        case eNinetyDays:
            timeFrameName = qApp->translate("PerformanceDataApp", "Last 90 Days");
            break;
        case eCurrentMonth:
            timeFrameName = qApp->translate("PerformanceDataApp", "Current Month");
            break;
        case eLastMonth:
        case eLastSecondMonth:
        case eLastThirdMonth:
        case eLastFourthMonth:
        case eLastFifthMonth:
        case eLastSixthMonth:
        {
            int monthVal = QDate::currentDate().addMonths(-aTimeFrame).month();
            timeFrameName = m_locale.monthName(monthVal, QLocale::LongFormat);
            break;
        }
        case eUndefined:
        default:
            VJ_ERROR("Undefined Timeframe");
            break;
    }
    return timeFrameName;
}

QList<QString> xel::EventLogManager::PopulateQuery(const QList<Flags> &eventFlags,
                                                   const QList<EventTypes> &eventTypes)
{
    QList<QString> queryFormatter;

    // Prepare category flags query
    QString flagsQuery;
    int size = eventFlags.size();
    int i;

    // if only Alarms need to be shown then don't get categories
    if (0 < size)
    {
        // i < (size - 1) to avoid appending 'OR' operator after the last element
        for (i = 0; i < (size - 1); i++)
        {
            flagsQuery.append(QString("%1=1 OR ").arg(m_flagsMap.value(eventFlags[i])));
        }
        flagsQuery.append(QString("%1=1").arg(m_flagsMap.value(eventFlags[i])));
    }

    queryFormatter.push_back(flagsQuery);

    // Prepare event types query
    QString eventTypeQuery;
    size = eventTypes.size();

    if (0 < size)
    {
        // i < (size - 1) to avoid appending 'OR' operator after the last element
        for (i = 0; i < (size - 1); i++)
        {
            eventTypeQuery.append(QString("Type=%1 OR ").arg(eventTypes.at(i)));
        }
        eventTypeQuery.append(QString("Type=%1").arg(eventTypes.at(i)));
    }

    queryFormatter.push_back(eventTypeQuery);

    return queryFormatter;
}

// Main UI and Worker thread (Public API)
void xel::EventLogManager::RaiseInternalEvent(const int eventID, const QString &eventName, bool closed)
{
    event_lock lock(m_mutex);
    // create category flags
    QMap<QString, int> categoryFlagValues;
    categoryFlagValues.insert(kSystem, 1);
    im::LogEventClass logClass = im::eStatus;

    // Event start time
    QString startTime = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);

    // Now insert record into database
    if (!closed)
    {
        // make an entry into open events map
        m_openEvents.insert(eventID, startTime);
        event_lock logrec(m_eventlogrecs_mutex);
        m_eventlogrecs.push_back(EventLogRec(logClass, eventID, eventName, startTime, "", categoryFlagValues));
    }
    else
    {
        event_lock logrec(m_eventlogrecs_mutex);
        // Starttime == endtime when raising a closed event
        m_eventlogrecs.push_back(EventLogRec(logClass, eventID, eventName, startTime, startTime, categoryFlagValues));
    }

    if (eventID == m_kSystemStartupEventID)
    {
        // Calculate OEE Data on Startup
        // this needs to be fixed.
        RestoreOEEAvailabilityData();
        InitDailyTimer();
    }

    // VJ_TRACE("Raised Internal Event: " << eventID << " : " << eventName.toStdString());
}

bool xel::EventLogManager::DumpTableToCsvFile(const QString &csvFilePath, const QString &tableName)
{
    event_lock lock(m_mutex);
    bool outcome = true;
    if (!m_db.open())
    {
        VJ_ERROR("The Event Log Database is not open");
        outcome = false;
    }
    
    WriteEventLogRecords();

    QFile outputFile(csvFilePath);
    outputFile.open(QIODevice::WriteOnly);
    if (outcome)
    {
        if (!outputFile.isOpen())
        {
            VJ_ERROR("Error unable to open " << csvFilePath.toStdString());
            outcome = false;
        }
    }

    if (outcome)
    {
        VJ_INFO("Dumping database table [" << tableName.toStdString()
                                           << "] to : " << csvFilePath.toStdString());

        // Count the total number of records. This will be used to show the
        // progress in the UI.
        int rowsTotal(-1);
        QSqlQuery countQuery(m_db);
        if ((countQuery.exec(QString("SELECT COUNT(*) FROM %1").arg(tableName))))
        {
            if (countQuery.next())
            {
                QSqlRecord record = countQuery.record();
                if (record.count() > 0)
                {
                    rowsTotal = record.value(0).toInt();
                }
            }
        }

        QSqlQuery query(m_db);
        bool header = true;

        // This query directly calculates the duration by using Start and End
        // and puts the result in a temporary column "Duration" to be added to the Export log.
        QString preparedQuery =
            QString("SELECT *, cast(  (strftime('%s',End_Time) - strftime('%s',Start_Time) ) AS real  ) \
                                         AS Duration FROM %1").arg(tableName);
        if (query.exec(preparedQuery))
        {

            int rowsSoFar = 0;
            while (query.next())
            {
                QString outputString;
                QTextStream outStream(&outputString);
                outStream.setCodec("UTF-8");

                QStringList columns;
                QStringList values;
                QString varText = "";
                QSqlRecord record;

                record = query.record();
                // record.count() is the number of columns (+ 1 for sql Calculated Duration)
                for (int i = record.count() - 1; i >= 0; i--)
                {
                    if (header && (record.fieldName(i) != "SId"))
                    {
                       columns.prepend(record.fieldName(i));
                    }

                    if (record.fieldName(i) == "Event_Var_Data")
                    {
                        varText = record.value(i).toString();
                        values.prepend(varText);
                    }
                    else if (record.fieldName(i) == "Event_Description")
                    {
                        QString alarmName = record.value(i).toString();
                        QStringList variableTexts = varText.split("|"); // from Event_Var_Data
                        values.prepend(
                            xplatform::alarms::AlarmManager::GetTranslatedAlarmDescription(
                                alarmName, &variableTexts));
                    }
                    else if (record.fieldName(i) != "SId")
                    {
                        values.prepend(record.value(i).toString());
                    }
                }

                if (header)
                {
                    outStream << columns.join(",");
                    outStream << "\n";
                    header = false;
                }

                outStream << values.join(",");
                outStream << "\n";

                QByteArray out = outputString.toUtf8();
                qint64 bytesWritten = outputFile.write(out);
                if (bytesWritten != out.length())
                {
                    VJ_ERROR("Error saving to file log record #"
                             << rowsSoFar + 1 << ": " << outputString.toStdString()
                             << "bytes written for record: " << bytesWritten
                             << ", total bytes in record: " << out.length());

                    outcome = false;
                    break;
                }

                // Update progress and move on to the next row
                ++rowsSoFar;
                emit updateExportProgress(rowsSoFar, rowsTotal);

                qApp->processEvents();  // allow other events to be processed, slots to be called,
                                        // etc.
            }                           // while
        }
        else
        {
            VJ_ERROR("SQL Query failed with error: " << query.lastError().text().toStdString());
            outcome = false;
        }
    }

    if (!outcome)
    {
        emit exportFailed();
    }
    return outcome;
}

// Main UI thread
QList<xel::EventLogData> xel::EventLogManager::GetEventLogs(
    const QString & preparedSelectQuery, QList<xel::EventLogData> eventLogDataList,
    bool isHijraCalender)
{
    event_lock lock(m_mutex);
    if (!m_db.isOpen())
    {
        VJ_ERROR("Event database is not ready yet!");
        return eventLogDataList;
    }
    
    WriteEventLogRecords();

    QSqlQuery query(m_db);
    bool ok = query.exec(preparedSelectQuery);
    if (!ok)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return eventLogDataList;  // ???
    }

    if (query.next())
    {
        QString start_time = query.value(kStart_Time).toString();
        QDateTime dateTime = QDateTime::fromString(start_time, Qt::ISODate);

        // Get timezone
        QTimeZone timeZone = dateTime.toTimeSpec(Qt::LocalTime).timeZone();
        do
        {
            // Fetch each column of row from database
            EventLogData data;
            data.eventType = query.value(kType).toInt();

            int eventId = query.value(kEvent_Id).toInt();
            data.eventName = query.value(kEvent_Description).toString();

            QString start_time = query.value(kStart_Time).toString();
            QDateTime dateTime = QDateTime::fromString(start_time, Qt::ISODate);

            // Based on time zone, get and add number of seconds to UTC to obtain the local time.
            dateTime = dateTime.addSecs(timeZone.offsetFromUtc(dateTime));
            if (!isHijraCalender)
            {
                data.date = GetLocalizedDate(dateTime.date());
            }
            else
            {
                data.date = dateTime.date().toString("d/M/yyyy");
            }
            data.time = GetLocalizedTime(dateTime.time());

            int duration = query.value("Duration").toInt();
            if (duration == 0)
            {
                duration = -1;
            }
            data.duration = duration;

            if (query.value(kEvent_UserCleared).isNull())
            {
                data.userCleared = "";
            }
            else
            {
                data.userCleared = query.value(kEvent_UserCleared).toString();
            }

            // Check if the event open or not and set status accordingly
            if (m_openEvents.find(eventId) != m_openEvents.end())
            {
                QString startTime = m_openEvents.value(eventId);
                if (0 == startTime.compare(start_time))
                {
                    data.openStatus = true;
                }
            }

            // store variable text data
            if (query.value(kEvent_Var_Data).isNull())
            {
                data.varText = "";
            }
            else
            {
                data.varText = query.value(kEvent_Var_Data).toString();
            }

            data.translationContext = GetTranslationContext( eventId );
            // Insert row into list
            eventLogDataList.push_back(data);

        } while (query.next());
    }
    return eventLogDataList;  // ???
}

// Worker thread
int xel::EventLogManager::GetEventLogSize(const QString & preparedSelectQuery)
{
    event_lock lock(m_mutex);

    int eventLogSize = 0;

    if (!m_db.isOpen())
    {
        VJ_ERROR("Event database is not ready yet!");
        return 0;
    }
    
    WriteEventLogRecords();

    QSqlQuery query(m_db);
    bool ok = query.exec(preparedSelectQuery);
    if (!ok)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return 0;
    }

    if (query.next())
    {
        QSqlRecord record = query.record();
        if (record.count() > 0)
        {
            eventLogSize = record.value(0).toInt();
        }
    }

    return eventLogSize;
}

bool xel::EventLogManager::UpdateTodaysOEEData()
{
    event_lock lock(m_mutex);

    int printerDownTime = 0;
    int printerTotalTime = 0;
    int operationalDownTime = 0;
    int operationalTotalTime = 0;
    int operationalDownTimeProxy = 0;
    int operationalTotalTimeProxy = 0;

    // Get todays eventlog entries (dayoffset = 0)
    // Corresponding calc methods for operational down/total time
    CalculateOEETotalTimes(0, printerTotalTime, printerDownTime);
    CalculateOEETotalTimes(0, operationalTotalTime, operationalDownTime, Operational1);
    CalculateOEETotalTimes(0, operationalTotalTimeProxy, operationalDownTimeProxy, Operational2);

    QString date = m_pIdtHandler->getCurrentDateTimeUtc().date().toString(Qt::ISODate);

    // now add todays entry in the OEE Table,
    return InsertIntoOEEAvailabilityTable(date, printerDownTime, printerTotalTime,
                                          operationalDownTime, operationalTotalTime,
                                          operationalDownTimeProxy, operationalTotalTimeProxy);
}

void xel::EventLogManager::timerEvent(QTimerEvent *e)
{
    WriteEventLogRecords();
    UpdateTodaysOEEData();
    emit updateDuration();
    Q_UNUSED(e);
}

//
// Public API, main UI thread
bool xel::EventLogManager::SetupDB(const QString &db_path)
{
    event_lock lock(m_mutex);
    bool retVal = true;
    retVal &= CreateTables(db_path);

    bool tablesOk = true;
    tablesOk &= CheckTableStructure(kEventLogTableName, kEventLogColumnNames);
    tablesOk &= CheckTableStructure(kEventTableName, kEventTableColumnNames);
    tablesOk &= CheckTableStructure(kOEEAvailLogTableName, kOeeLogColumnNames);
    tablesOk &= CheckTableStructure(kDownTimeAlarmListTableName, kDownTimeAlarmListColumnNames);

    if (!tablesOk)
    {
        VJ_ERROR("Ill formated database " << db_path.toStdString());
        m_db.close();
        // Backup the ill formated eventlog Database
        if (!QFile::rename(db_path, db_path + ".bak"))
        {
            VJ_ERROR("Could not create a backup from ill formated database "
                     << db_path.toStdString());
        }
        // create a new table
        // if structure is bad again something is totally wrong! -> Terminate XP
        retVal &= CreateTables(db_path);
        retVal &= CheckTableStructure(kEventLogTableName, kEventLogColumnNames);
        retVal &= CheckTableStructure(kEventTableName, kEventTableColumnNames);
        retVal &= CheckTableStructure(kOEEAvailLogTableName, kOeeLogColumnNames);
        retVal &= CheckTableStructure(kDownTimeAlarmListTableName, kDownTimeAlarmListColumnNames);
        // Store error state in a flag to raise Alarm in "postinit" Stage
        m_BackupDbCreated = true;
    }

    if (retVal)
    {
        QString sqliteVersion = xplatform::common_functions::XpSqliteDbVersion::GetVersion(m_db);
        VJ_DEBUG("Sqlite driver version : " << sqliteVersion.toStdString());
    }
    
    VJ_VERIFY(connect(this,
                      &xel::EventLogManager::exitEventLogThreadRequest,
                      this,
                      &xel::EventLogManager::ShutDownAction,
                      Qt::QueuedConnection));

    VJ_VERIFY(connect(this,
                     &xel::EventLogManager::setDateTime,
                     this,
                     &xel::EventLogManager::SetDateTimeHandlerAction,
                     Qt::BlockingQueuedConnection));
    
    VJ_VERIFY(connect(this,
                     &xel::EventLogManager::synchronise,
                     this,
                     &xel::EventLogManager::SynchroniseAction,
                     Qt::BlockingQueuedConnection));

    VJ_VERIFY(connect(this,
                     &xel::EventLogManager::eventRaised,
                     this,
                     &xel::EventLogManager::OnEventTriggerAction,
                     Qt::QueuedConnection));
    
    VJ_VERIFY(connect(this,
                     &xel::EventLogManager::performClearEvent,
                     this,
                     &xel::EventLogManager::OnClearEventAction,
                     Qt::QueuedConnection));

    VJ_VERIFY(connect(this,
                     &xel::EventLogManager::parameterChanged,
                     this,
                     &xel::EventLogManager::GetLogDataAndInsertAction,
                     Qt::QueuedConnection));

    VJ_VERIFY(connect(this,
                     &xel::EventLogManager::machineStatusChange,
                     this,
                     &xel::EventLogManager::OnParameterMachineStatusChangedAction,
                     Qt::QueuedConnection));

    VJ_VERIFY(connect(this,
                     &xel::EventLogManager::getPrinterLogs,
                     this,
                     &xel::EventLogManager::GetEventLogsForPrinterLogsAction,
                     Qt::QueuedConnection));

    moveToThread(&m_thread);
    m_thread.setObjectName(kEventLogManagerThreadName);
    m_thread.start(QThread::LowPriority);

    return retVal;
}

// Worker thread
bool xel::EventLogManager::ResetOnPowerCycle()
{
    bool retVal = true;
    retVal &= DeleteOldEventlogEntries();
    retVal &= RestoreMissingEndTime();
    retVal &= StartUpdateTimer();
    return retVal;
}

bool xel::EventLogManager::CreateTables(const QString & db_path)
{
    event_lock lock(m_mutex);
    VJ_DEBUG("Event Log database: " << db_path.toStdString());
    bool success = true;

    if (db_path.isEmpty())
    {
        return false;
    }

    if (!m_db.isValid())
    {
        // Create event log database
        m_db = QSqlDatabase::addDatabase("QSQLITE", "event_data");
        m_db.setDatabaseName(db_path);
        
    }
    if (!m_db.isOpen())
    {
        // Open database
        success &= m_db.open();
        if (!success)
        {
            return false;
        }
    }

    // Create table
    QString queryText = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS EVENT_LOG(\
                    Id INTEGER PRIMARY KEY ASC AUTOINCREMENT,\
                    Type INTEGER NOT NULL,\
                    SId INTEGER NOT NULL,\
                    Production_Flag INTEGER DEFAULT(0),\
                    Event_Flag INTEGER DEFAULT(0),\
                    Service_Flag INTEGER DEFAULT(0),\
                    System_Flag INTEGER DEFAULT(0),\
                    Security_Flag INTEGER DEFAULT(0),\
                    Printer_Availability_Flag INTEGER DEFAULT(0),\
                    Operational_Availability_Proxy1_Flag INTEGER DEFAULT(0),\
                    Operational_Availability_Proxy2_Flag INTEGER DEFAULT(0),\
                    Start_Time TEXT NOT NULL,\
                    End_Time TEXT DEFAULT(NULL),\
                    User_Event_Occurred TEXT DEFAULT(NULL),\
                    User_Event_Cleared TEXT DEFAULT(NULL),\
                    Event_Var_Data TEXT DEFAULT(NULL))");

    QSqlQuery query(m_db);
    success &= query.exec(queryText);
    if (!success)
    {
        VJ_ERROR("FAILED to create EVENT_LOG table: " << query.lastError().text().toStdString());
        return false;
    }

    queryText = QString(
                    "CREATE TABLE IF NOT EXISTS %1(\
                        Id INTEGER PRIMARY KEY ASC AUTOINCREMENT,\
                        %2 TEXT DEFAULT(NULL),\
                        %3 INTEGER DEFAULT(0),\
                        %4 INTEGER DEFAULT(0),\
                        %5 INTEGER DEFAULT(0),\
                        %6 INTEGER DEFAULT(0),\
                        %7 INTEGER DEFAULT(0),\
                        %8 INTEGER DEFAULT(0))")
                    .arg(kOEEAvailLogTableName)
                    .arg(kDate)
                    .arg(kPrinter_Down_Time)
                    .arg(kPrinter_Total_Time)
                    .arg(kProxy1_Down_Time)
                    .arg(kProxy1_Total_Time)
                    .arg(kProxy2_Down_Time)
                    .arg(kProxy2_Total_Time);

    success &= query.exec(queryText);
    if (!success)
    {
        VJ_ERROR(
            "FAILED to create OEE_AVAILABILITY table: " << query.lastError().text().toStdString());
        return false;
    }
    queryText = QString(
                    "CREATE TABLE IF NOT EXISTS %1(\
                    SId INTEGER PRIMARY KEY ASC AUTOINCREMENT,\
                    Event_Id INTEGER NOT NULL,\
                    Event_Description TEXT NOT NULL)").arg(kEventTableName);

    success &= query.exec(queryText);
    if (!success)
    {
        VJ_ERROR(
            "FAILED to create EVENT_TABLE table: " << query.lastError().text().toStdString());
        return false;
    }

    queryText = QString(
                    "CREATE TABLE IF NOT EXISTS DOWNTIME_ALARM_LIST(\
                    Id INTEGER PRIMARY KEY ASC AUTOINCREMENT,\
                    Alarm_Id INTEGER NOT NULL,\
                    Operational_Availability_Proxy2_Flag INTEGER NOT NULL)");

    success &= query.exec(queryText);
    if (!success)
    {
        VJ_ERROR(
            "FAILED to create DOWNTIME_ALARM_LIST table: " << query.lastError().text().toStdString());
        return false;
    }
    
    VJ_TRACE("Finished setting up event database: " << (success ? "OK" : "FAIL"));
    return success;
}

// Main UI thread
bool xel::EventLogManager::CheckTableStructure(const QString &tableName,
                                               const QVector<QString> &columnNames)
{
    event_lock lock(m_mutex);

    QString queryText = QString("PRAGMA table_info(%1)").arg(tableName);
    QSqlQuery query(m_db);
    if (!query.exec(queryText))
    {
        VJ_ERROR("FAILED to check structure of " << tableName.toStdString() << " table: "
                                                 << query.lastError().text().toStdString());
        return false;
    }

    int iCnt = 0;
    while (query.next())
    {
        if (iCnt <= columnNames.size())
        {
            if (QString::compare(columnNames[iCnt], query.value("Name").toString(),
                                 Qt::CaseSensitive) != 0)
            {
                VJ_ERROR("Column " << query.value("Name").toString().toStdString()
                                   << " does not exist in " << tableName.toStdString()
                                   << " (ExpectedName " << columnNames[iCnt].toStdString() << ")");
                return false;
            }
        }
        else
        {
            VJ_ERROR("Column count mismatch in " << tableName.toStdString() << "! - found: " << iCnt
                                                 << " Columns expected: " << columnNames.size());
            return false;
        }
        iCnt++;
    }

    if (iCnt != columnNames.size())
    {
        VJ_ERROR("Column count mismatch in " << tableName.toStdString() << "! - found: " << iCnt
                                             << " Columns expected: " << columnNames.size());
        return false;
    }
    return true;
}

// Main UI thread
bool xel::EventLogManager::ClearTables()
{
    event_lock lock(m_mutex);

    bool result = true;
    // Delete all entries from OEE_AVAILABILITY table
    QString preparedSelectQuery = QStringLiteral("DELETE FROM OEE_AVAILABILITY");

    QSqlQuery query(m_db);
    result &= query.exec(preparedSelectQuery);

    // Delete all entries from EVENT_LOG table
    preparedSelectQuery = QStringLiteral("DELETE FROM EVENT_LOG");
    result &= query.exec(preparedSelectQuery);

     // Delete all entries from EVENT_TABLE table
    preparedSelectQuery = QStringLiteral("DELETE FROM EVENT_TABLE");
    result &= query.exec(preparedSelectQuery);

    // delete all entries from DOWNTIME_ALARM_LIST
    preparedSelectQuery = QStringLiteral("DELETE FROM DOWNTIME_ALARM_LIST");
    result &= query.exec(preparedSelectQuery);
    return result;
}

// Worker thread - called at system restart
bool xel::EventLogManager::DeleteOldEventlogEntries()
{
    event_lock lock(m_mutex);
    
    WriteEventLogRecords();

    // Get date exactly 18 months earlier from now
    QString date18MonthsEarlier =
        m_pIdtHandler->getCurrentDateTimeUtc().addMonths(-kMaxEventAgeMonths).toString(Qt::ISODate);

    // Prepare query to delete old records.
    QString preparedDeleteQuery =
        QStringLiteral(
            "DELETE FROM EVENT_LOG WHERE (julianday(date(Start_Time)) - julianday(date('%1')) < 0)")
            .arg(date18MonthsEarlier);
    // Delete old records:
    QSqlQuery deleteQuery(m_db);
    // not checking error because this will often fail if there are no records older than the
    // cut-off date (18 months at the moment)
    return deleteQuery.exec(preparedDeleteQuery);
}

// Worker thread - called at system restart
bool xel::EventLogManager::RestoreMissingEndTime()
{
    event_lock lock(m_mutex);
    bool retVal = true;
    QString now = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);

    WriteEventLogRecords();

    // Prepare query to find records with no endTime stamp.
    QString preparedSelectQuery =
        QStringLiteral("UPDATE EVENT_LOG SET End_Time='%1' WHERE (End_Time IS NULL OR End_Time='')")
            .arg(now);
    QSqlQuery query(m_db);
    retVal = query.exec(preparedSelectQuery);
    if (!retVal)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
    }
    return retVal;
}

// Worker thread - called at system restart
bool xel::EventLogManager::StartUpdateTimer()
{
    event_lock lock(m_mutex);
    bool retVal = false;
    // Start timer to update database records at regular interval (1 minute)
    // first check and stop timer if already running.
    StopTimer();
    m_timer_id = startTimer(kInterval);

    if (m_timer_id != 0)
    {
        retVal = true;
    }
    return retVal;
}

// Worker thread - called at shutdown
void xel::EventLogManager::StopTimer()
{
    event_lock lock(m_mutex);
    if (m_timer_id)
    {
        killTimer(m_timer_id);
        m_timer_id = 0;
    }
}

xel::EventLogManager::~EventLogManager()
{
    m_Instance = NULL;
}

// Worker thread
bool xel::EventLogManager::ShutDownAction()
{
    event_lock lock(m_mutex);
    
    WriteEventLogRecords();
    
    // disconnect from all notifications
    for (NotificationDisconnectorCollection::iterator i = m_disconnectors.begin();
         i != m_disconnectors.end(); ++i)
    {
        NotificationDisconnector disconnector = *i;
        if (disconnector)
        {
            disconnector();
        }
    }

    // Clear all maps
    m_openEvents.clear();

    // Stop timer
    StopTimer();

    // Finally, close the database
    m_db.close();
    m_thread.exit(0);
    return true;
}

// Main and worker thread
void xel::EventLogManager::WriteEventLogRecords()
{
    QList<EventLogRec> records;
    bool  throttling = false;

    {
        event_lock lock(m_eventlogrecs_mutex);
        
        if (m_eventlogrecs.empty())
        {
            return;
        }

        if (m_maxItemsToWrite != 0 && m_eventlogrecs.count() > m_maxItemsToWrite)
        {
            int diff = m_eventlogrecs.count() - m_maxItemsToWrite;

            throttling = true;
            records    = m_eventlogrecs;
            m_eventlogrecs.erase(m_eventlogrecs.begin(), m_eventlogrecs.begin() + diff);
            records.erase(records.begin() + diff, records.end());
            alarms::AlarmManager::RaiseAlarm(QStringLiteral("EventLogThrottle"));
        }
        else
        {
            throttling = false;
            records = m_eventlogrecs;
            m_eventlogrecs.clear();
        }
    }

    m_throttling = throttling;
    QTime      writeTimer;
    
    writeTimer.start();
    foreach (EventLogRec rec, records)
    {
        InsertRecord
        (
            rec.m_eventType,
            rec.m_eventId,
            rec.m_eventDescription,
            rec.m_start_time, 
            rec.m_end_time, 
            rec.m_categoryFlagValues,
            rec.m_varText
        );
    }

    int   milliseconds = writeTimer.elapsed();
    float timePerItem  = (float)records.size() / milliseconds;
    
    if (m_maxItemsToWrite == 0)
    {
        m_maxItemsToWrite = (float)kInterval / timePerItem;
    }
    else
    {
        m_maxItemsToWrite = (m_maxItemsToWrite + (float)kInterval / timePerItem) / 2.0;
    }

    VJ_INFO("Total Values written :  " << records.size() 
            << " Max Values can be written " << m_maxItemsToWrite);
}

// Main UI thread
bool xel::EventLogManager::ShutDown()
{
     // Wait until the worker thread has exited
    if (m_thread.isRunning())
    {
        emit exitEventLogThreadRequest();
        m_thread.wait();
        VJ_INFO("EventLogManager thread stopped.");
    }
    return true;
}

// Main UI thread
void xel::EventLogManager::OnAppAboutToQuit()
{
   /* event_lock lock(m_mutex);
    if (m_Instance)
    {
        m_Instance->deleteLater();
        m_Instance = NULL;
    }*/
}

// Worker thread
bool xel::EventLogManager::CalculateOEETotalTimes(int dayOffset, int &totalTime, int &totalDownTime,
                                                  AvailabilityType type)
{
    event_lock lock(m_mutex);
    WriteEventLogRecords();
    bool ok = false;
    totalTime = 0;
    totalDownTime = 0;

    /*
     * Retrieve a start and an end date for the given day (via offset)
     * We need to take the start of the next day (00:00:00) as qt's secto() function does not take
     * millisecond under consideration. -> A complete day is 86400 seconds. If we would use 23:59:59
     * as end time of a day
     * we would receive 86399 seconds for the whole day.
     */
    QString startDateTime =
        m_pIdtHandler->getCurrentDate().addDays(-dayOffset).toString(Qt::ISODate) + "T00:00:00Z";
    QString endDateTime =
        m_pIdtHandler->getCurrentDate().addDays(-dayOffset + 1).toString(Qt::ISODate) +
        "T00:00:00Z";
    QDateTime now = m_pIdtHandler->getCurrentDateTime().addDays(-dayOffset);

    StatusEventNames statusNames;
    SetStatusEventNamesFromType(type, statusNames);

    /*
     * This query gathers events of a given day. Depending on the given type ("Printer"
     * "Operational1" or "Operational2")
     * we will get corresponding events related to startup, shutdown, up and down.
     * For example if we have selected the type "Printer" we will get all events of
     * "System_Startup", "System_Shutdown",
     * "Printer Up", "Printer Down" on the give day. For Operational1 we will get all events of
     * "Proxy1_System_Startup",
     * "Proxy1_System_Shutdown", "Proxy1_Up" and "Proxy1_Down".
     */
    QString preparedSelectQuery =
        QStringLiteral(
        "SELECT Event_Description, Start_Time, End_Time FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE (Event_Description = '%1' \
                                                  OR Event_Description = '%2' OR Event_Description = '%3' OR Event_Description = '%4') \
                                                  AND Start_Time >= '%5' AND Start_Time <= '%6' ORDER BY datetime(Start_Time) asc")
            .arg(statusNames.eventStartupName)
            .arg(statusNames.eventShutdownName)
            .arg(statusNames.eventDownName)
            .arg(statusNames.eventUpName)
            .arg(startDateTime)
            .arg(endDateTime);

    QSqlQuery query(m_db);
    ok = query.exec(preparedSelectQuery);
    if (!ok)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return false;
    }

    QDateTime newStoredDateTime;

    // Start of Today 00:00:00
    QDateTime lastStoredDateTime = QDateTime::fromString(startDateTime, Qt::ISODate);

    // End of today (= start of next day 00:00:00 )
    QDateTime endDateTimeToday = QDateTime::fromString(endDateTime, Qt::ISODate);

    /*
     * The following code area can be seen as a small state machine.
     * First of all we will gather information about the previous state (up or down).
     * The following source comments will use the AvailabilityType "Printer" as an example.
     */
    // Determine if Printer was up or down before.
    bool eventWasUpBefore = IsEventTypeAlreadyUp(type, startDateTime);

    while (query.next())
    {
        QDateTime eventStart_time =
            QDateTime::fromString(query.value(kStart_Time).toString(), Qt::ISODate);
        newStoredDateTime = eventStart_time;

        QString eventName = query.value(kEvent_Description).toString();

        // Current event is a "Startup" event
        if (eventName == statusNames.eventStartupName)
        {
            // add total time and down time, if we were in a down state before.
            if (!eventWasUpBefore)
            {
                qint64 timediff = lastStoredDateTime.secsTo(eventStart_time);
                totalTime += timediff;
                totalDownTime += timediff;
            }
        }

        // Current event is a "Shutdown" event
        if (eventName == statusNames.eventShutdownName)
        {
            qint64 timediff = lastStoredDateTime.secsTo(eventStart_time);

            // if there is a shutdown, the system was running before.
            // add the seconds from the latest event till start time of this event to the total time
            totalTime += timediff;
            if (!eventWasUpBefore)
            {
                // also add the seconds to the down time if the printer was down before
                totalDownTime += timediff;
            }
        }

        // Current event is a "Up" event
        if (eventName == statusNames.eventUpName)
        {
            qint64 timediff = lastStoredDateTime.secsTo(eventStart_time);

            // dont care if the printer was up or down,
            // inc total time
            totalTime += timediff;
            if (!eventWasUpBefore)
            {
                // if the printer was down before, we need to inc the down time..
                totalDownTime += timediff;
            }
            eventWasUpBefore = true;
        }

        // Current event is a "Down" event
        if (eventName == statusNames.eventDownName)
        {
            // we will add the total time.
            // Note, we will add the down time add the end if the event is the
            // one that is currently active.
            // if no "Printer Up" is coming, then the printer is still down, otherwise we will add
            // the down time
            // with the next "Printer Up" event
            qint64 timediff = lastStoredDateTime.secsTo(eventStart_time);
            totalTime += timediff;
            eventWasUpBefore = false;
        }
        lastStoredDateTime = eventStart_time;
    }

    /* Now we need to finalize the calculation. We know the latest status of the events,
     * so we need to add the seconds that are remaining for the selected day.
     * If we have are calculating for the current day, we can only calculate till now (i.e. 15:45).
     */
    qint64 timediff = -1;
    if (lastStoredDateTime.date() == m_pIdtHandler->getCurrentDate())
    {
        // we are calculating for current day (today / testdate)
        timediff = lastStoredDateTime.secsTo(now);
    }
    else  // an other day
    {
        timediff = lastStoredDateTime.secsTo(endDateTimeToday);
    }

    totalTime += timediff;
    if (!eventWasUpBefore)
    {
        // Add the timediff from latest event till (now or end of day) to the down time
        // if we are in a "down" state
        totalDownTime += timediff;
    }

    return ok;
}

// Main UI and Worker thread
void xel::EventLogManager::SetStatusEventNamesFromType(AvailabilityType type, StatusEventNames &statusEventNames)
{
    if (type == Printer)
    {
        statusEventNames.eventStartupName = m_kSystemStartupEventName;
        statusEventNames.eventShutdownName = m_kSystemShutdownEventName;
        statusEventNames.eventDownName = m_kPrinterDownEventName;
        statusEventNames.eventUpName = m_kPrinterUpEventName;
    }
    else if (type == Operational1)  // for proxy1
    {
        statusEventNames.eventStartupName = m_kProxy1SystemStartupEventName;
        statusEventNames.eventShutdownName = m_kProxy1SystemShutdownEventName;
        statusEventNames.eventDownName = m_kProxy1PrinterDownEventName;
        statusEventNames.eventUpName = m_kProxy1PrinterUpEventName;
    }
    else if (type == Operational2)  // for proxy2
    {
        statusEventNames.eventStartupName = m_kProxy2SystemStartupEventName;
        statusEventNames.eventShutdownName = m_kProxy2SystemShutdownEventName;
        statusEventNames.eventDownName = m_kProxy2PrinterDownEventName;
        statusEventNames.eventUpName = m_kProxy2PrinterUpEventName;
    }
}

// Worker thread
bool xel::EventLogManager::IsEventTypeAlreadyUp(AvailabilityType type, QString startDateTime)
{
    event_lock lock(m_mutex);

    WriteEventLogRecords();
    StatusEventNames statusNames;

    SetStatusEventNamesFromType(type, statusNames);

    // Determine if there was a down before shutdown before referenceDate
    QString preparedSelectQuery =
        QStringLiteral(
            "SELECT Event_Description, Start_Time, End_Time FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE ( \
                                                (Event_Description = '%1' or Event_Description = '%2') \
                                                AND Start_Time < '%3' AND (End_Time IS NULL  OR End_Time > '%3')) \
                                                ORDER BY datetime(Start_Time) desc LIMIT 1")
            .arg(statusNames.eventUpName)
            .arg(statusNames.eventDownName)
            .arg(startDateTime);

    QSqlQuery query(m_db);
    bool ok = query.exec(preparedSelectQuery);
    if (!ok)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return false;
    }
    //   VJ_INFO("execute SQL query: " << preparedSelectQuery.toStdString());

    bool eventAlreadyUp = true;

    if (query.next())
    {
        QString eventName = query.value(kEvent_Description).toString();
        if (eventName == statusNames.eventDownName)
        {
            eventAlreadyUp = false;
        }
    }
    return eventAlreadyUp;
}

// Worker thread
QDate xel::EventLogManager::GetLastOEEEntryDate()
{
    event_lock lock(m_mutex);

    WriteEventLogRecords();
    QDate lastDate(0, 0, 0);
    QString preparedSelectQuery =
        QStringLiteral("SELECT %1 FROM %2 ORDER BY datetime(%1) desc LIMIT 1").arg(kDate).arg(
            kOEEAvailLogTableName);

    QSqlQuery query(m_db);
    bool ok = query.exec(preparedSelectQuery);
    if (!ok)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return lastDate;
    }

    // well, there should be just one maximum...
    while (query.next())
    {
        QString lastDateStr = query.value(kDate).toString();
        VJ_TRACE("Latest Entry Date: " << lastDateStr.toStdString());
        lastDate = QDate::fromString(lastDateStr, Qt::ISODate);
    }
    return lastDate;
}

QDate xel::EventLogManager::GetFirstEventLogDate()
{
    event_lock lock(m_mutex);
    WriteEventLogRecords();

    QDate firstDate(0, 0, 0);
    QString preparedSelectQuery = QStringLiteral(
        "SELECT Start_Time FROM EVENT_LOG ORDER BY datetime(Start_Time) asc LIMIT 1");

    QSqlQuery query(m_db);
    bool ok = query.exec(preparedSelectQuery);
    if (!ok)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return firstDate;
    }

    // well, there should be just one maximum...
    while (query.next())
    {
        QString firstDateStr = query.value(kStart_Time).toString();
        firstDate = QDate::fromString(firstDateStr, Qt::ISODate);
    }
    return firstDate;
}

// Worker thread
void xel::EventLogManager::RestoreOEEAvailabilityData()
{
    event_lock lock(m_mutex);

    WriteEventLogRecords();

    // Get Last Entry in OEE Table to check what has to be added...
    QDate lastOEEDate = GetLastOEEEntryDate();

    QDate checkDate = m_pIdtHandler->getCurrentDate();

    int dayOffset = 220;  // if no entry has been found, try to recover as much as possible
    if (lastOEEDate.isValid())
    {
        // Latest Date entry in OEE Table should be the start time for
        // calculation of the next days till yesterday
        // positive offset i.e. 2 days
        dayOffset = lastOEEDate.daysTo(checkDate);
    }
    else
    {
        // we have no entry in the OEE Table. lets find the first possible entry in the event log to
        // recover from
        QDate firstEventLogEntry = GetFirstEventLogDate();
        if (firstEventLogEntry.isValid())
        {
            dayOffset = firstEventLogEntry.daysTo(checkDate);
        }
    }

    // We need to readd at maximum 6 full Month + 30 Days (1 almost complete month) ~ 220
    // Older diffs are not important, as we are not storing them anymore..
    // this is a very seldom case where a user shutdown the printer for more than 7 month
    // but well, it's a simple statement.
    if (dayOffset > 220)
    {
        dayOffset = 220;
    }

    for (int offset = dayOffset; offset > -1; offset--)
    {
        // Add latest data to the Table
        int printerDownTime = 0;
        int printerTotalTime = 0;
        CalculateOEETotalTimes(offset, printerTotalTime, printerDownTime);

        int operationalDownTime = 0;
        int operationalTotalTime = 0;
        CalculateOEETotalTimes(offset, operationalTotalTime, operationalDownTime, Operational1);

        int operationalDownTimeProxy = 0;
        int operationalTotalTimeProxy = 0;
        CalculateOEETotalTimes(offset, operationalTotalTimeProxy, operationalDownTimeProxy,
                               Operational2);

        QString dateTime =
            m_pIdtHandler->getCurrentDateTimeUtc().date().addDays(-(offset)).toString(Qt::ISODate);

        // now add these missing entries to the Table
        InsertIntoOEEAvailabilityTable(dateTime, printerDownTime, printerTotalTime,
                                       operationalDownTime, operationalTotalTime,
                                       operationalDownTimeProxy, operationalTotalTimeProxy);
    }

    // check if a new month is completed
    // if so delete the former entries from the 6th month
    if (DeleteObsoleteEntries())
    {
        VJ_TRACE("Old records deleted");
    }
}

// Worker thread
bool xel::EventLogManager::DeleteObsoleteEntries()
{
    event_lock lock(m_mutex);
    
    WriteEventLogRecords();

    // we need to keep 7 month as we want to have 6 month + current month in the table
    QString dateSevenMonthAgo = m_pIdtHandler->getCurrentDate().addMonths(-7).toString(Qt::ISODate);
    QString preparedDeleteQuery =
        QString("DELETE FROM %1 WHERE %2 <= '%3'").arg(kOEEAvailLogTableName).arg(kDate).arg(
            dateSevenMonthAgo);

    // Delete old records:
    QSqlQuery deleteQuery(m_db);
    // you should not check error because this will fail if there are no records older than the
    // cut-off date (7 months)
    // needed for Unit test though...
    return deleteQuery.exec(preparedDeleteQuery);
}

void xel::EventLogManager::GetDatesFromTimeFrame(OEETimeFrame currTimeFrame, QString &startDate,
                                                 QString &endDate)
{
    switch (currTimeFrame)
    {
        case eThirtyDays:
        case eNinetyDays:
            GetLastNDaysTimeFrame(currTimeFrame, startDate, endDate);
            break;
        case eCurrentMonth:
            GetCurrentMonthTimeFrame(startDate, endDate);
            break;
        case eLastMonth:
        case eLastSecondMonth:
        case eLastThirdMonth:
        case eLastFourthMonth:
        case eLastFifthMonth:
        case eLastSixthMonth:
            GetLastNthMonthTimeFrame(currTimeFrame, startDate, endDate);
            break;
        case eUndefined:
        default:
            VJ_ERROR("Undefined Timeframe");
            startDate.clear();
            endDate.clear();
            break;
    }
}

// Main and worker thread
bool xel::EventLogManager::CalculateOEEAvailabity()
{
    bool ok = false;

    QList<OEETimeFrame>::iterator iterator_time_frame;
    iterator_time_frame = m_TimeFrameList.begin();

    QString preparedSelectQuery;
    QString startDate, endDate;
    OEETimeFrame currTimeFrame;
    QList<OEEAvailableData> result;

    event_lock lock(m_mutex);
    WriteEventLogRecords();
    QSqlQuery query(m_db);
    for (; iterator_time_frame != m_TimeFrameList.end(); ++iterator_time_frame)
    {
        OEEAvailableData recordSet;
        currTimeFrame = *iterator_time_frame;
        startDate.clear();
        endDate.clear();

        GetDatesFromTimeFrame(currTimeFrame, startDate, endDate);

        preparedSelectQuery = QString(
                                  "SELECT  %1, %2, \
                                              %3, %4, %5, %6 \
                                              FROM %7 WHERE %8 >= '%9' AND %8 <= '%10' \
                                              ORDER BY datetime(%8) desc")
                                  .arg(kPrinter_Down_Time)
                                  .arg(kPrinter_Total_Time)
                                  .arg(kProxy1_Down_Time)
                                  .arg(kProxy1_Total_Time)
                                  .arg(kProxy2_Down_Time)
                                  .arg(kProxy2_Total_Time)
                                  .arg(kOEEAvailLogTableName)
                                  .arg(kDate)
                                  .arg(startDate)
                                  .arg(endDate);
        //                 VJ_TRACE("Prepared SQL: " << preparedSelectQuery.toStdString());

        ok = query.exec(preparedSelectQuery);
        if (!ok)
        {
            VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
            VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
            return ok;
        }

        bool entriesFound = false;
        while (query.next())
        {
            entriesFound = true;
            recordSet.printerDownTime += query.value(kPrinter_Down_Time).toInt();
            recordSet.printerTotalTime += query.value(kPrinter_Total_Time).toInt();
            recordSet.operationalDownTime += query.value(kProxy1_Down_Time).toInt();
            recordSet.operationalTotalTime += query.value(kProxy1_Total_Time).toInt();
            recordSet.operationalDownTimeProxy += query.value(kProxy2_Down_Time).toInt();
            recordSet.operationalTotalTimeProxy += query.value(kProxy2_Total_Time).toInt();
        }

        // we dont need to add or calculate time frames that are not ready jet...
        if (entriesFound)
        {
            recordSet.timeFrame = currTimeFrame;
            recordSet.printerAvailability = 0;
            recordSet.opAvProxy1 = 0;
            recordSet.opAvProxy2 = 0;

            if (recordSet.printerTotalTime > 0)
            {
                recordSet.printerAvailability =
                    (1.0 - ((float)recordSet.printerDownTime / recordSet.printerTotalTime)) * 100.0;
            }

            if (recordSet.operationalTotalTime > 0)
            {
                recordSet.opAvProxy1 = (1.0 - ((float)recordSet.operationalDownTime /
                                               recordSet.operationalTotalTime)) *
                                       100.0;
            }

            if (recordSet.operationalTotalTimeProxy > 0)
            {
                recordSet.opAvProxy2 = (1.0 - ((float)recordSet.operationalDownTimeProxy /
                                               recordSet.operationalTotalTimeProxy)) *
                                       100.0;
            }

            result.append(recordSet);
        }
    }
    m_AvailabilityLogDataList.clear(); 
    m_AvailabilityLogDataList = result;
    return ok;
}

// Main UI thread.
QList<xel::OEEAvailableData> xel::EventLogManager::GetOEEAvailability()
{
    event_lock lock(m_mutex);
    return m_AvailabilityLogDataList;
}

// Main UI thread
bool xel::EventLogManager::PopulateEventStatistics(OEETimeFrame timeFrame, AvailabilityType oeeType,
                                                   EventStatistics &eventStats)
{
    WriteEventLogRecords();
    bool retval = false;
    event_lock lock(m_mutex);

    const QString kTotalDuration = "TotalDuration";
    const QString kTotalCount = "TotalCount";

    QString startDate;
    QString endDate;
    QString flagName;
    QString preparedSelectQuery;

    QSqlQuery query(m_db);

    startDate.clear();
    endDate.clear();

    GetDatesFromTimeFrame(timeFrame, startDate, endDate);

    QString startDateTime = startDate + "T00:00:00Z";

    // Hmm well.. I am open to any suggestion to improve this..
    // We cannot use the regular timeframe here as we need to involve the start of the next day for
    // proper calculation..
    QString endDateTime =
        QDate::fromString(endDate, Qt::ISODate).addDays(1).toString(Qt::ISODate) + "T00:00:00Z";

    eventStats.eventStatItems.clear();

    // TODO when integrating this function we need to check if we need to recalculate or
    //     if we can use parameters from the table to get existing values
    eventStats.availabilityPercentage = 66.6f;  // Demo value.
    eventStats.oeeType = oeeType;
    eventStats.timeFrame = timeFrame;

    switch (oeeType)
    {
        case Printer:
            flagName = kPrinterAvailabilityFlag;
            break;
        case Operational1:
            flagName = kProxy1AvailabilityFlag;
            break;
        case Operational2:
            flagName = kProxy2AvailabilityFlag;
            break;
        default:
            break;
    }

    QString currentTime = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);

    // The following SQL query is returning a list of events that are/were open during the selected
    // time frame and sums up
    // the duration and quantity of each event.
    // Explanation:
    // 1. Base selector (see end of the query)
    //      "FROM %5 WHERE ( %2 = '1' AND\( (%3 >= '%6' AND %3 <= '%7') OR (%3 < '%6' AND ((%4 >
    //      '%6') OR %4 IS NULL) ) )"
    //    This part selects any event that has the given flagName (%2 i.e. Printer_Availability_Flag
    //    ) and is present
    //    even partly in the given time frame. This selector filters all events that have a
    //    Start_Time within the given time frame ((%3 >= '%6' AND %3 <= '%7')) or the event started
    //    before the beginning of
    //    the time frame and ends there or is still open.
    //
    // 2. Sum + Case Section:
    //    The whole Case Section is meant to determine several conditions of start and end times/
    //    open events
    //    generally the following conditions must be checked :
    //      Is event still open (endtime == NULL)?
    //      Was the event start time before the current time frame?
    //      Is the end time within the timeframe ?
    //      Is the end time before the test date ?
    //    Depending on those results we are calculating the duration via cast AS REAL. The result
    //    will be added up for each event
    //    and stored in a new (internal) column called TotalDuration (via kTotalDuration argument).
    //
    // 3. Count/ Frequency:
    //    The total amount of appearance of a single event will be stored in an other (internal)
    //    column called TotalCount (via kTotalCount)

    preparedSelectQuery = QString(
                              " SELECT %1,\
                                            SUM(\
                                              CASE WHEN %3 < '%6'  THEN\
                                                CASE WHEN %4 IS NULL THEN\
                                                   CASE WHEN '%10' > '%7' THEN\
                                                     cast(  (strftime('%s','%7') - strftime('%s',  '%6' ) ) AS real  )\
                                                   ELSE\
                                                     cast(  (strftime('%s','%10') - strftime('%s',  '%6' ) ) AS real  )\
                                                   END\
                                                ELSE\
                                                  CASE WHEN (%4 > '%7' ) THEN\
                                                    cast(  (strftime('%s','%7') - strftime('%s',  '%6' ) ) AS real  )\
                                                  ELSE\
                                                    cast(  (strftime('%s',%4) - strftime('%s',  '%6' ) ) AS real  )\
                                                  END\
                                                END\
                                              ELSE\
                                                CASE WHEN %4 IS NULL THEN\
                                                  CASE WHEN  '%10'  > '%7' THEN\
                                                     cast(  (strftime('%s', '%7'  ) - strftime('%s',  %3  ) ) AS real  )\
                                                  ELSE\
                                                     cast(  (strftime('%s', '%10'  ) - strftime('%s',  %3  ) ) AS real  )\
                                                  END\
                                                ELSE\
                                                  CASE WHEN (%4 > '%7' ) THEN\
                                                    cast(  (strftime('%s','%7' ) - strftime('%s', %3) ) AS real  )\
                                                  ELSE\
                                                    cast(  (strftime('%s',%4) - strftime('%s', %3) )  AS real  )\
                                                  END\
                                                END\
                                              END\
                                           )\
                                         AS %8,\
                                         COUNT(*) AS %9\
                                         FROM %5 NATURAL JOIN %11 WHERE ( %2 = 1 AND\
                                              ( (%3 >= '%6' AND %3 <= '%7') OR (%3 < '%6' AND ((%4 > '%6') OR %4 IS NULL) ) )\
                                            )\
                                         GROUP BY %1")
                              .arg(kEvent_Id)           // %1
                              .arg(flagName)            // %2
                              .arg(kStart_Time)         // %3
                              .arg(kEnd_Time)           // %4
                              .arg(kEventLogTableName)  // %5
                              .arg(startDateTime)       // %6
                              .arg(endDateTime)         // %7
                              .arg(kTotalDuration)      // %8
                              .arg(kTotalCount)         // %9
                              .arg(currentTime)         // %10
                              .arg(kEventTableName);    // %11

    // For Debugging Purpose only:
    // VJ_TRACE("preparedSelectQuery:  " << preparedSelectQuery.toStdString());

    retval = query.exec(preparedSelectQuery);
    if (!retval)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return retval;  // early return
    }

    while (query.next())
    {
        int id = query.value(kEvent_Id).toInt();
        int duration = query.value(kTotalDuration).toInt();
        int count = query.value(kTotalCount).toInt();

        // VJ_TRACE("Desc: "<< eventDesc.toStdString() << " duration: " << duration << " Count: " <<
        // count);

        // NOTE: In future EVENT_LOG table should be replaced to use proper table of unique event id
        // and event description

        QString selectQuery = QString(
                                  "SELECT DISTINCT Event_Description, Event_Var_Data \
                                                       FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE Event_Id=%1").arg(id);

        QSqlQuery query1(m_db);
        bool ret = query1.exec(selectQuery);
        if (!ret)
        {
            VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
            VJ_ERROR("SQL Error: " << query1.lastError().text().toStdString());
            return ret;  // early return
        }

        QString desc;
        while (query1.next())
        {
            QStringList varTextList = query1.value(kEvent_Var_Data).toString().split("|");
            desc = xplatform::alarms::AlarmManager::GetTranslatedAlarmDescription(
                query1.value(kEvent_Description).toString(), &varTextList);
        }

        EventStatItem eventStatItem(id, desc, duration, count);

        eventStats.eventStatItems.append(eventStatItem);
        m_EventStatItemList = eventStats.eventStatItems;
    }

    m_EventStatItemList.clear();
    m_EventStatItemList = eventStats.eventStatItems;
    return retval;
}

xel::AvailabilityType xel::EventLogManager::GetAvailabilityType(const QString &type) const
{
    AvailabilityType avType = Printer;

    if (type == kPrinterReadyTotalTime)
    {
        avType = Operational2;
    }
    else if (type == kPrinterTotalTime)
    {
        avType = Operational1;
    }

    return avType;
}

// Main UI thread
bool xel::EventLogManager::PopulateEventOccurences(OEETimeFrame timeFrame, int eventId,
                                                   EventOccurence &eventOccurences)
{
    WriteEventLogRecords();
    bool        retval = false;
    event_lock  lock(m_mutex);
   
    const QString kTotalDuration = QStringLiteral("TotalDuration");

    QString startDate;
    QString endDate;

    QSqlQuery query(m_db);

    GetDatesFromTimeFrame(timeFrame, startDate, endDate);

    QString startDateTime = startDate + QStringLiteral("T00:00:00Z");

    // Hmm well.. I am open to any suggestion to improve this..
    // We cannot use the regular timeframe here as we need to involve the start of the next
    // day for proper calculation..
    QString endDateTime =
        QDate::fromString(endDate, Qt::ISODate).addDays(1).toString(Qt::ISODate) + "T00:00:00Z";

    eventOccurences.eventOccurences.clear();

    eventOccurences.eventId = eventId;
    eventOccurences.timeFrame = timeFrame;

    QString currentTime = m_pIdtHandler->getCurrentDateTimeUtc().toString(Qt::ISODate);

    // The following SQL query is returning a list of the specific event that is/was open
    // during the selected time frame
    //
    // Explanation:
    // 1. Base selector (see end of the query)
    //      "FROM %5 WHERE ( %2 = %1 AND\( (%3 >= '%6' AND %3 <= '%7') OR (%3 < '%6' AND
    //      ((%4 > '%6') OR %4 IS NULL) ) )"
    //    This part selects any event that has the given eventid (%2 i.e. 3000 ) and is
    //    present
    //    even partly in the given time frame. This selector filters all events that have a
    //    Start_Time within the given time frame ((%3 >= '%6' AND %3 <= '%7')) or the event
    //    started before the beginning of
    //    the time frame and ends there or is still open.
    //
    // 2. Case Section:
    //    The whole case section is meant to determine several conditions of start and end
    //    times/ open events
    //    generally the following conditions must be checked :
    //      Is event still open (endtime == NULL)?
    //      Was the event start time before the current time frame?
    //      Is the end time within the timeframe ?
    //      Is the end time before the test date ?
    //    Depending on those results we are calculating the duration via cast AS REAL.
    //    The result will be  stored in a new (internal) column called TotalDuration (via
    //    kTotalDuration argument).
    //
    // 3. Count/ Frequency:
    //    The total amount of appearance of a single event will be stored in an other
    //    (internal) column called TotalCount (via kTotalCount)

    QString preparedSelectQuery = QString(
                                      " SELECT %1,\
                                              CASE WHEN %3 < '%6'  THEN\
                                                CASE WHEN %4 IS NULL THEN\
                                                   CASE WHEN '%9' > '%7' THEN\
                                                     cast(  (strftime('%s','%7') - strftime('%s',  '%6' ) ) AS real  )\
                                                   ELSE\
                                                     cast(  (strftime('%s','%9') - strftime('%s',  '%6' ) ) AS real  )\
                                                   END\
                                                ELSE\
                                                  CASE WHEN (%4 > '%7' ) THEN\
                                                    cast(  (strftime('%s','%7') - strftime('%s',  '%6' ) ) AS real  )\
                                                  ELSE\
                                                    cast(  (strftime('%s',%4) - strftime('%s',  '%6' ) ) AS real  )\
                                                  END\
                                                END\
                                              ELSE\
                                                CASE WHEN %4 IS NULL THEN\
                                                  CASE WHEN  '%9'  > '%7' THEN\
                                                     cast(  (strftime('%s', '%7'  ) - strftime('%s',  %3  ) ) AS real  )\
                                                  ELSE\
                                                     cast(  (strftime('%s', '%9'  ) - strftime('%s',  %3  ) ) AS real  )\
                                                  END\
                                                ELSE\
                                                  CASE WHEN (%4 > '%7' ) THEN\
                                                    cast(  (strftime('%s','%7' ) - strftime('%s', %3) ) AS real  )\
                                                  ELSE\
                                                    cast(  (strftime('%s',%4) - strftime('%s', %3) )  AS real  )\
                                                  END\
                                                END\
                                              END\
                                         AS %8, %3\
                                         FROM %5 NATURAL JOIN %10 WHERE ( %1 = %2 AND\
                                              ( (%3 >= '%6' AND %3 <= '%7') OR (%3 < '%6' AND ((%4 > '%6') OR %4 IS NULL) ) ))")

                                      .arg(kEvent_Id)           // %1
                                      .arg(eventId)             // %2
                                      .arg(kStart_Time)         // %3
                                      .arg(kEnd_Time)           // %4
                                      .arg(kEventLogTableName)  // %5
                                      .arg(startDateTime)       // %6
                                      .arg(endDateTime)         // %7
                                      .arg(kTotalDuration)      // %8
                                      .arg(currentTime)         // %9
                                      .arg(kEventTableName);    // %10

    // For Debugging Purpose only:
    // VJ_TRACE("preparedSelectQuery:  " << preparedSelectQuery.toStdString());

    retval = query.exec(preparedSelectQuery);
    if (!retval)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
        return retval;  // early return
    }

    QDateTime eventStartDateTime;

    while (query.next())
    {
        QString start_time = query.value(kStart_Time).toString();
        QDateTime dateTime = QDateTime::fromString(start_time, Qt::ISODate);
        // Get timezone
        QTimeZone timeZone = dateTime.toTimeSpec(Qt::LocalTime).timeZone();
        // Based on time zone, get and add number of seconds to UTC to obtain the local
        // time.
        dateTime = dateTime.addSecs(timeZone.offsetFromUtc(dateTime));
        QString latesttime = GetLocalizedTime(dateTime.time());
        QString latestdate = GetLocalizedDate(dateTime.date());

        int duration = query.value(kTotalDuration).toInt();
        EventOccurenceItem eventOccurenceItem(

            latestdate, latesttime, duration);
        eventOccurences.eventOccurences.append(eventOccurenceItem);
        // VJ_TRACE("Found Date: " << eventOccurenceItem.date.toStdString() << " time: " <<
        // eventOccurenceItem.time.toStdString() << " duration: " <<
        // duration.toStdString());
    }
    m_EventOccurences.clear();
    m_EventOccurences = eventOccurences.eventOccurences;
    return retval;
}

void xel::EventLogManager::GetLastNDaysTimeFrame(int n, QString &startDate, QString &endDate)
{
    event_lock lock(m_mutex);
    QDate date = m_pIdtHandler->getCurrentDate();
    startDate = date.addDays(-n).toString(Qt::ISODate);

    // Until and incl day before relative day. NOT INCLUDING relative day! (see XPF-1573 COS 1)
    endDate = date.addDays(-1).toString(Qt::ISODate);
}

void xel::EventLogManager::GetCurrentMonthTimeFrame(QString &startDate, QString &endDate)
{
    event_lock lock(m_mutex);
    QDate date = m_pIdtHandler->getCurrentDate();
    QDate firstDate = QDate(date.year(), date.month(), 1);
    startDate = firstDate.toString(Qt::ISODate);

    // Until and including relativeDate day (see XPF-1638 COS 1)
    endDate = date.toString(Qt::ISODate);
}

void xel::EventLogManager::GetLastNthMonthTimeFrame(int n, QString &startDate, QString &endDate)
{
    event_lock lock(m_mutex);
    QDate date = m_pIdtHandler->getCurrentDate();
    QDate firstDate = QDate(date.year(), date.month() - n, 1);
    startDate = firstDate.toString(Qt::ISODate);

    QDate lastDate = QDate(date.year(), firstDate.month(), firstDate.daysInMonth());
    endDate = lastDate.toString(Qt::ISODate);
}

// Worker thread
bool xel::EventLogManager::InsertIntoOEEAvailabilityTable(
    QString date, int printerDownTime, int printerTotalTime, int operationalDownTime,
    int operationalTotalTime, int operationalDownTimeProxy, int operationalTotalTimeProxy)
{
    event_lock  lock(m_mutex);
    bool        retval = false;
    QString     preparedInsertUpdateQuery;

    preparedInsertUpdateQuery =
        QString(
            "INSERT OR REPLACE INTO %15(Id, %8 ,%9 , %10 ,%11, %12, %13, %14) \
                                                VALUES((SELECT Id FROM %15 WHERE %8 = '%1'),'%1','%2','%3','%4','%5','%6','%7')")
            .arg(date)
            .arg(printerDownTime)
            .arg(printerTotalTime)
            .arg(operationalDownTime)
            .arg(operationalTotalTime)
            .arg(operationalDownTimeProxy)
            .arg(operationalTotalTimeProxy)
            .arg(kDate)
            .arg(kPrinter_Down_Time)
            .arg(kPrinter_Total_Time)
            .arg(kProxy1_Down_Time)
            .arg(kProxy1_Total_Time)
            .arg(kProxy2_Down_Time)
            .arg(kProxy2_Total_Time)
            .arg(kOEEAvailLogTableName);

    if (!m_db.isOpen())
    {
        VJ_ERROR("Event database is not ready yet!");
        return retval;
    }

    QSqlQuery query(m_db);
    retval = query.exec(preparedInsertUpdateQuery);
    if (!retval)
    {
        VJ_ERROR("FAILED to execute SQL query: " << preparedInsertUpdateQuery.toStdString());
        VJ_ERROR("SQL Error: " << query.lastError().text().toStdString());
    }

    return retval;
}

// Worker thread
void xel::EventLogManager::SetDateTimeHandlerAction(QSharedPointer<IDateTimeHandler> pDateTimeHandler)
{
    event_lock lock(m_mutex);

    if (pDateTimeHandler != NULL)
    {
        m_pIdtHandler = pDateTimeHandler;
    }
}

// Main thread
void xel::EventLogManager::SetDateTimeHandler(QSharedPointer<IDateTimeHandler> pDateTimeHandler)
{
    if (m_thread.isRunning())
    {
        emit setDateTime(pDateTimeHandler);
    }
    else
    {
        SetDateTimeHandlerAction(pDateTimeHandler);
    }
}

// Main thread
void xel::EventLogManager::UpdateExistingServiceRecords(int nUniqueIndex, QString qEventDescription,
                                                        const QString & qStartTime, QString qUser)
{
    StoreRowCounts();

    event_lock lock(m_mutex);
    
    WriteEventLogRecords();

    if (!m_db.isOpen())
    {
        VJ_INFO("Event database is not ready yet!");
        return;
    }

    bool bEventDescription = qEventDescription.contains("'");
    if (bEventDescription)
    {
        qEventDescription.replace("'", "''");
    }

    bool bUser = qUser.contains("'");
    if (bUser)
    {
        qUser.replace("'", "''");
    }
    QSqlQuery query(m_db);
    
    QString selectQuery   = QString("SELECT Event_Id \
                                      FROM EVENT_LOG NATURAL JOIN EVENT_TABLE\
                                      WHERE Id=%1").arg(nUniqueIndex);
    
    bool result = query.exec(selectQuery);
    
    if (!result)
    {
        VJ_WARN("FAILED to update event duration on database: " << query.lastError().text().toStdString());
        return;
    }

    int eventId = 0;

    if (query.next())
    {
        eventId = query.value(kEvent_Id).toInt();
    }
    else
    {
        VJ_WARN("FAILED to get event ID ");
        return;
    }
    selectQuery = QString("SELECT DISTINCT SId  FROM EVENT_TABLE\
                          WHERE((Event_Id = %1) AND (Event_Description = '%2'))")
                          .arg(eventId)
                          .arg(qEventDescription);
    
    result = query.exec(selectQuery);
    if (!result)
    {
        VJ_WARN("FAILED to update event duration on database: " 
                 << query.lastError().text().toStdString());
        return;
    }
    
    int index = 0;

    if (query.next())
    {
        index = query.value(kSId).toInt();
    }
    else
    {
        // get a new one
        //
        // Event_Id and Event_Description is not present
        //
        QString preparedInsertQuery = QString("INSERT INTO EVENT_TABLE(Event_Id, Event_Description)\
                                       VALUES(%1,'%2')").arg(eventId).arg(qEventDescription);
        if (!query.exec(preparedInsertQuery))
        {
            VJ_TRACE("SQL query: " << preparedInsertQuery.toStdString() << "failed");
            return;
        }
        if (!query.exec(selectQuery))
        {
            VJ_TRACE("SQL query: " << selectQuery.toStdString() << " failed ");
            return;
        }

        if (query.next())
        {
            index =  query.value(kSId).toInt();
        }
        else
        {
            VJ_TRACE("Failed to move to next record");
            return;
        }
    }

    // Prepare query for record selection to update event duration for events which have NOT NULL
    // duration.
    QString preparedQuery = QString(
                                "UPDATE EVENT_LOG SET SId=%4, Start_Time='%1', \
                                             User_Event_Occurred='%2' \
                                             WHERE Id=%3")
                                .arg(qStartTime)
                                .arg(qUser)
                                .arg(nUniqueIndex)
                                .arg(index);

    // Update Record
    result = query.exec(preparedQuery);
    if (!result)
    {
        VJ_WARN("FAILED to update event duration on database: "
                << query.lastError().text().toStdString());
    }
}

// Main thread - used for Unit test
QList<xel::EventLogData> xel::EventLogManager::GetRecordUsingEventId(int nEventId)
{
    if (m_thread.isRunning())
    {
        // block untill the worker thread queue
        // flushed
        emit synchronise();
    }
    else
    {
        // direct slot invocation
        SynchroniseAction();
    }

    QList<EventLogData> eventLogDataList;
    
    event_lock lock(m_mutex);
    WriteEventLogRecords();

    if (!m_db.isOpen())
    {
        VJ_INFO("Event database is not ready yet!");
        return eventLogDataList;
    }

    // Prepare query for record
    QString preparedSelectQuery =
        QString(
            "SELECT DISTINCT Id, Type, Event_Id, Event_Description, Start_Time, \
             User_Event_Occurred, User_Event_Cleared FROM EVENT_LOG NATURAL JOIN EVENT_TABLE WHERE Event_Id=%1")
            .arg(nEventId);

    QSqlQuery query(m_db);
    bool ok = query.exec(preparedSelectQuery);
    if (!ok)
    {
        VJ_INFO("FAILED to execute SQL query: " << preparedSelectQuery.toStdString());
        VJ_INFO("SQL Error: " << query.lastError().text().toStdString());
        return eventLogDataList;
    }

    if (query.next())
    {
        // Now fetch one date to get the timezone to find number of seconds to add to UTC to obtain
        // the local time
        QString start_time = query.value(kStart_Time).toString();
        QDateTime dateTime = QDateTime::fromString(start_time, Qt::ISODate);

        // Get timezone
        QTimeZone timeZone = dateTime.toTimeSpec(Qt::LocalTime).timeZone();

        do
        {
            // Fetch each column of row from database
            EventLogData data;
            data.userCleared = "";

            // Fetch the Type of event from database
            data.eventType = query.value(kType).toInt();

            // Fetch the Table Index from database
            data.UniqueIndex = query.value(kTableIndex).toInt();

            // Fetch the Event ID from database
            int eventId = query.value(kEvent_Id).toInt();

            // Fetch the Description from database
            data.eventName = query.value(kEvent_Description).toString();

            // Fetch the User from database
            data.user = query.value(kEvent_UserOccurred).toString();

            QString start_time = query.value(kStart_Time).toString();
            QDateTime dateTime = QDateTime::fromString(start_time, Qt::ISODate);

            // Based on time zone, get and add number of seconds to UTC to obtain the local time.
            dateTime = dateTime.addSecs(timeZone.offsetFromUtc(dateTime));

            data.dateTime = dateTime.toString(Qt::ISODate);

            data.date = GetLocalizedDate(dateTime.date());
            data.time = GetLocalizedTime(dateTime.time());

            if (!(query.value(kEvent_UserCleared).isNull()))
            {
                data.userCleared = query.value(kEvent_UserCleared).toString();
            }

            // Check if the event open or not and set status accordingly
            if (m_openEvents.find(eventId) != m_openEvents.end())
            {
                QString startTime = m_openEvents.value(eventId);
                if (0 == startTime.compare(start_time))
                {
                    data.openStatus = true;
                }
            }
            data.translationContext = GetTranslationContext( eventId );

            // Insert row into list
            eventLogDataList.push_back(data);

        } while (query.next());
    }

    return eventLogDataList;  // ???
}

// Main UI thread
bool xel::EventLogManager::GetBackupDbCreated()
{
    event_lock lock(m_mutex);
    return m_BackupDbCreated;
}

void xel::EventLogManager::SynchroniseAction()
{
    // do nothing right now
}

bool xel::EventLogManager::ReadTranslationContexts()
{
    // Get root node of log items
    QStringList logItemsXPathList;
    logItemsXPathList << QString::fromStdWString (gd::xpath_PlatformEvents)
        << QString::fromStdWString (gd::xpath_MachineOverallState);

    // Now iterate through all items and subscribe for notification
    foreach (const QString &str, logItemsXPathList)
    {
        im::Parameter logItemParam = im::InformationManager::GetParameter( str );
        if (logItemParam.IsNull())
        {
            return false;
        }

        im::ParameterList itemsList = logItemParam.Children();
        if (itemsList.IsEmpty())
        {
            continue;
        }
        
        im::ParameterList::iterator itr;
        im::ParameterList::iterator end = itemsList.end();

        // Now iterate through all items and subscribe for notification
        for (itr = itemsList.begin(); itr != end; ++itr)
        {
            int eventID = 0;
            QString stringContext;
            im::Parameter item = (*itr);
            im::Int32Parameter eventIDparam = item.GetParameter( gd::xml_EventID );
            im::StringParameter transaltionContextParam = item.GetParameter( gd::xml_EventLogTranslationContext );
            VJ_ASSERT( !eventIDparam.IsNull() );
            VJ_ASSERT( !eventIDparam.IsNull() );
            eventIDparam.GetValue( eventID );
            transaltionContextParam.GetValue( stringContext );

            VJ_ASSERT( eventID > 0 );
            m_translationContexts.insert( eventID, stringContext );

        }
    }
    return true;
}

bool xel::EventLogManager::ReadandAddTranslationContexts(xel::EventLogItemPtrLst& addeventlst)
{
    xel::EventLogItemPtrLst::iterator itr;
    itr = addeventlst.begin();
    //end = eventlst.end();
    for (; itr != addeventlst.end(); ++itr)
    {
        xel::EventLogItemPtr item = (*itr);
        int eventID = item->GetEventID();
        QString stringContext = item->GetTranslationContext();
        VJ_ASSERT( eventID > 0 );
        if(!m_translationContexts.contains(eventID))
        {
            m_translationContexts.insert( eventID, stringContext );
        }
    }
    return true;
}

QString xel::EventLogManager::GetTranslationContext( int eventID )
{
    if ( !m_translationContexts.isEmpty() )
    {
        //  TODO: temporarily commented out until investigated 
        //         VJ_ASSERT( !(m_translationContexts.value( eventID ).isEmpty()) );
        return m_translationContexts.value( eventID );
    }
    else
    {
        return QString();
    }
}

void xel::EventLogManager::StoreRowCounts()
{
#ifdef DB_PERFORMANCE_REPORTING
    // Create the nodes if they don't exist yet - will only be done once
    im::Parameter root = im::InformationManager::GetParameter(gd::xpath_Root);
    VJ_ASSERT(root);
    if (!m_rowCountEventLog)
    {
        m_rowCountEventLog = root.InsertChild(L"RowCountEventLog", im::Int32Parameter(0));
    }
    if (!m_rowCountEventTable)
    {
        m_rowCountEventTable = root.InsertChild(L"RowCountEventTable", im::Int32Parameter(0));
    }
    if (!m_rowCountDowntimeAlarmList)
    {
        m_rowCountDowntimeAlarmList = root.InsertChild(L"RowCountDowntimeAlarmList", im::Int32Parameter(0));
    }
    
    event_lock lock(m_mutex);
    WriteEventLogRecords();

    // query row count of each of the 3 tables used and store them in above nodes
    {
        int rowsTotal(-1);
        QSqlQuery countQuery(m_db);
        if (countQuery.exec(QString("SELECT COUNT(*) FROM DOWNTIME_ALARM_LIST")))
        {
            if (countQuery.next())
            {
                QSqlRecord record = countQuery.record();
                if (record.count() > 0)
                {
                    rowsTotal = record.value(0).toInt();
                }
            }
        }
        m_rowCountDowntimeAlarmList.SetValue(rowsTotal);
    }
    {
        int rowsTotal= -1;
        QSqlQuery countQuery(m_db);
        if (countQuery.exec(QString("SELECT COUNT(*) FROM EVENT_TABLE")))
        {
            if (countQuery.next())
            {
                QSqlRecord record = countQuery.record();
                if (record.count() > 0)
                {
                    rowsTotal = record.value(0).toInt();
                }
            }
        }
        m_rowCountEventTable.SetValue(rowsTotal);
    }
    {
        int rowsTotal= -1;
        QSqlQuery countQuery(m_db);
        if (countQuery.exec(QString("SELECT COUNT(*) FROM EVENT_LOG")))
        {
            if (countQuery.next())
            {
                QSqlRecord record = countQuery.record();
                if (record.count() > 0)
                {
                    rowsTotal = record.value(0).toInt();
                }
            }
        }
        m_rowCountEventLog.SetValue(rowsTotal);
    }
#endif
}

int xel::EventLogManager::GetTotalNumberofRecords()
{
    event_lock lock(m_mutex);

    WriteEventLogRecords();

    int rowsTotal(-1);
    QString tableName = kEventLogTableName + " NATURAL JOIN " + kEventTableName;

    QSqlQuery countQuery(m_db);
    if ((countQuery.exec(QString("SELECT COUNT(*) FROM %1").arg(tableName))))
    {
        if (countQuery.next())
        {
            QSqlRecord record = countQuery.record();
            if (record.count() > 0)
            {
                rowsTotal = record.value(0).toInt();
            }
        }
    }
    return rowsTotal;
}

bool xel::EventLogManager::ConnectToOnEventTriggerSignal(const QObject * pReceiver, const char* slot)
{
    bool outcome = false;
    xel::EventLogManagerPtr pInstance = xel::EventLogManager::GetInstance();
    if (pInstance)
    {
        outcome = QObject::connect(pInstance, SIGNAL(onEventTrigger(Parameter)), pReceiver, slot,
                                   Qt::QueuedConnection);
    }
    return outcome;
}

void xel::EventLogManager::UpdateSystemEventNamesAndIDs()
{
    // Get Printer Down Event Name and Event ID
    im::LogEventParameter printerDownEventParam = im::InformationManager::GetParameter(gd::xpath_PrinterDown);
    GetSystemEventNameAndID(printerDownEventParam, m_kPrinterDownEventName, m_kPrinterDownEventID);

    // Get Printer Up Event Name and Event ID
    im::LogEventParameter printerUpEventParam = im::InformationManager::GetParameter(gd::xpath_PrinterUp);
    GetSystemEventNameAndID(printerUpEventParam, m_kPrinterUpEventName, m_kPrinterUpEventID);

    // Get System Startup Event Name and Event ID
    im::LogEventParameter systemStartupEventParam = im::InformationManager::GetParameter(gd::xpath_SystemStartup);
    GetSystemEventNameAndID(systemStartupEventParam, m_kSystemStartupEventName, m_kSystemStartupEventID);

    // Get System Shutdown Event Name and Event ID
    im::LogEventParameter systemShutdownEventParam = im::InformationManager::GetParameter(gd::xpath_SystemShutdown);
    GetSystemEventNameAndID(systemShutdownEventParam, m_kSystemShutdownEventName, m_kSystemShutdownEventID);

    // Get Proxy1 PrinterDown Event Name and Event ID
    im::LogEventParameter proxy1PrinterDownEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy1Down);
    GetSystemEventNameAndID(proxy1PrinterDownEventParam, m_kProxy1PrinterDownEventName, m_kProxy1PrinterDownEventID);

    // Get Proxy1 PrinterUp Event Name and Event ID
    im::LogEventParameter proxy1PrinterUpEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy1Up);
    GetSystemEventNameAndID(proxy1PrinterUpEventParam, m_kProxy1PrinterUpEventName, m_kProxy1PrinterUpEventID);

    // Get Proxy2 PrinterDown Event Name and Event ID
    im::LogEventParameter proxy2PrinterDownEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy2Down);
    GetSystemEventNameAndID(proxy2PrinterDownEventParam, m_kProxy2PrinterDownEventName, m_kProxy2PrinterDownEventID);

    // Get Proxy2 PrinterUp Event Name and Event ID
    im::LogEventParameter proxy2PrinterUpEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy2Up);
    GetSystemEventNameAndID(proxy2PrinterUpEventParam, m_kProxy2PrinterUpEventName, m_kProxy2PrinterUpEventID);

    // Get Proxy1 System Shutdown Event Name and Event ID
    im::LogEventParameter proxy1SystemShutdownEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy1SystemShutdown);
    GetSystemEventNameAndID(proxy1SystemShutdownEventParam, m_kProxy1SystemShutdownEventName, m_kProxy1SystemShutdownEventID);

    // Get Proxy1 System Startup Event Name and Event ID
    im::LogEventParameter proxy1SystemStartupEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy1SystemStartup);
    GetSystemEventNameAndID(proxy1SystemStartupEventParam, m_kProxy1SystemStartupEventName, m_kProxy1SystemStartupEventID);

    // Get Proxy2 System Shutdown Event Name and Event ID
    im::LogEventParameter proxy2SystemShutdownEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy2SystemShutdown);
    GetSystemEventNameAndID(proxy2SystemShutdownEventParam, m_kProxy2SystemShutdownEventName, m_kProxy2SystemShutdownEventID);

    // Get Proxy2 System Startup Event Name and Event ID
    im::LogEventParameter proxy2SystemStartupEventParam = im::InformationManager::GetParameter(gd::xpath_Proxy2SystemStartup);
    GetSystemEventNameAndID(proxy2SystemStartupEventParam, m_kProxy2SystemStartupEventName, m_kProxy2SystemStartupEventID);
}

void xel::EventLogManager::GetSystemEventNameAndID(im::LogEventParameter systemEventParam, QString &systemEventName, int &systemEventID)
{
    if (systemEventParam)
    {
        // Get System Event Name
        WString eventName;
        systemEventParam.GetName(eventName);
        systemEventName = fromWString(eventName);

        // Get System Event ID
        int eventId;
        im::Int32Parameter eventIDParam = systemEventParam.GetParameter(gd::xml_EventID);
        eventIDParam.GetValue(eventId);
        systemEventID = eventId;
    }
}

void xel::EventLogManager::RaiseSystemShutdownEvent()
{
    RaiseInternalEvent(m_kSystemShutdownEventID, m_kSystemShutdownEventName);
}

void xel::EventLogManager::RaiseProxy1SystemShutdownEvent()
{
    RaiseInternalEvent(m_kProxy1SystemShutdownEventID, m_kProxy1SystemShutdownEventName);
}
