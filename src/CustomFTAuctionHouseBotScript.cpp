/*
 * Custom FT Auction House Bot Script
 * Based on mod-ah-bot-plus-master
 */

#include "Chat.h"
#include "ScriptMgr.h"
#include "CustomFTAuctionHouseBot.h"
#include "Log.h"
#include "Mail.h"
#include "Player.h"
#include "WorldSession.h"

class CustomFTAHBot_WorldScript : public WorldScript
{
private:
    bool HasPerformedStartup;

public:
    CustomFTAHBot_WorldScript() : WorldScript("CustomFTAHBot_WorldScript"), HasPerformedStartup(false) { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        if (!customft_auctionbot->IsModuleEnabled())
            return;

        customft_auctionbot->InitializeConfiguration();
        if (HasPerformedStartup == true)
        {
            LOG_INFO("server.loading", "CustomFTAuctionHouseBot: Reloading custom items from database...");
            customft_auctionbot->LoadCustomItemsFromDatabase();
        }
    }

    void OnStartup() override
    {
        if (!customft_auctionbot->IsModuleEnabled())
            return;

        LOG_INFO("server.loading", "CustomFTAuctionHouseBot: Loading custom items from database...");
        customft_auctionbot->LoadCustomItemsFromDatabase();
        HasPerformedStartup = true;
    }
};

class CustomFTAHBot_AuctionHouseScript : public AuctionHouseScript
{
public:
    CustomFTAHBot_AuctionHouseScript() : AuctionHouseScript("CustomFTAHBot_AuctionHouseScript") { }

    void OnBeforeAuctionHouseMgrSendAuctionSuccessfulMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* /*auction*/, Player* owner, uint32& /*owner_accId*/, uint32& /*profit*/, bool& sendNotification, bool& updateAchievementCriteria, bool& /*sendMail*/) override
    {
        if (owner)
        {
            bool isAHBot = false;
            for (CustomFTAuctionHouseBotCharacter character : customft_auctionbot->AHCharacters)
            {
                if (character.CharacterGUID == owner->GetGUID().GetCounter())
                {
                    isAHBot = true;
                    break;
                }
            }
            if (isAHBot == true)
            {
                sendNotification = false;
                updateAchievementCriteria = false;
            }
        }
    }

    void OnBeforeAuctionHouseMgrSendAuctionExpiredMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* /*auction*/, Player* owner, uint32& /*owner_accId*/, bool& sendNotification, bool& sendMail) override
    {
        if (owner)
        {
            bool isAHBot = false;
            for (CustomFTAuctionHouseBotCharacter character : customft_auctionbot->AHCharacters)
            {
                if (character.CharacterGUID == owner->GetGUID().GetCounter())
                {
                    isAHBot = true;
                    break;
                }
            }
            if (isAHBot == true)
            {
                sendNotification = false;
                sendMail = false; // Don't return expired items to bot
            }
        }
    }

    void OnBeforeAuctionHouseMgrSendAuctionOutbiddedMail(AuctionHouseMgr* /*auctionHouseMgr*/, AuctionEntry* auction, Player* oldBidder, uint32& /*oldBidder_accId*/, Player* newBidder, uint32& newPrice, bool& /*sendNotification*/, bool& /*sendMail*/) override
    {
        if (oldBidder && !newBidder)
            oldBidder->GetSession()->SendAuctionBidderNotification((uint32)auction->GetHouseId(), auction->Id, ObjectGuid::Create<HighGuid::Player>(customft_auctionbot->CurrentBotCharGUID), newPrice, auction->GetAuctionOutBid(), auction->item_template);
    }

    void OnBeforeAuctionHouseMgrUpdate() override
    {
        customft_auctionbot->Update();
    }
};

class CustomFTAHBot_MailScript : public MailScript
{
public:
    CustomFTAHBot_MailScript() : MailScript("CustomFTAHBot_MailScript") { }

    void OnBeforeMailDraftSendMailTo(MailDraft* /*mailDraft*/, MailReceiver const& receiver, MailSender const& sender, MailCheckMask& /*checked*/, uint32& /*deliver_delay*/, uint32& /*custom_expiration*/, bool& deleteMailItemsFromDB, bool& sendMail) override
    {
        bool isAHBot = false;
        for (CustomFTAuctionHouseBotCharacter character : customft_auctionbot->AHCharacters)
        {
            if (character.CharacterGUID == receiver.GetPlayerGUIDLow())
            {
                isAHBot = true;
                break;
            }
        }
        if (isAHBot == true)
        {
            if (sender.GetMailMessageType() == MAIL_AUCTION)
                deleteMailItemsFromDB = true;
            sendMail = false;
        }
    }
};

class CustomFTAHBot_CommandScript : public CommandScript
{
public:
    CustomFTAHBot_CommandScript() : CommandScript("CustomFTAHBot_CommandScript") { }

    Acore::ChatCommands::ChatCommandTable GetCommands() const override
    {
        static Acore::ChatCommands::ChatCommandTable CustomFTAHBotCommandTable = {
            {"update", HandleCustomFTAHBotUpdateCommand, SEC_GAMEMASTER, Acore::ChatCommands::Console::Yes},
            {"reload", HandleCustomFTAHBotReloadCommand, SEC_GAMEMASTER, Acore::ChatCommands::Console::Yes},
            {"empty",  HandleCustomFTAHBotEmptyCommand,  SEC_GAMEMASTER, Acore::ChatCommands::Console::Yes},
            {"help",  HandleCustomFTAHBotHelpCommand,  SEC_GAMEMASTER, Acore::ChatCommands::Console::Yes}
        };

        static Acore::ChatCommands::ChatCommandTable commandTable = {
            {"ftahbot", CustomFTAHBotCommandTable},
        };

        return commandTable;
    }

    static bool HandleCustomFTAHBotUpdateCommand(ChatHandler* handler, const char* /*args*/)
    {
        LOG_INFO("module", "CustomFTAuctionHouseBot: Updating Auction House...");
        handler->PSendSysMessage("CustomFTAuctionHouseBot: Updating Auction House...");
        CustomFTAuctionHouseBot::instance()->Update();
        LOG_INFO("module", "CustomFTAuctionHouseBot: Auction House Updated.");
        handler->PSendSysMessage("CustomFTAuctionHouseBot: Auction House Updated.");
        return true;
    }

    static bool HandleCustomFTAHBotReloadCommand(ChatHandler* handler, char const* /*args*/)
    {
        LOG_INFO("module", "CustomFTAuctionHouseBot: Reloading Config...");
        handler->PSendSysMessage("CustomFTAuctionHouseBot: Reloading Config...");

        sConfigMgr->LoadModulesConfigs(true, false);
        CustomFTAuctionHouseBot::instance()->InitializeConfiguration();

        LOG_INFO("module", "CustomFTAuctionHouseBot: Config reloaded.");
        handler->PSendSysMessage("CustomFTAuctionHouseBot: Config reloaded.");
        return true;
    }

    static bool HandleCustomFTAHBotEmptyCommand(ChatHandler* handler, char const* /*args*/)
    {
        LOG_INFO("module", "CustomFTAuctionHouseBot: Emptying Auction House...");
        handler->PSendSysMessage("CustomFTAuctionHouseBot: Emptying Auction House...");
        CustomFTAuctionHouseBot::instance()->EmptyAuctionHouses();
        CustomFTAuctionHouseBot::instance()->CleanupExpiredAuctionItems();
        LOG_INFO("module", "CustomFTAuctionHouseBot: Auction Houses Emptied.");
        handler->PSendSysMessage("CustomFTAuctionHouseBot: Auction Houses Emptied.");
        return true;
    }

    static bool HandleCustomFTAHBotHelpCommand(ChatHandler* handler, char const* /*args*/)
    {
        handler->PSendSysMessage("CustomFTAuctionHouseBot commands:");
        handler->PSendSysMessage("  .ftahbot reload - Reloads configuration");
        handler->PSendSysMessage("  .ftahbot empty  - Removes all CustomFTAuctionHouseBot auctions");
        handler->PSendSysMessage("  .ftahbot update - Runs an update cycle");
        return true;
    }
};

void AddCustomFTAHBotScripts()
{
    new CustomFTAHBot_WorldScript();
    new CustomFTAHBot_AuctionHouseScript();
    new CustomFTAHBot_MailScript();
    new CustomFTAHBot_CommandScript();
}

