#ifndef LINEAGE_INST_IMPL_H
#define LINEAGE_INST_IMPL_H

#include "LineageExp.h"

void LineageExp::ConfigSGP_InstLib() {
  // Configure the instruction set.
  // - Default instruction set.
  sgp_inst_lib->AddInst("Inc", SGP__hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  sgp_inst_lib->AddInst("Dec", SGP__hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  sgp_inst_lib->AddInst("Not", SGP__hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  sgp_inst_lib->AddInst("Add", SGP__hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
  sgp_inst_lib->AddInst("Sub", SGP__hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
  sgp_inst_lib->AddInst("Mult", SGP__hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
  sgp_inst_lib->AddInst("Div", SGP__hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
  sgp_inst_lib->AddInst("Mod", SGP__hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
  sgp_inst_lib->AddInst("TestEqu", SGP__hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
  sgp_inst_lib->AddInst("TestNEqu", SGP__hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
  sgp_inst_lib->AddInst("TestLess", SGP__hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  sgp_inst_lib->AddInst("If", SGP__hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  sgp_inst_lib->AddInst("While", SGP__hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  sgp_inst_lib->AddInst("Countdown", SGP__hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
  sgp_inst_lib->AddInst("Close", SGP__hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  sgp_inst_lib->AddInst("Break", SGP__hardware_t::Inst_Break, 0, "Break out of current block.");
  sgp_inst_lib->AddInst("Call", SGP__hardware_t::Inst_Call, 0, "Call function that best matches call affinity.", emp::ScopeType::BASIC, 0, {"affinity"});
  sgp_inst_lib->AddInst("Return", SGP__hardware_t::Inst_Return, 0, "Return from current function if possible.");
  sgp_inst_lib->AddInst("SetMem", SGP__hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  sgp_inst_lib->AddInst("CopyMem", SGP__hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
  sgp_inst_lib->AddInst("SwapMem", SGP__hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
  sgp_inst_lib->AddInst("Input", SGP__hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
  sgp_inst_lib->AddInst("Output", SGP__hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
  sgp_inst_lib->AddInst("Commit", SGP__hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
  sgp_inst_lib->AddInst("Pull", SGP__hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
  sgp_inst_lib->AddInst("Nop", SGP__hardware_t::Inst_Nop, 0, "No operation.");
  // - Non-default instruction set.
  sgp_inst_lib->AddInst("Fork",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_Fork(hw, inst); },
                        0, "Generate internally-handled signal tagged with instuction's tag (spawns another thread w/local memory as new thread's input memory).");
  sgp_inst_lib->AddInst("GetBoardWidth",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_GetBoardWidth(hw, inst); },
                        1, "WM[ARG1] = Othello board width");
  sgp_inst_lib->AddInst("EndTurn",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP_Inst_EndTurn(hw, inst); },
                        0, "End current othello turn");
  sgp_inst_lib->AddInst("SetMoveXY",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_SetMoveXY(hw, inst); },
                        2, "MoveXY = (WM[ARG1], WM[ARG2])");
  sgp_inst_lib->AddInst("SetMoveID",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_SetMoveID(hw, inst); },
                        1, "MoveID = (WM[ARG1])");
  sgp_inst_lib->AddInst("GetMoveXY",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetMoveXY(hw, inst); },
                        2, "WM[ARG1] = Current moveX; WM[ARG2] = Current moveY");
  sgp_inst_lib->AddInst("GetMoveID",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetMoveID(hw, inst); },
                        1, "WM[ARG1] = Current moveID");
  sgp_inst_lib->AddInst("IsValidXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidXY_HW(hw, inst); },
                        3, "WM[ARG3] = IsValidMoveXY(WM[ARG1], WM[ARG2])");
  sgp_inst_lib->AddInst("IsValidID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidID_HW(hw, inst); },
                        2, "WM[ARG2] = IsValidMoveID(WM[ARG1])");
  sgp_inst_lib->AddInst("IsValidOppXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidOppXY_HW(hw, inst); },
                        3, "WM[ARG3] = IsValidOppMoveXY(WM[ARG1], WM[ARG2])");
  sgp_inst_lib->AddInst("IsValidOppID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsValidOppID_HW(hw, inst); },
                        2, "WM[ARG2] = IsValidOppMoveID(WM[ARG1])");
  sgp_inst_lib->AddInst("AdjacentXY",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_AdjacentXY(hw, inst); },
                        3, "Adjusts WM[ARG1], WM[ARG2] to give adjacent location (X,Y) in direction specified by WM[ARG3]");
  sgp_inst_lib->AddInst("AdjacentID",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_AdjacentID(hw, inst); },
                        2, "Adjusts WM[ARG1] to give adjacent location (ID) in direction specified by WM[ARG2]");
  sgp_inst_lib->AddInst("ValidMoveCnt-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_ValidMoveCnt_HW(hw, inst); },
                        1, "WM[ARG1] = Number of valid moves on active othello hardware board");
  sgp_inst_lib->AddInst("ValidOppMoveCnt-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_ValidOppMoveCnt_HW(hw, inst); },
                        1, "WM[ARG1] = Number of opponent's valid moves on active othello hardware board");
  sgp_inst_lib->AddInst("GetBoardValueXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetBoardValueXY_HW(hw, inst); },
                        3, "WM[ARG3] = owner of position X (WM[ARG1]), Y (WM[ARG2]) on othello hardware board");
  sgp_inst_lib->AddInst("GetBoardValueID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_GetBoardValueID_HW(hw, inst); },
                        2, "WM[ARG2] = owner of position ID (WM[ARG1]) on othello hardware board");
  sgp_inst_lib->AddInst("PlaceDiskXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceDiskXY_HW(hw, inst); },
                        3, "Place disk (of own type) on hardware board at position X (WM[ARG1]), Y (WM[ARG2]). Only successful if valid. WM[ARG3] is used to indicate move success.");
  sgp_inst_lib->AddInst("PlaceDiskID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceDiskID_HW(hw, inst); },
                        2, "Place disk (of own type) on hardware board at position ID (WM[ARG1]). Only successful if valid. WM[ARG3] is used to indicate move success");
  sgp_inst_lib->AddInst("PlaceOppDiskXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceOppDiskXY_HW(hw, inst); },
                        3, "Place disk (of opponent's type) on hardware board at position X (WM[ARG1]), Y (WM[ARG2]). Only successful if valid. WM[ARG3] is used to indicate move success");
  sgp_inst_lib->AddInst("PlaceOppDiskID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_PlaceOppDiskID_HW(hw, inst); },
                        2, "Place disk (of opponent's type) on hardware board at position ID (WM[ARG1]). Only successful if valid. WM[ARG3] is used to indicate move success");
  sgp_inst_lib->AddInst("FlipCntXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_FlipCntXY_HW(hw, inst); },
                        3, "WM[ARG3] = Number of disk flips if agent places disk at X (WM[ARG1]), Y (WM[ARG2])");
  sgp_inst_lib->AddInst("FlipCntID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_FlipCntID_HW(hw, inst); },
                        2, "WM[ARG3] = Number of disk flips if agent places disk at ID (WM[ARG1])");
  sgp_inst_lib->AddInst("OppFlipCntXY-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_OppFlipCntXY_HW(hw, inst); },
                        3, "WM[ARG3] = Number of disk flips if agent's opponent places disk at X (WM[ARG1]), Y (WM[ARG2])");
  sgp_inst_lib->AddInst("OppFlipCntID-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_OppFlipCntID_HW(hw, inst); },
                        2, "WM[ARG3] = Number of disk flips if agent's opponent places disk at ID (WM[ARG1])");
  sgp_inst_lib->AddInst("FrontierCnt-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_FrontierCnt_HW(hw, inst); },
                        1, "WM[ARG1] = Agent's frontier count on othello hardware board");
  sgp_inst_lib->AddInst("ResetBoard-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_ResetBoard_HW(hw, inst); },
                        0, "Reset active othello hardware board.");
  sgp_inst_lib->AddInst("IsOver-HW",
                        [this](SGP__hardware_t & hw, const SGP__inst_t & inst) { this->SGP__Inst_IsOver_HW(hw, inst); },
                        1, "Is game over on active othello hardware board?");
}


void LineageExp::ConfigAGP_InstLib() {
  agp_inst_lib->AddInst("Dec", AGP__inst_lib_t::Inst_Dec, 1, "Decrement value in reg Arg1");
  agp_inst_lib->AddInst("Not", AGP__inst_lib_t::Inst_Not, 1, "Logically toggle value in reg Arg1");
  agp_inst_lib->AddInst("SetReg", AGP__inst_lib_t::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
  agp_inst_lib->AddInst("Add", AGP__inst_lib_t::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
  agp_inst_lib->AddInst("Sub", AGP__inst_lib_t::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
  agp_inst_lib->AddInst("Mult", AGP__inst_lib_t::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
  agp_inst_lib->AddInst("Div", AGP__inst_lib_t::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
  agp_inst_lib->AddInst("Inc", AGP__inst_lib_t::Inst_Inc, 1, "Increment value in reg Arg1");
  agp_inst_lib->AddInst("Mod", AGP__inst_lib_t::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
  agp_inst_lib->AddInst("TestEqu", AGP__inst_lib_t::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
  agp_inst_lib->AddInst("TestNEqu", AGP__inst_lib_t::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
  agp_inst_lib->AddInst("TestLess", AGP__inst_lib_t::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
  agp_inst_lib->AddInst("If", AGP__inst_lib_t::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
  agp_inst_lib->AddInst("While", AGP__inst_lib_t::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
  agp_inst_lib->AddInst("Countdown", AGP__inst_lib_t::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
  agp_inst_lib->AddInst("Break", AGP__inst_lib_t::Inst_Break, 1, "Break out of scope Arg1");
  agp_inst_lib->AddInst("Scope", AGP__inst_lib_t::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
  agp_inst_lib->AddInst("Define", AGP__inst_lib_t::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
  agp_inst_lib->AddInst("Call", AGP__inst_lib_t::Inst_Call, 1, "Call previously defined function Arg1");
  agp_inst_lib->AddInst("Push", AGP__inst_lib_t::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
  agp_inst_lib->AddInst("Pop", AGP__inst_lib_t::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
  agp_inst_lib->AddInst("Input", AGP__inst_lib_t::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
  agp_inst_lib->AddInst("Output", AGP__inst_lib_t::Inst_Output, 2, "Push reg Arg1 into output Arg2");
  agp_inst_lib->AddInst("CopyVal", AGP__inst_lib_t::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
  agp_inst_lib->AddInst("ScopeReg", AGP__inst_lib_t::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");
  agp_inst_lib->AddInst("Nop", [](AGP__hardware_t & hw, const AGP__inst_t & inst){ ; }, 0, "No operation.");

  // - Non-default instruction set.
  agp_inst_lib->AddInst("GetBoardWidth",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_GetBoardWidth(hw, inst); },
                        1, "...");
  agp_inst_lib->AddInst("EndTurn",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_EndTurn(hw, inst); },
                        0, "...");
  agp_inst_lib->AddInst("SetMoveXY",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_SetMoveXY(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("SetMoveID",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_SetMoveID(hw, inst); },
                        1, "...");
  agp_inst_lib->AddInst("GetMoveXY",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_GetMoveXY(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("GetMoveID",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_GetMoveID(hw, inst); },
                        1, "...");
  agp_inst_lib->AddInst("IsValidOppXY-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_IsValidOppXY_HW(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("IsValidOppID-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_IsValidOppID_HW(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("IsValidXY-HW",
                        [this](AGP__hardware_t &hw, const AGP__inst_t &inst) { this->AGP__Inst_IsValidXY_HW(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("IsValidID-HW",
                        [this](AGP__hardware_t &hw, const AGP__inst_t &inst) { this->AGP__Inst_IsValidID_HW(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("AdjacentXY",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_AdjacentXY(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("AdjacentID",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_AdjacentID(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("ValidMoveCnt-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_ValidMoveCnt_HW(hw, inst); },
                        1, "...");
  agp_inst_lib->AddInst("ValidOppMoveCnt-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_ValidOppMoveCnt_HW(hw, inst); },
                        1, "...");
  agp_inst_lib->AddInst("GetBoardValueXY-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_GetBoardValueXY_HW(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("GetBoardValueID-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_GetBoardValueID_HW(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("PlaceDiskXY-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_PlaceDiskXY_HW(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("PlaceDiskID-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_PlaceDiskID_HW(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("PlaceOppDiskXY-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_PlaceOppDiskXY_HW(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("PlaceOppDiskID-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_PlaceOppDiskID_HW(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("FlipCntXY-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_FlipCntXY_HW(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("FlipCntID-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_FlipCntID_HW(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("OppFlipCntXY-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_OppFlipCntXY_HW(hw, inst); },
                        3, "...");
  agp_inst_lib->AddInst("OppFlipCntID-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_OppFlipCntID_HW(hw, inst); },
                        2, "...");
  agp_inst_lib->AddInst("FrontierCnt-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_FrontierCnt_HW(hw, inst); },
                        1, "...");
  agp_inst_lib->AddInst("ResetBoard-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_ResetBoard_HW(hw, inst); },
                        0, "...");
  agp_inst_lib->AddInst("IsOver-HW",
                        [this](AGP__hardware_t & hw, const AGP__inst_t & inst) { this->AGP__Inst_IsOver_HW(hw, inst); },
                        1, "...");
}

// --- SGP instruction implementations ---
// SGP__Inst_Fork
void LineageExp::SGP__Inst_Fork(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  hw.SpawnCore(inst.affinity, hw.GetMinBindThresh(), state.local_mem);
}
// SGP_Inst_GetBoardWidth
void LineageExp::SGP_Inst_GetBoardWidth(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  state.SetLocal(inst.args[0], OTHELLO_BOARD_WIDTH);
}
// SGP_Inst_EndTurn
void LineageExp::SGP_Inst_EndTurn(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  hw.SetTrait(TRAIT_ID__DONE, 1);
}
// SGP__Inst_SetMoveXY
void LineageExp::SGP__Inst_SetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const othello_idx_t move(move_x, move_y);
  hw.SetTrait(TRAIT_ID__MOVE, move.pos);
}
// SGP__Inst_SetMoveID
void LineageExp::SGP__Inst_SetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  const size_t move_id = (size_t)state.GetLocal(inst.args[0]);
  hw.SetTrait(TRAIT_ID__MOVE, move_id);
}
// SGP__Inst_GetMoveXY
void LineageExp::SGP__Inst_GetMoveXY(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  const othello_idx_t move = GetOthelloIndex((size_t)hw.GetTrait(TRAIT_ID__MOVE));
  state.SetLocal(inst.args[0], move.x());
  state.SetLocal(inst.args[1], move.y());
}
// SGP__Inst_GetMoveID
void LineageExp::SGP__Inst_GetMoveID(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  const size_t move_id = hw.GetTrait(TRAIT_ID__MOVE);
  state.SetLocal(inst.args[0], move_id);
}
// SGP__Inst_IsValidXY
void LineageExp::SGP__Inst_IsValidXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const size_t move_x = state.GetLocal(inst.args[0]);
  const size_t move_y = state.GetLocal(inst.args[1]);
  const int valid = (int)dreamboard.IsValidMove(playerID, {move_x, move_y});
  state.SetLocal(inst.args[2], valid);
}
// SGP__Inst_IsValidID_HW
void LineageExp::SGP__Inst_IsValidID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const size_t move_id = state.GetLocal(inst.args[0]);
  const int valid = (int)dreamboard.IsValidMove(playerID, GetOthelloIndex(move_id));
  state.SetLocal(inst.args[1], valid);
}
// SGP__Inst_IsValidOppXY
void LineageExp::SGP__Inst_IsValidOppXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  const size_t move_x = state.GetLocal(inst.args[0]);
  const size_t move_y = state.GetLocal(inst.args[1]);
  const int valid = (int)dreamboard.IsValidMove(oppID, {move_x, move_y});
  state.SetLocal(inst.args[2], valid);
}
// SGP__Inst_IsValidOppID
void LineageExp::SGP__Inst_IsValidOppID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  const size_t move_id = state.GetLocal(inst.args[0]);
  const int valid = (int)dreamboard.IsValidMove(oppID, GetOthelloIndex(move_id));
  state.SetLocal(inst.args[1], valid);
}
// SGP__Inst_AdjacentXY
void LineageExp::SGP__Inst_AdjacentXY(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const facing_t dir  = IntToFacing(state.GetLocal(inst.args[2]));
  const othello_idx_t neighbor = dreamboard.GetNeighbor({move_x, move_y}, dir);
  if (!neighbor.IsValid()) {
    state.SetLocal(inst.args[0], AGENT_VIEW__ILLEGAL_ID);
    state.SetLocal(inst.args[1], AGENT_VIEW__ILLEGAL_ID);
  } else {
    state.SetLocal(inst.args[0], neighbor.x());
    state.SetLocal(inst.args[1], neighbor.y());
  }
}
// SGP__Inst_AdjacentID
void LineageExp::SGP__Inst_AdjacentID(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = (size_t)state.GetLocal(inst.args[0]);
  const facing_t dir = IntToFacing(state.GetLocal(inst.args[1]));
  const othello_idx_t neighbor = dreamboard.GetNeighbor(GetOthelloIndex(move_id), dir);
  state.SetLocal(inst.args[0], (!neighbor.IsValid()) ? AGENT_VIEW__ILLEGAL_ID : neighbor.pos);
}
// SGP_Inst_ValidMoveCnt_HW
void LineageExp::SGP__Inst_ValidMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  state.SetLocal(inst.args[0], dreamboard.GetMoveOptions(playerID).size());
}
// SGP_Inst_ValidOppMoveCnt_HW
void LineageExp::SGP__Inst_ValidOppMoveCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  state.SetLocal(inst.args[0], dreamboard.GetMoveOptions(oppID).size());
}
// SGP_Inst_GetBoardValueXY_HW
void LineageExp::SGP__Inst_GetBoardValueXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = state.GetLocal(inst.args[0]);
  const size_t move_y = state.GetLocal(inst.args[1]);
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  // If inputs are garbage, let the caller know.
  if (move.IsValid()) {
    const player_t owner = dreamboard.GetPosOwner({move_x, move_y});
    if (owner == playerID) { state.SetLocal(inst.args[2], AGENT_VIEW__SELF_ID); }
    else if (owner == oppID) { state.SetLocal(inst.args[2], AGENT_VIEW__OPP_ID); }
    else { state.SetLocal(inst.args[2], AGENT_VIEW__OPEN_ID); }
  } else {
    state.SetLocal(inst.args[2], AGENT_VIEW__ILLEGAL_ID);
  }
}
// SGP_Inst_GetBoardValueID_HW
void LineageExp::SGP__Inst_GetBoardValueID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_id = state.GetLocal(inst.args[0]);
  const othello_idx_t move(GetOthelloIndex(move_id));
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  // If inputs are garbage, let the caller know.
  if (move.IsValid()) {
    const player_t owner = dreamboard.GetPosOwner(move_id);
    if (owner == playerID) { state.SetLocal(inst.args[1], AGENT_VIEW__SELF_ID); }
    else if (owner == oppID) { state.SetLocal(inst.args[1], AGENT_VIEW__OPP_ID); }
    else { state.SetLocal(inst.args[1], AGENT_VIEW__OPEN_ID); }
  } else {
    state.SetLocal(inst.args[1], AGENT_VIEW__ILLEGAL_ID);
  }
}
// SGP_Inst_PlaceDiskXY_HW
void LineageExp::SGP__Inst_PlaceDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  if (dreamboard.IsValidMove(playerID, move)) {
    dreamboard.DoMove(playerID, move);
    state.SetLocal(inst.args[2], 1);
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_PlaceDiskID_HW
void LineageExp::SGP__Inst_PlaceDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex(state.GetLocal(inst.args[0]));
  const player_t playerID = othello_dreamware->GetPlayerID();
  if (dreamboard.IsValidMove(playerID, move)) {
    dreamboard.DoMove(playerID, move);
    state.SetLocal(inst.args[1], 1);
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_PlaceOppDiskXY_HW
void LineageExp::SGP__Inst_PlaceOppDiskXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move)) {
    dreamboard.DoMove(oppID, move);
    state.SetLocal(inst.args[2], 1);
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_PlaceOppDiskID_HW
void LineageExp::SGP__Inst_PlaceOppDiskID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex(state.GetLocal(inst.args[0]));
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move)) {
    dreamboard.DoMove(oppID, move);
    state.SetLocal(inst.args[1], 1);
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_FlipCntXY_HW
void LineageExp::SGP__Inst_FlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  if (dreamboard.IsValidMove(playerID, move)) {
    state.SetLocal(inst.args[2], dreamboard.GetFlipCount(playerID, move));
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_FlipCntID_HW
void LineageExp::SGP__Inst_FlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex((size_t)state.GetLocal(inst.args[0]));
  const player_t playerID = othello_dreamware->GetPlayerID();
  if (dreamboard.IsValidMove(playerID, move)) {
    state.SetLocal(inst.args[1], dreamboard.GetFlipCount(playerID, move));
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_OppFlipCntXY_HW
void LineageExp::SGP__Inst_OppFlipCntXY_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)state.GetLocal(inst.args[0]);
  const size_t move_y = (size_t)state.GetLocal(inst.args[1]);
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move)) {
    state.SetLocal(inst.args[2], dreamboard.GetFlipCount(oppID, move));
  } else {
    state.SetLocal(inst.args[2], 0);
  }
}
// SGP_Inst_OppFlipCntID_HW
void LineageExp::SGP__Inst_OppFlipCntID_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex((size_t)state.GetLocal(inst.args[0]));
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move)) {
    state.SetLocal(inst.args[1], dreamboard.GetFlipCount(oppID, move));
  } else {
    state.SetLocal(inst.args[1], 0);
  }
}
// SGP_Inst_FrontierCnt_HW
void LineageExp::SGP__Inst_FrontierCnt_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  state.SetLocal(inst.args[0], dreamboard.CountFrontierPos(playerID));
}
// SGP_Inst_ResetBoard_HW
void LineageExp::SGP__Inst_ResetBoard_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  othello_dreamware->ResetActive(testcases[cur_testcase].GetInput().game);
}
// SGP_Inst_IsOver_HW
void LineageExp::SGP__Inst_IsOver_HW(SGP__hardware_t & hw, const SGP__inst_t & inst) {
  SGP__state_t & state = hw.GetCurState();
  othello_t & dreamboard = othello_dreamware->GetActiveDreamOthello();
  state.SetLocal(inst.args[0], (int)dreamboard.IsOver());
}


// AGP_Inst_GetBoardWidth
void LineageExp::AGP__Inst_GetBoardWidth(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  hw.regs[inst.args[0]] = OTHELLO_BOARD_WIDTH;
}
// AGP_Inst_EndTurn
void LineageExp::AGP__Inst_EndTurn(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  hw.SetTrait(TRAIT_ID__DONE, 1);
}
// AGP__Inst_SetMoveXY
void LineageExp::AGP__Inst_SetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const othello_idx_t move(move_x, move_y);
  hw.SetTrait(TRAIT_ID__MOVE, move.pos);
}
// AGP__Inst_SetMoveID
void LineageExp::AGP__Inst_SetMoveID(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  const size_t move_id = hw.regs[inst.args[0]];
  hw.SetTrait(TRAIT_ID__MOVE, move_id);
}
// AGP__Inst_GetMoveXY
void LineageExp::AGP__Inst_GetMoveXY(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  const othello_idx_t move = GetOthelloIndex(hw.GetTrait(TRAIT_ID__MOVE));
  hw.regs[inst.args[0]] =  move.x();
  hw.regs[inst.args[1]] =  move.y();
}
// AGP__Inst_GetMoveID
void LineageExp::AGP__Inst_GetMoveID(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  const size_t move_id = hw.GetTrait(TRAIT_ID__MOVE);
  hw.regs[inst.args[0]] = move_id;
}
// AGP__Inst_IsValidXY
void LineageExp::AGP__Inst_IsValidXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const int valid = (int)dreamboard.IsValidMove(playerID, {move_x, move_y});
  hw.regs[inst.args[2]] = valid;
}
// AGP__Inst_IsValidID_HW
void LineageExp::AGP__Inst_IsValidID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const othello_idx_t move = GetOthelloIndex(hw.regs[inst.args[0]]);
  const int valid = (int)dreamboard.IsValidMove(playerID, move);
  hw.regs[inst.args[1]] = valid;
}
// AGP__Inst_IsValidXY
void LineageExp::AGP__Inst_IsValidOppXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = dreamboard.GetOpponent(othello_dreamware->GetPlayerID());
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const int valid = (int)dreamboard.IsValidMove(playerID, {move_x, move_y});
  hw.regs[inst.args[2]] = valid;
}
// AGP__Inst_IsValidID_HW
void LineageExp::AGP__Inst_IsValidOppID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = dreamboard.GetOpponent(othello_dreamware->GetPlayerID());
  const othello_idx_t move = GetOthelloIndex(hw.regs[inst.args[0]]);
  const int valid = (int)dreamboard.IsValidMove(playerID, move);
  hw.regs[inst.args[1]] = valid;
}
// AGP__Inst_AdjacentXY
void LineageExp::AGP__Inst_AdjacentXY(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const facing_t dir  = IntToFacing(hw.regs[inst.args[2]]);
  const othello_idx_t neighbor = dreamboard.GetNeighbor({move_x, move_y}, dir);
  if (!neighbor.IsValid()) {
    hw.regs[inst.args[0]] = AGENT_VIEW__ILLEGAL_ID;
    hw.regs[inst.args[1]] = AGENT_VIEW__ILLEGAL_ID;
  } else {
    hw.regs[inst.args[0]] = neighbor.x();
    hw.regs[inst.args[1]] = neighbor.y();
  }
}
// AGP__Inst_AdjacentID
void LineageExp::AGP__Inst_AdjacentID(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex(hw.regs[inst.args[0]]);
  const facing_t dir  = IntToFacing(hw.regs[inst.args[1]]);
  const othello_idx_t neighbor = dreamboard.GetNeighbor(move, dir);
  hw.regs[inst.args[0]] = (!neighbor.IsValid()) ? AGENT_VIEW__ILLEGAL_ID : neighbor.pos;
}
// AGP_Inst_ValidMoveCnt_HW
void LineageExp::AGP__Inst_ValidMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  hw.regs[inst.args[0]] = dreamboard.GetMoveOptions(playerID).size();
}
// AGP_Inst_ValidOppMoveCnt_HW
void LineageExp::AGP__Inst_ValidOppMoveCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  hw.regs[inst.args[0]] = dreamboard.GetMoveOptions(oppID).size();
}
// AGP_Inst_GetBoardValueXY_HW
void LineageExp::AGP__Inst_GetBoardValueXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = hw.regs[inst.args[0]];
  const size_t move_y = hw.regs[inst.args[1]];
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  // If inputs are garbage, let the caller know.
  if (move.IsValid()) {
    const player_t owner = dreamboard.GetPosOwner(move);
    if (owner == playerID) {
      hw.regs[inst.args[2]] = AGENT_VIEW__SELF_ID;
    } else if (owner == oppID) {
      hw.regs[inst.args[2]] = AGENT_VIEW__OPP_ID;
    } else {
      hw.regs[inst.args[2]] = AGENT_VIEW__OPEN_ID;
    }
  } else {
    hw.regs[inst.args[2]] = AGENT_VIEW__ILLEGAL_ID;
  }
}
// AGP_Inst_GetBoardValueID_HW
void LineageExp::AGP__Inst_GetBoardValueID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex(hw.regs[inst.args[0]]);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  // If inputs are garbage, let the caller know.
  if (move.IsValid()) {
    const player_t owner = dreamboard.GetPosOwner(move);
    if (owner == playerID) {
      hw.regs[inst.args[1]] = AGENT_VIEW__SELF_ID;
    } else if (owner == oppID) {
      hw.regs[inst.args[1]] = AGENT_VIEW__OPP_ID;
    } else {
      hw.regs[inst.args[1]] = AGENT_VIEW__OPEN_ID;
    }
  } else {
    hw.regs[inst.args[1]] = AGENT_VIEW__ILLEGAL_ID;
  }
}
// AGP_Inst_PlaceDiskXY_HW
void LineageExp::AGP__Inst_PlaceDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  if (dreamboard.IsValidMove(playerID, move)) {
    dreamboard.DoMove(playerID, move);
    hw.regs[inst.args[2]] = 1;
  } else {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_PlaceDiskID_HW
void LineageExp::AGP__Inst_PlaceDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  const othello_idx_t move = GetOthelloIndex(hw.regs[inst.args[0]]);
  if (dreamboard.IsValidMove(playerID, move)) {
    dreamboard.DoMove(playerID, move);
    hw.regs[inst.args[1]] = 1;
  } else {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_PlaceOppDiskXY_HW
void LineageExp::AGP__Inst_PlaceOppDiskXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move))
  {
    dreamboard.DoMove(oppID, move);
    hw.regs[inst.args[2]] = 1;
  }
  else
  {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_PlaceOppDiskID_HW
void LineageExp::AGP__Inst_PlaceOppDiskID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex((size_t)hw.regs[inst.args[0]]);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move))
  {
    dreamboard.DoMove(oppID, move);
    hw.regs[inst.args[1]] = 1;
  }
  else
  {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_FlipCntXY_HW
void LineageExp::AGP__Inst_FlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  if (dreamboard.IsValidMove(playerID, move))
  {
    hw.regs[inst.args[2]] = dreamboard.GetFlipCount(playerID, move);
  }
  else
  {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_FlipCntID_HW
void LineageExp::AGP__Inst_FlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex((size_t)hw.regs[inst.args[0]]);
  const player_t playerID = othello_dreamware->GetPlayerID();
  if (dreamboard.IsValidMove(playerID, move))
  {
    hw.regs[inst.args[1]] = dreamboard.GetFlipCount(playerID, move);
  }
  else
  {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_OppFlipCntXY_HW
void LineageExp::AGP__Inst_OppFlipCntXY_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const size_t move_x = (size_t)hw.regs[inst.args[0]];
  const size_t move_y = (size_t)hw.regs[inst.args[1]];
  const othello_idx_t move(move_x, move_y);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move))
  {
    hw.regs[inst.args[2]] = dreamboard.GetFlipCount(oppID, move);
  }
  else
  {
    hw.regs[inst.args[2]] = 0;
  }
}
// AGP_Inst_OppFlipCntID_HW
void LineageExp::AGP__Inst_OppFlipCntID_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const othello_idx_t move = GetOthelloIndex((size_t)hw.regs[inst.args[0]]);
  const player_t playerID = othello_dreamware->GetPlayerID();
  const player_t oppID = dreamboard.GetOpponent(playerID);
  if (dreamboard.IsValidMove(oppID, move))
  {
    hw.regs[inst.args[1]] = dreamboard.GetFlipCount(oppID, move);
  }
  else
  {
    hw.regs[inst.args[1]] = 0;
  }
}
// AGP_Inst_FrontierCnt_HW
void LineageExp::AGP__Inst_FrontierCnt_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  const player_t playerID = othello_dreamware->GetPlayerID();
  hw.regs[inst.args[0]] = dreamboard.CountFrontierPos(playerID);
}
// AGP_Inst_ResetBoard_HW
void LineageExp::AGP__Inst_ResetBoard_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_dreamware->ResetActive(testcases[cur_testcase].GetInput().game);
}
// AGP_Inst_IsOver_HW
void LineageExp::AGP__Inst_IsOver_HW(AGP__hardware_t &hw, const AGP__inst_t &inst)
{
  othello_t &dreamboard = othello_dreamware->GetActiveDreamOthello();
  hw.regs[inst.args[0]] = (int)dreamboard.IsOver();
}

#endif
