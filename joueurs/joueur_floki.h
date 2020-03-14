#pragma once
#include "joueur.h"
#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <utility>
#include <chrono>
#include <thread>
#include <cstdint>

/*
 * Flóki Vilgerðarson
 * He uses the power of the alphabeta to defeat his enemies
 * He observes and watches thoroughly
 * and has a great memory to remember his moves as
 * he follows the guidance of the Norse God, Zobrist
 */

#define infinity 2147483647
#define FLOKI_BETA_DEPTH  4
#define FLOKI_MAX_HAUTEUR 6
#define FLOKI_MAX_LARGEUR 7

#define FLOKI_EVAL_PARAM 138

#define FLOKI_STATE_MAXIMIZING_PLAYER_WIN 1
#define FLOKI_STATE_MINIMIZING_PLAYER_WIN 2
#define FLOKI_STATE_DRAW                  0
#define FLOKI_STATE_ONGOING              -1

#define FLOKI_MAX_MASK 3
#define FLOKI_MEMORY_SIZE 65536

class Floki : public Joueur
{

public:
    using eval_play = std::pair<int, int>;

    class TranspositionTable {
    private:
        using hash_t = uint32_t;
        using packet = struct {
            bool occupied = false;
            int evaluation;
        };

        hash_t  _hashes[FLOKI_MAX_LARGEUR][FLOKI_MAX_HAUTEUR][FLOKI_MAX_MASK];
        packet  _memory[FLOKI_MEMORY_SIZE];
        int h(hash_t hash);

    public:
        TranspositionTable();

        hash_t hash(bool map[FLOKI_MAX_LARGEUR][FLOKI_MAX_HAUTEUR], int _heights[]);
        bool is_occupied(hash_t hash);
        int  get_eval(hash_t hash);
        void store(hash_t hash, int eval);

        void clear();
    };

    class Observateur {
    private:
        std::chrono::time_point<std::chrono::system_clock> _timestamp_before;
        int _nodes;
        int _FLOKI_cutoffs;
        int _beta_cutoffs;

    public:
        void add_node();
        void add_FLOKI_cutoff();
        void add_beta_cutoff();

        void reset();
        void show();
    };

    class VirtualGame {
    private:
        std::vector<int> _plays;
        int  _state;
        int _eval;

        void update_plays();

    public:
        bool _map[FLOKI_MAX_LARGEUR][FLOKI_MAX_HAUTEUR];
        int  _heights[FLOKI_MAX_LARGEUR];
        VirtualGame(Plateau const & p);

        int mask(int x, int y);
        int get_state() { return _state; }
        int get_eval() { return _eval + FLOKI_EVAL_PARAM; }
        std::vector<int> const & get_plays();
        void play(int x, bool maximizingPlayer);
        void unplay(int x);
        bool ended();
    };

private:

    std::unique_ptr<VirtualGame> vgame;
    Observateur observateur;
    TranspositionTable transposition_table;

public:
    Floki() = delete;
    Floki(std::string nom, bool joueur);

    char nom_abbrege() const override;
    void recherche_coup(Jeu, int & c) override;

    void init_vgame(Jeu &);
    eval_play alphabeta(int depth, int alpha, int beta, bool maximizingPlayer);
    int evaluation(bool maximizingPlayer);
    virtual ~Floki() = default;
};
