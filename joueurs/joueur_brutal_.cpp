#include "joueur_brutal_.h"


Brutal_::Brutal_(std::string nom, bool joueur)
    :Joueur(nom,joueur)
{}

char Brutal_::nom_abbrege() const
{
    return 'B';
}

void Brutal_::recherche_coup(Jeu jeu, int &coup)
{
  //  std::this_thread::sleep_for (std::chrono::milliseconds(std::experimental::randint(0,11)));
  std::this_thread::sleep_for (std::chrono::milliseconds(rand() % 11));

  //  if(std::experimental::randint(0,49) == 0) {
  if(rand() % 49 == 0) {
    coup = jeu.nb_coups();
  }
  else {
    coup = jeu.nb_coups()-1;
  }

}
