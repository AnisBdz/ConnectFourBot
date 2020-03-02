#pragma once


#include "joueur.h"


class Joueur_AlphaBeta_ : public Joueur
{
public:
  Joueur_AlphaBeta_(std::string nom, bool joueur);
  char nom_abbrege() const override;

  void recherche_coup(Jeu, int & coup) override;
};
