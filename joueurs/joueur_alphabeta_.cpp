#include "joueur_alphabeta_.h"

int win_prob[ALPHA_MAX_HAUTEUR][ALPHA_MAX_LARGEUR] =
                        {{ 3, 4,  5,  7,  5, 4, 3},
                         { 4, 6,  8, 10,  8, 6, 4},
                         { 5, 8, 11, 13, 11, 8, 5},
                         { 5, 8, 11, 13, 11, 8, 5},
                         { 4, 6,  8, 10,  8, 6, 4},
                         { 3, 4,  5,  7,  5, 4, 3}};

bool Variation::activated = true;

void Variation::load(std::stack<int> const & q) {
    _stack = std::stack<int>(q);
}

int Variation::pop() {
    if (_stack.empty()) return -1;

    int play = _stack.top();
    _stack.pop();
    return play;
}

void Variation::step() {
    pop();
    pop();
}

int Variation::peek() {
    return _stack.empty() ? -1 : _stack.top();
}


void Observateur::add_node() { _nodes++; }
void Observateur::add_alpha_cutoff() { _alpha_cutoffs++; }
void Observateur::add_beta_cutoff() { _beta_cutoffs++; }
void Observateur::show() {
    auto self = this;
    std::thread th([self] () {
        auto after = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(after - self->_timestamp_before);
        auto ms = duration.count();

        std::cout << "---------------------------------------\n";
        std::cout << "took:          " << ms << "ms\n";
        std::cout << "nodes:         " << self->_nodes << "\n";
        std::cout << "alpha_cutoffs: " << self->_alpha_cutoffs << "\n";
        std::cout << "beta_cutoffs:  " << self->_beta_cutoffs << "\n";
        std::cout << "---------------------------------------\n";
    });

    th.detach();
}

void Observateur::reset() {
    _nodes = 0;
    _alpha_cutoffs = 0;
    _beta_cutoffs = 0;
    _timestamp_before = std::chrono::system_clock::now();
}

VirtualGame::VirtualGame(Plateau const & p) {
    for (int x = 0; x < ALPHA_MAX_LARGEUR; x++) {
        _heights[x] = p._hauteur[x];
        if (_heights[x] < ALPHA_MAX_HAUTEUR) _plays.push_back(x);

        for (int y = 0; y < _heights[x]; y++) {
            _map[x][y] = (((p._pions)[x]) & (1 << y));
        }
    }
}

std::vector<int> const & VirtualGame::get_plays() {
    return _plays;
}

std::vector<int> VirtualGame::get_plays(Variation & v) {
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


void VirtualGame::update_plays() {
    _state = STATE_DRAW;
    _plays.clear();

    int count_rows_ones[ALPHA_MAX_HAUTEUR] = {0};
    int count_rows_zeros[ALPHA_MAX_HAUTEUR] = {0};

    for (int x = 0; x < ALPHA_MAX_LARGEUR; x++) {
        if (_heights[x] < ALPHA_MAX_HAUTEUR) _plays.push_back(x);

        int count_columns_zeros = 0;
        int count_columns_ones  = 0;

        for (int y = 0; y < ALPHA_MAX_HAUTEUR; y++) {
            if (y >= _heights[x]) {
                count_rows_ones[y] = 0;
                count_rows_zeros[y] = 0;
                continue;
            }

            if (_map[x][y] == 0) {
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
                _state = STATE_MINIMIZING_PLAYER_WIN;
                return;
            }

            if (count_columns_ones == 4) {
                _state = STATE_MAXIMIZING_PLAYER_WIN;
                return;
            }

            if (count_rows_zeros[y] == 4) {
                _state = STATE_MINIMIZING_PLAYER_WIN;
                return;
            }

            if (count_rows_ones[y] == 4) {
                _state = STATE_MAXIMIZING_PLAYER_WIN;
                return;
            }
        }
    }
}


void VirtualGame::play(int x, bool maximizingPlayer) {
    _map[x][_heights[x]++] = maximizingPlayer;
    update_plays();
}

void VirtualGame::unplay(int x) {
    _heights[x]--;
    update_plays();
}

bool VirtualGame::ended() {
    return _plays.empty();
}

int VirtualGame::mask(int x, int y) {
    if (y >= _heights[x]) return 0;
    else if (_map[x][y]) return 1;
         else return -1;
}

void Joueur_AlphaBeta_::init_vgame(Jeu & j) {
    if (vgame != nullptr) vgame.reset();
    vgame = std::make_unique<VirtualGame>((*(j.plateau())));
}



Joueur_AlphaBeta_::Joueur_AlphaBeta_(std::string nom, bool joueur)
    :Joueur(nom,joueur)
{}



char Joueur_AlphaBeta_::nom_abbrege() const
{
    return 'A';
}

void Joueur_AlphaBeta_::recherche_coup(Jeu jeu, int & c)
{
    observateur.reset();
    init_vgame(jeu);
    variation.step();

    auto val = alphabeta(ALPHA_BETA_DEPTH, -infinity, infinity, true);
    auto coup = val.second.top();

    c = -4;
    for (int i = 0; i < jeu.nb_coups(); i++) {
        if (jeu[i] == coup) {
            c = i;
            break;
        }
    }

    variation.load(val.second);
    observateur.show();
}

eval_var Joueur_AlphaBeta_::alphabeta(int depth, int alpha, int beta, bool maximizingPlayer) {
    // increment number of nodes
    observateur.add_node();

    // if depth = 0 or node is a terminal node then
    if (depth == 0 || vgame->ended()) {
        // return the heuristic value of node
        return eval_var(evaluation(maximizingPlayer), variation_stack());
    }

    // if maximizingPlayer then
    if (maximizingPlayer) {
        // value := −∞
        int value = -infinity;
        int coup = -2;
        variation_stack var;

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
        variation_stack var;

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
                observateur.add_alpha_cutoff();

                // break (* α cut-off *)
                break;
            }
        }

        // return value
        var.push(coup);
        return eval_var(value, var);
    }
}

int Joueur_AlphaBeta_::evaluation(bool maximizingPlayer) {
    if (vgame->ended()) {
        switch (vgame->get_state()) {
            case STATE_MAXIMIZING_PLAYER_WIN:
                return infinity;
            case STATE_MINIMIZING_PLAYER_WIN:
                return -infinity;
            case STATE_DRAW:
                return 0;
        }
    }


    int eval = 0;
    for (int x = 0; x < ALPHA_MAX_LARGEUR; x++) {
        for (int y = 0; y < ALPHA_MAX_HAUTEUR; y++) {
            int mask = vgame->mask(x, y);
            eval += mask * win_prob[y][x];
        }
    }

    return eval;
}
