// Define the time in days where the auto-evict system should auto-evict player's properties
#define AutoEvictTime   2

// Setup a custom type that holds all data about the auto-evict system
enum TAutoEvict
{
	AEMinutes, // Holds the amount of minutes the server is online
	AEHours, // Holds the amount of hours the server is online
	AEDays // Holds the amount of days the server is online
}
// Setup a variable to hold all data about the auto-evict system
new AutoEvict[TAutoEvict];

// This global auto-evict timer runs every minute and increases the server-online time
forward AutoEvictTimer();
public AutoEvictTimer()
{
	// Increase the amount of minutes the server is online
	AutoEvict[AEMinutes]++;

	// Increase the amount of hours if the minutes reaches 60
	if (AutoEvict[AEMinutes] == 60)
	{
		AutoEvict[AEHours]++;
		AutoEvict[AEMinutes] = 0;
	}

	// Increase the days the server is online if the hours reaches 24
	if (AutoEvict[AEHours] == 24)
	{
	    AutoEvict[AEDays]++;
	    AutoEvict[AEHours] = 0;

		// Auto-evict all properties if the owner didn't login within the AutoEvictTime
		AutoEvictProperties();
	}

	// Save the auto-evict-time to MySQL database
	AutoEvict_Save();

	return 1;
}

// This function scans all houses and businesses and auto-evicts them if needed
AutoEvictProperties()
{
	// Setup local variables
	new query[256];

	// Log the eviction start to MySQL database
	mysql_format(Database, query, sizeof(query),
		"INSERT INTO `auto_evict_log` (`log_message`, `log_date`) VALUES ('Auto-eviction started for day: %d', NOW())",
		AutoEvict[AEDays]);
	mysql_query(Database, query, false);

	// Query all owned houses that need to be auto-evicted
	mysql_format(Database, query, sizeof(query),
		"SELECT `HouseID`, `HouseName`, `Owner` FROM `houses` WHERE `Owned` = 1 AND `AutoEvictDays` < %d",
		AutoEvict[AEDays] - AutoEvictTime);
	mysql_tquery(Database, query, "OnAutoEvictHousesFound", "");

	// Query all owned businesses that need to be auto-evicted
	mysql_format(Database, query, sizeof(query),
		"SELECT `BusinessID`, `BusinessName`, `Owner` FROM `businesses` WHERE `Owned` = 1 AND `AutoEvictDays` < %d",
		AutoEvict[AEDays] - AutoEvictTime);
	mysql_tquery(Database, query, "OnAutoEvictBusinessesFound", "");

	// Log the eviction end to MySQL database
	mysql_format(Database, query, sizeof(query),
		"INSERT INTO `auto_evict_log` (`log_message`, `log_date`) VALUES ('Auto-eviction ended for day: %d', NOW())",
		AutoEvict[AEDays]);
	mysql_query(Database, query, false);
}

// Callback for houses that need to be auto-evicted
forward OnAutoEvictHousesFound();
public OnAutoEvictHousesFound()
{
	new Cache:evictRows = cache_save();
	new rows = cache_num_rows();

	if (rows > 0)
	{
		for (new i = 0; i < rows; i++)
		{
			cache_set_active(evictRows);

			new HouseID, HouseNameBuf[100], OwnerName[24];
			cache_get_value_name_int(i, "HouseID", HouseID);
			cache_get_value_name(i, "HouseName", HouseNameBuf, 100);
			cache_get_value_name(i, "Owner", OwnerName, 24);

			new query[160];
			mysql_format(Database, query, sizeof(query), "SELECT `HouseID` FROM `houses` WHERE `HouseID` = %d AND `Owned` = 1 AND `AutoEvictDays` < %d LIMIT 1", HouseID, AutoEvict[AEDays] - AutoEvictTime);
			new Cache:confirm = mysql_query(Database, query);
			new bool:stillEligible = (cache_num_rows() > 0);
			cache_delete(confirm);

			if (!stillEligible)
				continue;

			// Auto-Evict the house
			House_Evict(HouseID);
			// Log the auto-eviction
			LogAutoEvict(1, HouseNameBuf, OwnerName);
		}
	}

	cache_delete(evictRows);

	return 1;
}

// Callback for businesses that need to be auto-evicted
forward OnAutoEvictBusinessesFound();
public OnAutoEvictBusinessesFound()
{
	new Cache:evictRows = cache_save();
	new rows = cache_num_rows();

	if (rows > 0)
	{
		for (new i = 0; i < rows; i++)
		{
			cache_set_active(evictRows);

			new BusinessID, BusinessNameBuf[100], BusinessOwnerName[24];
			cache_get_value_name_int(i, "BusinessID", BusinessID);
			cache_get_value_name(i, "BusinessName", BusinessNameBuf, 100);
			cache_get_value_name(i, "Owner", BusinessOwnerName, 24);

			new query[160];
			mysql_format(Database, query, sizeof(query), "SELECT `BusinessID` FROM `businesses` WHERE `BusinessID` = %d AND `Owned` = 1 AND `AutoEvictDays` < %d LIMIT 1", BusinessID, AutoEvict[AEDays] - AutoEvictTime);
			new Cache:confirm = mysql_query(Database, query);
			new bool:stillEligible = (cache_num_rows() > 0);
			cache_delete(confirm);

			if (!stillEligible)
				continue;

			// Auto-Evict the business
			Business_Evict(BusinessID);
			// Log the auto-eviction
			LogAutoEvict(2, BusinessNameBuf, BusinessOwnerName);
		}
	}

	cache_delete(evictRows);

	return 1;
}

// This function informs all admins about the property that has been auto-evicted and logs to database
LogAutoEvict(PropertyType, PropertyName[], PropertyOwner[])
{
	// Setup local variables
	new Msg[128], query[256];

	// Determine the type of the property
	if (PropertyType == 1)
		format(Msg, sizeof(Msg), "{AAAAAA}House auto-evicted: %s, owner: %s", PropertyName, PropertyOwner);
	else
		format(Msg, sizeof(Msg), "{AAAAAA}Business auto-evicted: %s, owner: %s", PropertyName, PropertyOwner);

	// Loop through all players and inform admins
	for (new i; i < MAX_PLAYERS; i++)
	{
		if (APlayerData[i][PlayerLevel] > 0)
			SendClientMessage(i, COLOR_WHITE, Msg);
	}

	// Log to MySQL database
	if (PropertyType == 1)
		format(Msg, sizeof(Msg), "House auto-evicted: %s, owner: %s", PropertyName, PropertyOwner);
	else
		format(Msg, sizeof(Msg), "Business auto-evicted: %s, owner: %s", PropertyName, PropertyOwner);

	mysql_format(Database, query, sizeof(query),
		"INSERT INTO `auto_evict_log` (`log_message`, `log_date`) VALUES ('%e', NOW())", Msg);
	mysql_query(Database, query, false);
}

// This function loads the auto-evict time from MySQL database
AutoEvict_Load()
{
	new query[256];

	// Try to load auto-evict data from settings table
	mysql_format(Database, query, sizeof(query),
		"SELECT `setting_value` FROM `settings` WHERE `setting_name` = 'AutoEvictMinutes' LIMIT 1");
	new Cache:result = mysql_query(Database, query);

	if (cache_num_rows() > 0)
		cache_get_value_name_int(0, "setting_value", AutoEvict[AEMinutes]);
	else
		AutoEvict[AEMinutes] = 0;
	cache_delete(result);

	mysql_format(Database, query, sizeof(query),
		"SELECT `setting_value` FROM `settings` WHERE `setting_name` = 'AutoEvictHours' LIMIT 1");
	result = mysql_query(Database, query);

	if (cache_num_rows() > 0)
		cache_get_value_name_int(0, "setting_value", AutoEvict[AEHours]);
	else
		AutoEvict[AEHours] = 0;
	cache_delete(result);

	mysql_format(Database, query, sizeof(query),
		"SELECT `setting_value` FROM `settings` WHERE `setting_name` = 'AutoEvictDays' LIMIT 1");
	result = mysql_query(Database, query);

	if (cache_num_rows() > 0)
		cache_get_value_name_int(0, "setting_value", AutoEvict[AEDays]);
	else
		AutoEvict[AEDays] = 0;
	cache_delete(result);

	return 1;
}

// This function saves the auto-evict time to MySQL database
AutoEvict_Save()
{
	new query[256];

	// Save Minutes
	mysql_format(Database, query, sizeof(query),
		"INSERT INTO `settings` (`setting_name`, `setting_value`) VALUES ('AutoEvictMinutes', '%d') ON DUPLICATE KEY UPDATE `setting_value` = '%d'",
		AutoEvict[AEMinutes], AutoEvict[AEMinutes]);
	mysql_query(Database, query, false);

	// Save Hours
	mysql_format(Database, query, sizeof(query),
		"INSERT INTO `settings` (`setting_name`, `setting_value`) VALUES ('AutoEvictHours', '%d') ON DUPLICATE KEY UPDATE `setting_value` = '%d'",
		AutoEvict[AEHours], AutoEvict[AEHours]);
	mysql_query(Database, query, false);

	// Save Days
	mysql_format(Database, query, sizeof(query),
		"INSERT INTO `settings` (`setting_name`, `setting_value`) VALUES ('AutoEvictDays', '%d') ON DUPLICATE KEY UPDATE `setting_value` = '%d'",
		AutoEvict[AEDays], AutoEvict[AEDays]);
	mysql_query(Database, query, false);

	return 1;
}

// This function will evict the given business
Business_Evict(BusID)
{
	// Clear ownership of the business
	ABusinessData[BusID][Owned] = false;
	format(ABusinessData[BusID][Owner], 24, "");

	// Update business entrance
	Business_UpdateEntrance(BusID);

	// Save business to MySQL.
	Business_Save(BusID);
}
