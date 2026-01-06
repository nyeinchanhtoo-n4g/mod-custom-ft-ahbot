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

```sql
INSERT INTO `auctionhouse_custom_bot` (`item_id`, `price`, `enabled`) VALUES
(2589, 10000, 1),  -- Linen Cloth, 1 gold
(4306, 50000, 1);  -- Silk Cloth, 5 gold
```

## Configuration

Config file: `conf/mod_custom_ft_ahbot.conf`

အဓိက settings:
- `CustomFT_AuctionHouseBot.EnableSeller = true` - Seller bot ကို enable လုပ်ရန်
- `CustomFT_AuctionHouseBot.OwnerGUID = 1` - Bot character GUID
- `CustomFT_AuctionHouseBot.MinutesBetweenSellCycle = 10080` - 1 week (7 days * 24 hours * 60 minutes)

## In-Game Commands

GM commands:
- `.customft_ahbot reload` - Configuration ကို reload လုပ်ရန်
- `.customft_ahbot empty` - Auction house မှ bot auctions အားလုံးကို ဖယ်ရှားရန်
- `.customft_ahbot update` - Immediate update cycle run လုပ်ရန်
- `.customft_ahbot help` - Help message ပြရန်

## Notes

- Bot သည် `auctionhouse_custom_bot` table မှ item တွေကိုသာ ရောင်းချသည်
- Server မှ item တွေကို ရောင်းချမည် မဟုတ်ပါ
- Buyer bot ကို future use အတွက် prepare လုပ်ထားသော်လည်း လက်ရှိတွင် disabled ဖြစ်သည်
- Reload time သည် 1 week (10080 minutes) ဖြစ်သည်

## Credits

- Based on mod-ah-bot-plus-master by Nathan Handley
- Customized for FT Server requirements

