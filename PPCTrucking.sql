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