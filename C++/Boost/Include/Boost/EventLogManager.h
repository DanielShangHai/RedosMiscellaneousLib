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
// Description :  Declaration of EventLogManager Class

#ifndef XPLATFORM_EVENT_LOG_MANAGER_H
#define XPLATFORM_EVENT_LOG_MANAGER_H

#ifdef UNIT_TEST_BUILD
    #define PRIVATE public
#else
    #define PRIVATE private
#endif

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QLocale>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QSharedPointer>
#include <QtSql/QSqlDatabase>
#include <parameter/StringParameter.h>
#include <parameter/LogEventParameter.h>
#include <parameter/Int32Parameter.h>
#include <parameter/ReferenceParameter.h>
#include <eventlogging/Declarations.h>
#include <eventlogging/IDateTimeHandler.h>
#include <alarm_manager/Alarm.h>
#include <MachineStatus/MachineStatus.h>
#include <os/mutex.h>
#include <eventlogging/EventLogTypes.h>

namespace xplatform
{
    namespace event_logging
    {
        struct EventLogData
        {
            int     UniqueIndex;
            int     eventType;
            int32   duration;
            QString eventName;
            QString date;
            QString time;
            bool    openStatus;
            QString userCleared;
            QString user;
            QString dateTime;
            QString varText;
            QString translationContext;

            EventLogData()
                : UniqueIndex( 0 )
                , eventType( 0 )
                , duration( 0 )
                , eventName( "" )
                , date ( "" )
                , time ( "" )
                , openStatus ( false )
                , userCleared ( "" )
                , user( "" )
                , dateTime( "" )
                , varText("")
                , translationContext("")
            { }

        };

        typedef QSharedPointer<QList<EventLogData> > EventLogDataLstPtr;

        /**
         * @brief Data struct for availability log data
         * This struct is used to populate log data to the UI Model
        */        
        struct AvailabilityLogData
        {
            QString timeframe;
            QString printerAvailability;
            QString opAvProxy1;
            QString opAvProxy2;

            AvailabilityLogData()
                : timeframe("")
                , printerAvailability("")
                , opAvProxy1("")
                , opAvProxy2("")
            {}
        };

        //OEE time frame enum
        enum OEETimeFrame
        {
            eUndefined = -1,
            eLastMonth = 1,
            eLastSecondMonth,
            eLastThirdMonth,
            eLastFourthMonth,
            eLastFifthMonth,
            eLastSixthMonth,
            eCurrentMonth,
            eThirtyDays = 30,
            eNinetyDays = 90
        };

        /**
         * @brief This struct contains OEE data for calculation
         * and will be used to determine availability data
         */
        struct OEEAvailableData
        {
            OEETimeFrame timeFrame;
            float printerAvailability;
            float opAvProxy1;
            float opAvProxy2;
            int printerDownTime;
            int printerTotalTime;
            int operationalDownTime;
            int operationalTotalTime;
            int operationalDownTimeProxy;
            int operationalTotalTimeProxy;
            OEEAvailableData()
                : timeFrame(eUndefined)
                , printerAvailability(0.0)
                , opAvProxy1(0.0)
                , opAvProxy2(0.0)
                , printerDownTime(0)
                , printerTotalTime(0)
                , operationalDownTime(0)
                , operationalTotalTime(0)
                , operationalDownTimeProxy(0)
                , operationalTotalTimeProxy(0)
            {
            }
        };


        enum AvailabilityType
        {
            Printer,
            Operational1, // 1st Proxy i.e. Power On
            Operational2  // 2nd Proxy i.e. Jets On
        };

        /**
         * @brief This is a container for availability event names.
         *        When searching for i.e. printer availability events we need to
         *        look for "Printer Up" event, when searching for proxy 1 related events
         *        we need to look for "Proxy1_Up"
         */
        struct StatusEventNames
        {
            QString eventStartupName;
            QString eventShutdownName;
            QString eventDownName;
            QString eventUpName;
            StatusEventNames()
                : eventStartupName()
                , eventShutdownName()
                , eventDownName()
                , eventUpName()
            {
            }
        };

        struct EventStatItem
        {
           int alarmID;
           QString eventDesc;
           int totalDuration;
           int alarmCount;
           EventStatItem()
               : alarmID(0)
               , eventDesc("")
               , totalDuration(0)
               , alarmCount(0)
           {
           }
           EventStatItem(int idVal, QString eventDesc, int totalDuration, int alarmCount)
               : alarmID(idVal)
               , eventDesc(eventDesc)
               , totalDuration(totalDuration)
               , alarmCount(alarmCount)
           {
           }

        };

        struct EventStatistics
        {
            float availabilityPercentage;
            OEETimeFrame timeFrame;
            AvailabilityType oeeType;
            QList<EventStatItem> eventStatItems;
            EventStatistics()
                : availabilityPercentage(0.0)
                , timeFrame(eUndefined)
                , oeeType(Printer)
                , eventStatItems()
            {
            }
        };

        struct EventOccurenceItem
        {
            QString date;
            QString time;
            int duration;
            EventOccurenceItem()
                : date()
                , time()
                , duration(0)
            {
            };
            EventOccurenceItem(QString date, QString time, int duration)
                : date(date)
                , time(time)
                , duration(duration)
            {
            }
        };

        struct EventOccurence
        {
            int eventId;
            OEETimeFrame timeFrame;
            QList<EventOccurenceItem> eventOccurences;
            EventOccurence()
                : eventId(0)
                , timeFrame(eUndefined)
                , eventOccurences()
            {
            }
        };

        /**
         * @brief This is a struct for Open alarm events
         * which is used for alarm raised/cleared
         */
        struct OpenAlarmEvents
        {
            int eventId;
            QStringList varTexts;
            QString startTime;
            OpenAlarmEvents()
                : eventId(0)
                , varTexts()
                , startTime()
            {
            }
        };

        /** Event flags */
        enum Flags
        {
            Production,
            Event,
            Service,
            System,
            Security,
            Printer_Availability,
            Operational_Availability_Proxy1,
            Operational_Availability_Proxy2
        };

        /** Event Types */
        enum EventTypes
        {
            UserAction = 1,
            Warning,
            Alarm,
            Status,
            eUnknown
        };

        /** Event Types for emitting logevent signal */
        enum EventTypesForSignal
        {
            AvailabilityEvents = 1,
            Proxy1OperationalAvailabilityEvents = 2,
            Proxy2OperationalAvailabilityEvents = 3,
            AlarmEvents,
            AllEvents
        };

        class XPLATFORM_EVENTLOG_MANAGER_DLL_API EventLogManager : public QObject
        {
            Q_OBJECT

        public:

            typedef EventLogManager*  EventLogManagerPtr;

            /** @brief Destructor */
            ~EventLogManager();

            /** @brief Returns EventLogManager Instance */
            static EventLogManagerPtr GetInstance();

            /** @brief Add Events configuration from json file this function can be called multiple times
            *   @param file the json file where contain the event configuration.
            */
            bool AddEventConfiguration(const QString &file);

            /** @brief Get Number of platform events.
            *   @param Wstring indicated the type of the events
            */
            int GetNumOfEventConfiguration(const WString& /*xpath*/);


            /**
            * @brief Setup the database, as well as starts the worker thread, and establishes
            *        connection with the worker thread.
            * @param db_path the path where the database will be created
            */
            bool SetupDB(const QString &db_path);

            /** @brief Method to retrieve event logs and Alarms from database
             *  @param [in] eventFlags Flags to filter records while fetching from database
             *  @param [in] eventTypes Filtering based on event types
             *  @param [in] true if selected calender is Hijra
             *  @param [in] offset to indicate from where data to be fetched
             *  @param [in] limit to indicate the number of data to be fetched
             *  @return list containing all event logs and Alarms from database at the moment
             */
            QList<EventLogData> GetEventLogsForPrinterLogs(const QList<Flags> & eventFlags, const QList<EventTypes> &eventTypes = QList<EventTypes>(),bool isHijraCalender = false, int offset = 0, int limit = 0);

            /** @brief Method to retrieve event logs and Alarms from database
             *  @param [in] eventFlags Flags to filter records while fetching from database
             *  @param [in] eventTypes Filtering based on event types
             *  @param [in] true if selected calender is Hijra
             *  @param [in] offset to indicate from where data to be fetched
             *  @param [in] limit to indicate the number of data to be fetched
             *  @return list containing all event logs and Alarms from database at the moment
             */
            void GetEventLogsForPrinterLogsAsync
            (
                const QList<Flags>      &eventFlags,
                const QList<EventTypes> &eventTypes,
                bool                    isHijraCalender = false,
                int                     offset          = 0,
                int                     limit           = 0
            );

            /** @brief Method to retrieve event logs and Alarms from database
             *  @param [in] eventFlags Flags to filter records while fetching from database
             *  @param [in] eventTypes Filtering based on event types
             *  @return size size of all event logs in database at the moment
             */
            int GetEventLogsSizeForPrinterLogs(const QList<Flags> &eventFlags, const QList<EventTypes> &eventTypes = QList<EventTypes>());

            /** @brief Method to retrieve availability logs from an internal data list
             *  @return list containing availability log data from list
             */
            QList<AvailabilityLogData> GetAvailabilityLogs();

            /** @brief Method to retrieve EventStatItem logs
             *  @return list containing EventStatItem log data from list
             */
            QList<EventStatItem> GetEventStatItems();

            /**
             * @brief Method to retrieve AvailabilityType
             * @return AvailabilityType enum value corresponding to passed string
             */
            AvailabilityType GetAvailabilityType(const QString & type) const;

            /**
             * @brief Method to retrieve EventOccurenceItem logs.
             * @return list of occurence items for fault ids.
             */
            QList<EventOccurenceItem> GetEventOccurenceItems();

            /** @brief Subscribes for notification for
             *         - Language & Country value update
             *         - For all AvailabilityEvents in IM, whenever an event is raised or cleared.
             *         - For all parameter change, whenever value of any parameter changes
             *         - For all alarms
             *         - For all other custom events which can be raised
             *  @return true if subscription is successful else return false
             */
            bool SubscribeForParameterUpdate();

            /**
            * @brief Close the database, unsubscribe from parameter updates, and 
            * do any further clean-up that is required
            */            
            bool ShutDown();

            /**
             * @brief Raises an Internal event with the given ID and a given Name
             * @param eventId Id of the internal event
             * @param eventName Name of the internal event
             * @param closed Indicates that the event shall not be added to openEventList (default: false)
             */
            void RaiseInternalEvent(const int eventID, const QString &eventName, bool closed=false);

            /**
             * @brief Raises an Internal System Shutdown Event
             */
            void RaiseSystemShutdownEvent();

            /**
             * @brief Raises an Internal Proxy1 System Shutdown Event
             */
            void RaiseProxy1SystemShutdownEvent();

            /**
             * @brief Returns the column names of Time frame, printer availability and operational
             * availability from the OEE_Availablility table
             * @return list of the above mentioned column names
             */
            QStringList GetOEETableColumnNames();

            /**
             * @brief Method to update parameters of the record
             * @param [in] UniqueEventId - Unique Id of the event under updation
             * @param [in] eventDescription - Description to be updated to the record
             * @param [in] StartDate - Date to be updated to the record
             * @param [in] StartTime - Time to be updated to the record
             */
            void UpdateExistingServiceRecords( int nUniqueIndex, QString qEventDescription, const QString & qStartTime, QString qUser );

            /**
             * @brief Method to retrieve event logs from database
             * @param [in] nEventId - To fetch the records corresponding to Event ID
             * @return event record
             */
            QList<EventLogData> GetRecordUsingEventId( int nEventId );

            /**
             * @brief Function to get the whether a backup has been created or not
             * @return true if structure was bad and backup has been created, else false
             */
            bool GetBackupDbCreated();

            /** Method to localize the given date
             *  @param [in] date Date to be localized
             *  @return localized date as QString
             */
            QString GetLocalizedDate(const QDate &date) const;

            /**
             * @brief Function to get the total number of records available in database
             * @return total number of records
             */
            int GetTotalNumberofRecords();

            /**
             * Connects a slot to the EventLogManager signal that is triggered every time an event is raised.
             * @param pReceiver The object which slot will be invoked.
             * @param slot The encoded slot name with signature MySlot(xplatform::information_manager::Parameter node),
             * i.e.you should pass SLOT(MySlot(Parameter))
             */
            static bool ConnectToOnEventTriggerSignal(const QObject * pReceiver, const char* slot);

        public slots:
            /**
            * @brief Dump the event log data to a comma-separated-values file.
            * @param [in] csvFilePath The path to the output csv file.
            * @return false on file or db errors, else true
            */
            bool DumpEventLogDataToCsvFile (const QString &csvFilePath);

            /**
            * @brief Dump the availability log data to a comma-separated-values file.
            * @param [in] csvFilePath The path to the output csv file.
            * @return false on file or db errors, else true
            */
            bool DumpAvailabilityDataToCsvFile (const QString &csvFilePath);

            /**
            * @brief Dump a table from the eventlog.db to a comma-separated-values file.
            * @param [in] csvFilePath The path to the output csv file.
            * @param [in] tableName The name of the table.
            * @return false on file or db errors, else true
            */
            bool DumpTableToCsvFile (const QString &csvFilePath, const QString &tableName);

        signals:
            /**
            * @brief Signal to notify duration update
            */
            void updateDuration();
            /**
            * @brief Signal to notify table update
            */
            void updateDatabase();
            /**
             * @brief Signal to trigger an OEE model update
             */
            void updateOEEModel();
            /**
            * @brief Signal to notify that event is cleared
            */
            void clearEvent();
            /**
            * @brief Signal to notify that an event has been logged
            * @param int will determine whether it is a availability event or not
            */
            void logEvent(int);

            /**
             * @brief Emitted to notify of the progress of an export operation.
             * @param saved The number of log entries exported so far.
             * @param total The total number of log entries in the database.
             */
             void updateExportProgress(int saved, int total);

             /**
              * @brief exportFailed Emitted to notify that the export operation failed.
              */
             void exportFailed();

            /**
             * @brief Emitted to notify that event has triggered
             * @param Parameter node for which the event is triggered
             */
             void onEventTrigger(xplatform::information_manager::Parameter);
            
            /**
             * @brief Emitted to notify that printer log data has been retrieved
             *        from the database
             * @param [out] dataList        retrieved data
               @param [out] isHijriCalender true if selected calender was Hijri
             * @param [out] offset          to indicate from where data were fetched
             * @param [out] limit           to indicate the number of data were fetched
             */
             void printerLogData
             (
                 xplatform::event_logging::EventLogDataLstPtr dataList, 
                 bool                                         isHijriCalender,
                 int                                          offset,
                 int                                          limit
             );

        PRIVATE:

            /** @brief Constructor */
            EventLogManager();

            /**
             * @brief executes a query that creates tables for Event log and OEE availability.
             * @param dp_path path where the db shall be created
             * @return true if sql query succeeded, else false
             */
            bool CreateTables(const QString & db_path);

            /**
             * @brief Checks the structure of the table that got created or that is existing.
             * @param tableName Is the name of the table to be checked.
             * @param columnNames This is a vector of all column names that are expected to be present in the table.
             * @return false if the expected structure could not be found, else true.
             */
            bool CheckTableStructure(const QString &tableName, const QVector<QString> & columnNames);

            /**
             * @brief helper function for testing. Removes ALL entries from Event log AND OEE availability table.
             * @return true if sql query succeeded, else false
             */
            bool ClearTables();

            /**
             * @brief removes event logs older then 18 month
             * @return true if sql query succeeded, else false
             */
            bool DeleteOldEventlogEntries();

            /**
             * @brief restarts the timer for event log update
             * @return true if timer has been initialized, else false
             */
            bool StartUpdateTimer();

            /** @brief This event handler is re-implemented receive timer events for the object */
            void timerEvent(QTimerEvent *);

            /** @brief This handler gets called whenever RaiseEvent is called for any custom event for which
             *         event logging is required. This method then take appropriate actions e.g. insert log data in database
             *  @param [in] node event raising parameter
             */
            void OnEventTrigger( xplatform::information_manager::Parameter /*node*/);

            /** @brief This handler gets called whenever ClearEvent is raised for any open event
             *         This method then take appropriate actions i.e. Updating duration of event in database
             *         and updating open events map by deleting entry for this event
             *  @param [in] node event raising parameter
             */
            void OnClearEvent( xplatform::information_manager::Parameter /*node*/);

            /** Method to update database records at regular interval ( 1 minute ) */
            void UpdateRecords();

            /**
             * @brief Helper method to recalculate todays OEE data.
             *        This method reads todays eventlogs and overwrites todays OEE data with the updated data.
             * @return true if insertion into OEE table was successful, false if not
             */
            bool UpdateTodaysOEEData();

            /** Method to update duration of one record
             *  @param [in] eventId Id of the event under updation
             *  @param [in] startTime start time of the event
             */
            void UpdateRecordDuration(int /*eventId*/, QString /*startTime*/);

            /** Method to stop timer if already running */
            void StopTimer();

            /** Method to update locale information ( Language - Country) whenever its updated in IM */
            void UpdateLocale();

            /** Method to localize the given time
             *  @param [in] time Time to be localized
             *  @return localized time as QString
             */
            QString GetLocalizedTime(const QTime & time) const;

            /** @brief Subscribes for events in IM, to get notification
             *  whenever an event is raised or cleared.
             *  @param [in] xpath Log manager registers for notification for all children under this xpath subtree
             *  @return true if registration is successful else return false
             */
            bool RegisterForEventNotification(const WString& /*xpath*/);

            /** @brief Subscribes for Locale update whenever language or country changed in IM
             *  @return true if registration is successful else return false
             */
            bool RegisterForLocaleUpdate();

            /** @brief Subscribes for all parameter change events, to get notification whenever value of the
             *         parameter changes, new parameter gets added under a given node
             *         as a result of some action (e.g. new line added or new job loaded) or any parameter gets
             *         deleted under a given node
             *  @param [in] xpath Log manager registers for notification for all children under this xpath subtree
             *  @return true if registration is successful else return false
             */
            bool RegisterForParameterChangeEvents(const WString& /*xpath*/);

            /** @brief Subscribes to all AlarmManager notifications.
             *  @return true if registration is successful else return false
             */
            bool RegisterForAlarmManagerUpdates();

            /** @brief Subscribes for machine status parameter value change event.
             *  @return true if registration is successful else return false
             */
            bool RegisterForParameterMachineStatusChanged();

            /** @brief This handler gets called whenever any parameter value changes for which log manager has
             *  subscribed. This handler then fetch all the required data and log into database.
             *  @param [in] node Notifying parameter whose value has changed
             */
            void OnParameterUpdate(xplatform::information_manager::Parameter /*node*/ );

            /** @brief This handler gets called whenever new parameter gets added into IM under specific xpath, for which
             *  log manager has subscribed for the notification.
             *  Handler then fetch all the required data and log into database.
             *  @param [in] node Notifying parameter under which new child parameter is added
             */
            void OnParameterInsert(xplatform::information_manager::Parameter /*node*/ );

            /** @brief This handler gets called whenever a parameter gets deleted into IM under specific xpath, for which
             *  log manager has subscribed for the notification.
             *  Handler then fetch all the required data and log into database.
             *  @param [in] node Notifying parameter whose child is deleted
             */
            void OnParameterDelete(xplatform::information_manager::Parameter /*node*/ );

            /** @brief This handler gets called when machine status parameter is changed.
             *  This handler ends downtime for those alarms that have end downtime
             *  matching machine status value.
             *  @param [in] node machine status parameter
             */
            void OnParameterMachineStatusChanged(xplatform::information_manager::Parameter /*node*/ );

            /** This method is called internally whenever any open event (alarm or availability event) gets cleared;
             *  It then updates database with required information
             *  @param [in] eventId ID of the cleared event
             */
            void ClearEvent(int eventId);

            /** This method is called internally whenever any open alarm gets cleared;
             *  It then updates database with required information
             *  @param [in] alarm - alarm to be cleared
             */
            void ClearAlarm(xplatform::alarms::Alarm alarm);

            /** @brief This method fills the flagValueMap from the list of flags for the alarm
             *  @param [in] categoryList list of categories for an alarm
             *  @param [out] flagValueMap map to hold all flags information
             */
            void GetCategoryFlagInfo(xplatform::alarms::AlarmPropertyLstPtr categoryList, QMap<QString, int> &flagValueMap);

            /** @brief This method fetches the flags information from IM for the given log item and fills the flagValueMap
             *  @param [in] node parent node. Each category is assumed to be in a child node.
             *  @param [out] flagValueMap map to hold all flags information
             */
            void GetCategoryFlagInfo(xplatform::information_manager::Parameter node, QMap<QString, int> &flagValueMap);

            /** @brief Method to retrieve event logs from database
             *  @param [in] preparedSelectQuery SQL query to fetch data from database
             *  @param [in] true if selected calender is Hijra
             *  @return list containing all event logs from database at the moment
             */
            QList<EventLogData> GetEventLogs(const QString &preparedSelectQuery,
                                             QList<EventLogData> eventLogDataList,
                                             bool isHijraCalender = false);

            /** @brief Method to retrieve event logs from database
             *  @param [in] preparedSelectQuery SQL query to fetch data from database
             *  @return size of all event logs in database at the moment
             */
            int GetEventLogSize(const QString & preparedSelectQuery);

            /**
             *  @brief Method to form the condition to execute as part of the query
             *  @param [in] eventFlags Flags to filter records while fetching from database
             *  @param [in] eventTypes Filtering based on event types
             *  @return list containing the condition for the categories and events
             */
            QList<QString> PopulateQuery(const QList<Flags> & eventFlags, const QList<EventTypes> &eventTypes);

            /**
             *  @brief Method to insert record into database.
             *  @param [in] eventType        Type of the event (e.g. Alarm, Warning, Status etc. )
             *  @param [in] eventId          Event ID
             *  @param [in] eventDescription Name of event along with any additional information, if any
             *  @param [in] start_time       Date and time of event occurrence
             *  @param [in] end_time         Date and time of event clearance
             *  @param [in] varText          Data for Alarms with variable content
             *  @param [out] flag map        a map with flags set
             *  @return true if record has been inserted, false on error (db closed or sql)
             */
            bool InsertRecord(int /*eventType*/, int /*eventId*/, QString /*eventDescription*/,
                    QString /*start_time*/, QString /*end_time*/, const QMap<QString, int> &flagValueMap, QString varText = NULL /*User_Event_Var_Data"*/);

            /**
             * @brief this function checks event log table for new entries that
             * has to be added to the OEE Availability table. Missing days will be added and obsolete days will be removed.
             * Calculation of OEEAvailabity will be triggered
             */
            void RestoreOEEAvailabilityData();

            /**
             * @brief removes obsolete entries from OEE Availability Table (older than 7 month)
             */
            bool DeleteObsoleteEntries();

            /**
             * @brief Add end times to all blank End_Time fields. This means closing open events from the former session.
             * @return true if SQL query succeeded else false (fail)
             */
            bool RestoreMissingEndTime();

            /**
             * @brief Calculate OEEAvailabity data for all available time frames and adds data to member to be
             *          accessed by the ui model
             * @return true on success, false on fail
             */
            bool CalculateOEEAvailabity();

            /**
             * @brief For Test purpose only! This functions returns the pre-calculated OEE data.
             *        Usually these Data will be shown in the UI, for tests we will return it here.
             * @return A QList with calculated Availability data.
             */
            QList<OEEAvailableData> GetOEEAvailability();

            /** @brief Method to insert a record into OEE_AVAILABILITY table
             *  @param [in] date this is a date stamp representing a completely finished day
             *  @param [in] printerDownTime The value indicating the printer down time
             *  @param [in] printerTotalTime The value indicating the printer total time
             *  @param [in] operationalDownTime The value indicating the operational down time
             *  @param [in] operationalTotalTime The value indicating the total operational run time
             *  @param [in] operationalDownTimeProxy Remote operational down time
             *  @param [in] operationalTotalTimeProxy Remote operational total time
             *  @return true on success, false on failure
             */
            bool InsertIntoOEEAvailabilityTable(QString date, int printerDownTime, int printerTotalTime,
                                                              int operationalDownTime, int operationalTotalTime,
                                                              int operationalDownTimeProxy, int operationalTotalTimeProxy);


            /**
             * @brief This function calculates the totalTime and the totalDownTime for a given availability
             *        type on a given day (via dayOffset).
             * @param [in] dayOffset Offset in days relative to the given current Date from the DateTimeHandler.
             *        In test situation this is a fixed date, in real life it is now.
             * @param [out] reference to the totalTime
             * @param [out] reference to the totalDownTime
             * @param [in] type  Availability type (Printer, Operational_1 or Operational_2)
             * @return true on success, false on failure (i.e. SQL Errors)
            */
            bool CalculateOEETotalTimes(int dayOffset, int & totalTime, int & totalDownTime, AvailabilityType type = Printer);

            /**
             * @brief In this function the daily timer will be initialized and the current day will be set to a member
             */
            void InitDailyTimer();

            /**
             * @brief this is a little helper method to get the Date of the latest OEE Table entry
             * this is needed to determine how many days we have to look up and add from the event log
             * @return the date of the last entry
             */
            QDate GetLastOEEEntryDate();

            /**
             * @brief this is a little helper method to get the Date of the first EventLog Table entry
             * this is needed to determine the earliest possible date from the event log
             * @return the date of the first entry in event log
             */
            QDate GetFirstEventLogDate();

            /**
             * @brief This function can be used to receive a start and an end date for a specific amount of days (n)
             * in the past.
             * @param [in] n amount of days
             * @param [out] startDate the first date (n-days ago)
             * @param [out] endDate yesterday
             */
            void GetLastNDaysTimeFrame(int n, QString & startDate, QString &endDate);

            /**
             * @brief This function can be used to receive a start and an end date for the current month
             * @param [out] startDate the first day of the current month
             * @param [out] endDate yesterday
             */
            void GetCurrentMonthTimeFrame(QString & startDate, QString &endDate);

            /**
             * @brief This function can be used to receive a start and an end date for the current month
             * @param [in] n-th month in the past
             * @param [out] startDate the first day of the n-th month in the past
             * @param [out] endDate last day of the n-th month in the past
             */
            void GetLastNthMonthTimeFrame(int n, QString & startDate, QString &endDate);

            /**
             * @brief This function creates a human readable string according to the given time frame
             * @param [in] aTimeFrame the time frame to get a human readable string for
             * @return A human readable string (month names are displayed in Long version i.e. October)
             */
            QString GetTimeFrameName(OEETimeFrame aTimeFrame);

            /**
            * @brief Restore Missing endtime in table entries and start the timer on power cycle
            */
            bool ResetOnPowerCycle();

            /**
             * @brief This function is used to end downtime for an alarm
             * @param [in] downTimeStartAlarm alarm that started the downtime
             */
            void EndDownTime(const xplatform::alarms::Alarm &downTimeStartAlarm);

            /**
             * @brief This function is used to end downtime for a given downtime end condtion
             * @param [in] type downtime end condition type
             * @param [in] id down type end contion identifier
             */
            void ProcessDowntimeEndConditon(xplatform::alarms::DowntimeEndConditionType type, QVariant Id);
            
            /**
             * @brief This function builds the list of alarms that have unfinished downtime.
             * This function must be called at power cycle. 
             * This function examines the event log database and populates 
             * m_DowntimeStartAlarmList.
             */
            void BuildStartDownTimeAlarmList();

            /**
             * @brief This functions reads all Event log items and maintains the log string context details of
             * each event log
             */
            bool ReadTranslationContexts();

            /**
             * @brief This functions reads and add all Event log items and maintains the log string context details of
             * each event log
             * @param [in] eventlst the EventLogItemPtrLst need to get and add TranslationContexts to m_translationContexts
              */
            bool ReadandAddTranslationContexts(xplatform::event_logging::EventLogItemPtrLst& eventlst);

            /**
             * @brief This function updates all the system events with thier
             *        corresponding names and event ids from IM.
             */
            void UpdateSystemEventNamesAndIDs();

            /**
             * @brief This function gets the system event name and event id.
             * @param systemEventParam System event log parameter
             * @param systemEventName System event name
             * @param systemEventID System event id
             */
            void GetSystemEventNameAndID(xplatform::information_manager::LogEventParameter systemEventParam, QString &systemEventName,
                                         int &systemEventID);

            /**
             * @brief This functions returns the event log string's context details 
             * @param [in] Event ID of the event log whose tring's context details is required
             * @return context of event log string.
             */
            QString GetTranslationContext( int eventID );

            // Testing method to store values for table sizes into IM - used for diagnostic logging in continuous run health checks
            void StoreRowCounts();
#ifdef DB_PERFORMANCE_REPORTING
            xplatform::information_manager::Int32Parameter m_rowCountEventLog;
            xplatform::information_manager::Int32Parameter m_rowCountEventTable;
            xplatform::information_manager::Int32Parameter m_rowCountDowntimeAlarmList;
#endif
        public:

            /**
             * @brief Updates printer/operational nodes with last 30 days value
             */
            void UpdateOEENodesForLastThirtyDay();

            /**
             * @brief Fills a given EventStatistics object with corresponding data from the event log table
             *        A list of events that has been raised in the given time frame
             *        for the given availability type will be generated in this function.
             * @param [in] timeFrame which will be used to look for events in the event log table
             * @param [in] oeeType that shall be used as a filter for the table search
             * @param [out] eventStats with current search results. Note: Prior content will be cleared when
             *              calling this function.
             * @return true if sql query on event log table was successful, else false
             */
            bool PopulateEventStatistics(OEETimeFrame timeFrame, AvailabilityType oeeType, EventStatistics &eventStats);
            /**
             * @brief Fills a given EventOccurence structure with corresponding data from the event log table.
             *        All events of the given id in the given time frame will be considered
             * @param [in] timeFrame which will be used to look for events in the event log table
             * @param [in] eventId The ID of an event that shall be inspected/ added to the occurrence list
             * @param [out] eventOccurences Contains all occurrences of the given event in the given time frame after call.
             * @return true if sql query on event log table was successful, else false
             */
            bool PopulateEventOccurences(OEETimeFrame timeFrame, int eventId, EventOccurence &eventOccurences);
        PRIVATE:

            /**
             * @brief Helper method to retrieve start and end date for a given time frame i.e. last 90 days
             * @param [in] timeFrame to be used to retrieve start and end date
             * @param [out] startDate for the given time frame (empty on error)
             * @param [out] endDate for the given time frame (empty on error)
             */
            void GetDatesFromTimeFrame(OEETimeFrame timeFrame, QString& startDate, QString& endDate);

            /**
             * @brief Use this method to set a DateTimeHandler (i.e. Default or Test)
             * Handler then returns a corresponding date time.
             * @param pDateTimeHandler shared pointer to the DateTimeHandler object
             */
            void SetDateTimeHandler(QSharedPointer<IDateTimeHandler> pDateTimeHandler);

            /**
             * @brief This function sets the content of a StatusEventNames struct depending on the given availability type
             * @param [in] type availability type taken as a selector
             * @param [out] statusEventNames a reference to the struct that will be filled on calling this function.
             */
            void SetStatusEventNamesFromType(AvailabilityType type, StatusEventNames &statusEventNames);

            /**
             * @brief Call this function to determine if a the Event for the given availability type has been up before
             *          the given date-time. If no event could be found in the DB "true" / i.e. "Printer Up" will be assumed.
             * @param [in] type availability type taken as a selector
             * @param [in] startDateTime time (ISODate) from which we are going to look back and search for event up indicators.
             * @return true if event type has been up before.
             */
            bool IsEventTypeAlreadyUp(AvailabilityType type, QString startDateTime);

            /**
             * @brief Writes the buffered event log records into the database.
             */
            void WriteEventLogRecords();


        private slots:
            /**
             * @brief Notification on application close, for cleaning up instance
             */
            void OnAppAboutToQuit();

            /**
             * @brief Slot that will be called when an alarm has been raised.
             */
            void OnAlarmRaised(xplatform::alarms::Alarm);

            /**
             * @brief Slot that will be called when an alarm has been cleared.
             */
            void OnAlarmCleared(xplatform::alarms::Alarm);

            /**
             * @brief Will be called on a daily basis by the daily timer.
             */
            void OnNewDay();

        public slots:
            /**
             * @brief Will be called when PostInitialisation finishes for all components
             */
            void OnSystemRestart();

        private:
            typedef xplatform::information_manager::fn_notification_disconnector_t
                            NotificationDisconnector;
            typedef std::vector< NotificationDisconnector >
                            NotificationDisconnectorCollection;
            
            /**
             * @brief database used for event and alarm logging
             */
            QSqlDatabase                                    m_db;

            /**
             * @brief list of disconnectors for IM signal handlers
             */
            NotificationDisconnectorCollection              m_disconnectors;

            /**
             * @brief current logged in user
             */
            xplatform::information_manager::StringParameter m_logged_in_user;
            
            /**
             * @brief current language set in the UI
             */
            xplatform::information_manager::Int32Parameter  m_language;

            /**
             * @brief current country set in the UI
             */
            xplatform::information_manager::Int32Parameter  m_country;

            /**
             * @brief: Map with all the event log's context details mapped to Event ID
             */
            QMap<int, QString>                              m_translationContexts;

            /**
             * @brief: Map with current open events. int: EventID, QString: StartTime
             */
            QMap<int, QString>                              m_openEvents;

            /**
             * @brief: QList with currently open Alram events. int eventId, QStringList varTexts, QString startTime
             */
            QList<OpenAlarmEvents>                          m_openAlarmEventsLst;

            /**
             * @brief: List of raised events that started at least
             * one downtime
             */
            QList<xplatform::alarms::Alarm>                 m_DowntimeStartAlarmList;

            int                                             m_timer_id;
            QLocale                                         m_locale;
            static EventLogManagerPtr                       m_Instance;
            QMap<Flags, QString>                            m_flagsMap;

            /**
             * @brief a list with calculated availability data. (i.e. used for UI Model)
             */
            QList<OEEAvailableData>                         m_AvailabilityLogDataList;

            /**
             * @brief a EventStatItems list to be used for UI Model
             */
            QList<EventStatItem>                            m_EventStatItemList;

            /**
             * @brief m_EventOccurences list of all occurences per duration type.
             */
            QList<EventOccurenceItem>                       m_EventOccurences;
            
            /**
             * @brief: Counter for current active events that are flagged as availability event
             */
            unsigned int                                    m_AvailabilityEventCnt;

            /**
             * @brief: Counter for current active events that are flagged as proxy1 operational availability event
             */
            unsigned int                                    m_Proxy1AvailabilityEventCnt;

            /**
             * @brief: Counter for current active events that are flagged as proxy2 operational availability event
             */
            unsigned int                                    m_Proxy2AvailabilityEventCnt;

            /**
             * @brief: A daily timer to calculate/update the OEE availability
             */
            QTimer*                                         m_pDailyTimer;

            /**
             * @brief: represents the current day for the daily timer
             */
            int                                             m_CurrentDay;

            /**
             * @brief a list of all available time frames for OEE availability
             */
            QList<OEETimeFrame>                             m_TimeFrameList;

            /**
             * @brief pointer to DateTimeHandler. This handler can be used to get either fixed or current Date-Times
             */
            QSharedPointer<IDateTimeHandler>                m_pIdtHandler;

            /**
             * @brief flag to indicate that a backup DB has been created due to a bad structure.
             */
            bool                                            m_BackupDbCreated;

            /**
             * @brief worker thread object.
             */
            QThread                                         m_thread;

            /**
             * @brief synchronizes database and data member access.
             */
            mutable xplatform::mutex                        m_mutex;

            typedef xplatform::mutex::scoped_lock           event_lock;

            int m_kSystemStartupEventID;                 // ID of internal Event when Printer starts up
            int m_kPrinterUpEventID;                     // ID of internal Event when Printer is Available again
            int m_kPrinterDownEventID;                   // ID of internal Event when Printer is not Available
            int m_kSystemShutdownEventID;                // ID of internal Event when Printer is shutting down
            int m_kProxy1PrinterDownEventID;             // ID of Event when Printer is not Available for Proxy1(i.e. in Fault state)
            int m_kProxy1PrinterUpEventID;               // ID of Event when Printer is Available again for Proxy1(i.e. in Fault cleared)
            int m_kProxy2PrinterDownEventID;             // ID of Event when Printer is not Available for Proxy2(i.e. in Fault state)
            int m_kProxy2PrinterUpEventID;               // ID of Event when Printer is Available again for Proxy2(i.e. in Fault cleared)
            int m_kProxy1SystemStartupEventID;           // ID of Event when Printer starts up for Proxy1
            int m_kProxy1SystemShutdownEventID;          // ID of Event when Printer is shutting down for Proxy1
            int m_kProxy2SystemStartupEventID;           // ID of Event when Printer starts up for Proxy2
            int m_kProxy2SystemShutdownEventID;          // ID of Event when Printer is shutting down for Proxy2

            QString m_kPrinterUpEventName;
            QString m_kPrinterDownEventName;
            QString m_kSystemShutdownEventName;
            QString m_kSystemStartupEventName;
            QString m_kProxy1PrinterDownEventName;
            QString m_kProxy1PrinterUpEventName;
            QString m_kProxy2PrinterDownEventName;
            QString m_kProxy2PrinterUpEventName;
            QString m_kProxy1SystemStartupEventName;
            QString m_kProxy1SystemShutdownEventName;
            QString m_kProxy2SystemStartupEventName;
            QString m_kProxy2SystemShutdownEventName;

            class EventLogRec
            {
            public:
                /**
                 * Alarm or Event ?
                 */
                int                         m_eventType;

                /**
                 * Alarm or Event identifier
                 */
                int                         m_eventId;

                /**
                 * Additional information for an alarm or event
                 */
                QString                     m_eventDescription;

                /**
                 * When the alarm/event occured.
                 */
                QString                     m_start_time;

                /**
                 * When the alarm/event cleared.
                 */
                QString                     m_end_time;

                /**
                 * Stores category information of the alarm or event
                 */
                QMap<QString, int>          m_categoryFlagValues;

                /**
                 * Variable text information for an alarm
                 */
                QString                     m_varText;

                /**
                 * Event parameter node
                 */
                xplatform::information_manager::Parameter
                                            m_node;

                /**
                 * True when the event is open.
                 */
                bool                        m_eventIsOpen;

                EventLogRec
                (
                    int                 eventType,
                    int                 eventId,
                    QString             eventDescription,
                    QString             start_time,
                    QString             end_time,
                    QMap<QString, int>  flagValueMap,
                    QString varText = NULL /*User_Event_Var_Data"*/
                );

                EventLogRec
                (
                    int                 eventType,
                    QString             eventDescription,
                    QString             start_time,
                    QString             end_time,
                    xplatform::information_manager::Parameter node,
                    bool                eventIsOpen
                );
            };

            class EventLogParameterRec
            {
            public:
                /**
                 *  Indicates the EventLog list where the parameter changed
                 *  node has a target path.
                 */
                const xplatform::event_logging::TargetEventLogItemPtrMapPtr
                                                                m_logEventItemsPtr;

                /**
                 * Additonal information for the parameter changed.
                 */
                const QString                                   m_additionalInfo;
                /**
                 *   Help to get the event node path.
                 */
                const QString                                   m_targetpath;

                /**
                 * Parameter that has changed.
                 */
                const xplatform::information_manager::Parameter m_node;

                EventLogParameterRec
                (
                    xplatform::event_logging::TargetEventLogItemPtrMapPtr  logEventItems,
                    QString                                        &additionalInfo,
                    QString                                        &targetpath,
                    xplatform::information_manager::Parameter      &node
                )
                    : m_logEventItemsPtr(logEventItems)
                    , m_additionalInfo(additionalInfo)
                    , m_targetpath(targetpath)
                    , m_node(node)
                {
                }
            };

            /**
             * @brief list of buffered alarm or event records
             */
            QList<EventLogRec>                              m_eventlogrecs;

            /**
             * @brief protects m_eventlogrecs
             */
            mutable xplatform::mutex                        m_eventlogrecs_mutex;
            
            /**
             * @brief if true then Event log manager discards
             * events or alarm raised.
             */
            bool                                            m_throttling;

            /**
             * @brief Indicates database writing speed. Updated dynamically.
             */
            int                                             m_maxItemsToWrite;
            
            /**
             * Map for a parameter node to another parameter node that contains
             * the additonal information string for the key parameter.
             */
            QMap<xplatform::information_manager::Parameter, xplatform::information_manager::ReferenceParameter>
                                                            m_additonalInfo;
            QMap<xplatform::information_manager::Parameter, QString>    m_OnUpdateTargetPathMap;
            QMap<xplatform::information_manager::Parameter, QString>    m_OnInsertTargetPathMap;
            QMap<xplatform::information_manager::Parameter, QString>    m_OnDeleteTargetPathMap;
            /**
             * @brief Machince status enum parameter.
             */
            xplatform::information_manager::EnumParameter<xplatform::core_component::EOverallState> m_machine_status;

            /**
             * @brief List of LogEventParameter for which event log manager listens
             *        for IM update signal.
             */
            xplatform::event_logging::TargetEventLogItemPtrMapPtr   m_OnUpdateLogItemsMapPtr;

            /**
             * @brief List of LogEventParameter for which event log manager listens
             *        for IM insert child signal.
             */
            xplatform::event_logging::TargetEventLogItemPtrMapPtr   m_OnInsertLogItemsMapPtr;

            /**
             * @brief List of LogEventParameter for which event log manager listens
             *        for IM delete child signal.
             */
            xplatform::event_logging::TargetEventLogItemPtrMapPtr   m_OnDeleteLogItemsMapPtr;


            int                                             m_AvailibilityProxy2EventID;
        private slots:

            /**
             * @brief - signal handler for signal parameterChanged().
             *          Executes in the context of the worker thread.
             * @param param contains the information for LogEventParameter
             */
            void GetLogDataAndInsertAction(QSharedPointer<EventLogParameterRec> param);
            
            /**
             * @brief - signal handler for signal eventRaised()
             * Executes in the context of the worker thread.
             */
            void OnEventTriggerAction(QSharedPointer<EventLogRec>);

            /** @brief This handler gets called whenever ClearEvent is raised for any open event
             *         This method then take appropriate actions i.e. Updating duration of event in database
             *         and updating open events map by deleting entry for this event
             *  @param [in] node event raising parameter
             */
            void OnClearEventAction(const xplatform::information_manager::Parameter &/*node*/);
            
            /**
             * @brief - signal handler for signal machineStatusChange()
             * Executes in the context of the worker thread.
             */
            void OnParameterMachineStatusChangedAction(QString str);
            
            /**
             * @brief - signal handler for signal synchronise()
             * Executes in the context of the worker thread.
             */
            void SynchroniseAction();
            
            /**
             * @brief performs shutdown process
             * Executes in the context of the worker thread.
             */
            bool ShutDownAction();
            
            /**
             * @brief performs shutdown process
             * Executes in the context of the worker thread.
             */
            void SetDateTimeHandlerAction(QSharedPointer<IDateTimeHandler> pDateTimeHandler);

            /**
             * @brief gathers printer log information from the database
             * Executes in the context of the worker thread.
             */
             void GetEventLogsForPrinterLogsAction
             (
                const QList<Flags>      &eventFlags,
                const QList<EventTypes> &eventTypes,
                bool                    isHijriCalender,
                int                     offset,
                int                     limit
            );

        signals:
            /**
             * @brief - this signal is emitted to request
             * worker thread to clear an event
             * @param node Event node that is cleared
             */
             void performClearEvent(const xplatform::information_manager::Parameter &node);

            /**
             * @brief - this signal is emitted to request
             *          worker thread to log event records into the database
             * @param event information regarding the event node
             */
            void eventRaised(QSharedPointer<EventLogRec> event);

            /**
             * @brief - this signal is emitted to request
             *          worker thread to log event records into the database.
             *          The event records are
             *          gathered from an IM parameter changed event.
             * @param rec contains information for the changed parameter
             */
            void parameterChanged(QSharedPointer<EventLogParameterRec> rec);

            /**
             * @brief - this signal is emitted to request
             * worker to process machine status change event.
             * @param str   : Machine status name
             */
            void machineStatusChange(QString str);
            
            /**
             * @brief Blocks untill SynchroniseAction() completes.
             * This a signal to be used for flushing worker
             * thread's event queue to a particular point.
             */
            void synchronise();
            
            /**
             * @brief signal to worker thread to shutdown itself.
             */
            void exitEventLogThreadRequest();
            
            /**
             * @brief signal to worker thread to update time handler
             */
            void setDateTime(QSharedPointer<IDateTimeHandler> pDateTimeHandler);

            /**
             * @brief signal to worker to return printer logs
             */
            void getPrinterLogs
            (
                const QList<Flags>      &eventFlags,
                const QList<EventTypes> &eventTypes,
                bool                    isHijriCalender,
                int                     offset,
                int                     limit
            );
        };

        typedef EventLogManager::EventLogManagerPtr EventLogManagerPtr;

    }
}
#endif // XPLATFORM_EVENT_LOG_MANAGER_H
