-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Nov 29, 2025 at 10:09 AM
-- Server version: 10.4.32-MariaDB
-- PHP Version: 8.2.12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `rfid_monitoring`
--

-- --------------------------------------------------------

--
-- Table structure for table `migrations`
--

CREATE TABLE `migrations` (
  `id` int(10) UNSIGNED NOT NULL,
  `migration` varchar(255) NOT NULL,
  `batch` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Dumping data for table `migrations`
--

INSERT INTO `migrations` (`id`, `migration`, `batch`) VALUES
(1, '2025_11_04_102402_create_rfids_table', 1);

-- --------------------------------------------------------

--
-- Table structure for table `rfids`
--

CREATE TABLE `rfids` (
  `id` bigint(20) UNSIGNED NOT NULL,
  `rfid_number` varchar(255) NOT NULL,
  `status` int(11) NOT NULL DEFAULT 0,
  `registered` tinyint(1) NOT NULL DEFAULT 0,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Dumping data for table `rfids`
--

INSERT INTO `rfids` (`id`, `rfid_number`, `status`, `registered`, `created_at`, `updated_at`) VALUES
(1, '22C19E7A', 0, 1, '2025-11-28 22:29:33', '2025-11-28 22:29:33'),
(2, 'FFC6AADE', 0, 1, '2025-11-28 22:29:33', '2025-11-28 22:29:33'),
(3, 'FFC6AADE', 1, 1, '2025-11-29 00:21:54', '2025-11-29 00:21:54'),
(4, 'FFC6AADE', 0, 1, '2025-11-29 00:23:15', '2025-11-29 00:23:15'),
(5, 'FFC6AADE', 1, 1, '2025-11-29 00:23:18', '2025-11-29 00:23:18'),
(6, '7DBC3C05', 0, 0, '2025-11-29 00:23:39', '2025-11-29 00:23:39'),
(7, 'FFC6AADE', 0, 1, '2025-11-29 00:23:44', '2025-11-29 00:23:44'),
(8, 'FFC6AADE', 1, 1, '2025-11-29 00:23:55', '2025-11-29 00:23:55'),
(9, '22C19E7A', 1, 1, '2025-11-29 00:24:20', '2025-11-29 00:24:20'),
(10, '22C19E7A', 0, 1, '2025-11-29 00:24:23', '2025-11-29 00:24:23'),
(11, '22C19E7A', 1, 1, '2025-11-29 00:24:25', '2025-11-29 00:24:25'),
(12, '22C19E7A', 0, 1, '2025-11-29 00:24:30', '2025-11-29 00:24:30'),
(13, '22C19E7A', 1, 1, '2025-11-29 00:24:31', '2025-11-29 00:24:31'),
(14, '22C19E7A', 0, 1, '2025-11-29 00:24:32', '2025-11-29 00:24:32'),
(15, '22C19E7A', 1, 1, '2025-11-29 00:24:33', '2025-11-29 00:24:33'),
(16, '22C19E7A', 0, 1, '2025-11-29 00:24:37', '2025-11-29 00:24:37'),
(17, '22C19E7A', 1, 1, '2025-11-29 00:24:39', '2025-11-29 00:24:39'),
(18, '22C19E7A', 0, 1, '2025-11-29 00:24:49', '2025-11-29 00:24:49'),
(19, '22C19E7A', 1, 1, '2025-11-29 00:24:51', '2025-11-29 00:24:51'),
(20, '22C19E7A', 0, 1, '2025-11-29 00:24:57', '2025-11-29 00:24:57'),
(21, '22C19E7A', 1, 1, '2025-11-29 00:24:58', '2025-11-29 00:24:58'),
(22, '22C19E7A', 0, 1, '2025-11-29 00:25:00', '2025-11-29 00:25:00');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `migrations`
--
ALTER TABLE `migrations`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `rfids`
--
ALTER TABLE `rfids`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `migrations`
--
ALTER TABLE `migrations`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `rfids`
--
ALTER TABLE `rfids`
  MODIFY `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=23;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
