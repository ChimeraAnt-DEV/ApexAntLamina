#pragma once

namespace ll::command {

void registerCommands();

bool isRegisterCommandsAvailable();

void registerVersionCommand(bool isClientSide = false);
void registerTpdimCommand(bool isClientSide = false);
void registerCrashCommand(bool isClientSide = false);
void registerModManageCommand(bool isClientSide = false);

} // namespace ll::command
