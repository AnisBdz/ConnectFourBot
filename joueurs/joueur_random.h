#pragma once

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <utility>
// #include <mutex>
// #include <experimental/random>
#include "joueur.h"


class Random : public Joueur
{

public:
    Random(std::string nom,bool joueur);
    char nom_abbrege() const override;


    void recherche_coup(Jeu jeu, int & coup) override;
};
