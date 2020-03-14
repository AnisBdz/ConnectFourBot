  #include "joueur_floki.h"

  int floki_win_prob[FLOKI_MAX_HAUTEUR][FLOKI_MAX_LARGEUR] =
                          {{ 3, 4,  5,  7,  5, 4, 3},
                           { 4, 6,  8, 10,  8, 6, 4},
                           { 5, 8, 11, 13, 11, 8, 5},
                           { 5, 8, 11, 13, 11, 8, 5},
                           { 4, 6,  8, 10,  8, 6, 4},
                           { 3, 4,  5,  7,  5, 4, 3}};



Floki::TranspositionTable::TranspositionTable() {
    for (int x = 0; x < FLOKI_MAX_LARGEUR; x++) {
        for (int y = 0; y < FLOKI_MAX_HAUTEUR; y++) {
            for (int m = 0; m < FLOKI_MAX_MASK; m++) {
                _hashes[x][y][m] = rand() % FLOKI_MEMORY_SIZE;
            }
        }
    }

    clear();
}

Floki::TranspositionTable::hash_t Floki::TranspositionTable::hash(bool map[FLOKI_MAX_LARGEUR][FLOKI_MAX_HAUTEUR], int heights[]) {
    hash_t h = 0;

    for (int x = 0; x < FLOKI_MAX_LARGEUR; x++) {
        for (int y = 0; y < FLOKI_MAX_HAUTEUR; y++) {
            int m;

            if (y >= heights[x]) m = 0;
            else if (map[x][y])  m = 1;
            else                 m = 2;

            h ^= _hashes[x][y][m];
        }
    }

    return h;
}


bool Floki::TranspositionTable::is_occupied(hash_t hash) {
    return _memory[h(hash)].occupied;
}

int  Floki::TranspositionTable::get_eval(hash_t hash) {
    return _memory[h(hash)].evaluation;
}

void Floki::TranspositionTable::store(hash_t hash, int eval) {
    _memory[h(hash)].evaluation = eval;
    _memory[h(hash)].occupied = true;
}

int Floki::TranspositionTable::h(hash_t hash) {
    return hash & 0xFFFF;
}

void Floki::TranspositionTable::clear() {
    for (int i = 0; i < FLOKI_MEMORY_SIZE; i++) {
        _memory[i].occupied = false;
    }
}

  void Floki::Observateur::add_node() { _nodes++; }
  void Floki::Observateur::add_FLOKI_cutoff() { _FLOKI_cutoffs++; }
  void Floki::Observateur::add_beta_cutoff() { _beta_cutoffs++; }
  void Floki::Observateur::show() {
      auto self = this;
      std::thread th([self] () {
          auto after = std::chrono::system_clock::now();
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after - self->_timestamp_before);
          auto ms = duration.count();

          std::cout << "----------------FLOKI-------------------\n";
          std::cout << "took:          " << ms << "ms\n";
          std::cout << "nodes:         " << self->_nodes << "\n";
          std::cout << "FLOKI_cutoffs: " << self->_FLOKI_cutoffs << "\n";
          std::cout << "beta_cutoffs:  " << self->_beta_cutoffs << "\n";
          std::cout << "---------------------------------------\n";
      });

      th.detach();
  }

  void Floki::Observateur::reset() {
      _nodes = 0;
      _FLOKI_cutoffs = 0;
      _beta_cutoffs = 0;
      _timestamp_before = std::chrono::system_clock::now();
  }

  Floki::VirtualGame::VirtualGame(Plateau const & p) : _eval(0) {
      for (int x = 0; x < FLOKI_MAX_LARGEUR; x++) {
          _heights[x] = p._hauteur[x];

          for (int y = 0; y < _heights[x]; y++) {
              _map[x][y] = (((p._pions)[x]) & (1 << y));

              _eval += mask(x, y) * floki_win_prob[y][x];
          }
      }

      update_plays();
  }

  std::vector<int> const & Floki::VirtualGame::get_plays() {
      return _plays;
  }

  void Floki::VirtualGame::update_plays() {
      _state = FLOKI_STATE_ONGOING;
      _plays.clear();

      int count_rows_ones[FLOKI_MAX_HAUTEUR] = {0};
      int count_rows_zeros[FLOKI_MAX_HAUTEUR] = {0};

      for (int x = 0; x < FLOKI_MAX_LARGEUR; x++) {
          if (_heights[x] < FLOKI_MAX_HAUTEUR) _plays.push_back(x);

          int count_columns_zeros = 0;
          int count_columns_ones  = 0;

          for (int y = 0; y < FLOKI_MAX_HAUTEUR; y++) {
              if (y >= _heights[x]) {
                  count_rows_ones[y] = 0;
                  count_rows_zeros[y] = 0;
                  continue;
              }

              if (!_map[x][y]) {
                  count_columns_zeros++;
                  count_columns_ones = 0;
                  count_rows_zeros[y]++;
                  count_rows_ones[y] = 0;
              }

              else {
                  count_columns_zeros = 0;
                  count_columns_ones++;
                  count_rows_zeros[y] = 0;
                  count_rows_ones[y]++;
              }

              if (count_columns_zeros == 4) {
                  _state = FLOKI_STATE_MINIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_columns_ones == 4) {
                  _state = FLOKI_STATE_MAXIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_rows_zeros[y] == 4) {
                  _state = FLOKI_STATE_MINIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_rows_ones[y] == 4) {
                  _state = FLOKI_STATE_MAXIMIZING_PLAYER_WIN;
                  return;
              }
          }
      }
  }


  void Floki::VirtualGame::play(int x, bool maximizingPlayer) {
      int y = _heights[x];
      _map[x][y] = maximizingPlayer;
      _heights[x]++;
      _eval += mask(x, y) * floki_win_prob[y][x];
      update_plays();
  }

  void Floki::VirtualGame::unplay(int x) {
      int y = _heights[x] - 1;
      _eval -= mask(x, y) * floki_win_prob[y][x];
      _heights[x]--;
      update_plays();
  }

  bool Floki::VirtualGame::ended() {
      return (_state != FLOKI_STATE_ONGOING);
  }

  int Floki::VirtualGame::mask(int x, int y) {
      if (y >= _heights[x]) return 0;
      else if (_map[x][y]) return 1;
           else return -1;
  }

  Floki::Floki(std::string nom, bool joueur)
  :Joueur(nom,joueur)
  {}

  void Floki::init_vgame(Jeu & j) {
      if (vgame != nullptr) vgame.reset();
      vgame = std::make_unique<Floki::VirtualGame>((*(j.plateau())));
  }


  char Floki::nom_abbrege() const
  {
      return 'A';
  }

  void Floki::recherche_coup(Jeu jeu, int & c)
  {
      observateur.reset();
      init_vgame(jeu);
      transposition_table.clear();

      auto val = alphabeta(FLOKI_BETA_DEPTH, -infinity, infinity, true);
      auto coup = val.second;

      c = 0;
      for (int i = 0; i < jeu.nb_coups(); i++) {
          if (jeu[i] == coup) {
              c = i;
              break;
          }
      }

      // observateur.show();
  }

  Floki::eval_play Floki::alphabeta(int depth, int alpha, int beta, bool maximizingPlayer) {
      // increment number of nodes
      observateur.add_node();

      // if depth = 0 or node is a terminal node then
      if (depth == 0 || vgame->ended()) {
          // return the heuristic value of node
          return eval_play(evaluation(maximizingPlayer), -1);
      }

      // if maximizingPlayer then
      if (maximizingPlayer) {
          // value := −∞
          int value = -infinity;
          int coup = -2;

          // for each child of node do
          for (auto play : vgame->get_plays()) {
              vgame->play(play, maximizingPlayer);

              int v;

              auto hash = transposition_table.hash(vgame->_map, vgame->_heights);
              if (transposition_table.is_occupied(hash)) {
                 v = transposition_table.get_eval(hash);
              }

              else {
                  auto ab = alphabeta(depth - 1, alpha, beta, false);
                  v = ab.first;
                  transposition_table.store(hash, ab.first);
              }

              vgame->unplay(play);

              // update `coup`
              // value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
              if (v > value) {
                  coup  = play;
                  value = v;
              }

              // α := max(α, value)
              alpha = std::max(alpha, value);

              // if α ≥ β then
              if (alpha >= beta) {
                  // increment number of beta cutoffs
                  observateur.add_beta_cutoff();

                  // break (* β cut-off *)
                  break;
              }
          }

          // return value
          return eval_play(value, coup);
      }

      // else
      else {
          // value := +∞
          int value = infinity;
          int coup = -3;

          // for each child of node do
          for (auto play : vgame->get_plays()) {
              vgame->play(play, maximizingPlayer);

              int v;

              auto hash = transposition_table.hash(vgame->_map, vgame->_heights);
              if (transposition_table.is_occupied(hash)) {
                v = transposition_table.get_eval(hash);
              }

              else {
                  auto ab = alphabeta(depth - 1, alpha, beta, true);
                  v = ab.first;
                  transposition_table.store(hash, ab.first);
              }

              vgame->unplay(play);

              // update `coup`
              // value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
              if (v < value) {
                  coup  = play;
                  value = v;
              }

              // β := min(β, value)
              beta = std::min(beta, value);

              // if α ≥ β then
              if (alpha >= beta) {
                  // increment number of alpha cutoffs
                  observateur.add_FLOKI_cutoff();

                  // break (* α cut-off *)
                  break;
              }
          }

          // return value
          return eval_play(value, coup);
      }
  }

  int Floki::evaluation(bool maximizingPlayer) {
      if (vgame->ended()) {
          switch (vgame->get_state()) {
              case FLOKI_STATE_MAXIMIZING_PLAYER_WIN:
                  return infinity;
              case FLOKI_STATE_MINIMIZING_PLAYER_WIN:
                  return -infinity;
              case FLOKI_STATE_DRAW:
                  return -1;
          }
      }

      return vgame->get_eval();
  }
