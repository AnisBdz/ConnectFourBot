#include "joueur_random.h"

Joueur_Random::Joueur_Random(std::string nom, bool joueur)
    :Joueur(nom,joueur)
{}



char Joueur_Random::nom_abbrege() const
{
    return 'R';
}

void Joueur_Random::recherche_coup(Jeu jeu, int &coup)
{
  //  coup = std::experimental::randint(0, jeu.nb_coups()-1);
  coup = jeu.nb_coups() == 1 ?  0 : rand() % (jeu.nb_coups()-1);
}
