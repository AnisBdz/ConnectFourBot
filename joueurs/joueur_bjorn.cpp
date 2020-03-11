  #include "joueur_bjorn.h"

  int bjorn_win_prob[BJORN_MAX_HAUTEUR][BJORN_MAX_LARGEUR] =
                          {{ 3, 4,  5,  7,  5, 4, 3},
                           { 4, 6,  8, 10,  8, 6, 4},
                           { 5, 8, 11, 13, 11, 8, 5},
                           { 5, 8, 11, 13, 11, 8, 5},
                           { 4, 6,  8, 10,  8, 6, 4},
                           { 3, 4,  5,  7,  5, 4, 3}};

  bool Bjorn::Variation::activated = true;

  void Bjorn::Variation::load(std::stack<int> const & q) {
      _stack = std::stack<int>(q);
  }

  int Bjorn::Variation::pop() {
      if (_stack.empty()) return -1;

      int play = _stack.top();
      _stack.pop();
      return play;
  }

  void Bjorn::Variation::step() {
      pop();
      pop();
  }

  int Bjorn::Variation::peek() {
      return _stack.empty() ? -1 : _stack.top();
  }


  void Bjorn::Observateur::add_node() { _nodes++; }
  void Bjorn::Observateur::add_BJORN_cutoff() { _BJORN_cutoffs++; }
  void Bjorn::Observateur::add_beta_cutoff() { _beta_cutoffs++; }
  void Bjorn::Observateur::show() {
      auto self = this;
      std::thread th([self] () {
          auto after = std::chrono::system_clock::now();
          auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after - self->_timestamp_before);
          auto ms = duration.count();

          std::cout << "---------------------------------------\n";
          std::cout << "took:          " << ms << "ms\n";
          std::cout << "nodes:         " << self->_nodes << "\n";
          std::cout << "BJORN_cutoffs: " << self->_BJORN_cutoffs << "\n";
          std::cout << "beta_cutoffs:  " << self->_beta_cutoffs << "\n";
          std::cout << "---------------------------------------\n";
      });

      th.detach();
  }

  void Bjorn::Observateur::reset() {
      _nodes = 0;
      _BJORN_cutoffs = 0;
      _beta_cutoffs = 0;
      _timestamp_before = std::chrono::system_clock::now();
  }

  Bjorn::VirtualGame::VirtualGame(Plateau const & p) : _eval(0) {
      for (int x = 0; x < BJORN_MAX_LARGEUR; x++) {
          _heights[x] = p._hauteur[x];

          for (int y = 0; y < _heights[x]; y++) {
              _map[x][y] = (((p._pions)[x]) & (1 << y));

              _eval += mask(x, y) * bjorn_win_prob[y][x];
          }
      }

      update_plays();
  }

  std::vector<int> const & Bjorn::VirtualGame::get_plays() {
      return _plays;
  }

  std::vector<int> Bjorn::VirtualGame::get_plays(Bjorn::Variation & v) {
      if (!v.activated) return get_plays();

      int play = v.pop();

      bool found = false;
      for (auto it = _plays.begin(); it != _plays.end(); it++) {
          if ((*it) == play) {
              found = true;
              _plays.erase(it);
              break;
          }
      }

      if (found) {
          auto copy(_plays);
          copy.insert(copy.begin(), play);
          return copy;
      }


      return _plays;
  }


  void Bjorn::VirtualGame::update_plays() {
      _state = BJORN_STATE_ONGOING;
      _plays.clear();

      int count_rows_ones[BJORN_MAX_HAUTEUR] = {0};
      int count_rows_zeros[BJORN_MAX_HAUTEUR] = {0};

      for (int x = 0; x < BJORN_MAX_LARGEUR; x++) {
          if (_heights[x] < BJORN_MAX_HAUTEUR) _plays.push_back(x);

          int count_columns_zeros = 0;
          int count_columns_ones  = 0;

          for (int y = 0; y < BJORN_MAX_HAUTEUR; y++) {
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
                  _state = BJORN_STATE_MINIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_columns_ones == 4) {
                  _state = BJORN_STATE_MAXIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_rows_zeros[y] == 4) {
                  _state = BJORN_STATE_MINIMIZING_PLAYER_WIN;
                  return;
              }

              if (count_rows_ones[y] == 4) {
                  _state = BJORN_STATE_MAXIMIZING_PLAYER_WIN;
                  return;
              }
          }
      }
  }


  void Bjorn::VirtualGame::play(int x, bool maximizingPlayer) {
      int y = _heights[x];
      _map[x][y] = maximizingPlayer;
      _heights[x]++;
      _eval += mask(x, y) * bjorn_win_prob[y][x];
      update_plays();
  }

  void Bjorn::VirtualGame::unplay(int x) {
      int y = _heights[x] - 1;
      _eval -= mask(x, y) * bjorn_win_prob[y][x];
      _heights[x]--;
      update_plays();
  }

  bool Bjorn::VirtualGame::ended() {
      return (_state != BJORN_STATE_ONGOING);
  }

  int Bjorn::VirtualGame::mask(int x, int y) {
      if (y >= _heights[x]) return 0;
      else if (_map[x][y]) return 1;
           else return -1;
  }

  Bjorn::Bjorn(std::string nom, bool joueur)
  :Joueur(nom,joueur)
  {}

  void Bjorn::init_vgame(Jeu & j) {
      if (vgame != nullptr) vgame.reset();
      vgame = std::make_unique<Bjorn::VirtualGame>((*(j.plateau())));
  }


  char Bjorn::nom_abbrege() const
  {
      return 'A';
  }

  void Bjorn::recherche_coup(Jeu jeu, int & c)
  {
      observateur.reset();
      init_vgame(jeu);
      variation.step();

      auto val = alphabeta(BJORN_BETA_DEPTH, -infinity, infinity, true);
      auto coup = val.second.top();

      c = 0;
      for (int i = 0; i < jeu.nb_coups(); i++) {
          if (jeu[i] == coup) {
              c = i;
              break;
          }
      }

      variation.load(val.second);
      // observateur.show();
  }

  Bjorn::eval_var Bjorn::alphabeta(int depth, int alpha, int beta, bool maximizingPlayer) {
      // increment number of nodes
      observateur.add_node();

      // if depth = 0 or node is a terminal node then
      if (depth == 0 || vgame->ended()) {
          // return the heuristic value of node
          return eval_var(evaluation(maximizingPlayer), Bjorn::variation_stack());
      }

      // if maximizingPlayer then
      if (maximizingPlayer) {
          // value := −∞
          int value = -infinity;
          int coup = -2;
          Bjorn::variation_stack var;

          // for each child of node do
          for (auto play : vgame->get_plays(variation)) {
              vgame->play(play, maximizingPlayer);
              auto ab = alphabeta(depth - 1, alpha, beta, false);
              vgame->unplay(play);

              // update `coup`
              // value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
              if (ab.first > value) {
                  coup  = play;
                  value = ab.first;
                  var   = ab.second;
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
          var.push(coup);
          return eval_var(value, var);
      }

      // else
      else {
          // value := +∞
          int value = infinity;
          int coup = -3;
          Bjorn::variation_stack var;

          // for each child of node do
          for (auto play : vgame->get_plays(variation)) {
              vgame->play(play, maximizingPlayer);
              auto ab = alphabeta(depth - 1, alpha, beta, true);
              vgame->unplay(play);

              // update `coup`
              // value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
              if (ab.first < value) {
                  coup  = play;
                  var   = ab.second;
                  value = ab.first;
              }

              // β := min(β, value)
              beta = std::min(beta, value);

              // if α ≥ β then
              if (alpha >= beta) {
                  // increment number of alpha cutoffs
                  observateur.add_BJORN_cutoff();

                  // break (* α cut-off *)
                  break;
              }
          }

          // return value
          var.push(coup);
          return eval_var(value, var);
      }
  }

  int Bjorn::evaluation(bool maximizingPlayer) {
      if (vgame->ended()) {
          switch (vgame->get_state()) {
              case BJORN_STATE_MAXIMIZING_PLAYER_WIN:
                  return infinity;
              case BJORN_STATE_MINIMIZING_PLAYER_WIN:
                  return -infinity;
              case BJORN_STATE_DRAW:
                  return -1;
          }
      }

      return vgame->get_eval();
  }
