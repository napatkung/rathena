--
-- Table `market` for market shop persistency
--

CREATE TABLE IF NOT EXISTS `market` (
  `name` varchar(32) NOT NULL DEFAULT '',
  `nameid` SMALLINT(5) UNSIGNED NOT NULL,
  `amount` INT(11) UNSIGNED NOT NULL,
  PRIMARY KEY  (`name`,`nameid`)
) ENGINE = InnoDB;
