/*
 * Custom FT Auction House Bot
 * Based on mod-ah-bot-plus-master
 * This bot only sells items from the auctionhouse_custom_bot database table
 */

#ifndef CUSTOM_FT_AUCTION_HOUSE_BOT_H
#define CUSTOM_FT_AUCTION_HOUSE_BOT_H

#include "Common.h"
#include "ObjectGuid.h"

#include <map>
#include <vector>
#include <unordered_map>

struct AuctionEntry;
class Player;
class WorldSession;

#include "ItemTemplate.h"
#include "SharedDefines.h"

class FactionSpecificAuctionHouseConfig
{
private:
    uint32 AHID;
    uint32 AHFID;
    uint32 minItems;
    uint32 maxItems;

public:
    FactionSpecificAuctionHouseConfig(uint32 ahid)
    {
        AHID = ahid;
        switch(ahid)
        {
        case 2:
            AHFID = 55;
            break;
        case 6:
            AHFID = 29;
            break;
        case 7:
            AHFID = 120;
            break;
        default:
            AHFID = 120;
            break;
        }
    }
    FactionSpecificAuctionHouseConfig()
    {
    }
    uint32 GetAHID()
    {
        return AHID;
    }
    uint32 GetAHFID()
    {
        return AHFID;
    }
    void SetMinItems(uint32 value)
    {
        minItems = value;
    }
    uint32 GetMinItems()
    {
        if ((minItems == 0) && (maxItems))
            return maxItems;
        else if ((maxItems) && (minItems > maxItems))
            return maxItems;
        else
            return minItems;
    }
    void SetMaxItems(uint32 value)
    {
        maxItems = value;
    }
    uint32 GetMaxItems()
    {
        return maxItems;
    }
    ~FactionSpecificAuctionHouseConfig()
    {
    }
};

class CustomFTAuctionHouseBotCharacter
{
public:
    CustomFTAuctionHouseBotCharacter(uint32 accountID, uint32 characterGUID) :
        AccountID(accountID),
        CharacterGUID(characterGUID) { }
    uint32 AccountID;
    ObjectGuid::LowType CharacterGUID;
};

class CustomFTAuctionHouseBot
{
public:
    std::vector<CustomFTAuctionHouseBotCharacter> AHCharacters;
    uint32 CurrentBotCharGUID;

private:
    bool debug_Out;
    bool debug_Out_Filters;

    bool SellingBotEnabled;
    bool BuyingBotEnabled;
    uint32 CyclesBetweenSellActionMin;
    uint32 CyclesBetweenSellAction;
    uint32 CyclesBetweenSellActionMax;
    uint32 MaxBuyoutPriceInCopper;
    float BuyoutVariationReducePercent;
    float BuyoutVariationAddPercent;
    float BidVariationHighReducePercent;
    float BidVariationLowReducePercent;
    uint32 ListingExpireTimeInSecondsMin;
    uint32 ListingExpireTimeInSecondsMax;
    float BuyingBotAcceptablePriceModifier;
    bool BuyingBotAlwaysBidMaxCalculatedPrice;
    bool BuyingBotWillBidAgainstPlayers;
    bool PreventOverpayingForVendorItems;
    uint32 BuyingBotBuyCandidatesPerBuyCycleMin;
    uint32 BuyingBotBuyCandidatesPerBuyCycleMax;
    std::string AHCharactersGUIDsForQuery;
    uint32 ItemsPerCycle;
    uint32 OwnerGUID;

    // Custom database items and prices
    std::vector<uint32> CustomItemIDs;
    std::unordered_map<uint32, uint64> CustomItemPrices; // item_id -> base price in copper
    std::unordered_map<uint32, uint64> CustomItemBidPrices; // item_id -> bid price in copper (0 = use calculated)
    std::unordered_map<uint32, uint64> CustomItemBuyoutPrices; // item_id -> buyout price in copper (0 = use calculated)
    std::unordered_map<uint32, uint32> CustomItemStackCounts; // item_id -> stack count
    std::unordered_map<uint32, uint32> CustomItemMaxAmounts; // item_id -> max amount to list (0 = unlimited)

    FactionSpecificAuctionHouseConfig AllianceConfig;
    FactionSpecificAuctionHouseConfig HordeConfig;
    FactionSpecificAuctionHouseConfig NeutralConfig;

    uint32 LastBuyCycleCount;
    uint32 LastSellCycleCount;

    CustomFTAuctionHouseBot();

public:
    static CustomFTAuctionHouseBot* instance()
    {
        static CustomFTAuctionHouseBot instance;
        return &instance;
    }

    ~CustomFTAuctionHouseBot();

    void Update();
    bool IsModuleEnabled();
    void InitializeConfiguration();
    void EmptyAuctionHouses();
    void SetCyclesBetweenSell();
    void GetConfigMinAndMax(std::string config, uint32& min, uint32& max);
    void AddCharacter(uint32 characterGUID);
    uint32 GetStackSizeForItem(uint32 itemID, ItemTemplate const* itemProto) const;
    void CalculateItemValue(uint32 itemID, uint64 basePrice, uint64& outBidPrice, uint64& outBuyoutPrice);
    void LoadCustomItemsFromDatabase();
    uint32 GetRandomItemIDForListing();
    void AddNewAuctions(std::vector<Player*> AHBPlayers, FactionSpecificAuctionHouseConfig* config);
    void AddNewAuctionBuyerBotBid(std::vector<Player*> AHBPlayers, FactionSpecificAuctionHouseConfig* config);
    void CleanupExpiredAuctionItems();
};

#define customft_auctionbot CustomFTAuctionHouseBot::instance()

#endif

