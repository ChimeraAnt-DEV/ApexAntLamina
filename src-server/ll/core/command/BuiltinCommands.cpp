#include "ll/core/command/BuiltinCommands.h"

#include <memory>

#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/Listener.h"
#include "ll/api/event/command/ServerCommandRegisterEvent.h"
#include "ll/api/event/server/ServerStoppingEvent.h"
#include "ll/api/service/Bedrock.h"

namespace ll::command {

ll::event::ListenerPtr serverRegisterListener;

bool isRegisterCommandsAvailable() { return ll::service::getCommandRegistry(false).has_value(); }

void registerCommands() {
    auto& bus = ll::event::EventBus::getInstance();

    if (!serverRegisterListener) {
        serverRegisterListener = bus.emplaceListener<ll::event::command::ServerCommandRegisterEvent>(
            [](ll::event::command::ServerCommandRegisterEvent&) {
                auto& registrar = CommandRegistrar::getServerInstance();
                registrar.clear();
                registerTpdimCommand(false);
                registerVersionCommand(false);
                registerCrashCommand(false);
                registerModManageCommand(false);
            }
        );
    }
}

} // namespace ll::command
