// ll::service::inline bedrock impl for Android.
//
// The desktop server build installs constructor/hook registrations to track
// the singleton game objects (DedicatedServer, ServerNetworkHandler,
// CommandRegistry, Minecraft, ...). libminecraftpe.so on Android does not
// export those symbols to the mod process in the same way; instead the
// LeviLaunchroid preloader (libpreloader.so) owns the game objects and can
// publish them to the mod through its runtime bridge.
//
// This file keeps the same ll::service::getX() APIs but returns the
// pointers only when the launcher has published them (see
// launchcore_android in src-pl-android). Before the game starts every
// accessor returns nullopt, which LeviLamina callers already tolerate.

#include "ll/api/service/Bedrock.h"

#include <atomic>

#include "mc/deps/core/utility/optional_ref.h"

#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/NetworkSystem.h"
#include "mc/resources/ResourcePackRepository.h"
#include "mc/server/ServerInstance.h"
#include "mc/server/commands/CommandRegistry.h"
#include "mc/server/DedicatedServer.h"
#include "mc/world/Minecraft.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/storage/DBStorage.h"

namespace ll::service::inline bedrock {

namespace {
std::atomic<Minecraft*>            minecraft{};
std::atomic<Level*>                level{};
std::atomic<ServerNetworkHandler*> serverNetworkHandler{};
std::atomic<NetworkSystem*>        networkSystem{};
std::atomic<CommandRegistry*>      commandRegistry{};
std::atomic<ServerInstance*>       serverInstance{};
std::atomic<DBStorage*>            dbStorage{};
std::atomic<ResourcePackRepository*> resourcePackRepository{};
} // namespace

// Published by the launcher when it observes the corresponding game object
// lifetime events.
void publishGamePointers(
    Minecraft*               mc,
    Level*                   lvl,
    ServerNetworkHandler*    snh,
    NetworkSystem*           ns,
    CommandRegistry*         cr,
    ServerInstance*          si,
    DBStorage*               db,
    ResourcePackRepository*  rpr
) noexcept {
    minecraft.store(mc);
    level.store(lvl);
    serverNetworkHandler.store(snh);
    networkSystem.store(ns);
    commandRegistry.store(cr);
    serverInstance.store(si);
    dbStorage.store(db);
    resourcePackRepository.store(rpr);
}

void setDBStorage(DBStorage* storage) noexcept { dbStorage.store(storage); }

optional_ref<Minecraft> getMinecraft(bool) { return minecraft.load(); }

optional_ref<Level> getLevel() { return level.load(); }

optional_ref<ServerNetworkHandler> getServerNetworkHandler(bool) { return serverNetworkHandler.load(); }

optional_ref<NetworkSystem> getNetworkSystem(bool) { return networkSystem.load(); }

optional_ref<CommandRegistry> getCommandRegistry(bool) { return commandRegistry.load(); }

optional_ref<ServerInstance> getServerInstance() { return serverInstance.load(); }

optional_ref<DBStorage> getDBStorage() { return dbStorage.load(); }

optional_ref<ResourcePackRepository> getResourcePackRepository(bool) { return resourcePackRepository.load(); }

} // namespace ll::service::inline bedrock