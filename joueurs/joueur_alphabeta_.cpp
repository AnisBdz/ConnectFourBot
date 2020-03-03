#include "joueur_alphabeta_.h"

int win_prob[ALPHA_MAX_HAUTEUR][ALPHA_MAX_LARGEUR] =
                        {{ 3, 4,  5,  7,  5, 4, 3},
                         { 4, 6,  8, 10,  8, 6, 4},
                         { 5, 8, 11, 13, 11, 8, 5},
                         { 5, 8, 11, 13, 11, 8, 5},
                         { 4, 6,  8, 10,  8, 6, 4},
                         { 3, 4,  5,  7,  5, 4, 3}};

VirtualGame::VirtualGame(Plateau const & p): _last_played(0) {
    for (int x = 0; x < ALPHA_MAX_LARGEUR; x++) {
        _heights[x] = p._hauteur[x];
        _plays.push_back(x);

        for (int y = 0; y < _heights[x]; y++) {
            _map[x][y] = (((p._pions)[x]) & (1 << y));
        }
    }
}

std::vector<int> const & VirtualGame::get_plays() {
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
    _map[x][_heights[x]] = maximizingPlayer;
    _heights[x]++;
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
    else return _map[x][y] * 2 - 1;
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
    init_vgame(jeu);

    auto val_cp = alphabeta(ALPHA_BETA_DEPTH, -infinity, infinity, true);
    std::cout << "value " << val_cp.first << "\n";

    for (int i = 0; i < jeu.nb_coups(); i++) {
        if (jeu[i] == val_cp.second) {
            c = i;
            break;
        }
    }
}

value_coup Joueur_AlphaBeta_::alphabeta(int depth, int alpha, int beta, bool maximizingPlayer) {
    // if depth = 0 or node is a terminal node then
    if (depth == 0 || vgame->ended()) {
        // return the heuristic value of node
        return value_coup(evaluation(maximizingPlayer), 0);
    }

    // if maximizingPlayer then
    if (maximizingPlayer) {
        // value := −∞
        int value = -infinity;
        int coup = -1;

        // for each child of node do
        for (auto play : vgame->get_plays()) {
            vgame->play(play, maximizingPlayer);
            auto ab = alphabeta(depth - 1, alpha, beta, false);
            vgame->unplay(play);

            // update `coup`
            if (ab.first > value) coup = play;

            // value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
            value = std::max(value, ab.first);

            // α := max(α, value)
            alpha = std::max(alpha, value);

            // if α ≥ β then
            if (alpha >= beta) {
                // break (* β cut-off *)
                break;
            }
        }

        // return value
        return value_coup(value, coup);
    }

    // else
    else {
        // value := +∞
        int value = infinity;
        int coup = -1;

        // for each child of node do
        for (auto play : vgame->get_plays()) {
            vgame->play(play, maximizingPlayer);
            auto ab = alphabeta(depth - 1, alpha, beta, true);
            vgame->unplay(play);

            // update `coup`
            if (ab.first < value) coup = play;

            // value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
            value = std::min(value, ab.first);

            // if α ≥ β then
            if (alpha >= beta) {
              // break (* α cut-off *)
              break;
            }
        }

        // return value
        return value_coup(value, coup);
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
            eval += mask * win_prob[y][x] * (maximizingPlayer ? 1 : -1);
        }
    }

    return eval;
}
