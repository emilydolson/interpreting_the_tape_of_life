#ifndef OTHELLO_HW_H
#define OTHELLO_HW_H

#include "base/vector.h"
#include "games/Othello8.h"
#include "tools/random_utils.h"
#include "tools/math.h"
#include "tools/string_utils.h"

// NOTE: we don't actually need this for test case evaluations...
class OthelloHardware {

protected:
  using othello_t = emp::Othello8;
  using player_t = othello_t::Player;
  emp::vector<othello_t> dreams; ///< Let's lean into that whole 'othello dream' terminology...
  size_t active_dream;
  player_t playerID;

public:
  OthelloHardware(size_t dream_cnt, player_t pID=player_t::DARK)
  : dreams(dream_cnt), active_dream(0), playerID(pID)
  { emp_assert(dream_cnt > 0); }

  othello_t & GetActiveDreamOthello() { return dreams[active_dream]; }

  void SetActiveDream(size_t id) {
    emp_assert(id < dreams.size());
    active_dream = id;
  }

  void SetPlayerID(player_t pID) { playerID = pID; }
  player_t GetPlayerID() const { return playerID; }

  void Reset() {
    for (size_t i = 0; i < dreams.size(); ++i) dreams[i].Reset();
  }

  void Reset(const othello_t & other) {
    for (size_t i = 0; i < dreams.size(); ++i) {
      dreams[i].Reset();
      dreams[i].SetBoard(other.GetBoard());
    }
  }

  void ResetActive() {
    dreams[active_dream].Reset();
  }

  void ResetActive(const othello_t & other) {
    dreams[active_dream].Reset();
    dreams[active_dream].SetBoard(other.GetBoard());
  }

};


#endif
