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
 * Bjorn Ironside
 * He uses the power of the alphabeta to conquer his enemies
 * with the help of strategic shorcuts of the principal variation.
 * he has an observator that monitors his moves and plays
 */

#define infinity 2147483647
#define BJORN_BETA_DEPTH  4
#define BJORN_MAX_HAUTEUR 6
#define BJORN_MAX_LARGEUR 7

#define BJORN_EVAL_PARAM 138

#define BJORN_STATE_MAXIMIZING_PLAYER_WIN 1
#define BJORN_STATE_MINIMIZING_PLAYER_WIN 2
#define BJORN_STATE_DRAW                  0
#define BJORN_STATE_ONGOING              -1

class Bjorn : public Joueur
{

public:
    using variation_stack = std::stack<int>;
    using eval_var = std::pair<int, variation_stack>;
    class Variation {
        private:
            variation_stack _stack;

        public:
            static bool activated;

            int pop();
            void load(std::stack<int> const & s);
            void step();
            int peek();
    };

    class Observateur {
    private:
        std::chrono::time_point<std::chrono::system_clock> _timestamp_before;
        int _nodes;
        int _BJORN_cutoffs;
        int _beta_cutoffs;

    public:
        void add_node();
        void add_BJORN_cutoff();
        void add_beta_cutoff();

        void reset();
        void show();
    };

    class VirtualGame {
    private:
        std::vector<int> _plays;
        bool _map[BJORN_MAX_LARGEUR][BJORN_MAX_HAUTEUR];
        int  _heights[BJORN_MAX_LARGEUR];
        int  _state;
        int _eval;

        void update_plays();

    public:
        VirtualGame(Plateau const & p);

        int mask(int x, int y);
        int get_state() { return _state; }
        int get_eval() { return _eval + BJORN_EVAL_PARAM; }
        std::vector<int> const & get_plays();
        std::vector<int> get_plays(Variation &);
        void play(int x, bool maximizingPlayer);
        void unplay(int x);
        bool ended();
    };

private:

    std::unique_ptr<VirtualGame> vgame;
    Observateur observateur;
    Variation variation;

public:
    Bjorn() = delete;
    Bjorn(std::string nom, bool joueur);

    char nom_abbrege() const override;
    void recherche_coup(Jeu, int & c) override;

    void init_vgame(Jeu &);
    eval_var alphabeta(int depth, int alpha, int beta, bool maximizingPlayer);
    int evaluation(bool maximizingPlayer);
    virtual ~Bjorn() = default;
};
