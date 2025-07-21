CREATE DATABASE IF NOT EXISTS `ppctrucking` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
USE `ppctrucking`;

CREATE TABLE IF NOT EXISTS `players` (
  `PlayerID` int(11) NOT NULL AUTO_INCREMENT,
  `PlayerName` varchar(24) NOT NULL,
  `PlayerPassword` varchar(64) DEFAULT NULL,
  `PlayerLevel` int(11) NOT NULL DEFAULT 1,
  `PlayerJailed` int(11) NOT NULL DEFAULT 0,
  `WantedLevel` int(11) NOT NULL DEFAULT 0,
  `Bans` int(11) NOT NULL DEFAULT 0,
  `BanTime` int(11) NOT NULL DEFAULT 0,
  `TruckerLicense` int(11) NOT NULL DEFAULT 0,
  `BusLicense` int(11) NOT NULL DEFAULT 0,
  `Muted` int(11) NOT NULL DEFAULT 0,
  `Money` int(11) NOT NULL DEFAULT 0,
  `Score` int(11) NOT NULL DEFAULT 0,
  `RulesRead` enum('Yes','No') NOT NULL DEFAULT 'No',
  `StatsMetersDriven` float NOT NULL DEFAULT 0,
  `StatsTruckerJobs` int(11) NOT NULL DEFAULT 0,
  `StatsConvoyJobs` int(11) NOT NULL DEFAULT 0,
  `StatsBusDriverJobs` int(11) NOT NULL DEFAULT 0,
  `StatsPilotJobs` int(11) NOT NULL DEFAULT 0,
  `StatsMafiaJobs` int(11) NOT NULL DEFAULT 0,
  `StatsMafiaStolen` int(11) NOT NULL DEFAULT 0,
  `StatsPoliceFined` int(11) NOT NULL DEFAULT 0,
  `StatsPoliceJailed` int(11) NOT NULL DEFAULT 0,
  `StatsAssistance` int(11) NOT NULL DEFAULT 0,
  `StatsCourierJobs` int(11) NOT NULL DEFAULT 0,
  `StatsRoadworkerJobs` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`PlayerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

CREATE TABLE IF NOT EXISTS `banks` (
  `BankID` int(11) NOT NULL AUTO_INCREMENT,
  `PlayerName` varchar(24) NOT NULL,
  `BankPassword` varchar(128) NOT NULL,
  `BankMoney` int(11) NOT NULL DEFAULT 0,
  `LastInterestTime` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`BankID`),
  UNIQUE KEY `PlayerName` (`PlayerName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

CREATE TABLE IF NOT EXISTS `bank_transfers` (
  `TransferID` int(11) NOT NULL AUTO_INCREMENT,
  `SenderName` varchar(24) NOT NULL,
  `ReceiverName` varchar(24) NOT NULL,
  `TransferAmount` int(11) NOT NULL,
  `TransferTime` int(11) NOT NULL,
  `TransferMessage` varchar(128) DEFAULT NULL,
  `IsProcessed` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`TransferID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

CREATE TABLE IF NOT EXISTS `settings` (
  `setting_id` int(11) NOT NULL AUTO_INCREMENT,
  `setting_name` varchar(64) NOT NULL,
  `setting_value` varchar(512) NOT NULL,
  PRIMARY KEY (`setting_id`),
  UNIQUE KEY `setting_name` (`setting_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

CREATE TABLE IF NOT EXISTS `cameras` (
  `CameraID` int(11) NOT NULL AUTO_INCREMENT,
  `CamX` float NOT NULL,
  `CamY` float NOT NULL,
  `CamZ` float NOT NULL,
  `CamAngle` float NOT NULL,
  `CamSpeed` int(11) NOT NULL,
  `CreatedBy` varchar(24) DEFAULT NULL,
  `DateCreated` timestamp DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`CameraID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- Houses table
CREATE TABLE IF NOT EXISTS `houses` (
  `HouseID` int(11) NOT NULL AUTO_INCREMENT,
  `HouseName` varchar(50) DEFAULT 'House',
  `HouseX` float NOT NULL,
  `HouseY` float NOT NULL, 
  `HouseZ` float NOT NULL,
  `HouseLevel` int(11) DEFAULT 1,
  `HouseMaxLevel` int(11) DEFAULT 10,
  `HousePrice` int(11) NOT NULL,
  `Owned` tinyint(1) DEFAULT 0,
  `Owner` varchar(24) DEFAULT NULL,
  `Insurance` tinyint(1) DEFAULT 0,
  `AutoEvictDays` int(11) DEFAULT 0,
  `DateCreated` timestamp DEFAULT CURRENT_TIMESTAMP,
  `LastAccess` timestamp DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`HouseID`),
  KEY `idx_owner` (`Owner`),
  KEY `idx_owned` (`Owned`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- House vehicles table
CREATE TABLE IF NOT EXISTS `house_vehicles` (
  `VehicleID` int(11) NOT NULL AUTO_INCREMENT,
  `HouseID` int(11) NOT NULL,
  `CarSlot` int(11) NOT NULL,
  `VehicleModel` int(11) NOT NULL,
  `Fuel` int(11) DEFAULT 100,
  `VehiclePaintJob` int(11) DEFAULT -1,
  `VehicleSpoiler` int(11) DEFAULT 0,
  `VehicleHood` int(11) DEFAULT 0,
  `VehicleRoof` int(11) DEFAULT 0,
  `VehicleSideSkirt` int(11) DEFAULT 0,
  `VehicleLamps` int(11) DEFAULT 0,
  `VehicleNitro` int(11) DEFAULT 0,
  `VehicleExhaust` int(11) DEFAULT 0,
  `VehicleWheels` int(11) DEFAULT 0,
  `VehicleStereo` int(11) DEFAULT 0,
  `VehicleHydraulics` int(11) DEFAULT 0,
  `VehicleFrontBumper` int(11) DEFAULT 0,
  `VehicleRearBumper` int(11) DEFAULT 0,
  `VehicleVentRight` int(11) DEFAULT 0,
  `VehicleVentLeft` int(11) DEFAULT 0,
  `Color1` int(11) DEFAULT -1,
  `Color2` int(11) DEFAULT -1,
  `VehicleX` float NOT NULL,
  `VehicleY` float NOT NULL,
  `VehicleZ` float NOT NULL,
  `VehicleAngle` float NOT NULL,
  `NeonsApplied` int(11) DEFAULT 0,
  `Clamped` tinyint(1) DEFAULT 0,
  PRIMARY KEY (`VehicleID`),
  KEY `idx_house` (`HouseID`),
  FOREIGN KEY (`HouseID`) REFERENCES `houses`(`HouseID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- Player-House relationship table (since players can own multiple houses)
CREATE TABLE IF NOT EXISTS `player_houses` (
  `PlayerID` int(11) NOT NULL,
  `HouseID` int(11) NOT NULL,
  `PurchaseDate` timestamp DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`PlayerID`, `HouseID`),
  FOREIGN KEY (`PlayerID`) REFERENCES `players`(`PlayerID`) ON DELETE CASCADE,
  FOREIGN KEY (`HouseID`) REFERENCES `houses`(`HouseID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;