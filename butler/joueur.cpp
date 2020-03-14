  #include "joueur_player.h"

  int player_win_prob[PLAYER_MAX_HAUTEUR][PLAYER_MAX_LARGEUR] =
                          {{ 3, 4,  5,  7,  5, 4, 3},
                           { 4, 6,  8, 10,  8, 6, 4},
                           { 5, 8, 11, 13, 11, 8, 5},
                           { 5, 8, 11, 13, 11, 8, 5},
                           { 4, 6,  8, 10,  8, 6, 4},
                           { 3, 4,  5,  7,  5, 4, 3}};

  void Player::Observateur::add_node() { _nodes++; }
  void Player::Observateur::add_PLAYER_cutoff() { _PLAYER_cutoffs++; }
  void Player::Observateur::add_beta_cutoff() { _beta_cutoffs++; }
  void Player::Observateur::show() {
      auto self = this;
      std::thread th([self] () {
          auto after = std::chrono::system_clock::now();
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after - self->_timestamp_before);
          auto ms = duration.count();

          std::cout << "----------------PLAYER-------------------\n";
          std::cout << "took:          " << ms << "ms\n";
          std::cout << "nodes:         " << self->_nodes << "\n";
          std::cout << "PLAYER_cutoffs: " << self->_PLAYER_cutoffs << "\n";
          std::cout << "beta_cutoffs:  " << self->_beta_cutoffs << "\n";
          std::cout << "---------------------------------------\n";
      });

      th.detach();
  }

  void Player::Observateur::reset() {
      _nodes = 0;
      _PLAYER_cutoffs = 0;
      _beta_cutoffs = 0;
      _timestamp_before = std::chrono::system_clock::now();
  }

  Player::VirtualGame::VirtualGame(Plateau const & p) : _eval(0) {
      for (int x = 0; x < PLAYER_MAX_LARGEUR; x++) {
          _heights[x] = p._hauteur[x];

          for (int y = 0; y < _heights[x]; y++) {
              _map[x][y] = (((p._pions)[x]) & (1 << y));

              _eval += mask(x, y) * player_win_prob[y][x];
          }
      }

      update_plays();
  }

  std::vector<int> const & Player::VirtualGame::get_plays() {
      return _plays;
  }

  void Player::VirtualGame::update_plays() {
      _state = PLAYER_STATE_ONGOING;
      _plays.clear();

      int count_rows_ones[PLAYER_MAX_HAUTEUR] = {0};
      int count_rows_zeros[PLAYER_MAX_HAUTEUR] = {0};

      for (int x = 0; x < PLAYER_MAX_LARGEUR; x++) {
          if (_heights[x] < PLAYER_MAX_HAUTEUR) _plays.push_back(x);

          int count_columns_zeros = 0;
          int count_columns_ones  = 0;

          for (int y = 0; y < PLAYER_MAX_HAUTEUR; y++) {
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
                  _state = PLAYER_STATE_MINIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_columns_ones == 4) {
                  _state = PLAYER_STATE_MAXIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_rows_zeros[y] == 4) {
                  _state = PLAYER_STATE_MINIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_rows_ones[y] == 4) {
                  _state = PLAYER_STATE_MAXIMIZING_PLAYER_WIN;
                  return;
              }
          }
      }
  }


  void Player::VirtualGame::play(int x, bool maximizingPlayer) {
      int y = _heights[x];
      _map[x][y] = maximizingPlayer;
      _heights[x]++;
      _eval += mask(x, y) * player_win_prob[y][x];
      update_plays();
  }

  void Player::VirtualGame::unplay(int x) {
      int y = _heights[x] - 1;
      _eval -= mask(x, y) * player_win_prob[y][x];
      _heights[x]--;
      update_plays();
  }

  bool Player::VirtualGame::ended() {
      return (_state != PLAYER_STATE_ONGOING);
  }

  int Player::VirtualGame::mask(int x, int y) {
      if (y >= _heights[x]) return 0;
      else if (_map[x][y]) return 1;
           else return -1;
  }

  Player::Player(std::string nom, bool joueur)
  :Joueur(nom,joueur)
  {}

  void Player::init_vgame(Jeu & j) {
      if (vgame != nullptr) vgame.reset();
      vgame = std::make_unique<Player::VirtualGame>((*(j.plateau())));
  }


  char Player::nom_abbrege() const
  {
      return 'A';
  }

  void Player::recherche_coup(Jeu jeu, int & c)
  {
      observateur.reset();
      init_vgame(jeu);

      auto val = alphabeta(PLAYER_BETA_DEPTH, -infinity, infinity, true);
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

  Player::eval_play Player::alphabeta(int depth, int alpha, int beta, bool maximizingPlayer) {
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
              auto ab = alphabeta(depth - 1, alpha, beta, false);
              vgame->unplay(play);

              // update `coup`
              // value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
              if (ab.first > value) {
                  coup  = play;
                  value = ab.first;
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
              auto ab = alphabeta(depth - 1, alpha, beta, true);
              vgame->unplay(play);

              // update `coup`
              // value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
              if (ab.first < value) {
                  coup  = play;
                  value = ab.first;
              }

              // β := min(β, value)
              beta = std::min(beta, value);

              // if α ≥ β then
              if (alpha >= beta) {
                  // increment number of alpha cutoffs
                  observateur.add_PLAYER_cutoff();

                  // break (* α cut-off *)
                  break;
              }
          }

          // return value
          return eval_play(value, coup);
      }
  }

  int Player::evaluation(bool maximizingPlayer) {
      if (vgame->ended()) {
          switch (vgame->get_state()) {
              case PLAYER_STATE_MAXIMIZING_PLAYER_WIN:
                  return infinity;
              case PLAYER_STATE_MINIMIZING_PLAYER_WIN:
                  return -infinity;
              case PLAYER_STATE_DRAW:
                  return -1;
          }
      }

      return vgame->get_eval();
  }
