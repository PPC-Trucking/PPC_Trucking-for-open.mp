PlayerFile_Create(playerid)
{
	new file[100], File:PFile, Name[24];//, LineForFile[150];
	format(Name, sizeof(Name), APlayerData[playerid][PlayerName]); // Get the playername
	format(file, sizeof(file), PlayerFile, Name); // Construct the complete filename for this player's account


	PFile = fopen(file, io_write); // Open the playerfile for writing

	fclose(PFile); // Close the file
	return 1;
}

// This function will load the player's datafile (used when a player connects to the server)
PlayerFile_Load(playerid)
{
    // Setup local variables
    new file[150], File:PFile, Name[24], LineFromFile[150], ParameterName[50], ParameterValue[50], HouseIndex, BusIndex;

    format(Name, sizeof(Name), APlayerData[playerid][PlayerName]); // Get the playername

    format(file, sizeof(file), PlayerFile, Name); // Construct the complete filename for this player's account

    // Check if the player's datafile exists
    if (fexist(file))
    {
        PFile = fopen(file, io_read); // Open the playerfile for reading

        fread(PFile, LineFromFile); // Read the first line of the file

        // Keep reading until the end of the file is found (no more data)
        while (strlen(LineFromFile) > 0)
        {
            StripNewLine(LineFromFile); // Strip any newline characters from the LineFromFile
            sscanf(LineFromFile, "s[50]s[50]", ParameterName, ParameterValue); // Extract parametername and parametervalue
			
            if (strcmp(ParameterName, "House", false) == 0) // If the parametername is correct ("House")
            {
                APlayerData[playerid][Houses][HouseIndex] = strval(ParameterValue); // Store the HouseID at the selected slot
                HouseIndex++; // Select the next house-slot in case another house-id is found
            }
            if (strcmp(ParameterName, "Business", false) == 0) // If the parametername is correct ("Business")
            {
                if (strcmp(ABusinessData[strval(ParameterValue)][Owner], Name, false) == 0) // Check if the player is the true owner of the business
                {
                    APlayerData[playerid][Business][BusIndex] = strval(ParameterValue); // Store the BusinessID at the selected slot
                    BusIndex++; // Select the next business-slot in case another business-id is found
                }
            }
			
            fread(PFile, LineFromFile); // Read the next line of the file
        }

        fclose(PFile); // Close the file

        return 1; // Return if the file was read correctly
    }
    else
        return 0; // Return 0 if the file couldn't be read (doesn't exist)
}

// This function will save all player-data (used when the player disconnects from the server)
PlayerFile_Save(playerid)
{
	new file[100], File:PFile, Name[24], LineForFile[100];
	format(Name, sizeof(Name), APlayerData[playerid][PlayerName]); // Get the playername
	format(file, sizeof(file), PlayerFile, Name); // Construct the complete filename for this player's account

	PFile = fopen(file, io_write); // Open the playerfile for writing

	// Save all the valid house-id's
	for (new i; i < MAX_HOUSESPERPLAYER; i++)
	{
		// Check if there is a valid house-id in this slot
		if (APlayerData[playerid][Houses][i] != 0)
		{
			format(LineForFile, sizeof(LineForFile), "House %i\r\n", APlayerData[playerid][Houses][i]); // Construct the line: "House <HouseID>"
			fwrite(PFile, LineForFile); // And save it to the file
		}
	}
	// Save all the valid business-id's
	for (new i; i < MAX_BUSINESSPERPLAYER; i++)
	{
		// Check if there is a valid business-id in this slot
		if (APlayerData[playerid][Business][i] != 0)
		{
			format(LineForFile, sizeof(LineForFile), "Business %i\r\n", APlayerData[playerid][Business][i]); // Construct the line: "Business <BusinessID>"
			fwrite(PFile, LineForFile); // And save it to the file
		}
	}

	format(LineForFile, sizeof(LineForFile), "Money %i\r\n", APlayerData[playerid][PlayerMoney]); // Construct the line: "Money <playermoney>"
	fwrite(PFile, LineForFile); // And save it to the file
	format(LineForFile, sizeof(LineForFile), "Score %i\r\n", APlayerData[playerid][PlayerScore]); // Construct the line: "Score <playerscore>"
	fwrite(PFile, LineForFile); // And save it to the file

	fclose(PFile); // Close the file

	// Also save the houses that this player owns - NOW USING MySQL
	for (new i; i < MAX_HOUSESPERPLAYER; i++)
	{
		// Check if there is a valid house-id in this slot
		if (APlayerData[playerid][Houses][i] != 0)
		{
		    // Save the house (and linked vehicles) using MySQL
		    House_Save(APlayerData[playerid][Houses][i]);
		}
	}
	// Save all the valid business-id's - NOW USING MySQL
	for (new i; i < MAX_BUSINESSPERPLAYER; i++)
	{
		// Check if there is a valid business-id in this slot
		if (APlayerData[playerid][Business][i] != 0)
		{
		    // Save the business using MySQL
		    Business_Save(APlayerData[playerid][Business][i]);
		}
	}

	// Save the player's data to the database
    SavePlayerData(playerid);

	return 1;
}

// This function loads all houses from MySQL database
Housing_LoadAll()
{
    new query[256];
    
    // Query to load all houses from database
    mysql_format(Database, query, sizeof(query), "SELECT * FROM `houses` ORDER BY `HouseID` ASC");
    mysql_tquery(Database, query, "OnHousesLoaded", "");
    
    return 1;
}

// Callback when houses are loaded from database
forward OnHousesLoaded();
public OnHousesLoaded()
{
    new rows = cache_num_rows();
    
    if (rows > 0)
    {
        for (new i = 0; i < rows; i++)
        {
            new HouseID;
            cache_get_value_name_int(i, "HouseID", HouseID);
            
            // Load house data from database row
            cache_get_value_name(i, "HouseName", AHouseData[HouseID][HouseName], 100);
            cache_get_value_name_float(i, "HouseX", AHouseData[HouseID][HouseX]);
            cache_get_value_name_float(i, "HouseY", AHouseData[HouseID][HouseY]);
            cache_get_value_name_float(i, "HouseZ", AHouseData[HouseID][HouseZ]);
            cache_get_value_name_int(i, "HouseLevel", AHouseData[HouseID][HouseLevel]);
            cache_get_value_name_int(i, "HouseMaxLevel", AHouseData[HouseID][HouseMaxLevel]);
            cache_get_value_name_int(i, "HousePrice", AHouseData[HouseID][HousePrice]);
            cache_get_value_name_int(i, "Owned", AHouseData[HouseID][Owned]);
            cache_get_value_name(i, "Owner", AHouseData[HouseID][Owner], 24);
            cache_get_value_name_int(i, "Insurance", AHouseData[HouseID][Insurance]);
            cache_get_value_name_int(i, "AutoEvictDays", AHouseData[HouseID][AutoEvictDays]);
            
            // Set defaults if not set
            if (AHouseData[HouseID][HouseMaxLevel] == 0)
                AHouseData[HouseID][HouseMaxLevel] = 10;
            
            // Set house as closed by default
            AHouseData[HouseID][HouseOpened] = false;
            
            // Create entrance pickup and 3D text
            House_CreateEntrance(HouseID);
            
            // Load vehicles for this house
            House_LoadVehicles(HouseID);
            
            // Count loaded houses
            TotalHouses++;
        }
        
        printf("Loaded %d houses from MySQL database", rows);
    }
    else
    {
        printf("No houses found in database");
    }
    
    return 1;
}

// This function loads a single house from MySQL database
House_Load(HouseID)
{
    new query[256];
    
    // Query to load specific house
    mysql_format(Database, query, sizeof(query), "SELECT * FROM `houses` WHERE `HouseID` = %d LIMIT 1", HouseID);
    new Cache:result = mysql_query(Database, query);
    
    if (cache_num_rows() > 0)
    {
        // Load house data from database
        cache_get_value_name(0, "HouseName", AHouseData[HouseID][HouseName], 100);
        cache_get_value_name_float(0, "HouseX", AHouseData[HouseID][HouseX]);
        cache_get_value_name_float(0, "HouseY", AHouseData[HouseID][HouseY]);
        cache_get_value_name_float(0, "HouseZ", AHouseData[HouseID][HouseZ]);
        cache_get_value_name_int(0, "HouseLevel", AHouseData[HouseID][HouseLevel]);
        cache_get_value_name_int(0, "HouseMaxLevel", AHouseData[HouseID][HouseMaxLevel]);
        cache_get_value_name_int(0, "HousePrice", AHouseData[HouseID][HousePrice]);
        cache_get_value_name_int(0, "Owned", AHouseData[HouseID][Owned]);
        cache_get_value_name(0, "Owner", AHouseData[HouseID][Owner], 24);
        cache_get_value_name_int(0, "Insurance", AHouseData[HouseID][Insurance]);
        cache_get_value_name_int(0, "AutoEvictDays", AHouseData[HouseID][AutoEvictDays]);
        
        // Set defaults
        if (AHouseData[HouseID][HouseMaxLevel] == 0)
            AHouseData[HouseID][HouseMaxLevel] = 10;
        
        AHouseData[HouseID][HouseOpened] = false;
        
        cache_delete(result);
        
        // Load vehicles for this house
        House_LoadVehicles(HouseID);
        
        return 1;
    }
    
    cache_delete(result);
    return 0;
}

// This function saves a house to MySQL database
House_Save(HouseID)
{
    new query[512];
    
    // Check if house exists in database
    mysql_format(Database, query, sizeof(query), "SELECT `HouseID` FROM `houses` WHERE `HouseID` = %d LIMIT 1", HouseID);
    new Cache:result = mysql_query(Database, query);
    
    if (cache_num_rows() > 0)
    {
        // Update existing house
        cache_delete(result);
        mysql_format(Database, query, sizeof(query), 
            "UPDATE `houses` SET `HouseName` = '%e', `HouseX` = %f, `HouseY` = %f, `HouseZ` = %f, `HouseLevel` = %d, `HouseMaxLevel` = %d, `HousePrice` = %d, `Owned` = %d, `Owner` = '%e', `Insurance` = %d, `AutoEvictDays` = %d, `LastAccess` = NOW() WHERE `HouseID` = %d",
            AHouseData[HouseID][HouseName],
            AHouseData[HouseID][HouseX],
            AHouseData[HouseID][HouseY],
            AHouseData[HouseID][HouseZ],
            AHouseData[HouseID][HouseLevel],
            AHouseData[HouseID][HouseMaxLevel],
            AHouseData[HouseID][HousePrice],
            AHouseData[HouseID][Owned],
            AHouseData[HouseID][Owner],
            AHouseData[HouseID][Insurance],
            AHouseData[HouseID][AutoEvictDays],
            HouseID
        );
    }
    else
    {
        // Insert new house
        cache_delete(result);
        mysql_format(Database, query, sizeof(query), 
            "INSERT INTO `houses` (`HouseID`, `HouseName`, `HouseX`, `HouseY`, `HouseZ`, `HouseLevel`, `HouseMaxLevel`, `HousePrice`, `Owned`, `Owner`, `Insurance`, `AutoEvictDays`) VALUES (%d, '%e', %f, %f, %f, %d, %d, %d, %d, '%e', %d, %d)",
            HouseID,
            AHouseData[HouseID][HouseName],
            AHouseData[HouseID][HouseX],
            AHouseData[HouseID][HouseY],
            AHouseData[HouseID][HouseZ],
            AHouseData[HouseID][HouseLevel],
            AHouseData[HouseID][HouseMaxLevel],
            AHouseData[HouseID][HousePrice],
            AHouseData[HouseID][Owned],
            AHouseData[HouseID][Owner],
            AHouseData[HouseID][Insurance],
            AHouseData[HouseID][AutoEvictDays]
        );
    }
    
    mysql_query(Database, query);
    
    if (CheckMySQLError("House_Save"))
        return 0;
    
    // Also save vehicles for this house
    House_SaveVehicles(HouseID);
    
    return 1;
}

// This function creates a new house in the database
House_Create(Float:x, Float:y, Float:z, Price, MaxLevel = 10)
{
    new query[256];
    
    // Insert new house into database
    mysql_format(Database, query, sizeof(query), 
        "INSERT INTO `houses` (`HouseName`, `HouseX`, `HouseY`, `HouseZ`, `HouseLevel`, `HouseMaxLevel`, `HousePrice`, `Owned`, `Owner`, `Insurance`, `AutoEvictDays`) VALUES ('House', %f, %f, %f, 1, %d, %d, 0, '', 0, 0)",
        x, y, z, MaxLevel, Price
    );
    
    mysql_tquery(Database, query, "OnHouseCreated", "fffi", x, y, z, Price);
    
    return 1;
}

// Callback when house is created
forward OnHouseCreated(Float:x, Float:y, Float:z, Price);
public OnHouseCreated(Float:x, Float:y, Float:z, Price)
{
    new HouseID = cache_insert_id();
    
    if (HouseID > 0)
    {
        // Initialize house data
        format(AHouseData[HouseID][HouseName], 100, "House");
        AHouseData[HouseID][HouseX] = x;
        AHouseData[HouseID][HouseY] = y;
        AHouseData[HouseID][HouseZ] = z;
        AHouseData[HouseID][HouseLevel] = 1;
        AHouseData[HouseID][HouseMaxLevel] = 10;
        AHouseData[HouseID][HousePrice] = Price;
        AHouseData[HouseID][Owned] = false;
        format(AHouseData[HouseID][Owner], 24, "");
        AHouseData[HouseID][Insurance] = 0;
        AHouseData[HouseID][AutoEvictDays] = 0;
        AHouseData[HouseID][HouseOpened] = false;
        
        // Clear vehicle slots
        for (new i = 0; i < 10; i++)
            AHouseData[HouseID][VehicleIDs][i] = 0;
        
        // Create entrance
        House_CreateEntrance(HouseID);
        
        printf("House %d created at position %.2f, %.2f, %.2f with price $%d", HouseID, x, y, z, Price);
        return HouseID;
    }
    
    return 0;
}

// This function deletes a house from the database
House_Delete(HouseID)
{
    new query[128];
    
    // First delete all vehicles associated with this house
    mysql_format(Database, query, sizeof(query), "DELETE FROM `house_vehicles` WHERE `HouseID` = %d", HouseID);
    mysql_query(Database, query);
    
    // Delete the house
    mysql_format(Database, query, sizeof(query), "DELETE FROM `houses` WHERE `HouseID` = %d", HouseID);
    mysql_query(Database, query);
    
    if (CheckMySQLError("House_Delete"))
        return 0;
    
    // Clear house data from memory
    format(AHouseData[HouseID][HouseName], 100, "");
    AHouseData[HouseID][HouseX] = 0.0;
    AHouseData[HouseID][HouseY] = 0.0;
    AHouseData[HouseID][HouseZ] = 0.0;
    AHouseData[HouseID][HouseLevel] = 0;
    AHouseData[HouseID][HouseMaxLevel] = 0;
    AHouseData[HouseID][HousePrice] = 0;
    AHouseData[HouseID][Owned] = false;
    format(AHouseData[HouseID][Owner], 24, "");
    AHouseData[HouseID][Insurance] = 0;
    AHouseData[HouseID][AutoEvictDays] = 0;
    
    // Destroy pickups and 3D text
    if (IsValidDynamicPickup(AHouseData[HouseID][PickupID]))
        DestroyDynamicPickup(AHouseData[HouseID][PickupID]);
    if (IsValidDynamic3DTextLabel(AHouseData[HouseID][DoorText]))
        DestroyDynamic3DTextLabel(AHouseData[HouseID][DoorText]);
    if (IsValidDynamicMapIcon(AHouseData[HouseID][MapIconID]))
        DestroyDynamicMapIcon(AHouseData[HouseID][MapIconID]);
    
    // Clear vehicle slots
    for (new i = 0; i < 10; i++)
    {
        if (AHouseData[HouseID][VehicleIDs][i] != 0)
        {
            DestroyVehicle(AHouseData[HouseID][VehicleIDs][i]);
            AHouseData[HouseID][VehicleIDs][i] = 0;
        }
    }
    
    printf("House %d deleted from database", HouseID);
    return 1;
}

// Load vehicles for a specific house
House_LoadVehicles(HouseID)
{
    new query[256];
    
    mysql_format(Database, query, sizeof(query), "SELECT * FROM `house_vehicles` WHERE `HouseID` = %d ORDER BY `CarSlot` ASC", HouseID);
    mysql_tquery(Database, query, "OnHouseVehiclesLoaded", "i", HouseID);
    
    return 1;
}

// Callback when house vehicles are loaded
forward OnHouseVehiclesLoaded(HouseID);
public OnHouseVehiclesLoaded(HouseID)
{
    new rows = cache_num_rows();
    
    if (rows > 0)
    {
        for (new i = 0; i < rows; i++)
        {
            new CarSlot, VehicleModel, Color1, Color2, Fuel, VehiclePaintJob, NeonsApplied, Clamped;
            new Float:VehicleX, Float:VehicleY, Float:VehicleZ, Float:VehicleAngle;
            new components[14];
            
            cache_get_value_name_int(i, "CarSlot", CarSlot);
            cache_get_value_name_int(i, "VehicleModel", VehicleModel);
            cache_get_value_name_int(i, "Color1", Color1);
            cache_get_value_name_int(i, "Color2", Color2);
            cache_get_value_name_int(i, "Fuel", Fuel);
            cache_get_value_name_int(i, "VehiclePaintJob", VehiclePaintJob);
            cache_get_value_name_int(i, "NeonsApplied", NeonsApplied);
            cache_get_value_name_int(i, "Clamped", Clamped);
            cache_get_value_name_float(i, "VehicleX", VehicleX);
            cache_get_value_name_float(i, "VehicleY", VehicleY);
            cache_get_value_name_float(i, "VehicleZ", VehicleZ);
            cache_get_value_name_float(i, "VehicleAngle", VehicleAngle);
            
            // Load components
            cache_get_value_name_int(i, "VehicleSpoiler", components[0]);
            cache_get_value_name_int(i, "VehicleHood", components[1]);
            cache_get_value_name_int(i, "VehicleRoof", components[2]);
            cache_get_value_name_int(i, "VehicleSideSkirt", components[3]);
            cache_get_value_name_int(i, "VehicleLamps", components[4]);
            cache_get_value_name_int(i, "VehicleNitro", components[5]);
            cache_get_value_name_int(i, "VehicleExhaust", components[6]);
            cache_get_value_name_int(i, "VehicleWheels", components[7]);
            cache_get_value_name_int(i, "VehicleStereo", components[8]);
            cache_get_value_name_int(i, "VehicleHydraulics", components[9]);
            cache_get_value_name_int(i, "VehicleFrontBumper", components[10]);
            cache_get_value_name_int(i, "VehicleRearBumper", components[11]);
            cache_get_value_name_int(i, "VehicleVentRight", components[12]);
            cache_get_value_name_int(i, "VehicleVentLeft", components[13]);
            
            // Create the vehicle if valid slot
            if (CarSlot >= 0 && CarSlot < 10)
            {
                House_AddVehicle(HouseID, VehicleModel, VehiclePaintJob, components, VehicleX, VehicleY, VehicleZ, VehicleAngle, Color1, Color2);
                
                // Set additional vehicle data
                new vid = AHouseData[HouseID][VehicleIDs][CarSlot];
                if (vid != 0)
                {
                    AVehicleData[vid][Fuel] = Fuel;
                    AVehicleData[vid][Clamped] = Clamped;
                    // Apply neons if needed
                    if (NeonsApplied != 0)
                    {
                        // Apply neon components based on NeonsApplied value
                        // This would need to be implemented based on your neon system
                    }
                }
            }
        }
    }
    
    return 1;
}

// Save all vehicles for a specific house
House_SaveVehicles(HouseID)
{
    new query[512];
    
    // First delete all existing vehicles for this house
    mysql_format(Database, query, sizeof(query), "DELETE FROM `house_vehicles` WHERE `HouseID` = %d", HouseID);
    mysql_query(Database, query);
    
    // Save current vehicles
    for (new CarSlot = 0; CarSlot < 10; CarSlot++)
    {
        new vid = AHouseData[HouseID][VehicleIDs][CarSlot];
        
        if (vid != 0)
        {
            new Float:vx, Float:vy, Float:vz, Float:vrot;
            GetVehiclePos(vid, vx, vy, vz);
            GetVehicleZAngle(vid, vrot);
            
            mysql_format(Database, query, sizeof(query), 
                "INSERT INTO `house_vehicles` (`HouseID`, `CarSlot`, `VehicleModel`, `Fuel`, `VehiclePaintJob`, `VehicleSpoiler`, `VehicleHood`, `VehicleRoof`, `VehicleSideSkirt`, `VehicleLamps`, `VehicleNitro`, `VehicleExhaust`, `VehicleWheels`, `VehicleStereo`, `VehicleHydraulics`, `VehicleFrontBumper`, `VehicleRearBumper`, `VehicleVentRight`, `VehicleVentLeft`, `Color1`, `Color2`, `VehicleX`, `VehicleY`, `VehicleZ`, `VehicleAngle`, `NeonsApplied`, `Clamped`) VALUES (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %f, %f, %f, %f, %d, %d)",
                HouseID, CarSlot, AVehicleData[vid][Model], AVehicleData[vid][Fuel], AVehicleData[vid][PaintJob],
                AVehicleData[vid][Components][0], AVehicleData[vid][Components][1], AVehicleData[vid][Components][2],
                AVehicleData[vid][Components][3], AVehicleData[vid][Components][4], AVehicleData[vid][Components][5],
                AVehicleData[vid][Components][6], AVehicleData[vid][Components][7], AVehicleData[vid][Components][8],
                AVehicleData[vid][Components][9], AVehicleData[vid][Components][10], AVehicleData[vid][Components][11],
                AVehicleData[vid][Components][12], AVehicleData[vid][Components][13], AVehicleData[vid][Color1],
                AVehicleData[vid][Color2], vx, vy, vz, vrot, 0, AVehicleData[vid][Clamped]
            );
            
            mysql_query(Database, query);
            
            if (CheckMySQLError("House_SaveVehicles"))
                continue;
        }
    }
    
    return 1;
}

// Load player's houses from MySQL database
PlayerHouses_Load(playerid)
{
    new Name[24], query[256];
    GetPlayerName(playerid, Name, sizeof(Name));
    
    // Clear current house slots
    for (new i = 0; i < MAX_HOUSESPERPLAYER; i++)
        APlayerData[playerid][Houses][i] = 0;
    
    // Load player's houses from database
    mysql_format(Database, query, sizeof(query), "SELECT `HouseID` FROM `houses` WHERE `Owner` = '%e' ORDER BY `HouseID` ASC", Name);
    mysql_tquery(Database, query, "OnPlayerHousesLoaded", "i", playerid);
    
    return 1;
}

// Callback for loading player houses
forward OnPlayerHousesLoaded(playerid);
public OnPlayerHousesLoaded(playerid)
{
    new rows = cache_num_rows();
    new HouseIndex = 0;
    
    if (rows > 0)
    {
        for (new i = 0; i < rows && HouseIndex < MAX_HOUSESPERPLAYER; i++)
        {
            new HouseID;
            cache_get_value_name_int(i, "HouseID", HouseID);
            
            // Store house ID in player's house array
            APlayerData[playerid][Houses][HouseIndex] = HouseID;
            HouseIndex++;
        }
        
        printf("Loaded %d houses for player %s", rows, APlayerData[playerid][PlayerName]);
    }
    
    return 1;
}

// This function loads all speedcameras
Camera_LoadAll()
{
    new query[128];
    
    // Query all cameras from database
    mysql_format(Database, query, sizeof(query), "SELECT * FROM `cameras` ORDER BY `CameraID`");
    new Cache:result = mysql_query(Database, query);
    
    new rows = cache_num_rows();
    if (rows > 0)
    {
        new CameraID, Float:x, Float:y, Float:z, Float:angle, maxSpeed;
        
        for (new i = 0; i < rows; i++)
        {
            cache_get_value_name_int(i, "CameraID", CameraID);
            cache_get_value_name_float(i, "CamX", x);
            cache_get_value_name_float(i, "CamY", y);
            cache_get_value_name_float(i, "CamZ", z);
            cache_get_value_name_float(i, "CamAngle", angle);
            cache_get_value_name_int(i, "CamSpeed", maxSpeed);
            
            // Setup camera in memory and create objects
            // Use CameraID-1 as array index since database IDs start from 1, arrays from 0
            if (CameraID > 0 && CameraID <= MAX_CAMERAS)
            {
                SetupSpeedCamera(CameraID - 1, x, y, z, angle, maxSpeed);
                TotalCameras++;
            }
        }
    }
    
    cache_delete(result);
    printf("[CAMERAS] Loaded %d cameras from database", TotalCameras);
    
    return 1;
}

// This function saves a camera to the database
CameraFile_Save(CamID, const createdBy[] = "")
{
    new query[256];
    
    // Insert camera into database (CamID+1 because database IDs start from 1)
    mysql_format(Database, query, sizeof(query), 
        "INSERT INTO `cameras` (`CamX`, `CamY`, `CamZ`, `CamAngle`, `CamSpeed`, `CreatedBy`) VALUES (%f, %f, %f, %f, %d, '%e')",
        ACameras[CamID][CamX],
        ACameras[CamID][CamY], 
        ACameras[CamID][CamZ],
        ACameras[CamID][CamAngle],
        ACameras[CamID][CamSpeed],
createdBy
   );
   
   mysql_query(Database, query);
   
   if (CheckMySQLError("CameraFile_Save"))
       return 0;
   
   return 1;
}

// This function deletes a camera from the database
CameraFile_Delete(CamID)
{
   new query[128];
   
   // Delete camera from database (CamID+1 because database IDs start from 1)
   mysql_format(Database, query, sizeof(query), "DELETE FROM `cameras` WHERE `CameraID` = %d", CamID + 1);
   mysql_query(Database, query);
   
   if (CheckMySQLError("CameraFile_Delete"))
       return 0;
   
   return 1;
}

// This function gets the next available camera slot
GetNextCameraSlot()
{
   for (new CamID = 0; CamID < MAX_CAMERAS; CamID++)
   {
       if (ACameras[CamID][CamSpeed] == 0) // Empty slot
           return CamID;
   }
   return -1; // No free slots
}

// This function loads all businesses from MySQL database
Business_LoadAll()
{
   // First load the business-time
   BusinessTime_Load();

   // Query to load all businesses from database
   new query[256];
   mysql_format(Database, query, sizeof(query), "SELECT * FROM `businesses` ORDER BY `BusinessID` ASC");
   mysql_tquery(Database, query, "OnBusinessesLoaded", "");
   
   return 1;
}

// Callback when businesses are loaded from database
forward OnBusinessesLoaded();
public OnBusinessesLoaded()
{
   new rows = cache_num_rows();
   
   if (rows > 0)
   {
   	for (new i = 0; i < rows; i++)
   	{
   		new BusinessID;
   		cache_get_value_name_int(i, "BusinessID", BusinessID);
   		
   		// Load business data from database row
   		cache_get_value_name(i, "BusinessName", ABusinessData[BusinessID][BusinessName], 100);
   		cache_get_value_name_float(i, "BusinessX", ABusinessData[BusinessID][BusinessX]);
   		cache_get_value_name_float(i, "BusinessY", ABusinessData[BusinessID][BusinessY]);
   		cache_get_value_name_float(i, "BusinessZ", ABusinessData[BusinessID][BusinessZ]);
   		cache_get_value_name_int(i, "BusinessType", ABusinessData[BusinessID][BusinessType]);
   		cache_get_value_name_int(i, "BusinessLevel", ABusinessData[BusinessID][BusinessLevel]);
   		cache_get_value_name_int(i, "LastTransaction", ABusinessData[BusinessID][LastTransaction]);
   		cache_get_value_name_int(i, "Owned", ABusinessData[BusinessID][Owned]);
   		cache_get_value_name(i, "Owner", ABusinessData[BusinessID][Owner], 24);
   		cache_get_value_name_int(i, "AutoEvictDays", ABusinessData[BusinessID][AutoEvictDays]);
   		
   		// Add a pickup and 3DText for this business
   		Business_CreateEntrance(BusinessID);
   		// Count the loaded businesses
   		TotalBusiness++;
   	}
   	
   	printf("Loaded %d businesses from MySQL database", rows);
   }
   else
   {
   	printf("No businesses found in database");
   }
   
   return 1;
}

// This function loads the current business-time from MySQL database
BusinessTime_Load()
{
   new query[128];
   
   mysql_format(Database, query, sizeof(query), 
   	"SELECT `setting_value` FROM `settings` WHERE `setting_name` = 'BusinessTransactionTime' LIMIT 1");
   new Cache:result = mysql_query(Database, query);
   
   if (cache_num_rows() > 0)
   {
   	cache_get_value_name_int(0, "setting_value", BusinessTransactionTime);
   	cache_delete(result);
   	return 1;
   }
   else
   {
   	// Create default entry if not found
   	mysql_format(Database, query, sizeof(query), 
   		"INSERT INTO `settings` (`setting_name`, `setting_value`) VALUES ('BusinessTransactionTime', '0')");
   	mysql_query(Database, query);
   	BusinessTransactionTime = 0;
   	cache_delete(result);
   	return 0;
   }
}

// This function saves the current business-time to MySQL database
BusinessTime_Save()
{
   new query[128];
   
   mysql_format(Database, query, sizeof(query), 
   	"INSERT INTO `settings` (`setting_name`, `setting_value`) VALUES ('BusinessTransactionTime', '%d') ON DUPLICATE KEY UPDATE `setting_value` = '%d'",
   	BusinessTransactionTime, BusinessTransactionTime);
   mysql_query(Database, query);
   
   return 1;
}

// This function will save the given business to MySQL database
Business_Save(BusID)
{
   new query[512];
   
   // Check if business exists in database
   mysql_format(Database, query, sizeof(query), "SELECT `BusinessID` FROM `businesses` WHERE `BusinessID` = %d LIMIT 1", BusID);
   new Cache:result = mysql_query(Database, query);
   
   if (cache_num_rows() > 0)
   {
   	// Update existing business
   	cache_delete(result);
   	mysql_format(Database, query, sizeof(query), 
   		"UPDATE `businesses` SET `BusinessName` = '%e', `BusinessX` = %f, `BusinessY` = %f, `BusinessZ` = %f, `BusinessType` = %d, `BusinessLevel` = %d, `LastTransaction` = %d, `Owned` = %d, `Owner` = '%e', `AutoEvictDays` = %d, `LastAccess` = NOW() WHERE `BusinessID` = %d",
   		ABusinessData[BusID][BusinessName],
   		ABusinessData[BusID][BusinessX],
   		ABusinessData[BusID][BusinessY],
   		ABusinessData[BusID][BusinessZ],
   		ABusinessData[BusID][BusinessType],
   		ABusinessData[BusID][BusinessLevel],
   		ABusinessData[BusID][LastTransaction],
   		ABusinessData[BusID][Owned],
   		ABusinessData[BusID][Owner],
   		ABusinessData[BusID][AutoEvictDays],
   		BusID
   	);
   }
   else
   {
   	// Insert new business
   	cache_delete(result);
   	mysql_format(Database, query, sizeof(query), 
   		"INSERT INTO `businesses` (`BusinessID`, `BusinessName`, `BusinessX`, `BusinessY`, `BusinessZ`, `BusinessType`, `BusinessLevel`, `LastTransaction`, `Owned`, `Owner`, `AutoEvictDays`) VALUES (%d, '%e', %f, %f, %f, %d, %d, %d, %d, '%e', %d)",
   		BusID,
   		ABusinessData[BusID][BusinessName],
   		ABusinessData[BusID][BusinessX],
   		ABusinessData[BusID][BusinessY],
   		ABusinessData[BusID][BusinessZ],
   		ABusinessData[BusID][BusinessType],
   		ABusinessData[BusID][BusinessLevel],
   		ABusinessData[BusID][LastTransaction],
   		ABusinessData[BusID][Owned],
   		ABusinessData[BusID][Owner],
   		ABusinessData[BusID][AutoEvictDays]
   	);
   }
   
   mysql_query(Database, query);
   
   if (CheckMySQLError("Business_Save"))
   	return 0;
   
   return 1;
}

// Load player's businesses from MySQL database
PlayerBusinesses_Load(playerid)
{
   new Name[24], query[256];
   GetPlayerName(playerid, Name, sizeof(Name));
   
   // Clear current business slots
   for (new i = 0; i < MAX_BUSINESSPERPLAYER; i++)
   	APlayerData[playerid][Business][i] = 0;
   
   // Load player's businesses from database
   mysql_format(Database, query, sizeof(query), "SELECT `BusinessID` FROM `businesses` WHERE `Owner` = '%e' ORDER BY `BusinessID` ASC", Name);
   mysql_tquery(Database, query, "OnPlayerBusinessesLoaded", "i", playerid);
   
   return 1;
}

// Callback for loading player businesses
forward OnPlayerBusinessesLoaded(playerid);
public OnPlayerBusinessesLoaded(playerid)
{
   new rows = cache_num_rows();
   new BusinessIndex = 0;
   
   if (rows > 0)
   {
   	for (new i = 0; i < rows && BusinessIndex < MAX_BUSINESSPERPLAYER; i++)
   	{
   		new BusinessID;
   		cache_get_value_name_int(i, "BusinessID", BusinessID);
   		
   		// Store business ID in player's business array
   		APlayerData[playerid][Business][BusinessIndex] = BusinessID;
   		BusinessIndex++;
   	}
   	
   	printf("Loaded %d businesses for player %s", rows, APlayerData[playerid][PlayerName]);
   }
   
   return 1;
}

// This function will load the player's bank account and process transfers that were made to his account when he was offline
BankFile_Load(playerid)
{
  new Name[24], query[256], Message[128], OldBankMoney, NewBankMoney;
  
  GetPlayerName(playerid, Name, sizeof(Name));
  
  // Check if the player has a bank account
  mysql_format(Database, query, sizeof(query), "SELECT * FROM `banks` WHERE `PlayerName` = '%e' LIMIT 1", Name);
  new Cache:result = mysql_query(Database, query);
  
  // Check if player has a bank account
  if (cache_num_rows() > 0) {
      // Send a message to the player to inform him that he has a bank account
      SendClientMessage(playerid, COLOR_DODGERBLUE, "You have a bank-account. Use {FFBB00}/bank{00BBFF} to access it");
      
      // Load bank data from database
      cache_get_value_name(0, "BankPassword", APlayerData[playerid][BankPassword], BCRYPT_HASH_LENGTH);
      cache_get_value_name_int(0, "BankMoney", APlayerData[playerid][BankMoney]);
      cache_get_value_name_int(0, "LastInterestTime", APlayerData[playerid][LastInterestTime]);
      
      // Calculate interest if enabled
      if (InterestEnabled == true && APlayerData[playerid][LastInterestTime] < CurrentInterestTime)
      {
          OldBankMoney = APlayerData[playerid][BankMoney];
          
          // Calculate compound interest for each time period missed
          new Float:CurrentAmount = float(APlayerData[playerid][BankMoney]);
          new PeriodsToCalculate = CurrentInterestTime - APlayerData[playerid][LastInterestTime];
          
          // Apply compound interest for each period
          for (new i = 0; i < PeriodsToCalculate; i++)
          {
              // Calculate interest on the current amount (compound interest)
              new Float:InterestEarned = floatmul(CurrentAmount, BankInterest);
              CurrentAmount = floatadd(CurrentAmount, InterestEarned);
          }
          
          // Convert back to integer and update player's bank money
          APlayerData[playerid][BankMoney] = floatround(CurrentAmount, floatround_floor);
          NewBankMoney = APlayerData[playerid][BankMoney];
          
          // Save the CurrentInterestTime for this bank account
          APlayerData[playerid][LastInterestTime] = CurrentInterestTime;
          
          // Only show message and save if interest was actually earned
          if (NewBankMoney > OldBankMoney)
          {
              // Inform the player that he received interest on his bank account and how much
              format(Message, sizeof(Message), "{00BBFF}You've received {FFBB00}$%i{00BBFF} interest on your bank account over %d period(s)", NewBankMoney - OldBankMoney, PeriodsToCalculate);
              SendClientMessage(playerid, COLOR_WHITE, Message);
              
              // Update the bank data after interest calculation
              BankFile_Save(playerid);
          }
      }
      
      // Check for any pending transfers to this player
      mysql_format(Database, query, sizeof(query), "SELECT * FROM `bank_transfers` WHERE `ReceiverName` = '%e' AND `IsProcessed` = 0", Name);
      new Cache:transferResult = mysql_query(Database, query);
      
      new rows = cache_num_rows();
      if (rows > 0) {
          new TransferAmount, SenderName[24], TransferTimeStamp, TransferTime[24], Day, Month, Year, Hour, Minute;
          
          for (new i = 0; i < rows; i++) {
              cache_get_value_name_int(i, "TransferAmount", TransferAmount);
              cache_get_value_name(i, "SenderName", SenderName, sizeof(SenderName));
              cache_get_value_name_int(i, "TransferTime", TransferTimeStamp);
              
              // Format the transfer time
              getdate(Year, Month, Day);
              gettime(Hour, Minute);
              format(TransferTime, sizeof(TransferTime), "%02d/%02d/%04d %02d:%02d", Day, Month, Year, Hour, Minute);
              
              // Add the money to the player's bank account
              APlayerData[playerid][BankMoney] += TransferAmount;
              
              // Inform the player about the transfer
              format(Message, sizeof(Message), "{00BBFF}Player {FFBB00}%s{00BBFF} has transferred {FFBB00}$%i{00BBFF}", SenderName, TransferAmount);
              SendClientMessage(playerid, COLOR_WHITE, Message);
              format(Message, sizeof(Message), "{00BBFF}to your bank account on {FFBB00}%s", TransferTime);
              SendClientMessage(playerid, COLOR_WHITE, Message);
          }
          
          // Mark all transfers as processed
          mysql_format(Database, query, sizeof(query), "UPDATE `bank_transfers` SET `IsProcessed` = 1 WHERE `ReceiverName` = '%e' AND `IsProcessed` = 0", Name);
          mysql_query(Database, query);
          
          // Update the bank account with the new money
          BankFile_Save(playerid);
      }
      
      cache_delete(transferResult);
      cache_delete(result);
      return 1;
  } else {
      // Send a message if the player doesn't have a bank account
      SendClientMessage(playerid, COLOR_DODGERBLUE, "You don't have a bank-account. Use {FFBB00}/bank{00BBFF} to set it up");
      cache_delete(result);
      return 0;
  }
}

// This function will save the player's bank account
BankFile_Save(playerid)
{
  // Setup local variables
  new Name[24], query[256];
  
  GetPlayerName(playerid, Name, sizeof(Name));
  
  // Check if the player already has a bank account
  mysql_format(Database, query, sizeof(query), "SELECT * FROM `banks` WHERE `PlayerName` = '%e' LIMIT 1", Name);
  new Cache:result = mysql_query(Database, query);
  
  if (CheckMySQLError("BankFile_Save - SELECT"))
  {
      cache_delete(result);
      return 0;
  }
  
  if (cache_num_rows() > 0) {
      // Update existing bank account
      mysql_format(Database, query, sizeof(query), "UPDATE `banks` SET `BankPassword` = '%e', `BankMoney` = %d, `LastInterestTime` = %d WHERE `PlayerName` = '%e'",
          APlayerData[playerid][BankPassword],
          APlayerData[playerid][BankMoney],
          APlayerData[playerid][LastInterestTime],
          Name
      );
      mysql_query(Database, query);
      
      if (CheckMySQLError("BankFile_Save - UPDATE"))
      {
          cache_delete(result);
          return 0;
      }
  } else {
      // Create a new bank account
      mysql_format(Database, query, sizeof(query), "INSERT INTO `banks` (`PlayerName`, `BankPassword`, `BankMoney`, `LastInterestTime`) VALUES ('%e', '%e', %d, %d)",
          Name,
          APlayerData[playerid][BankPassword],
          APlayerData[playerid][BankMoney],
          APlayerData[playerid][LastInterestTime]
      );
      mysql_query(Database, query);
      
      if (CheckMySQLError("BankFile_Save - INSERT"))
      {
          cache_delete(result);
          return 0;
      }
  }
  
  cache_delete(result);
  return 1;
}

// This function loads the file that holds the current interest-time
InterestTime_Load()
{
  // Setup local variables
  new query[128];
  
  // Query the database for the current interest time
  mysql_format(Database, query, sizeof(query), "SELECT * FROM `settings` WHERE `setting_name` = 'CurrentInterestTime' LIMIT 1");
  new Cache:result = mysql_query(Database, query);
  
  if (cache_num_rows() > 0)
  {
      cache_get_value_name_int(0, "setting_value", CurrentInterestTime);
      cache_delete(result);
      return 1;
  }
  else
  {
      // Create a default entry if not found
      mysql_format(Database, query, sizeof(query), "INSERT INTO `settings` (`setting_name`, `setting_value`) VALUES ('CurrentInterestTime', 0)");
      mysql_query(Database, query);
      CurrentInterestTime = 0;
      cache_delete(result);
      return 0;
  }
}

// This function saves the file that holds the current interest-time
InterestTime_Save()
{
  new query[128];
  
  // Update the current interest time in the database
  mysql_format(Database, query, sizeof(query), "UPDATE `settings` SET `setting_value` = %d WHERE `setting_name` = 'CurrentInterestTime'", CurrentInterestTime);
  mysql_query(Database, query);
  
  return 1;
}

// Function to check MySQL errors and log them
stock CheckMySQLError(const function_name[])
{
  new error[256];
  mysql_errno(Database);
  
  if (mysql_errno(Database) != 0)
  {
      mysql_error(error, sizeof(error), Database);
      printf("[MYSQL ERROR] in %s: Error %d - %s", function_name, mysql_errno(Database), error);
      return 1;
  }
  return 0;
}