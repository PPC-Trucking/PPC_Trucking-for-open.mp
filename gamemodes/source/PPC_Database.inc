stock Database_Connect()
{
    Database = mysql_connect_file("mysql.ini");

    new const 
		database_error = mysql_errno(Database)
	;

    if (database_error != 0)
    {
		print("---------------[MySQL]---------------");
		print(" ");
		printf("The gamemode was unable to connect to database: %d", database_error);
		print("Please check your 'mysql.ini' file and make sure the database is running.");
		print(" ");
		print("-------------------------------------\n");

        AddServerRule("Database", "disconnected");

        return ServerClose("DATABASE NOT VALID");
    }

    print("---------------[MySQL]---------------");
    print(" ");
    print("Database connected successfully!");
    print(" ");
    print("-------------------------------------\n");

    AddServerRule("database", "connected");

    return 1;

}

stock ServerClose(const reason[])
{
    if (!strlen(reason) || isnull(reason))
    {
        SendRconCommand("password 0");
        SendRconCommand("name PPC_Trucking [open.mp]");

        return 1;
    }

    new 
		command[64]
	;

    new const 
		random_password = random(65535)
	;

    format(command, sizeof(command), "password %d", random_password);
    SendRconCommand(command);

    format(command, sizeof(command), "name PPC_Trucking [%s]", reason);
    SendRconCommand(command);

    return 1;
}

forward OnPlayerAccountCheck(playerid);
public OnPlayerAccountCheck(playerid)
{
    if(cache_num_rows() > 0)
    {
        // An account exists with that name.
        cache_get_value(0, "PlayerPassword", APlayerData[playerid][PlayerPassword]);
        cache_get_value(0, "BanTime", APlayerData[playerid][BanTime]);

        // check if the player is permanently banned
        if (APlayerData[playerid][BanTime] == -1) 
        {
            SendClientMessage(playerid, COLOR_WHITE, TXT_BannedPermanently);
            SetTimerEx("TimedKick", 1000, false, "i", playerid); // Kick the player
        }

        // Check if the player is still banned
        else if (APlayerData[playerid][BanTime] > gettime()) // Player is still banned
        {
            ShowRemainingBanTime(playerid); // Show the remaining ban-time to the player is days, hours, minutes, seconds
            SetTimerEx("TimedKick", 1000, false, "i", playerid); // Kick the player            
        }

        // Player ban-time is passed or he isn't banned
        else // Player ban-time is passed
        { 
            ShowPlayerDialog(playerid, DialogLogin, DIALOG_STYLE_PASSWORD, TXT_DialogLoginTitle, TXT_DialogLoginMsg, TXT_DialogLoginButton1, TXT_DialogButtonCancel);
        }
    }
    else
    { 
        // That name is not registered. Player needs to register.
        ShowPlayerDialog(playerid, DialogRegister, DIALOG_STYLE_PASSWORD, TXT_DialogRegisterTitle, TXT_DialogRegisterMsg, TXT_DialogRegisterButton1, TXT_DialogButtonCancel);
    }

    return 1;
}

forward OnPlayerHashPassword(playerid);
public OnPlayerHashPassword(playerid)
{
    new hash[BCRYPT_HASH_LENGTH];
    bcrypt_get_hash(hash, sizeof (hash));

    RegisterAccountForPlayer(playerid, hash);

    return 1;
}

RegisterAccountForPlayer(playerid, hash[])
{
    new query[256];
    mysql_format(Database, query, sizeof (query), "INSERT INTO `players` (`PlayerName`, `PlayerPassword`) VALUES ('%e', '%s')", APlayerData[playerid][PlayerName], hash);
    mysql_tquery(Database, query, "OnPlayerFinishRegistration", "i", playerid);

    return 1;
}

forward OnPlayerFinishRegistration(playerid);
public OnPlayerFinishRegistration(playerid)
{
    APlayerData[playerid][PlayerID] = cache_insert_id(); // Get the Database ID of the player
	APlayerData[playerid][LoggedIn] = true; // The player has logged in properly

    // Create a player file
    PlayerFile_Create(playerid);

    // Load the player file
    PlayerFile_Load(playerid);

	SendClientMessage(playerid, COLOR_WHITE, TXT_AccountRegistered);
    return 1;
}

forward OnPlayerVerifyPassword(playerid, bool:success);
public OnPlayerVerifyPassword(playerid, bool:success)
{
	new 
		Msg[128]
	;

    if(!success)
    {
        // Password is wrong.
        SetPVarInt(playerid, "LoginAttempt", GetPVarInt(playerid, "LoginAttempt") + 1);

        if (GetPVarInt(playerid, "LoginAttempt") == MAX_LOGIN_ATTEMPTS) 
		{
			SendClientMessage(playerid, COLOR_RED, TXT_FailedLoginProperly);
			SetTimerEx("TimedKick", 1000, false, "i", playerid);
		} 
		else 
		{
			format(Msg, sizeof(Msg), TXT_WrongPassword, (MAX_LOGIN_ATTEMPTS-GetPVarInt(playerid, "LoginAttempt")));
			SendClientMessage(playerid, COLOR_WHITE, Msg);

			ShowPlayerDialog(playerid, DialogLogin, DIALOG_STYLE_PASSWORD, TXT_DialogLoginTitle, TXT_DialogLoginMsg, TXT_DialogLoginButton1, TXT_DialogButtonCancel);
		}    
	}
    else
    {
        // Password is valid.
        new query[256];
        mysql_format(Database, query, sizeof (query), "SELECT * FROM `players` WHERE `PlayerName` = '%e' LIMIT 1", APlayerData[playerid][PlayerName]);
        mysql_tquery(Database, query, "OnPlayerAccountLoad", "i", playerid);

    }

    return 1;
}

forward OnPlayerAccountLoad(playerid);
public OnPlayerAccountLoad(playerid)
{
    // Check if we actually found the player in the database
    if (cache_num_rows() == 0)
    {
        printf("Error: Player %d's account not found in database during login", playerid);
        SendClientMessage(playerid, COLOR_RED, "Account not found. Please register first.");
        SetTimerEx("TimedKick", 1000, false, "i", playerid);
        return 0;
    }

    // Mark the player as logged in
    APlayerData[playerid][LoggedIn] = true;

    // Load player data from the database
    cache_get_value_name_int(0, "PlayerID", APlayerData[playerid][PlayerID]);
    cache_get_value_name_int(0, "PlayerLevel", APlayerData[playerid][PlayerLevel]);
    cache_get_value_name_int(0, "PlayerJailed", APlayerData[playerid][PlayerJailed]);
    cache_get_value_name_int(0, "WantedLevel", APlayerData[playerid][WantedLevel]);
    cache_get_value_name_int(0, "Bans", APlayerData[playerid][Bans]);
    cache_get_value_name_int(0, "BanTime", APlayerData[playerid][BanTime]);
    cache_get_value_name_int(0, "TruckerLicense", APlayerData[playerid][TruckerLicense]);
    cache_get_value_name_int(0, "BusLicense", APlayerData[playerid][BusLicense]);
    cache_get_value_name_int(0, "Muted", APlayerData[playerid][Muted]);
    cache_get_value_name_int(0, "Money", APlayerData[playerid][PlayerMoney]);
    cache_get_value_name_int(0, "Score", APlayerData[playerid][PlayerScore]);

    // Load boolean values
    new rulesRead[5];
    cache_get_value_name(0, "RulesRead", rulesRead);
    APlayerData[playerid][RulesRead] = (strcmp(rulesRead, "Yes", true) == 0) ? true : false;

    // Load float values
    cache_get_value_name_float(0, "StatsMetersDriven", APlayerData[playerid][StatsMetersDriven]);

    // Load integer stats
    cache_get_value_name_int(0, "StatsTruckerJobs", APlayerData[playerid][StatsTruckerJobs]);
    cache_get_value_name_int(0, "StatsConvoyJobs", APlayerData[playerid][StatsConvoyJobs]);
    cache_get_value_name_int(0, "StatsBusDriverJobs", APlayerData[playerid][StatsBusDriverJobs]);
    cache_get_value_name_int(0, "StatsPilotJobs", APlayerData[playerid][StatsPilotJobs]);
    cache_get_value_name_int(0, "StatsMafiaJobs", APlayerData[playerid][StatsMafiaJobs]);
    cache_get_value_name_int(0, "StatsMafiaStolen", APlayerData[playerid][StatsMafiaStolen]);
    cache_get_value_name_int(0, "StatsPoliceFined", APlayerData[playerid][StatsPoliceFined]);
    cache_get_value_name_int(0, "StatsPoliceJailed", APlayerData[playerid][StatsPoliceJailed]);
    cache_get_value_name_int(0, "StatsAssistance", APlayerData[playerid][StatsAssistance]);
    cache_get_value_name_int(0, "StatsCourierJobs", APlayerData[playerid][StatsCourierJobs]);
    cache_get_value_name_int(0, "StatsRoadworkerJobs", APlayerData[playerid][StatsRoadworkerJobs]);

    // Validate that PlayerID was loaded correctly
    if (APlayerData[playerid][PlayerID] <= 0)
    {
        printf("Error: Player %d loaded invalid PlayerID (%d) from database", playerid, APlayerData[playerid][PlayerID]);
        SendClientMessage(playerid, COLOR_RED, "Database error loading your account. Please contact an administrator.");
        SetTimerEx("TimedKick", 1000, false, "i", playerid);
        return 0;
    }

    // Load the player file
    PlayerFile_Load(playerid);

    // Load the player's bank account
    BankFile_Load(playerid);

    // Set the player's wanted level
    SetPlayerWantedLevel(playerid, APlayerData[playerid][WantedLevel]);

    // Reward the player with money and score
    RewardPlayer(playerid, APlayerData[playerid][PlayerMoney], APlayerData[playerid][PlayerScore]);

    // Clear login attempt variable
    DeletePVar(playerid, "LoginAttempt");

    // Notify the player
    SendClientMessage(playerid, COLOR_WHITE, TXT_LoggedIn);
    
    printf("Player %d logged in successfully with PlayerID %d", playerid, APlayerData[playerid][PlayerID]);

    return 1;
}

SavePlayerData(playerid)
{
    // Validate player data before saving
    if (!IsPlayerConnected(playerid))
    {
        printf("Error: Attempted to save data for disconnected player %d", playerid);
        return 0;
    }
    
    if (APlayerData[playerid][PlayerID] <= 0)
    {
        printf("Error: Player %d has invalid PlayerID (%d), cannot save data", playerid, APlayerData[playerid][PlayerID]);
        return 0;
    }
    
    if (!APlayerData[playerid][LoggedIn])
    {
        printf("Error: Player %d is not logged in, cannot save data", playerid);
        return 0;
    }

    // Create a query string
    new query[1024];

    // Format the query with all player data
    mysql_format(Database, query, sizeof(query), 
        "UPDATE `players` SET \
        `PlayerLevel` = %i, \
        `PlayerJailed` = %i, \
        `WantedLevel` = %i, \
        `Bans` = %i, \
        `BanTime` = %i, \
        `TruckerLicense` = %i, \
        `BusLicense` = %i, \
        `Muted` = %i, \
        `Money` = %i, \
        `Score` = %i, \
        `RulesRead` = '%s', \
        `StatsMetersDriven` = %f, \
        `StatsTruckerJobs` = %i, \
        `StatsConvoyJobs` = %i, \
        `StatsBusDriverJobs` = %i, \
        `StatsPilotJobs` = %i, \
        `StatsMafiaJobs` = %i, \
        `StatsMafiaStolen` = %i, \
        `StatsPoliceFined` = %i, \
        `StatsPoliceJailed` = %i, \
        `StatsAssistance` = %i, \
        `StatsCourierJobs` = %i, \
        `StatsRoadworkerJobs` = %i \
        WHERE `PlayerID` = %i",
        APlayerData[playerid][PlayerLevel],
        APlayerData[playerid][PlayerJailed],
        APlayerData[playerid][WantedLevel],
        APlayerData[playerid][Bans],
        APlayerData[playerid][BanTime],
        APlayerData[playerid][TruckerLicense],
        APlayerData[playerid][BusLicense],
        APlayerData[playerid][Muted],
        APlayerData[playerid][PlayerMoney],
        APlayerData[playerid][PlayerScore],
        (APlayerData[playerid][RulesRead]) ? ("Yes") : ("No"),
        APlayerData[playerid][StatsMetersDriven],
        APlayerData[playerid][StatsTruckerJobs],
        APlayerData[playerid][StatsConvoyJobs],
        APlayerData[playerid][StatsBusDriverJobs],
        APlayerData[playerid][StatsPilotJobs],
        APlayerData[playerid][StatsMafiaJobs],
        APlayerData[playerid][StatsMafiaStolen],
        APlayerData[playerid][StatsPoliceFined],
        APlayerData[playerid][StatsPoliceJailed],
        APlayerData[playerid][StatsAssistance],
        APlayerData[playerid][StatsCourierJobs],
        APlayerData[playerid][StatsRoadworkerJobs],
        APlayerData[playerid][PlayerID]);

    // Execute the query with error handling
    mysql_tquery(Database, query, "OnPlayerDataSaved", "i", playerid);
    
    return 1;
}

// Optional callback to handle the query result
forward OnPlayerDataSaved(playerid);
public OnPlayerDataSaved(playerid)
{
    // Check if the query was successful
    if (cache_affected_rows() > 0)
    {
        printf("Player %d's data has been saved to the database.", playerid);
        
        // Only notify player if they're still connected
        if (IsPlayerConnected(playerid))
            SendClientMessage(playerid, COLOR_GREEN, "Your data has been saved successfully.");
    }
    else
    {
        printf("Failed to save player %d's data to the database. No rows affected.", playerid);
        
        // Log additional debugging info
        new Name[24];
        if (IsPlayerConnected(playerid))
        {
            GetPlayerName(playerid, Name, sizeof(Name));
            printf("Debug: Player %s (ID: %d, PlayerID: %d, LoggedIn: %d)", 
                Name, playerid, APlayerData[playerid][PlayerID], APlayerData[playerid][LoggedIn]);
        }
        
        // Check for MySQL errors
        if (mysql_errno(Database) != 0)
        {
            new error[256];
            mysql_error(error, sizeof(error), Database);
            printf("MySQL Error: %s", error);
        }
    }
    return 1;
}

// Function to check if database connection is alive
stock bool:IsDatabaseConnected()
{
    if (Database == MYSQL_INVALID_HANDLE)
    {
        printf("[MYSQL ERROR] Database handle is invalid");
        return false;
    }
    
    // Try a simple query to test connection
    new Cache:result = mysql_query(Database, "SELECT 1");
    if (CheckMySQLError("Database Connection Test"))
    {
        return false;
    }
    
    cache_delete(result);
    return true;
}