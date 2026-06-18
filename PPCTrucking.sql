CREATE DATABASE IF NOT EXISTS `ppctrucking` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
USE `ppctrucking`;

-- PPC_Trucking MySQL schema
--
-- Brownfield upgrade safety:
-- 1. Back up the current database and scriptfiles/ServerData before applying changes.
-- 2. Preflight existing partial MySQL databases before adding/expecting unique keys:
--      SELECT PlayerName, COUNT(*) FROM players GROUP BY PlayerName HAVING COUNT(*) > 1;
--      SELECT HouseID, CarSlot, COUNT(*) FROM house_vehicles GROUP BY HouseID, CarSlot HAVING COUNT(*) > 1;
--      SELECT MAX(HouseID) FROM houses;       -- must be less than MAX_HOUSES in PPC_ServerSettings.inc
--      SELECT MAX(BusinessID) FROM businesses; -- must be less than MAX_BUSINESS in PPC_ServerSettings.inc
--    If any duplicate or out-of-range query returns unsafe rows, manually reconcile them before continuing.
--    Example brownfield key additions after clean preflight, run only when the target key is missing:
--      ALTER TABLE players ADD UNIQUE KEY idx_players_name_unique (PlayerName);
--      ALTER TABLE bank_transfers ADD KEY idx_bank_transfers_receiver_processed (ReceiverName, IsProcessed);
--      ALTER TABLE bank_transfers ADD KEY idx_bank_transfers_sender (SenderName);
--      ALTER TABLE cameras ADD KEY idx_cameras_created_by (CreatedBy);
--      ALTER TABLE house_vehicles ADD UNIQUE KEY idx_house_vehicle_slot_unique (HouseID, CarSlot);
--      ALTER TABLE houses ADD KEY idx_owner (Owner);
--      ALTER TABLE houses ADD KEY idx_owned (Owned);
--      ALTER TABLE businesses ADD KEY idx_business_owner (Owner);
--      ALTER TABLE businesses ADD KEY idx_business_owned (Owned);
-- 3. Phase 1 ownership source of truth is owner name fields:
--      houses.Owner and businesses.Owner
--    player_houses is retained for compatibility/future normalization, but runtime code should not
--    depend on it unless all loaders and savers maintain it consistently.
-- 4. Camera identity is deterministic: Pawn CamID maps to SQL CameraID = CamID + 1.

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
  PRIMARY KEY (`PlayerID`),
  UNIQUE KEY `idx_players_name_unique` (`PlayerName`)
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
  PRIMARY KEY (`TransferID`),
  KEY `idx_bank_transfers_receiver_processed` (`ReceiverName`, `IsProcessed`),
  KEY `idx_bank_transfers_sender` (`SenderName`)
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
  PRIMARY KEY (`CameraID`),
  KEY `idx_cameras_created_by` (`CreatedBy`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- Houses table
CREATE TABLE IF NOT EXISTS `houses` (
  `HouseID` int(11) NOT NULL AUTO_INCREMENT,
  `HouseName` varchar(100) DEFAULT 'House',
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

-- House vehicles table (separate from houses for better normalization)
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
  UNIQUE KEY `idx_house_vehicle_slot_unique` (`HouseID`, `CarSlot`),
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

-- Auto-evict log table
CREATE TABLE IF NOT EXISTS `auto_evict_log` (
  `log_id` int(11) NOT NULL AUTO_INCREMENT,
  `log_message` varchar(256) NOT NULL,
  `log_date` timestamp DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- Businesses table  
CREATE TABLE IF NOT EXISTS `businesses` (
  `BusinessID` int(11) NOT NULL AUTO_INCREMENT,
  `BusinessName` varchar(100) DEFAULT 'Business',
  `BusinessX` float NOT NULL,
  `BusinessY` float NOT NULL,
  `BusinessZ` float NOT NULL,
  `BusinessType` int(11) NOT NULL,
  `BusinessLevel` int(11) DEFAULT 1,
  `LastTransaction` int(11) DEFAULT 0,
  `Owned` tinyint(1) DEFAULT 0,
  `Owner` varchar(24) DEFAULT NULL,
  `AutoEvictDays` int(11) DEFAULT 0,
  `DateCreated` timestamp DEFAULT CURRENT_TIMESTAMP,
  `LastAccess` timestamp DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`BusinessID`),
  KEY `idx_business_owner` (`Owner`),
  KEY `idx_business_owned` (`Owned`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

INSERT INTO `settings` (`setting_name`, `setting_value`) VALUES
  ('SchemaVersion', '2'),
  ('BusinessTransactionTime', '3'),
  ('CurrentInterestTime', '0'),
  ('AutoEvictMinutes', '0'),
  ('AutoEvictHours', '0'),
  ('AutoEvictDays', '0')
ON DUPLICATE KEY UPDATE `setting_value` = `setting_value`;

-- Default businesses converted from the legacy Dini files under
-- scriptfiles/ServerData/Business/*.ini.  Fresh imports get the bundled
-- map businesses in MySQL; brownfield servers keep existing rows because
-- duplicate BusinessID rows are left untouched.
INSERT INTO `businesses` (`BusinessID`, `BusinessName`, `BusinessX`, `BusinessY`, `BusinessZ`, `BusinessType`, `BusinessLevel`, `LastTransaction`, `Owned`, `Owner`, `AutoEvictDays`) VALUES
  (1, 'Stadium (Dirt Bike)', 1098.233886, 1601.218627, 12.546875, 29, 1, 0, 0, '', 0),
  (2, 'Business', 1158.880493, 2072.006835, 11.062500, 7, 1, 0, 0, '', 0),
  (3, 'Business', -2336.115966, -166.894561, 35.554687, 7, 1, 0, 0, '', 0),
  (4, 'Business', -2672.143798, 258.651794, 4.632812, 16, 1, 0, 0, '', 0),
  (5, 'Business', -2356.282470, 1008.206054, 50.898437, 7, 1, 0, 0, '', 0),
  (6, 'Business', 1087.580566, -922.713867, 43.390625, 24, 1, 0, 0, '', 0),
  (7, 'Business', -2624.662109, 1411.694702, 7.093750, 18, 1, 0, 0, '', 0),
  (8, 'Business', 2546.522705, 1972.031738, 10.820312, 2, 1, 0, 0, '', 0),
  (9, 'Business', 2392.848144, 2042.474731, 10.820312, 16, 1, 0, 0, '', 0),
  (10, 'Business', 2420.282470, 2064.352783, 10.820312, 24, 1, 0, 0, '', 0),
  (11, 'Business', 2441.184814, 2064.556640, 10.820312, 3, 1, 0, 0, '', 0),
  (12, 'Business', 2452.505615, 2064.078125, 10.820312, 1, 1, 0, 0, '', 0),
  (13, 'Business', 2471.838867, 2034.408325, 11.062500, 7, 1, 0, 0, '', 0),
  (14, 'Business', 2409.291259, 2016.161865, 10.820312, 24, 1, 0, 0, '', 0),
  (15, 'Business', 2366.181884, 2070.981689, 10.820312, 7, 1, 0, 0, '', 0),
  (16, 'Business', 2373.499755, 2167.598632, 10.824930, 10, 1, 0, 0, '', 0),
  (17, 'Business', 2506.750732, 2121.235107, 10.840039, 30, 1, 0, 0, '', 0),
  (18, 'Business', 2089.364013, 1452.253906, 10.820312, 9, 1, 0, 0, '', 0),
  (19, 'Business', 2540.614013, 2149.458007, 10.820312, 33, 1, 0, 0, '', 0),
  (20, 'Business', 2516.244873, 2297.554931, 10.820312, 24, 1, 0, 0, '', 0),
  (21, 'Business', 2019.315551, 1007.688049, 10.820312, 8, 1, 0, 0, '', 0),
  (22, 'Business', 2629.190917, 2348.101806, 10.820312, 22, 1, 0, 0, '', 0),
  (23, 'Business', 2196.255126, 1676.983032, 12.367187, 9, 1, 0, 0, '', 0),
  (24, 'Business', 2885.626953, 2453.559326, 11.068956, 1, 1, 0, 0, '', 0),
  (25, 'Business', 2194.301757, 1990.841918, 12.296875, 1, 1, 0, 0, '', 0),
  (26, 'Business', 2802.066894, 2429.807128, 11.062500, 14, 1, 0, 0, '', 0),
  (27, 'Business', 2085.922119, 2073.964355, 11.054687, 24, 1, 0, 0, '', 0),
  (28, 'Business', 2778.903808, 2452.852539, 11.062500, 13, 1, 0, 0, '', 0),
  (29, 'Business', 2756.094726, 2476.385742, 11.062500, 33, 1, 0, 0, '', 0),
  (30, 'Business', 2825.815917, 2407.118408, 11.062500, 12, 1, 0, 0, '', 0),
  (31, 'Business', 2094.287841, 2121.668212, 10.820312, 32, 1, 0, 0, '', 0),
  (32, 'Business', 2838.926757, 2407.366943, 11.068956, 16, 1, 0, 0, '', 0),
  (33, 'Business', 2102.253417, 2121.503906, 10.820312, 32, 1, 0, 0, '', 0),
  (34, 'Business', 2080.565185, 2122.865234, 10.820312, 5, 1, 0, 0, '', 0),
  (35, 'Business', 2003.684936, 2149.963867, 10.820312, 22, 1, 0, 0, '', 0),
  (36, 'Business', 2247.539794, 2397.341796, 10.820312, 2, 1, 0, 0, '', 0),
  (37, 'Business', 2082.958496, 2224.175048, 11.023437, 33, 1, 0, 0, '', 0),
  (38, 'Business', 2090.582519, 2224.701171, 11.023437, 15, 1, 0, 0, '', 0),
  (39, 'Business', 2102.741455, 2257.223388, 11.023437, 11, 1, 0, 0, '', 0),
  (40, 'Business', 2097.838867, 2224.694091, 11.023437, 1, 1, 0, 0, '', 0),
  (41, 'Business', 2101.892578, 2228.802246, 11.023437, 16, 1, 0, 0, '', 0),
  (42, 'Business', 2127.665771, 2377.549072, 10.820312, 8, 1, 0, 0, '', 0),
  (43, 'Business', 2165.370117, 2163.131591, 10.820312, 9, 1, 0, 0, '', 0),
  (44, 'Business', 2079.187988, 2154.216064, 10.820312, 22, 1, 0, 0, '', 0),
  (45, 'Business', 2218.405761, 2123.700927, 10.820312, 9, 1, 0, 0, '', 0),
  (46, 'Business', 2330.684814, 2165.486328, 10.828125, 8, 1, 0, 0, '', 0),
  (47, 'Business', 2317.259521, 2115.957763, 10.828125, 9, 1, 0, 0, '', 0),
  (48, 'Business', 2225.377197, 1839.208007, 10.820312, 22, 1, 0, 0, '', 0),
  (49, 'Business', 2089.827392, 1514.473266, 10.820312, 9, 1, 0, 0, '', 0),
  (50, 'Business', 1952.511840, 1342.997436, 15.374607, 8, 1, 0, 0, '', 0),
  (51, 'Business', 2436.608154, 1134.132324, 10.812517, 33, 1, 0, 0, '', 0),
  (52, 'Business', 172.341766, 1177.300781, 14.757812, 16, 1, 0, 0, '', 0),
  (53, 'Business', -2268.615234, -155.989227, 35.320312, 20, 1, 0, 0, '', 0),
  (54, 'Business', -2430.145019, -183.293106, 35.320312, 24, 1, 0, 0, '', 0),
  (55, 'Business', -2493.235839, -154.924880, 25.617187, 13, 1, 0, 0, '', 0),
  (56, 'Business', -2492.782958, -142.759338, 25.609390, 13, 1, 0, 0, '', 0),
  (57, 'Business', -2492.279785, -38.996864, 25.765625, 32, 1, 0, 0, '', 0),
  (58, 'Business', -1911.682983, 828.597351, 35.171875, 7, 1, 0, 0, '', 0),
  (59, 'Business', -2492.099853, -29.288175, 25.765625, 13, 1, 0, 0, '', 0),
  (60, 'Business', -1807.546875, 944.972656, 24.890625, 33, 1, 0, 0, '', 0),
  (61, 'Business', -2570.720214, 245.586685, 10.047043, 4, 1, 0, 0, '', 0),
  (62, 'Business', -1882.762207, 865.853637, 35.171875, 15, 1, 0, 0, '', 0),
  (63, 'Business', -2425.672851, 337.567504, 37.001281, 22, 1, 0, 0, '', 0),
  (64, 'Business', -1695.157348, 950.812316, 24.890625, 14, 1, 0, 0, '', 0),
  (65, 'Business', -2442.684082, 754.340087, 35.171875, 2, 1, 0, 0, '', 0),
  (66, 'Business', -2155.214599, 484.204650, 35.171875, 24, 1, 0, 0, '', 0),
  (67, 'Business', -2080.285400, -406.809570, 38.734375, 26, 1, 0, 0, '', 0),
  (68, 'Business', 811.251953, -1616.269165, 13.546875, 7, 1, 0, 0, '', 0),
  (69, 'Business', 823.469787, -1589.167724, 13.554450, 4, 1, 0, 0, '', 0),
  (70, 'Business', -2719.881103, -318.802032, 7.843750, 22, 1, 0, 0, '', 0),
  (71, 'Business', 928.482299, -1352.859252, 13.343750, 16, 1, 0, 0, '', 0),
  (72, 'Business', -2154.632080, -2460.861328, 30.851562, 16, 1, 0, 0, '', 0),
  (73, 'Business', -2119.317382, -2353.745361, 30.625000, 4, 1, 0, 0, '', 0),
  (74, 'Business', 499.443450, -1359.500000, 16.294696, 12, 1, 0, 0, '', 0),
  (75, 'Business', -2193.906738, -2256.659667, 30.683063, 22, 1, 0, 0, '', 0),
  (76, 'Business', -13.605294, -2502.360839, 36.655464, 22, 1, 0, 0, '', 0),
  (77, 'Business', 816.112426, -1387.141601, 13.609544, 17, 1, 0, 0, '', 0),
  (78, 'Business', 1112.171386, -1370.848266, 13.984375, 2, 1, 0, 0, '', 0),
  (79, 'Business', 1038.295898, -1339.812622, 13.726562, 7, 1, 0, 0, '', 0),
  (80, 'Business', 994.404479, -1296.280883, 13.546875, 1, 1, 0, 0, '', 0),
  (81, 'Business', 954.157897, -1335.553955, 13.538661, 24, 1, 2, 0, '', 0),
  (82, 'Business', 951.295288, -1294.890136, 14.310953, 1, 1, 0, 0, '', 0),
  (83, 'Business', 776.829711, -1036.929565, 24.277614, 14, 1, 0, 0, '', 0),
  (84, 'Business', 1036.934692, -945.321411, 42.741230, 1, 1, 0, 0, '', 0),
  (85, 'Business', 1199.494873, -919.105468, 43.114353, 7, 1, 0, 0, '', 0),
  (86, 'Business', 1315.581665, -898.590148, 39.578125, 2, 1, 0, 0, '', 0),
  (87, 'Business', 1290.710327, -1160.415405, 23.960971, 10, 1, 0, 0, '', 0),
  (88, 'Business', 1139.780761, -1131.808349, 23.828125, 24, 1, 0, 0, '', 0),
  (89, 'Business', 1321.671142, -1634.726440, 13.546875, 1, 1, 0, 0, '', 0),
  (90, 'Business', 1561.420898, -1855.871582, 13.546875, 23, 1, 0, 0, '', 0),
  (91, 'Business', 1577.070678, -1584.240478, 13.546875, 3, 1, 0, 0, '', 0),
  (92, 'Business', 1835.926635, -1682.630737, 13.369323, 18, 1, 0, 0, '', 0),
  (93, 'Business', 1832.710083, -1842.424316, 13.578125, 1, 1, 0, 0, '', 0),
  (94, 'Business', 1846.645263, -1871.554321, 13.578125, 1, 1, 0, 0, '', 0),
  (95, 'Business', 1953.556030, -2003.886596, 13.546875, 1, 1, 0, 0, '', 0),
  (96, 'Business', 1952.608154, -2041.433349, 13.546875, 3, 1, 0, 0, '', 0),
  (97, 'Business', 1976.467773, -2036.765014, 13.546875, 32, 1, 0, 0, '', 0),
  (98, 'Business', 1940.757934, -2115.854248, 13.695312, 24, 1, 0, 0, '', 0),
  (99, 'Business', 1975.728637, -2101.336914, 13.546875, 22, 1, 0, 0, '', 0),
  (100, 'Business', 2721.949951, -2026.470825, 13.547199, 4, 1, 0, 0, '', 0),
  (101, 'Business', 2695.042236, -1704.448730, 11.843750, 28, 1, 0, 0, '', 0),
  (102, 'Business', 2850.038818, -1479.619873, 10.916131, 1, 1, 0, 0, '', 0),
  (103, 'Business', 2861.443359, -1479.699096, 10.934373, 23, 1, 0, 0, '', 0),
  (104, 'Business', 2634.242431, -1344.235229, 35.884654, 1, 1, 0, 0, '', 0),
  (105, 'Business', 2442.639404, -1376.128417, 24.000000, 3, 1, 0, 0, '', 0),
  (106, 'Business', 2459.933837, -1343.692016, 24.000000, 3, 1, 0, 0, '', 0),
  (107, 'Business', 2421.734130, -1220.588378, 25.474021, 31, 1, 0, 0, '', 0),
  (108, 'Business', 2379.796875, -1213.658569, 27.422292, 2, 1, 0, 0, '', 0),
  (109, 'Business', 2363.246337, -1332.328247, 24.007812, 3, 1, 0, 0, '', 0),
  (110, 'Business', 2350.803466, -1411.945556, 23.992334, 2, 1, 0, 0, '', 0),
  (111, 'Business', 2352.018066, -1463.243041, 24.000000, 30, 1, 0, 0, '', 0),
  (112, 'Business', 2420.694824, -1509.244140, 24.000000, 16, 1, 0, 0, '', 0),
  (113, 'Business', 2310.018310, -1643.560058, 14.827047, 3, 1, 0, 0, '', 0),
  (114, 'Business', 2265.094482, -1670.437011, 15.359375, 5, 1, 0, 0, '', 0),
  (115, 'Business', 2244.493164, -1664.105957, 15.476562, 11, 1, 0, 0, '', 0),
  (116, 'Business', 2229.496093, -1721.924316, 13.567713, 19, 1, 0, 0, '', 0),
  (117, 'Business', 2177.794433, -1770.507080, 13.543832, 22, 1, 0, 0, '', 0),
  (118, 'Business', 2139.423583, -1744.108276, 13.552255, 24, 1, 0, 0, '', 0),
  (119, 'Business', 2104.499755, -1806.384521, 13.554687, 33, 1, 0, 0, '', 0),
  (120, 'Business', 2069.652832, -1779.539672, 13.559102, 32, 1, 0, 0, '', 0),
  (121, 'Business', 2073.034667, -1793.817260, 13.546875, 5, 1, 0, 0, '', 0),
  (122, 'Business', 2333.675537, 75.220291, 26.620975, 33, 1, 0, 0, '', 0),
  (123, 'Business', 2333.354003, 31.061429, 26.675220, 24, 1, 0, 0, '', 0),
  (124, 'Business', 2334.091552, -67.138992, 26.484375, 23, 1, 0, 0, '', 0),
  (125, 'Business', 2318.976806, -89.430030, 26.484375, 2, 1, 0, 0, '', 0),
  (126, 'Business', 1413.928833, 262.101867, 19.554687, 23, 1, 0, 0, '', 0),
  (127, 'Business', 1366.500366, 248.832946, 19.566932, 33, 1, 0, 0, '', 0),
  (128, 'Business', 1288.846923, 271.008483, 19.554687, 6, 1, 0, 0, '', 0)
ON DUPLICATE KEY UPDATE `BusinessID` = `BusinessID`;
