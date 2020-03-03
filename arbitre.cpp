#include "arbitre.h"


Arbitre::Arbitre(int graine, player player1, player player2, int nombre_parties)
  :
   _jeu(graine),
   _coups(nombre_parties, -1),
   _coups_mutex(nombre_parties),
   _nombre_parties(nombre_parties),
   _numero_partie(1),
   _player1(player1),
   _player2(player2)
{}

void Arbitre::initialisation()
{
  _joueur1=nullptr;
  _joueur2=nullptr;

  _jeu.reset();

  switch ((_numero_partie%2? _player1 : _player2)) {
  case player::A_:
    _joueur1 = std::make_shared<Joueur_AlphaBeta_> ("AlphaBeta",true);
    break;
  case player::BRUTAL_:
    _joueur1 = std::make_shared<Joueur_Brutal_> ("Brutal",true);
    break;
  case player::RAND:
    _joueur1 = std::make_shared<Joueur_Random> ("Random",true);
    break;
  default:
    break;
  }

  switch ((!(_numero_partie%2)? _player1 : _player2)) {
  case player::A_:
    _joueur2 = std::make_shared<Joueur_AlphaBeta_> ("AlphaBeta",false);
    break;
  case player::BRUTAL_:
    _joueur2 = std::make_shared<Joueur_Brutal_> ("Brutal",false);
    break;
  case player::RAND:
    _joueur2 = std::make_shared<Joueur_Random> ("Random",false);
    break;
  default:
    break;
  }

}

void Arbitre::challenge()
{
  initialisation(); // Au moins une fois pour que les objets de la ligne qui suit soient définis
    std::cout << "Le challenge de " << _nombre_parties << " parties "
              <<"entre " << _joueur1->nom() << " et " << _joueur2->nom()
             << " commence. " << std::endl;
    int victoire_joueur_1 = 0;
    int victoire_joueur_2 = 0;
    for(int i=0 ; i < _nombre_parties ; i++)
    {
        std::cout << "\n" << "Partie n°" << _numero_partie << " : ";
	int resultat = partie();
	if (resultat != 0)
	  (resultat==1?
	   ((_numero_partie%2)?
	    victoire_joueur_1++
	    :
	    victoire_joueur_2++ )
	   :
	   (!(_numero_partie%2)?
	    victoire_joueur_1++
	    :
	    victoire_joueur_2++ ));
        std::this_thread::sleep_for (std::chrono::milliseconds(250)); // temps de latence entre deux parties
        _numero_partie++;
        initialisation();
    }
    std::cout << "FIN DU CHALLENGE\n\t"
              << _joueur1->nom()<< " gagne " << ((_numero_partie%2)? victoire_joueur_1 : victoire_joueur_2)
              << "\n\t"<< _joueur2->nom()<< " gagne " << ((_numero_partie%2) ? victoire_joueur_2 : victoire_joueur_1) << std::endl;
}

int Arbitre::partie()
{
  int tour = 0;

  while(!_jeu.fini())
    {
      bool try_lock = false;
      tour++;
      std::cout << "tour : " << tour << std::endl;
      _coups[_numero_partie-1] = -1;
      _coups_mutex[_numero_partie-1].unlock();

      std::thread thread_joueur(&Joueur::jouer,
				((tour%2)? (_joueur1) :(_joueur2) ),
				_jeu,
				std::ref(_coups[_numero_partie-1]),
				std::ref(_coups_mutex[_numero_partie-1]));

      std::this_thread::sleep_for (std::chrono::milliseconds(TEMPS_POUR_UN_COUP));
      //        std::this_thread::sleep_for (std::chrono::seconds(TEMPS_POUR_UN_COUP));

      if (!_coups_mutex[_numero_partie-1].try_lock()) {
	std::cerr <<  std::endl << "mutex non rendu " << std::endl;
	try_lock = true;
      }
      else if(_coups[_numero_partie-1] == -1) {
	std::cerr << "coup invalide -1" << std::endl;
      }
      else if(!_jeu.coup_licite(_coups[_numero_partie-1])) {
	std::cerr << "coup invalide " << _coups[_numero_partie-1] << std::endl;
      }

thread_joueur.detach();

      if(try_lock ||
	 (_coups[_numero_partie-1] == -1) ||
	 !_jeu.coup_licite(_coups[_numero_partie-1]))
	{



	  if(_jeu.partie_nulle())
	    {
	      std::cout << "partie nulle (" << tour << ")" << std::endl;
	    }
	  else if(tour%2)
	    {
	      std::cout << _joueur2->nom() <<" gagne ! Nombre de tours : " << tour << std::endl;
	      return 2; // joueur 2 gagne
	    }
	  else
	    {
	      std::cout << _joueur1->nom() <<" gagne ! Nombre de tours : " << tour << std::endl;
	      return 1; // joueur 1 gagne
	    }
	}

      _coups[_numero_partie-1]+=1;

      if (tour%2 == 0) {
	_coups[_numero_partie-1] = -_coups[_numero_partie-1];
      }

      _jeu.joue(_coups[_numero_partie-1]);

      std::cout << ((tour%2) ? _joueur1->nom_abbrege() : _joueur2->nom_abbrege()) << _coups[_numero_partie-1] << std::endl << *(_jeu.plateau()) << std::endl;

    }
  if (_jeu.partie_nulle())
    {
      std::cout << std::endl << "Partie nulle" << std::endl;
      return 0;
    }
  else
    {
      std::cout << std::endl << ((tour%2)? _joueur1->nom() : _joueur2->nom() )  <<" gagne. Nombre de tours : " << tour << std::endl;
      return ((tour%2)? 1 : 2);
    }
}
