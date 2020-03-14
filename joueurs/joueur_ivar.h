#pragma once
#include "joueur.h"
#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <utility>
#include <chrono>
#include <thread>

/*
 * Ivar
 * He uses the power of the alphabeta to massacre his enemies
 * like his older brother Bjorn Ironside, Ivar has an observator that monitors his moves and plays
 * but he is a bit reckless, his brutal and doesn't like to take shorcuts
 */

#define infinity 2147483647
#define IVAR_BETA_DEPTH  4
#define IVAR_MAX_HAUTEUR 6
#define IVAR_MAX_LARGEUR 7

#define IVAR_EVAL_PARAM 138

#define IVAR_STATE_MAXIMIZING_PLAYER_WIN 1
#define IVAR_STATE_MINIMIZING_PLAYER_WIN 2
#define IVAR_STATE_DRAW                  0
#define IVAR_STATE_ONGOING              -1

class Ivar : public Joueur
{

public:
    using eval_play = std::pair<int, int>;

    class Observateur {
    private:
        std::chrono::time_point<std::chrono::system_clock> _timestamp_before;
        int _nodes;
        int _IVAR_cutoffs;
        int _beta_cutoffs;

    public:
        void add_node();
        void add_IVAR_cutoff();
        void add_beta_cutoff();

        void reset();
        void show();
    };

    class VirtualGame {
    private:
        std::vector<int> _plays;
        bool _map[IVAR_MAX_LARGEUR][IVAR_MAX_HAUTEUR];
        int  _heights[IVAR_MAX_LARGEUR];
        int  _state;
        int _eval;

        void update_plays();

    public:
        VirtualGame(Plateau const & p);

        int mask(int x, int y);
        int get_state() { return _state; }
        int get_eval() { return _eval + IVAR_EVAL_PARAM; }
        std::vector<int> const & get_plays();
        void play(int x, bool maximizingPlayer);
        void unplay(int x);
        bool ended();
    };

private:

    std::unique_ptr<VirtualGame> vgame;
    Observateur observateur;

public:
    Ivar() = delete;
    Ivar(std::string nom, bool joueur);

    char nom_abbrege() const override;
    void recherche_coup(Jeu, int & c) override;

    void init_vgame(Jeu &);
    eval_play alphabeta(int depth, int alpha, int beta, bool maximizingPlayer);
    int evaluation(bool maximizingPlayer);
    virtual ~Ivar() = default;
};
