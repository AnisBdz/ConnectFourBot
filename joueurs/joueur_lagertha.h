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
 * Lagertha Lothbrok
 * Oh, you know her!
 * What's special here is she can retreat before her time expires
 */

#define infinity 2147483647
#define LAGERTHA_BETA_DEPTH  5
#define LAGERTHA_MAX_HAUTEUR 6
#define LAGERTHA_MAX_LARGEUR 7

#define LAGERTHA_RETREAT_TIME 7

#define LAGERTHA_EVAL_PARAM 138

#define LAGERTHA_STATE_MAXIMIZING_LAGERTHA_WIN 1
#define LAGERTHA_STATE_MINIMIZING_LAGERTHA_WIN 2
#define LAGERTHA_STATE_DRAW                  0
#define LAGERTHA_STATE_ONGOING              -1

class Lagertha : public Joueur
{

public:
    using eval_play = std::pair<int, int>;

    class Observateur {
    private:
        std::chrono::time_point<std::chrono::system_clock> _timestamp_before;
        int _nodes;
        int _LAGERTHA_cutoffs;
        int _beta_cutoffs;

    public:
        void add_node();
        void add_LAGERTHA_cutoff();
        void add_beta_cutoff();

        void reset();
        void show();
        void show_time();
        bool time_expired();
    };

    class VirtualGame {
    private:
        std::vector<int> _plays;
        bool _map[LAGERTHA_MAX_LARGEUR][LAGERTHA_MAX_HAUTEUR];
        int  _heights[LAGERTHA_MAX_LARGEUR];
        int  _state;
        int _eval;

        void update_plays();

    public:
        VirtualGame(Plateau const & p);

        int mask(int x, int y);
        int get_state() { return _state; }
        int get_eval() { return _eval + LAGERTHA_EVAL_PARAM; }
        std::vector<int> const & get_plays();
        void play(int x, bool maximizingLagertha);
        void unplay(int x);
        bool ended();
    };

private:

    std::unique_ptr<VirtualGame> vgame;
    Observateur observateur;

public:
    Lagertha() = delete;
    Lagertha(std::string nom, bool joueur);

    char nom_abbrege() const override;
    void recherche_coup(Jeu, int & c) override;

    void init_vgame(Jeu &);
    eval_play alphabeta(int depth, int alpha, int beta, bool maximizingLagertha);
    int evaluation(bool maximizingLagertha);


    virtual ~Lagertha() = default;
};
