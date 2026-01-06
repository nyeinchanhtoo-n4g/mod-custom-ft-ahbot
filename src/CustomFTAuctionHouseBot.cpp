/*
 * Custom FT Auction House Bot Implementation
 * Based on mod-ah-bot-plus-master
 * This bot only sells items from the auctionhouse_custom_bot database table
 */

#include "ObjectMgr.h"
#include "AuctionHouseMgr.h"
#include "CustomFTAuctionHouseBot.h"
#include "Config.h"
#include "Player.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "ItemTemplate.h"
#include "SharedDefines.h"
#include "Log.h"

#include <set>
#include <unordered_map>

using namespace std;

CustomFTAuctionHouseBot::CustomFTAuctionHouseBot() :
    debug_Out(false),
    debug_Out_Filters(false),
    SellingBotEnabled(false),
    BuyingBotEnabled(false),
    CyclesBetweenSellActionMin(10080),
    CyclesBetweenSellAction(10080),
    CyclesBetweenSellActionMax(10080),
    MaxBuyoutPriceInCopper(1000000000),
    BuyoutVariationReducePercent(0.15f),
    BuyoutVariationAddPercent(0.25f),
    BidVariationHighReducePercent(0),
    BidVariationLowReducePercent(0.25f),
    ListingExpireTimeInSecondsMin(900),
    ListingExpireTimeInSecondsMax(86400),
    BuyingBotAcceptablePriceModifier(1),
    BuyingBotAlwaysBidMaxCalculatedPrice(false),
    BuyingBotWillBidAgainstPlayers(false),
    PreventOverpayingForVendorItems(true),
    BuyingBotBuyCandidatesPerBuyCycleMin(1),
    BuyingBotBuyCandidatesPerBuyCycleMax(1),
    AHCharactersGUIDsForQuery(""),
    ItemsPerCycle(150),
    OwnerGUID(1),
    LastBuyCycleCount(0),
    LastSellCycleCount(0)
{
    AllianceConfig = FactionSpecificAuctionHouseConfig(2);
    HordeConfig = FactionSpecificAuctionHouseConfig(6);
    NeutralConfig = FactionSpecificAuctionHouseConfig(7);
}

CustomFTAuctionHouseBot::~CustomFTAuctionHouseBot()
{
}

bool CustomFTAuctionHouseBot::IsModuleEnabled()
{
    return sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.EnableSeller", false);
}

void CustomFTAuctionHouseBot::InitializeConfiguration()
{
    debug_Out = sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.DEBUG", false);
    debug_Out_Filters = sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.DEBUG_FILTERS", false);

    SellingBotEnabled = sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.EnableSeller", true);
    BuyingBotEnabled = sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.Buyer.Enabled", false);

    std::string sellCyclesConfigString = sConfigMgr->GetOption<std::string>("CustomFT_AuctionHouseBot.MinutesBetweenSellCycle", "10080");
    GetConfigMinAndMax(sellCyclesConfigString, CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);
    CyclesBetweenSellAction = urand(CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);

    MaxBuyoutPriceInCopper = sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.MaxBuyoutPriceInCopper", 1000000000);
    BuyoutVariationReducePercent = sConfigMgr->GetOption<float>("CustomFT_AuctionHouseBot.BuyoutVariationReducePercent", 0.15f);
    BuyoutVariationAddPercent = sConfigMgr->GetOption<float>("CustomFT_AuctionHouseBot.BuyoutVariationAddPercent", 0.25f);
    BidVariationHighReducePercent = sConfigMgr->GetOption<float>("CustomFT_AuctionHouseBot.BidVariationHighReducePercent", 0);
    BidVariationLowReducePercent = sConfigMgr->GetOption<float>("CustomFT_AuctionHouseBot.BidVariationLowReducePercent", 0.25f);

    ListingExpireTimeInSecondsMin = sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.ListingExpireTimeInSecondsMin", 900);
    ListingExpireTimeInSecondsMax = sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.ListingExpireTimeInSecondsMax", 86400);

    ItemsPerCycle = sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.ItemsPerCycle", 150);
    OwnerGUID = sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.OwnerGUID", 1);

    // Buyer bot settings (prepared for future use)
    BuyingBotAcceptablePriceModifier = sConfigMgr->GetOption<float>("CustomFT_AuctionHouseBot.Buyer.AcceptablePriceModifier", 1);
    BuyingBotAlwaysBidMaxCalculatedPrice = sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.Buyer.AlwaysBidMaxCalculatedPrice", false);
    BuyingBotWillBidAgainstPlayers = sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.Buyer.BidAgainstPlayers", false);
    PreventOverpayingForVendorItems = sConfigMgr->GetOption<bool>("CustomFT_AuctionHouseBot.Buyer.PreventOverpayingForVendorItems", true);
    std::string candidatesPerCycleString = sConfigMgr->GetOption<string>("CustomFT_AuctionHouseBot.Buyer.BuyCandidatesPerBuyCycle", "1");
    GetConfigMinAndMax(candidatesPerCycleString, BuyingBotBuyCandidatesPerBuyCycleMin, BuyingBotBuyCandidatesPerBuyCycleMax);

    // Faction-specific settings
    if (!sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        AllianceConfig.SetMinItems(sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.Alliance.MinItems", 15000));
        AllianceConfig.SetMaxItems(sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.Alliance.MaxItems", 15000));

        HordeConfig.SetMinItems(sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.Horde.MinItems", 15000));
        HordeConfig.SetMaxItems(sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.Horde.MaxItems", 15000));
    }
    NeutralConfig.SetMinItems(sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.Neutral.MinItems", 15000));
    NeutralConfig.SetMaxItems(sConfigMgr->GetOption<uint32>("CustomFT_AuctionHouseBot.Neutral.MaxItems", 15000));

    // Load character
    AddCharacter(OwnerGUID);

    // Load custom items from database
    LoadCustomItemsFromDatabase();
}

void CustomFTAuctionHouseBot::AddCharacter(uint32 characterGUID)
{
    AHCharacters.clear();
    if (characterGUID == 0)
    {
        LOG_ERROR("module", "CustomFTAuctionHouseBot: Invalid character GUID (0). Be sure to set CustomFT_AuctionHouseBot.OwnerGUID");
        return;
    }

    QueryResult queryResult = CharacterDatabase.Query("SELECT `guid`, `account` FROM `characters` WHERE guid = {}", characterGUID);
    if (!queryResult || queryResult->GetRowCount() == 0)
    {
        LOG_ERROR("module", "CustomFTAuctionHouseBot: Character GUID {} not found in character database.", characterGUID);
        return;
    }

    Field* fields = queryResult->Fetch();
    uint32 guid = fields[0].Get<uint32>();
    uint32 account = fields[1].Get<uint32>();
    CustomFTAuctionHouseBotCharacter curChar = CustomFTAuctionHouseBotCharacter(account, guid);
    AHCharacters.push_back(curChar);
    AHCharactersGUIDsForQuery = std::to_string(guid);
}

void CustomFTAuctionHouseBot::LoadCustomItemsFromDatabase()
{
    CustomItemIDs.clear();
    CustomItemPrices.clear();
    CustomItemBidPrices.clear();
    CustomItemBuyoutPrices.clear();
    CustomItemStackCounts.clear();
    CustomItemMaxAmounts.clear();

    QueryResult result = CharacterDatabase.Query("SELECT `item_id`, `price`, `bid_price`, `buyout_price`, `stack_count`, `max_amount` FROM `auctionhouse_custom_bot` WHERE `enabled` = 1");
    if (!result)
    {
        LOG_ERROR("module", "CustomFTAuctionHouseBot: No items found in auctionhouse_custom_bot table or table does not exist.");
        return;
    }

    uint32 itemCount = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 itemID = fields[0].Get<uint32>();
        uint64 price = fields[1].Get<uint64>();
        uint64 bidPrice = fields[2].Get<uint64>();
        uint64 buyoutPrice = fields[3].Get<uint64>();
        uint32 stackCount = fields[4].Get<uint32>();
        uint32 maxAmount = fields[5].Get<uint32>();

        // Verify item exists in item_template
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemID);
        if (!itemTemplate)
        {
            if (debug_Out)
                LOG_WARN("module", "CustomFTAuctionHouseBot: Item ID {} from database does not exist in item_template, skipping.", itemID);
            continue;
        }

        // Validate stack_count (must be at least 1)
        if (stackCount < 1)
            stackCount = 1;

        CustomItemIDs.push_back(itemID);
        CustomItemPrices[itemID] = price;
        CustomItemBidPrices[itemID] = bidPrice;
        CustomItemBuyoutPrices[itemID] = buyoutPrice;
        CustomItemStackCounts[itemID] = stackCount;
        CustomItemMaxAmounts[itemID] = maxAmount;
        itemCount++;
    } while (result->NextRow());

    LOG_INFO("module", "CustomFTAuctionHouseBot: Loaded {} items from auctionhouse_custom_bot database.", itemCount);
}

void CustomFTAuctionHouseBot::GetConfigMinAndMax(std::string config, uint32& min, uint32& max)
{
    size_t pos = config.find(':');
    if (pos != std::string::npos)
    {
        min = std::stoul(config.substr(0, pos));
        max = std::stoul(config.substr(pos + 1));

        if (min < 1)
            min = 1;
        if (max < min)
            max = min;
    }
    else
        min = max = std::stoul(config);
}

void CustomFTAuctionHouseBot::SetCyclesBetweenSell()
{
    std::string sellCyclesConfigString = sConfigMgr->GetOption<std::string>("CustomFT_AuctionHouseBot.MinutesBetweenSellCycle", "10080");
    GetConfigMinAndMax(sellCyclesConfigString, CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);
    CyclesBetweenSellAction = urand(CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);
}

uint32 CustomFTAuctionHouseBot::GetStackSizeForItem(uint32 itemID, ItemTemplate const* itemProto) const
{
    if (itemProto == NULL)
        return 1;

    // Check if stack_count is set in database
    auto stackIt = CustomItemStackCounts.find(itemID);
    if (stackIt != CustomItemStackCounts.end() && stackIt->second > 0)
    {
        uint32 dbStackCount = stackIt->second;
        // Ensure stack count doesn't exceed item's max stackable amount
        uint32 maxStackable = static_cast<uint32>(itemProto->Stackable);
        if (maxStackable > 0 && dbStackCount > maxStackable)
            return maxStackable;
        return dbStackCount;
    }

    // Default to stack size of 1 if not set in database
    return 1;
}

void CustomFTAuctionHouseBot::CalculateItemValue(uint32 itemID, uint64 basePrice, uint64& outBidPrice, uint64& outBuyoutPrice)
{
    // Check if buyout_price is set in database
    auto buyoutIt = CustomItemBuyoutPrices.find(itemID);
    if (buyoutIt != CustomItemBuyoutPrices.end() && buyoutIt->second > 0)
    {
        outBuyoutPrice = buyoutIt->second;
    }
    else
    {
        // Use base price and apply variation
        outBuyoutPrice = basePrice;
        outBuyoutPrice = urand(
            static_cast<uint64>(outBuyoutPrice * (1.0f - BuyoutVariationReducePercent)),
            static_cast<uint64>(outBuyoutPrice * (1.0f + BuyoutVariationAddPercent))
        );
    }

    // Avoid price overflows
    if (outBuyoutPrice > MaxBuyoutPriceInCopper)
        outBuyoutPrice = MaxBuyoutPriceInCopper;

    // Ensure minimum price of 1
    if (outBuyoutPrice == 0)
        outBuyoutPrice = 1;

    // Check if bid_price is set in database
    auto bidIt = CustomItemBidPrices.find(itemID);
    if (bidIt != CustomItemBidPrices.end() && bidIt->second > 0)
    {
        outBidPrice = bidIt->second;
    }
    else
    {
        // Calculate bid price based on variance against buyout price
        float sellVarianceBidPriceTopPercent = 1.0f - BidVariationHighReducePercent;
        float sellVarianceBidPriceBottomPercent = 1.0f - BidVariationLowReducePercent;
        outBidPrice = urand(
            static_cast<uint64>(sellVarianceBidPriceBottomPercent * outBuyoutPrice),
            static_cast<uint64>(sellVarianceBidPriceTopPercent * outBuyoutPrice)
        );
    }

    // Ensure bid price is at least 1
    if (outBidPrice == 0)
        outBidPrice = 1;

    // Ensure bid price is not higher than buyout price
    if (outBidPrice > outBuyoutPrice)
        outBidPrice = outBuyoutPrice;
}

uint32 CustomFTAuctionHouseBot::GetRandomItemIDForListing()
{
    if (CustomItemIDs.empty())
    {
        LOG_ERROR("module", "CustomFTAuctionHouseBot: No items available for listing (CustomItemIDs is empty)");
        SellingBotEnabled = false;
        return 0;
    }

    return CustomItemIDs[urand(0, CustomItemIDs.size() - 1)];
}

void CustomFTAuctionHouseBot::AddNewAuctions(std::vector<Player*> AHBPlayers, FactionSpecificAuctionHouseConfig *config)
{
    if (!SellingBotEnabled)
    {
        if (debug_Out)
            LOG_INFO("module", "CustomFTAuctionHouseBot: Seller disabled");
        return;
    }

    if (AHBPlayers.empty())
    {
        if (debug_Out)
            LOG_ERROR("module", "CustomFTAuctionHouseBot: No bot characters available");
        return;
    }

    uint32 minItems = config->GetMinItems();
    uint32 maxItems = config->GetMaxItems();

    if (maxItems == 0)
        return;

    AuctionHouseEntry const* ahEntry = sAuctionMgr->GetAuctionHouseEntryFromFactionTemplate(config->GetAHFID());
    if (!ahEntry)
    {
        return;
    }
    AuctionHouseObject* auctionHouse = sAuctionMgr->GetAuctionsMap(config->GetAHFID());
    if (!auctionHouse)
    {
        return;
    }

    uint32 currentAuctionItemListCount = auctionHouse->Getcount();
    if (currentAuctionItemListCount >= minItems)
    {
        if (debug_Out)
            LOG_INFO("module", "CustomFTAuctionHouseBot: Auctions above minimum, so no auctions will be listed this cycle");
        return;
    }

    if (currentAuctionItemListCount >= maxItems)
    {
        if (debug_Out)
            LOG_INFO("module", "CustomFTAuctionHouseBot: Auctions at or above maximum, so no auctions will be listed this cycle");
        return;
    }

    uint32 newItemsToListCount = 0;
    if ((maxItems - currentAuctionItemListCount) >= ItemsPerCycle)
        newItemsToListCount = ItemsPerCycle;
    else
        newItemsToListCount = (maxItems - currentAuctionItemListCount);

    if (debug_Out)
        LOG_INFO("module", "CustomFTAuctionHouseBot: Adding {} Auctions", newItemsToListCount);

    // Count current listings per item for max_amount checking using database query
    // Query auction IDs and look up item_template from AuctionEntry objects
    std::unordered_map<uint32, uint32> itemListingCounts;
    if (!AHCharactersGUIDsForQuery.empty() && auctionHouse)
    {
        std::string queryString = "SELECT id FROM auctionhouse WHERE itemowner IN ({}) AND houseid = {}";
        QueryResult countResult = CharacterDatabase.Query(queryString, AHCharactersGUIDsForQuery, config->GetAHID());
        if (countResult)
        {
            do
            {
                Field* fields = countResult->Fetch();
                uint32 auctionID = fields[0].Get<uint32>();
                AuctionEntry* auction = auctionHouse->GetAuction(auctionID);
                if (auction && auction->item_template > 0)
                {
                    itemListingCounts[auction->item_template]++;
                }
            } while (countResult->NextRow());
        }
    }

    uint32 itemsGenerated = 0;
    uint32 attempts = 0;
    const uint32 maxAttempts = newItemsToListCount * 10; // Prevent infinite loop

    while (itemsGenerated < newItemsToListCount && attempts < maxAttempts)
    {
        attempts++;

        uint32 itemID = GetRandomItemIDForListing();
        if (itemID == 0)
        {
            if (debug_Out)
                LOG_ERROR("module", "CustomFTAuctionHouseBot: GetRandomItemIDForListing() returned 0");
            break;
        }

        // Check max_amount limit
        auto maxAmountIt = CustomItemMaxAmounts.find(itemID);
        if (maxAmountIt != CustomItemMaxAmounts.end() && maxAmountIt->second > 0)
        {
            uint32 currentCount = itemListingCounts[itemID];
            if (currentCount >= maxAmountIt->second)
            {
                if (debug_Out)
                    LOG_INFO("module", "CustomFTAuctionHouseBot: Item ID {} has reached max_amount limit ({}), skipping.", itemID, maxAmountIt->second);
                continue;
            }
        }

        // Get price from database
        auto priceIt = CustomItemPrices.find(itemID);
        if (priceIt == CustomItemPrices.end())
        {
            if (debug_Out)
                LOG_ERROR("module", "CustomFTAuctionHouseBot: No price found for item ID {}", itemID);
            continue;
        }

        ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(itemID);
        if (!prototype)
        {
            if (debug_Out)
                LOG_ERROR("module", "CustomFTAuctionHouseBot: prototype == NULL for item ID {}", itemID);
            continue;
        }

        // Start transaction only after all validations pass
        auto trans = CharacterDatabase.BeginTransaction();

        Player* AHBplayer = AHBPlayers[urand(0, AHBPlayers.size() - 1)];

        Item* item = Item::CreateItem(itemID, 1, AHBplayer);
        if (item == NULL)
        {
            if (debug_Out)
                LOG_ERROR("module", "CustomFTAuctionHouseBot: Item::CreateItem() returned NULL");
            CharacterDatabase.CommitTransaction(trans); // Rollback by committing empty transaction
            break;
        }
        item->AddToUpdateQueueOf(AHBplayer);

        uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(itemID);
        if (randomPropertyId != 0)
            item->SetItemRandomProperties(randomPropertyId);

        // Determine price from database
        uint64 buyoutPrice = 0;
        uint64 bidPrice = 0;
        CalculateItemValue(itemID, priceIt->second, bidPrice, buyoutPrice);

        // Define a duration
        uint32 etime = urand(ListingExpireTimeInSecondsMin, ListingExpireTimeInSecondsMax);

        // Set stack size from database
        uint32 stackCount = GetStackSizeForItem(itemID, prototype);
        item->SetCount(stackCount);

        uint32 dep = sAuctionMgr->GetAuctionDeposit(ahEntry, etime, item, stackCount);

        AuctionEntry* auctionEntry = new AuctionEntry();
        auctionEntry->Id = sObjectMgr->GenerateAuctionID();
        auctionEntry->houseId = AuctionHouseId(config->GetAHID());
        auctionEntry->item_guid = item->GetGUID();
        auctionEntry->item_template = item->GetEntry();
        auctionEntry->itemCount = item->GetCount();
        auctionEntry->owner = AHBplayer->GetGUID();
        auctionEntry->startbid = bidPrice * stackCount;
        auctionEntry->buyout = buyoutPrice * stackCount;
        auctionEntry->bid = 0;
        auctionEntry->deposit = dep;
        auctionEntry->expire_time = (time_t)etime + time(NULL);
        auctionEntry->auctionHouseEntry = ahEntry;
        item->SaveToDB(trans);
        item->RemoveFromUpdateQueueOf(AHBplayer);
        sAuctionMgr->AddAItem(item);
        auctionHouse->AddAuction(auctionEntry);
        auctionEntry->SaveToDB(trans);
        itemsGenerated++;
        itemListingCounts[itemID]++; // Update count after successful listing

        CharacterDatabase.CommitTransaction(trans);
    }

    if (debug_Out)
        LOG_INFO("module", "CustomFTAuctionHouseBot: Added {} items", itemsGenerated);
}

void CustomFTAuctionHouseBot::AddNewAuctionBuyerBotBid(std::vector<Player*> /*AHBPlayers*/, FactionSpecificAuctionHouseConfig* /*config*/)
{
    // Buyer bot functionality - prepared for future use
    // Currently disabled, but structure is ready
    if (!BuyingBotEnabled)
        return;

    // TODO: Implement buyer bot logic when needed
    // This would involve:
    // 1. Finding player auctions
    // 2. Calculating acceptable prices
    // 3. Bidding or buying out items
}

void CustomFTAuctionHouseBot::Update()
{
    if (AHCharacters.empty() == true)
        return;

    if ((SellingBotEnabled == false) && (BuyingBotEnabled == false))
        return;

    LastBuyCycleCount++;
    LastSellCycleCount++;

    bool buyReady = false;
    bool sellReady = false;

    // Check if an update cycle has been hit
    if (LastSellCycleCount >= CyclesBetweenSellAction)
    {
        LastSellCycleCount = 0;
        CyclesBetweenSellAction = urand(CyclesBetweenSellActionMin, CyclesBetweenSellActionMax);
        sellReady = true;
    }

    // Only update if a Buy or Sell update cycle has been hit
    if (!buyReady && !sellReady)
        return;

    // Load all AH Bot Players
    std::vector<std::pair<std::unique_ptr<Player>, std::unique_ptr<WorldSession>>> AHBPlayers;
    AHBPlayers.reserve(AHCharacters.size());
    for (uint32 botIndex = 0; botIndex < AHCharacters.size(); ++botIndex)
    {
        CurrentBotCharGUID = AHCharacters[botIndex].CharacterGUID;
        std::string accountName = "CustomFTAuctionHouseBot" + std::to_string(AHCharacters[botIndex].AccountID);

        auto session = std::make_unique<WorldSession>(
            AHCharacters[botIndex].AccountID, std::move(accountName), 0, nullptr,
            SEC_PLAYER, sWorld->getIntConfig(CONFIG_EXPANSION), 0, LOCALE_enUS, 0, false, false, 0
        );
        auto player = std::make_unique<Player>(session.get());
        player->Initialize(AHCharacters[botIndex].CharacterGUID);
        ObjectAccessor::AddObject(player.get());
        AHBPlayers.emplace_back(std::move(player), std::move(session));
    }

    // Create a vector of Player* for passing to methods
    std::vector<Player*> playersPointerVector;
    playersPointerVector.reserve(AHBPlayers.size());
    for (const auto& pair : AHBPlayers)
        playersPointerVector.emplace_back(pair.first.get());

    // List New Auctions
    if (sellReady)
    {
        if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION) == false)
        {
            AddNewAuctions(playersPointerVector, &AllianceConfig);
            AddNewAuctions(playersPointerVector, &HordeConfig);
        }
        AddNewAuctions(playersPointerVector, &NeutralConfig);
    }

    // Place New Bids (prepared for future use)
    if (buyReady && BuyingBotEnabled && BuyingBotBuyCandidatesPerBuyCycleMin > 0)
    {
        if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION) == false)
        {
            AddNewAuctionBuyerBotBid(playersPointerVector, &AllianceConfig);
            AddNewAuctionBuyerBotBid(playersPointerVector, &HordeConfig);
        }
        AddNewAuctionBuyerBotBid(playersPointerVector, &NeutralConfig);
    }

    // Clean up expired items
    CleanupExpiredAuctionItems();
}

void CustomFTAuctionHouseBot::EmptyAuctionHouses()
{
    if (AHCharactersGUIDsForQuery.empty())
    {
        LOG_ERROR("module", "CustomFTAuctionHouseBot: No character GUIDs found when emptying Auction Houses via '.ahbot empty' .");
        return;
    }

    struct AuctionInfo {
        uint32 itemID {0};
        uint32 characterGUID {0};
        uint32 houseID {0};
    };
    vector<AuctionInfo> ahBotActiveAuctions;
    auto trans = CharacterDatabase.BeginTransaction();

    std::string queryString = "SELECT id, buyguid, houseid FROM auctionhouse WHERE itemowner IN ({})";
    QueryResult result = CharacterDatabase.Query(queryString, AHCharactersGUIDsForQuery);
    if (!result)
        return;

    if (result->GetRowCount() > 0)
    {
        do
        {
            Field* fields = result->Fetch();
            AuctionInfo ai = {
                fields[0].Get<uint32>(),
                fields[1].Get<uint32>(),
                fields[2].Get<uint32>()
            };
            ahBotActiveAuctions.push_back(ai);
        } while (result->NextRow());

        AuctionHouseObject* auctionHouse;
        for (auto iter = ahBotActiveAuctions.begin(); iter != ahBotActiveAuctions.end(); ++iter)
        {
            AuctionInfo& ai = *iter;

            auctionHouse = nullptr;
            switch (ai.houseID) {
                case 2: auctionHouse = sAuctionMgr->GetAuctionsMap(AllianceConfig.GetAHFID()); break;
                case 6: auctionHouse = sAuctionMgr->GetAuctionsMap(HordeConfig.GetAHFID()); break;
                case 7: auctionHouse = sAuctionMgr->GetAuctionsMap(NeutralConfig.GetAHFID()); break;
            }
            if (auctionHouse == nullptr)
                continue;

            AuctionEntry* auction = auctionHouse->GetAuction(ai.itemID);
            if (!auction)
                continue;

            if (ai.characterGUID != 0)
                sAuctionMgr->SendAuctionCancelledToBidderMail(auction, trans);

            Item::DeleteFromDB(trans, auction->item_guid.GetCounter());

            auction->DeleteFromDB(trans);
            sAuctionMgr->RemoveAItem(auction->item_guid);
            auctionHouse->RemoveAuction(auction);
        }
    }

    CharacterDatabase.CommitTransaction(trans);
}

void CustomFTAuctionHouseBot::CleanupExpiredAuctionItems()
{
    // Cleanup expired auction items if needed
    // This is a placeholder for future implementation
}

