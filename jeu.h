#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define PARTIE_NON_TERMINEE 2
#define PARTIE_NULLE 0
#define ALIGNEMENT 1

#define MAX_HAUTEUR 6
#define MAX_LARGEUR 7

class Plateau {
 public :
  int _hauteur[MAX_LARGEUR];
  int _pions[MAX_LARGEUR];

  void afficher( std::ostream &flux) const;
};

std::ostream& operator<<( std::ostream &flux, Plateau const& plateau );

class Jeu {
 private :
  int _graine;
  Plateau _plateau;
  int _tab[MAX_LARGEUR];
  int _nombre;
  int _puissance[MAX_LARGEUR];
  int _alignement;
  int _dual_x[MAX_HAUTEUR];
  int _dual_o[MAX_HAUTEUR];
  int _etat;
  int _nb_tours;

 public :
  Jeu(int graine);
  void reset(); // Remet à la graine le jeu
  bool coup_licite(int); // Vérifie si le coup est licite
  void joue(int); // Joue le coup (sans vérifier qu'il est licite)
  int nb_coups(); // Le nombre de coups possibles
  int& operator[](int i); // Les coups possibles
  bool fini(); // Vérifie si un état final a été atteint (dans ce cas le jeu prend fin)
  bool partie_nulle(); // Vérifie si la partie est nulle
  void print_coups();
  Plateau* plateau();
};
