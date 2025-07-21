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

	// Also save the houses that this player owns
	for (new i; i < MAX_HOUSESPERPLAYER; i++)
	{
		// Check if there is a valid house-id in this slot
		if (APlayerData[playerid][Houses][i] != 0)
		{
		    // Save the house (and linked vehicles)
		    HouseFile_Save(APlayerData[playerid][Houses][i]);
		}
	}
	// Save all the valid business-id's
	for (new i; i < MAX_BUSINESSPERPLAYER; i++)
	{
		// Check if there is a valid business-id in this slot
		if (APlayerData[playerid][Business][i] != 0)
		{
		    // Save the business
		    BusinessFile_Save(APlayerData[playerid][Business][i]);
		}
	}

	// Save the player's data to the database
    SavePlayerData(playerid);

	return 1;
}


// This function loads all houses
Housing_LoadAll()
{
	// Loop through all houses and try to load them
	for (new HouseID = 1; HouseID < MAX_HOUSES; HouseID++)
	{
	    // Try to load the file and check if it was succesfully loaded (file exists)
		if (HouseFile_Load(HouseID) == 1)
		{
			// Add a pickup and 3DText for this house
			House_CreateEntrance(HouseID);
			// Count the amount of houses that are loaded
		    TotalHouses++;
		}
	}

	return 1;
}

// This function will load the house's datafile (used when the server is started to load all houses)
HouseFile_Load(HouseID, bool:OnlyLoadVehicles = false)
{
	new file[100], File:HFile, LineFromFile[100], ParameterName[50], ParameterValue[50];
	// Setup local variables
	new cModel, cPaint, components[14], Float:cx, Float:cy, Float:cz, Float:crot, Col1, Col2, vid, bool:VehicleClamped, cFuel = -1, Neons;

	format(file, sizeof(file), HouseFile, HouseID); // Construct the complete filename for this house-file

	if (fexist(file))
	{
		HFile = fopen(file, io_read); // Open the housefile for reading

		fread(HFile, LineFromFile); // Read the first line of the file

		// Set the house so it cannot be entered by anyone, except the owner (close the house)
		AHouseData[HouseID][HouseOpened] = false;

		// Keep reading until the end of the file is found (no more data)
		while (strlen(LineFromFile) > 0)
		{
			StripNewLine(LineFromFile); // Strip any newline characters from the LineFromFile
			sscanf(LineFromFile, "s[50]s[50]", ParameterName, ParameterValue); // Extract parametername and parametervalue

			// If OnlyLoadVehicles is "false", only load the house-data
			// If OnlyLoadVehicles is "true", only load the vehicle-data
			if (OnlyLoadVehicles == false)
			{
				// Store the proper value in the proper place
				if (strcmp(ParameterName, "HouseName", false) == 0) // If the parametername is correct ("HouseName")
				    format(AHouseData[HouseID][HouseName], 24, ParameterValue); // Store the HouseName
				if (strcmp(ParameterName, "HouseX", false) == 0) // If the parametername is correct ("HouseX")
					AHouseData[HouseID][HouseX] = floatstr(ParameterValue); // Store the HouseX
				if (strcmp(ParameterName, "HouseY", false) == 0) // If the parametername is correct ("HouseY")
					AHouseData[HouseID][HouseY] = floatstr(ParameterValue); // Store the HouseY
				if (strcmp(ParameterName, "HouseZ", false) == 0) // If the parametername is correct ("HouseZ")
					AHouseData[HouseID][HouseZ] = floatstr(ParameterValue); // Store the HouseZ
				if (strcmp(ParameterName, "HouseLevel", false) == 0) // If the parametername is correct ("HouseLevel")
					AHouseData[HouseID][HouseLevel] = strval(ParameterValue); // Store the HouseLevel
				if (strcmp(ParameterName, "HouseMaxLevel", false) == 0) // If the parametername is correct ("HouseMaxLevel")
					AHouseData[HouseID][HouseMaxLevel] = strval(ParameterValue); // Store the HouseMaxLevel
				if (strcmp(ParameterName, "HousePrice", false) == 0) // If the parametername is correct ("HousePrice")
					AHouseData[HouseID][HousePrice] = strval(ParameterValue); // Store the HousePrice
				if (strcmp(ParameterName, "Owned", false) == 0) // If the parametername is correct ("Owned")
				{
				    if (strcmp(ParameterValue, "Yes", false) == 0) // If the value "Yes" was read
						AHouseData[HouseID][Owned] = true; // House is owned
					else
						AHouseData[HouseID][Owned] = false; // House is not owned
				}
				if (strcmp(ParameterName, "Owner", false) == 0) // If the parametername is correct ("Owner")
					// Store the Owner (Owner will hold "1" if there is no owner (empty string), done by "sscanf" I guess)
					// But this doesn't matter, as the owner will never be displayed when the house is not owned by someone
				    format(AHouseData[HouseID][Owner], 24, ParameterValue);
				if (strcmp(ParameterName, "Insurance", false) == 0) // If the parametername is correct ("Insurance")
					AHouseData[HouseID][Insurance] = strval(ParameterValue); // Store the Insurance
				if (strcmp(ParameterName, "AutoEvictDays", false) == 0) // If the parametername is correct ("AutoEvictDays")
					AHouseData[HouseID][AutoEvictDays] = strval(ParameterValue); // Store the AutoEvictDays
			}
			else // OnlyLoadVehicles is "true", so only read the vehicle-data
			{
				if (strcmp(ParameterName, "[Vehicle]", false) == 0) // If the parametername is correct ("[Vehicle]")
				{
				    // Clear all data to start a new vehicle
				    cModel = 0;
				    cPaint = 0;
				    cFuel = -1;
				    Neons = 0;
				    for (new i; i < 14; i++)
				        components[i] = 0;
				}
				if (strcmp(ParameterName, "VehicleModel", false) == 0) // If the parametername is correct ("VehicleModel")
					cModel = strval(ParameterValue); // Store the VehicleModel
				if (strcmp(ParameterName, "Fuel", false) == 0) // If the parametername is correct ("Fuel")
					cFuel = strval(ParameterValue); // Store the Fuel
				if (strcmp(ParameterName, "VehiclePaintJob", false) == 0) // If the parametername is correct ("VehiclePaintJob")
					cPaint = strval(ParameterValue); // Store the VehiclePaintJob
				if (strcmp(ParameterName, "VehicleSpoiler", false) == 0) // If the parametername is correct ("VehicleSpoiler")
					components[0] = strval(ParameterValue); // Store the VehicleSpoiler
				if (strcmp(ParameterName, "VehicleHood", false) == 0) // If the parametername is correct ("VehicleHood")
					components[1] = strval(ParameterValue); // Store the VehicleHood
				if (strcmp(ParameterName, "VehicleRoof", false) == 0) // If the parametername is correct ("VehicleRoof")
					components[2] = strval(ParameterValue); // Store the VehicleRoof
				if (strcmp(ParameterName, "VehicleSideSkirt", false) == 0) // If the parametername is correct ("VehicleSideSkirt")
					components[3] = strval(ParameterValue); // Store the VehicleSideSkirt
				if (strcmp(ParameterName, "VehicleLamps", false) == 0) // If the parametername is correct ("VehicleLamps")
					components[4] = strval(ParameterValue); // Store the VehicleLamps
				if (strcmp(ParameterName, "VehicleNitro", false) == 0) // If the parametername is correct ("VehicleNitro")
					components[5] = strval(ParameterValue); // Store the VehicleNitro
				if (strcmp(ParameterName, "VehicleExhaust", false) == 0) // If the parametername is correct ("VehicleExhaust")
					components[6] = strval(ParameterValue); // Store the VehicleExhaust
				if (strcmp(ParameterName, "VehicleWheels", false) == 0) // If the parametername is correct ("VehicleWheels")
					components[7] = strval(ParameterValue); // Store the VehicleWheels
				if (strcmp(ParameterName, "VehicleStereo", false) == 0) // If the parametername is correct ("VehicleStereo")
					components[8] = strval(ParameterValue); // Store the VehicleStereo
				if (strcmp(ParameterName, "VehicleHydraulics", false) == 0) // If the parametername is correct ("VehicleHydraulics")
					components[9] = strval(ParameterValue); // Store the VehicleHydraulics
				if (strcmp(ParameterName, "VehicleFrontBumper", false) == 0) // If the parametername is correct ("VehicleFrontBumper")
					components[10] = strval(ParameterValue); // Store the VehicleFrontBumper
				if (strcmp(ParameterName, "VehicleRearBumper", false) == 0) // If the parametername is correct ("VehicleRearBumper")
					components[11] = strval(ParameterValue); // Store the VehicleRearBumper
				if (strcmp(ParameterName, "VehicleVentRight", false) == 0) // If the parametername is correct ("VehicleVentRight")
					components[12] = strval(ParameterValue); // Store the VehicleVentRight
				if (strcmp(ParameterName, "VehicleVentLeft", false) == 0) // If the parametername is correct ("VehicleVentLeft")
					components[13] = strval(ParameterValue); // Store the VehicleVentLeft

				if (strcmp(ParameterName, "Color1", false) == 0) // If the parametername is correct ("Color1")
					Col1 = strval(ParameterValue); // Store the Color1
				if (strcmp(ParameterName, "Color2", false) == 0) // If the parametername is correct ("Color2")
					Col2 = strval(ParameterValue); // Store the Color2

				if (strcmp(ParameterName, "VehicleX", false) == 0) // If the parametername is correct ("VehicleX")
					cx = floatstr(ParameterValue); // Store the VehicleX
				if (strcmp(ParameterName, "VehicleY", false) == 0) // If the parametername is correct ("VehicleY")
					cy = floatstr(ParameterValue); // Store the VehicleY
				if (strcmp(ParameterName, "VehicleZ", false) == 0) // If the parametername is correct ("VehicleZ")
					cz = floatstr(ParameterValue); // Store the VehicleZ
				if (strcmp(ParameterName, "VehicleAngle", false) == 0) // If the parametername is correct ("VehicleAngle")
					crot = floatstr(ParameterValue); // Store the VehicleAngle
				if (strcmp(ParameterName, "NeonsApplied", false) == 0) // If the parametername is correct ("NeonsApplied")
					Neons = strval(ParameterValue); // Store the NeonsApplied

				if (strcmp(ParameterName, "Clamped", false) == 0) // If the parametername is correct ("Clamped")
				{
				    if (strcmp(ParameterValue, "Yes", false) == 0) // If the value "Yes" was read
						VehicleClamped = true; // Vehicle is clamped
					else
						VehicleClamped = false; // Vehicle is not clamped
				}

				if (strcmp(ParameterName, "[/Vehicle]", false) == 0) // If the parametername is correct ("[/Vehicle]")
				{
					// Set both colors to 1 if they are 0 AND if there is a paintjob applied
					if ((Col1 == 0) && (cPaint != 0))
					    Col1 = 1;
					if ((Col2 == 0) && (cPaint != 0))
					    Col2 = 1;

					// The "[/Vehicle]" is found, this means that all data about this vehicle is now stored in the variables
					// Now add the vehicle to the house and set it's data
					vid = House_AddVehicle(HouseID, cModel, cPaint, components, cx, cy, cz, crot, Col1, Col2);
					AVehicleData[vid][Clamped] = VehicleClamped;
					// Apply the neons if they have been loaded
					if (Neons != 0)
					{
						AVehicleData[vid][NeonObjectModel] = Neons;
					    AVehicleData[vid][NeonLeft] = CreateObject(AVehicleData[vid][NeonObjectModel],0,0,0,0,0,0);
					    AVehicleData[vid][NeonRight] = CreateObject(AVehicleData[vid][NeonObjectModel],0,0,0,0,0,0);
						AttachObjectToVehicle(AVehicleData[vid][NeonLeft], vid, -0.8, 0.0, -0.70, 0.0, 0.0, 0.0);
						AttachObjectToVehicle(AVehicleData[vid][NeonRight], vid, 0.8, 0.0, -0.70, 0.0, 0.0, 0.0);
					}
					// Also set the fuel (set it to maximum when the fuel parameter wasn't inside the file)
					if (cFuel == -1)
						AVehicleData[vid][Fuel] = MaxFuel;
					else // If the parameter was there, store it
						AVehicleData[vid][Fuel] = cFuel;
				}
			}

            // Read the next line of the file
			fread(HFile, LineFromFile);
		}

		// Check if the maximum house-level isn't 0 (when updating to the new version)
		if (AHouseData[HouseID][HouseMaxLevel] == 0)
		    AHouseData[HouseID][HouseMaxLevel] = 10; // Set the maximum level to 10

        // Close the file
		fclose(HFile);

        // Return if the file was read correctly
		return 1;
	}
	else
	    return 0; // Return 0 if the file couldn't be read (doesn't exist)
}

// This function will save the given house
HouseFile_Save(HouseID)
{
	new file[100], File:HFile, LineForFile[100], vid;

    // Construct the complete filename for this house
	format(file, sizeof(file), HouseFile, HouseID);

	HFile = fopen(file, io_write); // Open the playerfile for writing

	format(LineForFile, sizeof(LineForFile), "HouseName %s\r\n", AHouseData[HouseID][HouseName]); // Construct the line: "HouseName <HouseName>"
	fwrite(HFile, LineForFile); // And save it to the file

	format(LineForFile, sizeof(LineForFile), "HouseX %f\r\n", AHouseData[HouseID][HouseX]); // Construct the line: "HouseX <HouseX>"
	fwrite(HFile, LineForFile); // And save it to the file

	format(LineForFile, sizeof(LineForFile), "HouseY %f\r\n", AHouseData[HouseID][HouseY]); // Construct the line: "HouseY <HouseY>"
	fwrite(HFile, LineForFile); // And save it to the file

	format(LineForFile, sizeof(LineForFile), "HouseZ %f\r\n", AHouseData[HouseID][HouseZ]); // Construct the line: "HouseZ <HouseZ>"
	fwrite(HFile, LineForFile); // And save it to the file

	format(LineForFile, sizeof(LineForFile), "HouseLevel %i\r\n", AHouseData[HouseID][HouseLevel]); // Construct the line: "HouseLevel <HouseLevel>"
	fwrite(HFile, LineForFile); // And save it to the file

	format(LineForFile, sizeof(LineForFile), "HouseMaxLevel %i\r\n", AHouseData[HouseID][HouseMaxLevel]); // Construct the line: "HouseMaxLevel <HouseMaxLevel>"
	fwrite(HFile, LineForFile); // And save it to the file

	format(LineForFile, sizeof(LineForFile), "HousePrice %i\r\n", AHouseData[HouseID][HousePrice]); // Construct the line: "HousePrice <HousePrice>"
	fwrite(HFile, LineForFile); // And save it to the file

	if (AHouseData[HouseID][Owned] == true) // Check if the house is owned
	{
		format(LineForFile, sizeof(LineForFile), "Owned Yes\r\n"); // Construct the line: "Owned Yes"
		fwrite(HFile, LineForFile); // And save it to the file
	}
	else
	{
		format(LineForFile, sizeof(LineForFile), "Owned No\r\n"); // Construct the line: "Owned No"
		fwrite(HFile, LineForFile); // And save it to the file
	}

	format(LineForFile, sizeof(LineForFile), "Owner %s\r\n", AHouseData[HouseID][Owner]); // Construct the line: "Owner <Owner>"
	fwrite(HFile, LineForFile); // And save it to the file

	format(LineForFile, sizeof(LineForFile), "Insurance %i\r\n", AHouseData[HouseID][Insurance]); // Construct the line: "Insurance <Insurance>"
	fwrite(HFile, LineForFile); // And save it to the file

	AHouseData[HouseID][AutoEvictDays] = AutoEvict[AEDays]; // Update the AutoEvictDays for this house when the player logs out
	format(LineForFile, sizeof(LineForFile), "AutoEvictDays %i\r\n", AutoEvict[AEDays]); // Construct the line: "AutoEvictDays <AutoEvictDays>"
	fwrite(HFile, LineForFile); // And save it to the file



	// Save the vehicle-data for every vehicle added to the house
	for (new CarSlot; CarSlot < 10; CarSlot++)
	{
	    // If a valid vehicle-id has been found
		if (AHouseData[HouseID][VehicleIDs][CarSlot] != 0)
		{
			// Get the vehicle id
			vid = AHouseData[HouseID][VehicleIDs][CarSlot];

		    format(LineForFile, sizeof(LineForFile), "[Vehicle]\r\n"); // Construct the line: "[Vehicle]"
			fwrite(HFile, LineForFile); // And save it to the file

		    format(LineForFile, sizeof(LineForFile), "VehicleModel %i\r\n", AVehicleData[vid][Model]); // Construct the line: "VehicleModel <VehicleModel>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "Fuel %i\r\n", AVehicleData[vid][Fuel]); // Construct the line: "Fuel <Fuel>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehiclePaintJob %i\r\n", AVehicleData[vid][PaintJob]); // Construct the line: "VehiclePaintJob <VehiclePaintJob>"
			fwrite(HFile, LineForFile); // And save it to the file

		    format(LineForFile, sizeof(LineForFile), "VehicleSpoiler %i\r\n", AVehicleData[vid][Components][0]); // Construct the line: "VehicleSpoiler <VehicleSpoiler>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleHood %i\r\n", AVehicleData[vid][Components][1]); // Construct the line: "VehicleHood <VehicleHood>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleRoof %i\r\n", AVehicleData[vid][Components][2]); // Construct the line: "VehicleRoof <VehicleRoof>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleSideSkirt %i\r\n", AVehicleData[vid][Components][3]); // Construct the line: "VehicleSideSkirt <VehicleSideSkirt>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleLamps %i\r\n", AVehicleData[vid][Components][4]); // Construct the line: "VehicleLamps <VehicleLamps>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleNitro %i\r\n", AVehicleData[vid][Components][5]); // Construct the line: "VehicleNitro <VehicleNitro>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleExhaust %i\r\n", AVehicleData[vid][Components][6]); // Construct the line: "VehicleSpoiler <VehicleSpoiler>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleWheels %i\r\n", AVehicleData[vid][Components][7]); // Construct the line: "VehicleWheels <VehicleWheels>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleStereo %i\r\n", AVehicleData[vid][Components][8]); // Construct the line: "VehicleStereo <VehicleStereo>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleHydraulics %i\r\n", AVehicleData[vid][Components][9]); // Construct the line: "VehicleHydraulics <VehicleHydraulics>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleFrontBumper %i\r\n", AVehicleData[vid][Components][10]); // Construct the line: "VehicleFrontBumper <VehicleFrontBumper>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleRearBumper %i\r\n", AVehicleData[vid][Components][11]); // Construct the line: "VehicleRearBumper <VehicleRearBumper>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleVentRight %i\r\n", AVehicleData[vid][Components][12]); // Construct the line: "VehicleVentRight <VehicleVentRight>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleVentLeft %i\r\n", AVehicleData[vid][Components][13]); // Construct the line: "VehicleVentLeft <VehicleVentLeft>"
			fwrite(HFile, LineForFile); // And save it to the file

		    format(LineForFile, sizeof(LineForFile), "Color1 %i\r\n", AVehicleData[vid][Color1]); // Construct the line: "Color1 <Color1>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "Color2 %i\r\n", AVehicleData[vid][Color2]); // Construct the line: "Color2 <Color2>"
			fwrite(HFile, LineForFile); // And save it to the file

		    format(LineForFile, sizeof(LineForFile), "VehicleX %f\r\n", AVehicleData[vid][SpawnX]); // Construct the line: "VehicleVentLeft <VehicleVentLeft>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleY %f\r\n", AVehicleData[vid][SpawnY]); // Construct the line: "VehicleVentLeft <VehicleVentLeft>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleZ %f\r\n", AVehicleData[vid][SpawnZ]); // Construct the line: "VehicleVentLeft <VehicleVentLeft>"
			fwrite(HFile, LineForFile); // And save it to the file
		    format(LineForFile, sizeof(LineForFile), "VehicleAngle %f\r\n", AVehicleData[vid][SpawnRot]); // Construct the line: "VehicleVentLeft <VehicleVentLeft>"
			fwrite(HFile, LineForFile); // And save it to the file
			format(LineForFile, sizeof(LineForFile), "NeonsApplied %i\r\n", AVehicleData[vid][NeonObjectModel]); // Construct the line: "NeonsApplied <NeonsApplied>"
			fwrite(HFile, LineForFile); // And save it to the file
			
			if (AVehicleData[vid][Clamped] == true)
			    format(LineForFile, sizeof(LineForFile), "Clamped Yes\r\n"); // Construct the line: "Clamped <Yes>"
			else
			    format(LineForFile, sizeof(LineForFile), "Clamped No\r\n"); // Construct the line: "Clamped <No>"

			fwrite(HFile, LineForFile); // And save it to the file

		    format(LineForFile, sizeof(LineForFile), "[/Vehicle]\r\n"); // Construct the line: "[/Vehicle]"
			fwrite(HFile, LineForFile); // And save it to the file
		}
	}

	fclose(HFile); // Close the file

	return 1;
}



// This function loads all speedcameras
Camera_LoadAll()
{
	// Loop through all cameras and try to load them
	for (new CamID; CamID < MAX_CAMERAS; CamID++)
	{
	    // Try to load the file and check if it was succesfully loaded (file exists)
		if (CameraFile_Load(CamID) == 1)
		{
			// Save the number of camera's that have been loaded, so it can be displayed
		    TotalCameras++;
		}
	}

	return 1;
}



// This function will load the speedcamera's datafile (used when the server is started to load all cameras)
CameraFile_Load(CamID)
{
	// Setup local variables
	new file[100], File:CFile, LineFromFile[100], ParameterName[50], ParameterValue[50];
	new Float:x, Float:y, Float:z, Float:rot, MaxSpeed;

	format(file, sizeof(file), CameraFile, CamID); // Construct the complete filename for this camera-file

	if (fexist(file))
	{
		CFile = fopen(file, io_read); // Open the camerafile for reading

		fread(CFile, LineFromFile); // Read the first line of the file

		// Keep reading until the end of the file is found (no more data)
		while (strlen(LineFromFile) > 0)
		{
			StripNewLine(LineFromFile); // Strip any newline characters from the LineFromFile
			sscanf(LineFromFile, "s[50]s[50]", ParameterName, ParameterValue); // Extract parametername and parametervalue

			// Store the proper value in the proper place
			if (strcmp(ParameterName, "CamX", false) == 0) // If the parametername is correct ("CamX")
			    x = floatstr(ParameterValue); // Store the CamX
			if (strcmp(ParameterName, "CamY", false) == 0) // If the parametername is correct ("CamY")
			    y = floatstr(ParameterValue); // Store the CamY
			if (strcmp(ParameterName, "CamZ", false) == 0) // If the parametername is correct ("CamZ")
			    z = floatstr(ParameterValue); // Store the CamZ
			if (strcmp(ParameterName, "CamAngle", false) == 0) // If the parametername is correct ("CamAngle")
			    rot = floatstr(ParameterValue); // Store the CamAngle
			if (strcmp(ParameterName, "CamSpeed", false) == 0) // If the parametername is correct ("CamSpeed")
			    MaxSpeed = strval(ParameterValue); // Store the CamSpeed

			fread(CFile, LineFromFile); // Read the next line of the file
		}

		fclose(CFile); // Close the file

		// All data has been read about the camera, now setup the camera
		SetupSpeedCamera(CamID, x, y, z, rot, MaxSpeed);

		return 1; // Return that the file was read correctly
	}
	else
	    return 0; // Return 0 if the file couldn't be read (doesn't exist)
}



// This function loads all businesses
Business_LoadAll()
{
	// First load the business-time file, this one holds the current business-time which is increased every minute the server if online
	BusinessTime_Load();

	// Loop through all businesses and try to load them
	for (new BusID = 1; BusID < MAX_BUSINESS; BusID++)
	{
	    // Try to load the file and check if it was succesfully loaded (file exists)
		if (BusinessFile_Load(BusID) == 1)
		{
			// Add a pickup and 3DText for this business
			Business_CreateEntrance(BusID);
			// Count the loaded businesses
		    TotalBusiness++;
		}
	}

	return 1;
}

// This function loads the file that holds the current business-time
BusinessTime_Load()
{
	// Setup local variables
	new File:BFile, LineFromFile[100], ParameterName[50], ParameterValue[50];

	// Try to load the businesstime file
	if (fexist("ServerData/Business/BusinessTime.ini"))
	{
		BFile = fopen("ServerData/Business/BusinessTime.ini", io_read); // Open the businesstime-file for reading

		fread(BFile, LineFromFile); // Read the first line of the file

		// Keep reading until the end of the file is found (no more data)
		while (strlen(LineFromFile) > 0)
		{
			StripNewLine(LineFromFile); // Strip any newline characters from the LineFromFile
			sscanf(LineFromFile, "s[50]s[50]", ParameterName, ParameterValue); // Extract parametername and parametervalue

			// Store the proper value in the proper place
			if (strcmp(ParameterName, "BusinessTime", false) == 0) // If the parametername is correct ("BusinessTime")
				BusinessTransactionTime = strval(ParameterValue); // Store the BusinessTime

            // Read the next line of the file
			fread(BFile, LineFromFile);
		}

        // Close the file
		fclose(BFile);

        // Return if the file was read correctly
		return 1;
	}
	else
	    return 0; // Return 0 if the file couldn't be read (doesn't exist)
}

// This function saves the file that holds the current business-time
BusinessTime_Save()
{
	new File:BFile, LineForFile[100];

	BFile = fopen("ServerData/Business/BusinessTime.ini", io_write); // Open the businesstime-file for writing

	format(LineForFile, sizeof(LineForFile), "BusinessTime %i\r\n", BusinessTransactionTime); // Construct the line: "BusinessTime <BusinessTransactionTime>"
	fwrite(BFile, LineForFile); // And save it to the file

	fclose(BFile); // Close the file

	return 1;
}



// This function will load the business's datafile (used when the server is started to load all businesses)
BusinessFile_Load(BusID)
{
	// Setup local variables
	new file[100], File:BFile, LineFromFile[100], ParameterName[50], ParameterValue[50];

	format(file, sizeof(file), BusinessFile, BusID); // Construct the complete filename for this business-file

	if (fexist(file))
	{
		BFile = fopen(file, io_read); // Open the housefile for reading

		fread(BFile, LineFromFile); // Read the first line of the file

		// Keep reading until the end of the file is found (no more data)
		while (strlen(LineFromFile) > 0)
		{
			StripNewLine(LineFromFile); // Strip any newline characters from the LineFromFile
			sscanf(LineFromFile, "s[50]s[50]", ParameterName, ParameterValue); // Extract parametername and parametervalue

			// Store the proper value in the proper place
			if (strcmp(ParameterName, "BusinessName", false) == 0) // If the parametername is correct ("BusinessName")
			    format(ABusinessData[BusID][BusinessName], 24, ParameterValue); // Store the BusinessName
			if (strcmp(ParameterName, "BusinessX", false) == 0) // If the parametername is correct ("BusinessX")
				ABusinessData[BusID][BusinessX] = floatstr(ParameterValue); // Store the BusinessX
			if (strcmp(ParameterName, "BusinessY", false) == 0) // If the parametername is correct ("BusinessY")
				ABusinessData[BusID][BusinessY] = floatstr(ParameterValue); // Store the BusinessY
			if (strcmp(ParameterName, "BusinessZ", false) == 0) // If the parametername is correct ("BusinessZ")
				ABusinessData[BusID][BusinessZ] = floatstr(ParameterValue); // Store the BusinessZ
			if (strcmp(ParameterName, "BusinessType", false) == 0) // If the parametername is correct ("BusinessType")
				ABusinessData[BusID][BusinessType] = strval(ParameterValue); // Store the BusinessType
			if (strcmp(ParameterName, "BusinessLevel", false) == 0) // If the parametername is correct ("BusinessLevel")
				ABusinessData[BusID][BusinessLevel] = strval(ParameterValue); // Store the BusinessLevel
			if (strcmp(ParameterName, "LastTransaction", false) == 0) // If the parametername is correct ("LastTransaction")
				ABusinessData[BusID][LastTransaction] = strval(ParameterValue); // Store the LastTransaction
			if (strcmp(ParameterName, "Owned", false) == 0) // If the parametername is correct ("Owned")
			{
			    if (strcmp(ParameterValue, "Yes", false) == 0) // If the value "Yes" was read
					ABusinessData[BusID][Owned] = true; // House is owned
				else
					ABusinessData[BusID][Owned] = false; // House is not owned
			}
			if (strcmp(ParameterName, "Owner", false) == 0) // If the parametername is correct ("Owner")
			    format(ABusinessData[BusID][Owner], 24, ParameterValue);
			
			if (strcmp(ParameterName, "AutoEvictDays", false) == 0) // If the parametername is correct ("AutoEvictDays")
				ABusinessData[BusID][AutoEvictDays] = strval(ParameterValue); // Store the AutoEvictDays

            // Read the next line of the file
			fread(BFile, LineFromFile);
		}

        // Close the file
		fclose(BFile);

        // Return if the file was read correctly
		return 1;
	}
	else
	    return 0; // Return 0 if the file couldn't be read (doesn't exist)
}

// This function will save the given business
BusinessFile_Save(BusID)
{
	new file[100], File:BFile, LineForFile[100];

    // Construct the complete filename for this business
	format(file, sizeof(file), BusinessFile, BusID);

	BFile = fopen(file, io_write); // Open the business-file for writing

	format(LineForFile, sizeof(LineForFile), "BusinessName %s\r\n", ABusinessData[BusID][BusinessName]); // Construct the line: "BusinessName <BusinessName>"
	fwrite(BFile, LineForFile); // And save it to the file
	format(LineForFile, sizeof(LineForFile), "BusinessX %f\r\n", ABusinessData[BusID][BusinessX]); // Construct the line: "BusinessX <BusinessX>"
	fwrite(BFile, LineForFile); // And save it to the file
	format(LineForFile, sizeof(LineForFile), "BusinessY %f\r\n", ABusinessData[BusID][BusinessY]); // Construct the line: "BusinessY <BusinessY>"
	fwrite(BFile, LineForFile); // And save it to the file
	format(LineForFile, sizeof(LineForFile), "BusinessZ %f\r\n", ABusinessData[BusID][BusinessZ]); // Construct the line: "BusinessZ <BusinessZ>"
	fwrite(BFile, LineForFile); // And save it to the file
	format(LineForFile, sizeof(LineForFile), "BusinessType %i\r\n", ABusinessData[BusID][BusinessType]); // Construct the line: "BusinessType <BusinessType>"
	fwrite(BFile, LineForFile); // And save it to the file
	format(LineForFile, sizeof(LineForFile), "BusinessLevel %i\r\n", ABusinessData[BusID][BusinessLevel]); // Construct the line: "BusinessLevel <BusinessLevel>"
	fwrite(BFile, LineForFile); // And save it to the file
	format(LineForFile, sizeof(LineForFile), "LastTransaction %i\r\n", ABusinessData[BusID][LastTransaction]); // Construct the line: "LastTransaction <LastTransaction>"
	fwrite(BFile, LineForFile); // And save it to the file

	ABusinessData[BusID][AutoEvictDays] = AutoEvict[AEDays]; // Update the AutoEvictDays for this business when the player logs out
	format(LineForFile, sizeof(LineForFile), "AutoEvictDays %i\r\n", AutoEvict[AEDays]); // Construct the line: "AutoEvictDays <AutoEvictDays>"
	fwrite(BFile, LineForFile); // And save it to the file

	if (ABusinessData[BusID][Owned] == true) // Check if the house is owned
	{
		format(LineForFile, sizeof(LineForFile), "Owned Yes\r\n"); // Construct the line: "Owned Yes"
		fwrite(BFile, LineForFile); // And save it to the file
	}
	else
	{
		format(LineForFile, sizeof(LineForFile), "Owned No\r\n"); // Construct the line: "Owned No"
		fwrite(BFile, LineForFile); // And save it to the file
	}

	format(LineForFile, sizeof(LineForFile), "Owner %s\r\n", ABusinessData[BusID][Owner]); // Construct the line: "Owner <Owner>"
	fwrite(BFile, LineForFile); // And save it to the file

	fclose(BFile); // Close the file

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
    
    if (cache_num_rows() > 0) {
        // Update existing bank account
        mysql_format(Database, query, sizeof(query), "UPDATE `banks` SET `BankPassword` = '%e', `BankMoney` = %d, `LastInterestTime` = %d WHERE `PlayerName` = '%e'",
            APlayerData[playerid][BankPassword],
            APlayerData[playerid][BankMoney],
            APlayerData[playerid][LastInterestTime],
            Name
        );
    } else {
        // Create a new bank account
        mysql_format(Database, query, sizeof(query), "INSERT INTO `banks` (`PlayerName`, `BankPassword`, `BankMoney`, `LastInterestTime`) VALUES ('%e', '%e', %d, %d)",
            Name,
            APlayerData[playerid][BankPassword],
            APlayerData[playerid][BankMoney],
            APlayerData[playerid][LastInterestTime]
        );
    }
    
    mysql_query(Database, query);
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

