#pragma once

void update(MineSweeper* m, nlohmann::json info);
void startGamefunction(int &col, int &row, int &mineNum);
void firstScriptTransfer(int col, int row, nlohmann::json& info);
void ingameTransfer(nlohmann::json* to_send, nlohmann::json* info, MineSweeper* m);