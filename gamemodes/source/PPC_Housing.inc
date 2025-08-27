// This function returns the first free house-slot for the given player
Player_GetFreeHouseSlot(playerid)
{
	// Check if the player has room for another house (he hasn't bought the maximum amount of houses per player yet)
	// and get the slot-id
	for (new HouseIndex; HouseIndex < MAX_HOUSESPERPLAYER; HouseIndex++) // Loop through all house-slots of the player
		if (APlayerData[playerid][Houses][HouseIndex] == 0) // Check if this house slot is free
		    return HouseIndex; // Return the free HouseIndex for this player

	// If there were no free house-slots, return "-1"
	return -1;
}

// This function returns the maximum number of car-slots, based on the house-level
House_GetMaxCarSlots(HouseID)
{
	// Return the maximum number of carslots, based on the house-level (every level has one carslot, so return the houselevel)
	return AHouseData[HouseID][HouseLevel];
}

// This function returns the first free carslot in the given house (or -1 if no free slot is found)
House_GetFreeCarSlot(HouseID)
{
	// Get the maximum number of carslots for this house (based on the house-level)
	new MaxCarSlots = House_GetMaxCarSlots(HouseID);

	// Get the maximum number of carslots for this house and make a loop through all carslots for this house
	for (new CarSlot; CarSlot < MaxCarSlots; CarSlot++)
	{
		// Check if the carslot is empty
		if (AHouseData[HouseID][VehicleIDs][CarSlot] == 0)
		    return CarSlot; // Return the carslot-id
	}

	// If no carslots are free, return -1
	return -1;
}

// This function sets ownership to the given player
House_SetOwner(playerid, HouseID)
{
	// Setup local variables
	new HouseSlotFree, Name[24], Msg[128];

	// Get the first free house-slot from this player
	HouseSlotFree = Player_GetFreeHouseSlot(playerid);

	// Check if the player has a free house-slot
	if (HouseSlotFree != -1)
	{
		// Get the player's name
		GetPlayerName(playerid, Name, sizeof(Name));

		// Store the house-id for the player
		APlayerData[playerid][Houses][HouseSlotFree] = HouseID;
		// Let the player pay for the house
		RewardPlayer(playerid, -AHouseData[HouseID][HousePrice], 0);

		// Set the house as owned
		AHouseData[HouseID][Owned] = true;
		// Store the owner-name for the house
		format(AHouseData[HouseID][Owner], 24, Name);
		// Set the level to 1
		AHouseData[HouseID][HouseLevel] = 1;
		// Set the default house-name ("<playername>'s house")
		format(AHouseData[HouseID][HouseName], 100, TXT_DefaultHouseName, Name);
		// Update AutoEvictDays to current days
		AHouseData[HouseID][AutoEvictDays] = AutoEvict[AEDays];

		// Also, update the pickup and map-icon for this house
		House_UpdateEntrance(HouseID);

		// Save the house to MySQL database
		House_Save(HouseID);
		
		// Save player data to MySQL database
		SavePlayerData(playerid);

		// Let the player know he bought the house
		format(Msg, sizeof(Msg), TXT_PlayerBoughtHouse, AHouseData[HouseID][HousePrice]);
		SendClientMessage(playerid, COLOR_WHITE, Msg);
	}
	else
	    SendClientMessage(playerid, COLOR_WHITE, TXT_PlayerOwnsMaxHouses);

	return true;
}



// This function is used to spawn back at the entrance of your house
House_Exit(playerid, HouseID)
{
	// Set the player in the normal world again
	SetPlayerVirtualWorld(playerid, 0);
	SetPlayerInterior(playerid, 0);
	// Set the position of the player at the entrance of his house
	SetPlayerPos(playerid, AHouseData[HouseID][HouseX], AHouseData[HouseID][HouseY], AHouseData[HouseID][HouseZ]);
	// Also clear the tracking-variable to track in which house the player is
	APlayerData[playerid][CurrentHouse] = 0;

	// Check if there is a timer-value set for exiting the house (this timer freezes the player while the environment is being loaded)
	if (ExitHouseTimer > 0)
	{
		// Don't allow the player to fall
	    TogglePlayerControllable(playerid, false);
		// Let the player know he's frozen for 5 seconds
		GameTextForPlayer(playerid, TXT_ExitHouseReloadEnv, ExitHouseTimer, 4);
		// Start a timer that will allow the player to fall again when the environment has loaded
		SetTimerEx("House_ExitTimer", ExitHouseTimer, false, "ii", playerid, HouseID);
	}

	return true;
}

forward House_ExitTimer(playerid, HouseID);
public House_ExitTimer(playerid, HouseID)
{
	// Allow the player to move again (environment should have been loaded now)
    TogglePlayerControllable(playerid, true);

	// Respawn the player's vehicles near the house (only the vehicles that belong to this house)
	for (new CarSlot; CarSlot < 10; CarSlot++)
		if (AHouseData[HouseID][VehicleIDs][CarSlot] != 0)
		    SetVehicleToRespawn(AHouseData[HouseID][VehicleIDs][CarSlot]);

	return true;
}



// This function adds a pickup for the given house
House_CreateEntrance(HouseID)
{
	// Setup local variables
	new Msg[128], Float:x, Float:y, Float:z;

	// Get the coordinates of the house's pickup (usually near the door)
	x = AHouseData[HouseID][HouseX];
	y = AHouseData[HouseID][HouseY];
	z = AHouseData[HouseID][HouseZ];

	// Add a new pickup at the house's location (usually near the door), green = free, blue = owned
	if (AHouseData[HouseID][Owned] == true)
	{
		// Create a blue house-pickup (house is owned)
 		AHouseData[HouseID][PickupID] = CreateDynamicPickup(1272, 1, x, y, z, 0);
		// Create the 3DText that appears above the house-pickup (displays the housename and the name of the owner)
		format(Msg, sizeof(Msg), TXT_PickupHouseOwned, AHouseData[HouseID][HouseName], AHouseData[HouseID][Owner], AHouseData[HouseID][HouseLevel]);
		AHouseData[HouseID][DoorText] = CreateDynamic3DTextLabel(Msg, COLOR_TEAL, x, y, z + 1.0, 50.0);
		// Add a streamed icon to the map (red house), type = 32, color = 0, world = 0, interior = 0, playerid = -1, drawdist = 150.0
		if (ShowBoughtHouses == true)
			AHouseData[HouseID][MapIconID] = CreateDynamicMapIcon(x, y, z, 32, 0, 0, 0, -1, 150.0);
	}
	else
	{
        // Create a green house-pickup (house is free)
		AHouseData[HouseID][PickupID] = CreateDynamicPickup(1273, 1, x, y, z, 0);
		// Create the 3DText that appears above the house-pickup (displays the price of the house)
		format(Msg, sizeof(Msg), TXT_PickupHouseForSale, HouseID, AHouseData[HouseID][HousePrice], AHouseData[HouseID][HouseMaxLevel]);
		AHouseData[HouseID][DoorText] = CreateDynamic3DTextLabel(Msg, COLOR_TEAL, x, y, z + 1.0, 50.0);
		// Add a streamed icon to the map (green house), type = 31, color = 0, world = 0, interior = 0, playerid = -1, drawdist = 150.0
		AHouseData[HouseID][MapIconID] = CreateDynamicMapIcon(x, y, z, 31, 0, 0, 0, -1, 150.0);
	}
}

// This function changes the pickup (and map-icon) for the given house (used when buying or selling a house)
House_UpdateEntrance(HouseID)
{
	// Setup local variables
	new Msg[128], Float:x, Float:y, Float:z;

	// Get the coordinates of the house's pickup (usually near the door)
	x = AHouseData[HouseID][HouseX];
	y = AHouseData[HouseID][HouseY];
	z = AHouseData[HouseID][HouseZ];

	// Destroy the pickup and map-icon near the house's entrance
	DestroyDynamicPickup(AHouseData[HouseID][PickupID]);
	DestroyDynamicMapIcon(AHouseData[HouseID][MapIconID]);

	// Add a new pickup at the house's location (usually near the door), green = free, blue = owned
	if (AHouseData[HouseID][Owned] == true)
	{
		// Create a blue house-pickup (house is owned)
 		AHouseData[HouseID][PickupID] = CreateDynamicPickup(1272, 1, x, y, z, 0);
		// Update the 3DText that appears above the house-pickup (displays the housename and the name of the owner)
		format(Msg, sizeof(Msg), TXT_PickupHouseOwned, AHouseData[HouseID][HouseName], AHouseData[HouseID][Owner], AHouseData[HouseID][HouseLevel]);
		UpdateDynamic3DTextLabelText(AHouseData[HouseID][DoorText], COLOR_TEAL, Msg);
		// Add a streamed icon to the map (red house), type = 32, color = 0, world = 0, interior = 0, playerid = -1, drawdist = 150.0
		if (ShowBoughtHouses == true)
			AHouseData[HouseID][MapIconID] = CreateDynamicMapIcon(x, y, z, 32, 0, 0, 0, -1, 150.0);
	}
	else
	{
        // Create a green house-pickup (house is free)
		AHouseData[HouseID][PickupID] = CreateDynamicPickup(1273, 1, x, y, z, 0);
		// Update the 3DText that appears above the house-pickup (displays the price of the house)
		format(Msg, sizeof(Msg), TXT_PickupHouseForSale, HouseID, AHouseData[HouseID][HousePrice], AHouseData[HouseID][HouseMaxLevel]);
		UpdateDynamic3DTextLabelText(AHouseData[HouseID][DoorText], COLOR_TEAL, Msg);
		// Add a streamed icon to the map (green house), type = 31, color = 0, world = 0, interior = 0, playerid = -1, drawdist = 150.0
		AHouseData[HouseID][MapIconID] = CreateDynamicMapIcon(x, y, z, 31, 0, 0, 0, -1, 150.0);
	}
}

// This function adds a vehicle to the house (if possible)
House_AddVehicle(HouseID, cModel, cPaint, const cComponents[], Float:cx, Float:cy, Float:cz, Float:crot, Col1, Col2)
{
	// Setup local variables
	new vid, CarSlot, vText[128];

	// Get a free carslot from the house
	CarSlot = House_GetFreeCarSlot(HouseID);

	// Check if there is a free carslot
	if (CarSlot != -1)
	{
		// Create a new vehicle and get the vehicle-id
		vid = CreateVehicle(cModel, cx, cy, cz, crot, Col1, Col2, 600);
		// Store the vehicle-id in the house's free carslot
		AHouseData[HouseID][VehicleIDs][CarSlot] = vid;

		// Save the model of the vehicle
		AVehicleData[vid][Model] = cModel;
		// Save the paintjob of the vehicle and apply it
		AVehicleData[vid][PaintJob] = cPaint;
		if (cPaint != 0)
			ChangeVehiclePaintjob(vid, cPaint - 1);

		// Also update the car-color
		ChangeVehicleColours(vid, Col1, Col2);
		// Save the colors of the vehicle
		AVehicleData[vid][Color1] = Col1;
		AVehicleData[vid][Color2] = Col2;

		// Save the components of the vehicle and apply them
		for (new i; i < 14; i++)
		{
			AVehicleData[vid][Components][i] = cComponents[i];
		    // Check if the componentslot has a valid component-id
			if (AVehicleData[vid][Components][i] != 0)
		        AddVehicleComponent(vid, AVehicleData[vid][Components][i]); // Add the component to the vehicle
		}

		// Save the spawn-data of the vehicle
        AVehicleData[vid][SpawnX] = cx;
        AVehicleData[vid][SpawnY] = cy;
        AVehicleData[vid][SpawnZ] = cz;
        AVehicleData[vid][SpawnRot] = crot;
		// Also set the fuel to maximum
		AVehicleData[vid][Fuel] = MaxFuel;
		// Also set the owner
		AVehicleData[vid][Owned] = true;
		format(AVehicleData[vid][Owner], 24, AHouseData[HouseID][Owner]);
		// Save the HouseID for the vehicle
		AVehicleData[vid][BelongsToHouse] = HouseID;
		// Add a 3DText label to the vehicle, displaying the name of the owner
        format(vText, sizeof(vText), "Vehicle owned by: \n%s", AVehicleData[vid][Owner]);
        AVehicleData[vid][VehicleText] = CreateDynamic3DTextLabel(vText, COLOR_WHITE, 0.0, 0.0, 0.0, 80.0, INVALID_PLAYER_ID, vid);
	}
	else // No free carslot was found, return 0
		return 0;

	// Exit the function and return the vehicle-id
	return vid;
}

// This function is used only when you respawn your vehicles by exiting your house
House_ReplaceVehicle(HouseID, CarSlot)
{
	// Setup local variables
	new vid, cModel, cPaint, cComponents[14], Float:cx, Float:cy, Float:cz, Float:crot, Col1, Col2, Float:Health, cFuel, Neons;
	new VEHICLE_PANEL_STATUS:panels, VEHICLE_DOOR_STATUS:doors, VEHICLE_LIGHT_STATUS:lights, VEHICLE_TYRE_STATUS:tires;

	// Get the data from the already existing vehicle that was parked before
	vid = AHouseData[HouseID][VehicleIDs][CarSlot];
	cModel = AVehicleData[vid][Model];
	cPaint = AVehicleData[vid][PaintJob];
	cFuel = AVehicleData[vid][Fuel];
	for (new i; i < 14; i++)
	    cComponents[i] = AVehicleData[vid][Components][i];
	Col1 = AVehicleData[vid][Color1];
	Col2 = AVehicleData[vid][Color2];
	cx = AVehicleData[vid][SpawnX];
	cy = AVehicleData[vid][SpawnY];
	cz = AVehicleData[vid][SpawnZ];
	crot = AVehicleData[vid][SpawnRot];
	Neons = AVehicleData[vid][NeonObjectModel];
	GetVehicleHealth(vid, Health);
	GetVehicleDamageStatus(vid, panels, doors, lights, tires);

	// Remove neons
	if (AVehicleData[vid][NeonObjectModel] != 0)
	{
        DestroyObject(AVehicleData[vid][NeonLeft]);
        DestroyObject(AVehicleData[vid][NeonRight]);
        AVehicleData[vid][NeonLeft] = 0;
        AVehicleData[vid][NeonRight] = 0;
		AVehicleData[vid][NeonObjectModel] = 0;
	}

	// Delete the vehicle and clear the data
	Vehicle_Delete(vid);

	// Create a new vehicle in the same carslot
	vid = House_AddVehicle(HouseID, cModel, cPaint, cComponents, Float:cx, Float:cy, Float:cz, Float:crot, Col1, Col2);
	// Update the fuel of the vehicle to the previous setting
	AVehicleData[vid][Fuel] = cFuel;
	// Update the health to what it was before and update the bodywork
	SetVehicleHealth(vid, Health);
	UpdateVehicleDamageStatus(vid, panels, doors, lights, tires);
	// Re-apply the neons if they were there
	if (Neons != 0)
	{
		AVehicleData[vid][NeonObjectModel] = Neons;
	    AVehicleData[vid][NeonLeft] = CreateObject(AVehicleData[vid][NeonObjectModel],0,0,0,0,0,0);
	    AVehicleData[vid][NeonRight] = CreateObject(AVehicleData[vid][NeonObjectModel],0,0,0,0,0,0);
		AttachObjectToVehicle(AVehicleData[vid][NeonLeft], vid, -0.8, 0.0, -0.70, 0.0, 0.0, 0.0);
		AttachObjectToVehicle(AVehicleData[vid][NeonRight], vid, 0.8, 0.0, -0.70, 0.0, 0.0, 0.0);
	}

	return vid;
}

// This function is used only when a player logs out (the vehicles are unloaded)
House_RemoveVehicles(HouseID)
{
	// Setup local variables
	new vid;

	// Loop through all carslots of this house
	for (new CarSlot; CarSlot < 10; CarSlot++)
	{
		// Get the vehicle-id
		vid = AHouseData[HouseID][VehicleIDs][CarSlot];

		// Check if there was a vehicle in this carslot
		if (vid != 0)
		{
			// Eject everyone from the vehicle
			RemoveAllPlayersFromVehicle(vid);
			// Delete the vehicle and clear the data
			SetTimerEx("TimedDestroyVehicle", 1000, false, "i", vid);
			AHouseData[HouseID][VehicleIDs][CarSlot] = 0;
			AVehicleData[vid][Owned] = false;
			AVehicleData[vid][Owner] = 0;
			AVehicleData[vid][Model] = 0;
			AVehicleData[vid][PaintJob] = 0;
			for (new i; i < 14; i++)
			    AVehicleData[vid][Components][i] = 0;
			AVehicleData[vid][SpawnX] = 0.0;
			AVehicleData[vid][SpawnY] = 0.0;
			AVehicleData[vid][SpawnZ] = 0.0;
			AVehicleData[vid][SpawnRot] = 0.0;
			AVehicleData[vid][BelongsToHouse] = 0;
			DestroyDynamic3DTextLabel(AVehicleData[vid][VehicleText]);

			// Remove neons
			if (AVehicleData[vid][NeonObjectModel] != 0)
			{
		        DestroyObject(AVehicleData[vid][NeonLeft]);
		        DestroyObject(AVehicleData[vid][NeonRight]);
		        AVehicleData[vid][NeonLeft] = 0;
		        AVehicleData[vid][NeonRight] = 0;
				AVehicleData[vid][NeonObjectModel] = 0;
			}
		}
	}
}

// This function calculates the sell-price for the given house
House_CalcSellPrice(HouseID)
{
	// Setup local variables
	new SellPrice, NumUpgrades, UpgradePrice;

	// Calculate 50% of the original buying price (base-price for selling)
	SellPrice = AHouseData[HouseID][HousePrice] / 2;
	// Calculate the number of upgrades applied to the house
	NumUpgrades = AHouseData[HouseID][HouseLevel] - 1;
	// Also calculate 50% for each upgrade, based on the percentage for upgrading the house
	UpgradePrice = ((AHouseData[HouseID][HousePrice] / 100) * HouseUpgradePercent) * NumUpgrades;

	// Add 50% of the upgrade-price to the sell-price
	SellPrice = SellPrice + UpgradePrice;

	// Return the total sell-price to the calling function
	return SellPrice;
}

// This function returns "1" if the given player is the owner of the given house
House_PlayerIsOwner(playerid, HouseID)
{
	// Loop through all houses
	for (new i; i < MAX_HOUSESPERPLAYER; i++)
	{
		// Check if the player owns the house in any of his house-slots
		if (APlayerData[playerid][Houses][i] == HouseID)
		    return true;
	}

	// If the player doesn't own the house, return 0
	return 0;
}

// This function will evict the given house and clear ownership
House_Evict(HouseID)
{
	// Setup local variables
	new Msg[128], Name[24], bool:PlayerOnline = false;

	// Check if the house is owned
	if (AHouseData[HouseID][Owned] == true)
	{
		// Get the owner name
		format(Name, sizeof(Name), AHouseData[HouseID][Owner]);

		// Loop through all players to find the owner (if he's online)
		for (new pid; pid < MAX_PLAYERS; pid++)
		{
			// Check if this player is online
		    if (IsPlayerConnected(pid))
		    {
		        // Get that player's name
		        new PlayerName[24];
		        GetPlayerName(pid, PlayerName, sizeof(PlayerName));
		        // Compare if this player has the same name as the owner of the house
				if (strcmp(AHouseData[HouseID][Owner], PlayerName, false) == 0)
				{
				    // Set PlayerOnline to "true"
				    PlayerOnline = true;

					// Inform the player that his house is being deleted
					format(Msg, sizeof(Msg), "{FF0000}Your house {FFFF00}\"%s\"{FF0000} is being evicted", AHouseData[HouseID][HouseName]);
					SendClientMessage(pid, COLOR_WHITE, Msg);

					// Also remove the HouseID from his list of houses
					for (new HouseSlot; HouseSlot < MAX_HOUSESPERPLAYER; HouseSlot++)
					    // Check if the house is stored in this houseslot
					    if (APlayerData[pid][Houses][HouseSlot] == HouseID)
					        // If the house is stored in this slot, clear the slot
					        APlayerData[pid][Houses][HouseSlot] = 0;

					// Save the player's account to database
					SavePlayerData(pid);

					// Stop this For-loop, as the owner was already found and informed
					break;
				}
		    }
		}
	}

	// First remove all vehicles from the house
	House_RemoveVehicles(HouseID);

	// Clear ownership of the house
	AHouseData[HouseID][Owned] = false;
	format(AHouseData[HouseID][Owner], 24, "");
	format(AHouseData[HouseID][HouseName], 100, "House");
	AHouseData[HouseID][HouseLevel] = 1;
	AHouseData[HouseID][AutoEvictDays] = 0;

	// Update the entrance of the house
	House_UpdateEntrance(HouseID);

	// Save the house to database
	House_Save(HouseID);
	
	printf("House %d evicted (was owned by %s)", HouseID, Name);
}

// This function will delete the given house completely
House_Delete(HouseID)
{
	// First remove all vehicles from the house
	House_RemoveVehicles(HouseID);

	// Remove the house entrance (pickup, 3DText, map-icon)
	if (IsValidDynamicPickup(AHouseData[HouseID][PickupID]))
		DestroyDynamicPickup(AHouseData[HouseID][PickupID]);
	if (IsValidDynamic3DTextLabel(AHouseData[HouseID][DoorText]))
		DestroyDynamic3DTextLabel(AHouseData[HouseID][DoorText]);
	if (IsValidDynamicMapIcon(AHouseData[HouseID][MapIconID]))
		DestroyDynamicMapIcon(AHouseData[HouseID][MapIconID]);

	// Clear all data from the house
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
	AHouseData[HouseID][PickupID] = 0;
	AHouseData[HouseID][DoorText] = Text3D:INVALID_3DTEXT_ID;
	AHouseData[HouseID][MapIconID] = 0;

	// Clear all vehicle slots
	for (new i = 0; i < 10; i++)
		AHouseData[HouseID][VehicleIDs][i] = 0;

	// Delete the house from MySQL database
	new query[128];
	mysql_format(Database, query, sizeof(query), "DELETE FROM `houses` WHERE `HouseID` = %d", HouseID);
	mysql_query(Database, query);
	
	// Also delete all house vehicles from database
	mysql_format(Database, query, sizeof(query), "DELETE FROM `house_vehicles` WHERE `HouseID` = %d", HouseID);
	mysql_query(Database, query);

	printf("House %d completely deleted from database", HouseID);
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