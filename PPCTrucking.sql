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