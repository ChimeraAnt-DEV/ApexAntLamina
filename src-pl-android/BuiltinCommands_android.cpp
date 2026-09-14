// ll::command::registerCommands for Android.
//
// The desktop builds hook ServerScriptManager/command registration events to
// populate the command registry lazily. On Android those hooks are disabled
// (the game symbols are not present), so we register the LeviLamina builtin
// commands directly by publishing the same ServerCommandRegisterEvent the
// mods listen for. The CommandRegistry comes from the loading game process
// via service::getCommandRegistry; if the game has not started yet the
// command registration is skipped and mods are still loaded.

#include "ll/core/command/BuiltinCommands.h"

#include <memory>

#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/command/ServerCommandRegisterEvent.h"
#include "ll/api/service/Bedrock.h"

namespace ll::command {

bool isRegisterCommandsAvailable() { return ll::service::getCommandRegistry(false).has_value(); }

void registerCommands() {
    auto& registrar = CommandRegistrar::getServerInstance();
    registrar.clear();

    if (auto registry = ll::service::getCommandRegistry(false)) {
        registerTpdimCommand(false);
        registerVersionCommand(false);
        registerModManageCommand(false);

        event::ServerCommandRegisterEvent ev{*registry, false};
        event::EventBus::getInstance().publish(ev);
    } else {
        // Game has not started yet. LeviLaunchroid publishes the CommandRegistry
        // through ll::service::publishGamePointers once it exists, after which
        // registerCommands() is called again (see AndroidMain.cpp).
    }
}

} // namespace ll::command