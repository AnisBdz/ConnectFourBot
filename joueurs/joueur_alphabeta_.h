#pragma once
#include "joueur.h"
#include <memory>
#include <vector>
#include <utility>

#define infinity 2147483647
#define ALPHA_BETA_DEPTH 6
#define ALPHA_MAX_HAUTEUR 6
#define ALPHA_MAX_LARGEUR 7

#define STATE_MAXIMIZING_PLAYER_WIN 1
#define STATE_MINIMIZING_PLAYER_WIN 2
#define STATE_DRAW 0

using value_coup = std::pair<int, int>;

class VirtualGame {
private:
    std::vector<int> _plays;
    bool _map[ALPHA_MAX_LARGEUR][ALPHA_MAX_HAUTEUR];
    int  _heights[ALPHA_MAX_HAUTEUR];
    int  _last_played;
    int  _state;

    void update_plays();

public:
    VirtualGame(Plateau const & p);

    int get_state() { return _state; }
    std::vector<int> const & get_plays();
    void play(int x, bool maximizingPlayer);
    void unplay(int x);
    bool ended();
};

class Joueur_AlphaBeta_ : public Joueur
{

private:
    std::unique_ptr<VirtualGame> vgame;

public:
  Joueur_AlphaBeta_(std::string nom, bool joueur);
  char nom_abbrege() const override;

  void recherche_coup(Jeu, int & c) override;

    void init_vgame(Jeu &);
    std::pair<int, int> alphabeta(int depth, int alpha, int beta, bool maximizingPlayer);
    int evaluation(bool maximizingPlayer);
    std::unique_ptr<VirtualGame> const & get_vgame() { return vgame; }
};
