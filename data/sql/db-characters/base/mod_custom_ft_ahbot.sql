-- Custom FT Auction House Bot Database Table
-- This table stores items and prices that the bot will sell
-- Only items in this table will be sold by the bot

DROP TABLE IF EXISTS `auctionhouse_custom_bot`;
CREATE TABLE `auctionhouse_custom_bot` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `item_id` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Item ID from item_template',
  `price` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Base price in copper (used if bid_price/buyout_price not set)',
  `bid_price` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Bid price in copper (0 = use calculated from price)',
  `buyout_price` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Buyout price in copper (0 = use calculated from price)',
  `stack_count` int(11) unsigned NOT NULL DEFAULT '1' COMMENT 'Stack count per auction',
  `max_amount` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Maximum amount of this item to list (0 = unlimited)',
  `enabled` tinyint(1) unsigned NOT NULL DEFAULT '1' COMMENT 'Whether this item is enabled for sale',
  PRIMARY KEY (`id`),
  UNIQUE KEY `item_id` (`item_id`),
  KEY `enabled` (`enabled`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='Custom FT Auction House Bot Items';

