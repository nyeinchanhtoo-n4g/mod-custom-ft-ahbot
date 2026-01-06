# Custom FT Auction House Bot

Custom FT Auction House Bot သည် mod-ah-bot-plus-master ကို reference ယူပြီး ရေးထားသော custom auction house bot ဖြစ်သည်။

## အထူးလုပ်ဆောင်ချက်များ

- **Custom Database Table**: `auctionhouse_custom_bot` database table မှ item နဲ့ price တွေကိုသာ ရောင်းချသည်
- **Reload Time**: 1 week (10080 minutes) အတွင်း reload လုပ်သည်
- **Owner GUID**: Character GUID 1 ကို owner အဖြစ် အသုံးပြုသည်
- **Seller Only**: လက်ရှိတွင် seller functionality သာ အလုပ်လုပ်သည် (buyer bot ကို future use အတွက် prepare လုပ်ထားသည်)

## Installation

1. Module ကို AzerothCore `modules` directory အောက်တွင် ထားပါ
2. CMake ကို re-run လုပ်ပြီး clean build လုပ်ပါ
3. Character database တွင် SQL file ကို run ပါ:
   ```
   data/sql/db-characters/base/mod_custom_ft_ahbot.sql
   ```
4. Config file ကို copy လုပ်ပါ:
   ```
   cp conf/mod_custom_ft_ahbot.conf.dist conf/mod_custom_ft_ahbot.conf
   ```
5. Config file ကို edit လုပ်ပြီး `CustomFT_AuctionHouseBot.OwnerGUID` ကို character GUID 1 သို့ set လုပ်ပါ

## Database Setup

`auctionhouse_custom_bot` table တွင် item နဲ့ price တွေကို insert လုပ်ပါ:

### Table Fields:
- `item_id` - Item ID from item_template (required)
- `price` - Base price in copper (used if bid_price/buyout_price not set)
- `bid_price` - Bid price in copper (0 = use calculated from price with variation)
- `buyout_price` - Buyout price in copper (0 = use calculated from price with variation)
- `stack_count` - Stack count per auction (default: 1)
- `max_amount` - Maximum amount of this item to list (0 = unlimited)
- `enabled` - Whether this item is enabled for sale (1 = enabled, 0 = disabled)

### Example:

```sql
-- Simple example with base price only (bid/buyout will be calculated with variation)
INSERT INTO `auctionhouse_custom_bot` (`item_id`, `price`, `enabled`) VALUES
(2589, 10000, 1),  -- Linen Cloth, 1 gold base price
(4306, 50000, 1);  -- Silk Cloth, 5 gold base price

-- Advanced example with all fields
INSERT INTO `auctionhouse_custom_bot` (`item_id`, `price`, `bid_price`, `buyout_price`, `stack_count`, `max_amount`, `enabled`) VALUES
(2589, 10000, 8000, 12000, 5, 10, 1),  -- Linen Cloth: bid 80s, buyout 1g20s, stack 5, max 10 listings
(4306, 50000, 0, 0, 1, 0, 1);  -- Silk Cloth: use calculated prices, stack 1, unlimited listings
```

**Notes:**
- `bid_price` နဲ့ `buyout_price` က 0 ဖြစ်ရင် `price` ကို base အဖြစ် သုံးပြီး variation percentages တွေနဲ့ calculate လုပ်မယ်
- `stack_count` က item ရဲ့ max stackable amount ထက် မကျော်ရဘူး
- `max_amount` က 0 ဖြစ်ရင် unlimited listings ဖြစ်မယ်

## Configuration

Config file: `conf/mod_custom_ft_ahbot.conf`

အဓိက settings:
- `CustomFT_AuctionHouseBot.EnableSeller = true` - Seller bot ကို enable လုပ်ရန်
- `CustomFT_AuctionHouseBot.OwnerGUID = 1` - Bot character GUID
- `CustomFT_AuctionHouseBot.MinutesBetweenSellCycle = 10080` - 1 week (7 days * 24 hours * 60 minutes)

## In-Game Commands

GM commands:
- `.ftahbot reload` - Configuration ကို reload လုပ်ရန်
- `.ftahbot empty` - Auction house မှ bot auctions အားလုံးကို ဖယ်ရှားရန်
- `.ftahbot update` - Immediate update cycle run လုပ်ရန်
- `.ftahbot help` - Help message ပြရန်

## Notes

- Bot သည် `auctionhouse_custom_bot` table မှ item တွေကိုသာ ရောင်းချသည်
- Server မှ item တွေကို ရောင်းချမည် မဟုတ်ပါ
- Buyer bot ကို future use အတွက် prepare လုပ်ထားသော်လည်း လက်ရှိတွင် disabled ဖြစ်သည်
- Reload time သည် 1 week (10080 minutes) ဖြစ်သည်

## Credits

- Based on mod-ah-bot-plus-master by Nathan Handley
- Customized for FT Server requirements

