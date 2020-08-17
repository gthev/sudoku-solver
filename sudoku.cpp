#include "grille.hpp"
#include <vector>
#include <stack>
#include <algorithm>

//=====================
//gestion de la stack d'hypotheses

std::stack<struct hypothese*> hypotheses;


//depile la dernière hypothèse, en supprimant l'option de la case où elle était (donc l'hypothèse était fausse)
//remplace l'argument (et le libère) par la nouvelle grille
void depile_hypothese(Grille &grille) {
    if(hypotheses.empty()) {
        std::cout << "On a dépilé jusqu'au bout. Exiting..." << std::endl;
        exit(1);
    }

    //pour afficher les hypothèses. jusqu'au trait : peut être supprimé
    /* for(int i=0; i<hypotheses.size(); i++) {
        std::cout << "\t";
    } */

    //std::cout << "X" << std::endl;
    //====


    struct hypothese *hyp = hypotheses.top();
    hypotheses.pop();
 
    grille = *(hyp->bakup);


/*
    Bon ici normalement, on libère la mémoire de la grille qui a été stockée en back up au moment de dépiler
    mais... on ne peut plus accéder à grille si on fait ça
    pourtant, il me semble qu'au dessus, on fait appel au constructeur de copie, donc que le fait de toucher à hyp->bakup ne touche pas à grille...
*/
//    delete hyp->bakup;

    //on met un appel récursif ici, si le fait d'éliminer cette hypothèse fait une contradiction (normalement, ça n'arrivera pas)
    if(grille.rem_opt(hyp->x, hyp->y, hyp->opt)) {
        depile_hypothese(grille);
    }

    free(hyp);
}

//ici, on fait une hypothèse, donc :
//on crée une nouvelle hypothèse
//on copie la grille courante, on la met dans la struct hypothese
//on la rajoute à la stack d'hypothèse
//on fait effectivement l'hypothèse, ie on met la valeur dans la case
void empile_hypothese(Grille &grille, unsigned short opt, int x, int y) {


    struct hypothese *new_hyp = (struct hypothese*)calloc(1, sizeof(struct hypothese));
    new_hyp->opt = opt;
    new_hyp->x = x;
    new_hyp->y = y;

    Grille *bakup_grille = new Grille(grille);
    new_hyp->bakup = bakup_grille;

    hypotheses.push(new_hyp);

    //==== idem : affichage superflu
    /* for(int i=0; i<hypotheses.size(); i++) {
        std::cout << "\t";
    } */

    //std::cout << opt << "(" << x << "," << y << ")" << std::endl;
    //======

    if(grille.set_val(x, y, opt))
    {
        depile_hypothese(grille);
    }
}
//=====================

//renvoie le nombre de cases contenant la valeur demandée
int count_number(Grille &grille, unsigned short val) {

    int count = 0;

    for(int x=0; x<9; x++) {
        for(int y=0; y<9; y++) {
            if(grille.get_cell(x,y)->val == val) count++;
        }
    }

    return count;
}

//initialise les possibilités dans toutes les cases non occupées :
//chaque case contient donc toutes les possibilités (options), que l'on supprimera ensuite au fur et à mesure des déductions logiques
void init_options(Grille &grille) {

    //on parcourt les nombres par nombre d'occurence décroissant
    std::vector<std::pair <unsigned short, int>> occurences;

    for(unsigned short x=1; x<10; x++) {
        occurences.push_back(std::pair<unsigned short, int>(x, count_number(grille, x)));
    }

    //on trie les paires d'occurences de manière décroissante
    std::sort(occurences.begin(), occurences.end(), [](std::pair<unsigned short, int>& a, std::pair<unsigned short, int>& b) {return a.second > b.second;});

    //puis on les parcourt, et on met toutes les options
    for(auto& it : occurences) {
        for(int x=0; x<9; x++) {
            for(int y=0; y<9; y++) {

                if(grille.is_cell_occupied(x,y)) continue;

                if(!grille.is_in_line(x, it.first) && !grille.is_in_row(y, it.first) && !grille.is_in_square(x, y, it.first)) {
                    grille.add_opt(x, y, it.first);
                }

            }
        }
    }
}

//============

//wrapper pour mettre une valeur, et enlevant l'option de toutes les cases de la ligne/colonne/carré
//true s'il y a une contradiction
bool place_valeur(Grille &grille, unsigned short val, int x, int y) {

    if(grille.set_val(x, y, val)) {
        return true;
    }

    //on supprime dans la ligne
    for(int j = 0; j<9; j++) {
        if(!grille.is_cell_occupied(x,j)) {
            if(grille.rem_opt(x,j, val)) return true;
        }
    }

    //on supprime dans la colonne
    for(int i = 0; i<9; i++) {
        if(!grille.is_cell_occupied(i,y)) {
                if(grille.rem_opt(i,y, val)) return true;
        }
    }

    //et dans le carré
    for(int i = (x/3)*3; i < (x/3)*3 + 3; i++) {
        for(int j = (y/3)*4 ; j < (y/3)*3 + 3; j++) {
            if(!grille.is_cell_occupied(i,j)) {
                if(grille.rem_opt(i,j,val)) return true;
            }
        }
    }


    return false;
}

//renvoie 0 si fini, -1 si problème (contradiction), 1 si on doit faire une hypothèse
int deduction_logique(Grille &grille) {

    //tant qu'on a modifié la grille (et qu'elle n'est pas pleine) on refait le parcourt des cases pour faire des déductions logiques
    do {


        grille.dirty = false;

        //les cases qui ne contiennent aucune possibilité déclenchent une contradiction,
        //et dans celles qui n'ont qu'une possibilité, on met celle-ci en valeur
        for(int x=0; x<9; x++) {
            for(int y=0; y<9; y++) {

                if(grille.is_cell_occupied(x,y)) continue;

                if(grille.get_cell(x,y)->opt->size() == 0) {
                    return -1;
                }

                if(grille.get_cell(x,y)->opt->size() == 1) {
                    if(place_valeur(grille, *grille.get_cell(x,y)->opt->begin(), x, y)) return -1;
                    grille.dirty = true;
                }

            }
        }

        //ici, on regarde pour chaque nombre...
        for(unsigned short num = 1; num<10; num++) {
            
            //... et pour chaque ligne si le nombre n'apparait qu'une fois en option dans la ligne (trop bien) ou aucune fois (contradiction)
            for(int x = 0; x<9; x++) {
                int nb_num = 0, last_y = 0;

                for(int y=0; y<9; y++) {

                    //si on le trouve directement, on saute ce nombre en mettant nb_num = 2 (saute ce nombre) et en break
                    if(grille.get_cell(x,y)->val == num) {
                        nb_num = 2;
                        break;
                    }

                    if(grille.get_cell(x,y)->opt->find(num) != grille.get_cell(x,y)->opt->end()) {
                        nb_num++;
                        last_y = y;
                    }
                }

                if(!nb_num) {
                    return -1;
                }

                if(nb_num == 1) {
                    if(place_valeur(grille, num, x, last_y))  return -1;
                    grille.dirty = true;
                }

            }

            //idem pour les colonnes
            for(int y = 0; y<9; y++) {
                int nb_num = 0, last_x = 0;

                for(int x=0; x<9; x++) {

                    if(grille.get_cell(x,y)->val == num) {
                        nb_num = 2;
                        break;
                    }

                    if(grille.get_cell(x,y)->opt->find(num) != grille.get_cell(x,y)->opt->end()) {
                        nb_num++;
                        last_x = x;
                    }
                }

                if(!nb_num) {
                    return -1;
                }

                if(nb_num == 1) {
                    if(place_valeur(grille, num, last_x, y))  return -1;
                    grille.dirty = true;
                }

            }

            

            //et enfin le carré i,j
            for(int i=0; i<3; i++) {
                for(int j=0; j<3; j++) {
                    
                    int nb_num = 0, last_x = 0, last_y = 0;

                    for(int x=i*3; x<(i*3) + 3; x++) {
                        for(int y=j*3; y<(j*3)+3; y++) {

                            if(grille.get_cell(x,y)->val == num) {
                                nb_num = 2;
                                break;
                            }

                            if(grille.get_cell(x,y)->opt->find(num) != grille.get_cell(x,y)->opt->end()) {
                                nb_num++;
                                last_x = x;
                                last_y = y;
                            }
                        }
                    }

                    if(!nb_num) {
                        return -1;
                    }

                    if(nb_num == 1) {
                        if(place_valeur(grille, num, last_x, last_y)) return -1;
                        grille.dirty = true;
                    }

                }
            }

        }

    } while(grille.dirty && nr_occupied(grille) < 82);

    //ici : soit on a fini et on est ok, soit on fait une hypothèse, et on recommence
    return (nr_occupied(grille) < 82)? 1: 0;
}

//=========

//fonction qui renvoie les coord de la cellule non occupées avec le + petit nb d'options
bool candidat_hypothese(Grille &grille, int *x, int *y) {
    int min_opt = 10, size; //10 car il y a au maximum 9 options

    for(int i=0; i<9; i++) {
        for(int j=0; j<9; j++) {

            if(grille.is_cell_occupied(i,j)) continue;

            size = grille.get_cell(i,j)->opt->size();
            
            if(size < min_opt && size > 0) {
                min_opt = size;
                *x = i;
                *y = j;
            }

        }
    }

    return (min_opt == 10);
}

//=========

//le "main" de la résolution du sudoku
int resolve_sudoku(Grille &grille) {
    int retCode, x, y;
    bool finished = false;

    //on initialise les possibilités
    init_options(grille);



    while(!finished) {

        //on fait la déduction logique
        retCode = deduction_logique(grille);


        switch (retCode)
        {
        //on a fini
        case 0:
            finished = true;
            break;
        
        //la déduction n'avance plus et on n'a pas fini, on fait une hypothèse
        case 1:
            if(candidat_hypothese(grille, &x, &y)) { finished = true; break;}
            empile_hypothese(grille, *grille.get_cell(x,y)->opt->begin(), x, y);
            break;/* grille.get_cells_left() > 0 */ 

        //contradiction : on revient en arrière
        case -1:
            depile_hypothese(grille);
            break;

        default:
            std::cout << "resolve_sudoku wtf retcode" << std::endl;
            return 1;
            break;
        }
    }

    return 0;
}

//============

int main() {

    Grille grille, grille_orig;
    FILE* input;
    size_t n_char = 64;
    char *ligne = (char*)malloc(64 * sizeof(char));
    std::string file_name;
    int choix = 1;

    srand(time(NULL));

    std::cout << "Bienvenue dans mon super programme de sudoku !" << std::endl ;
    std::cout << "Auteur : Gaspard Thévenon" << std::endl ;
    std::cout << "Pour résoudre un sudoku, il faut inscrire dans un fichier texte" << std::endl;
    std::cout << "la grille, avec un chiffre par case, 0 pour vide (inconnue), et 9 lignes en tout." << std::endl;
    std::cout << "Quel est le nom du fichier ?" << std::endl;

    std::cin >> file_name;

    input = fopen(file_name.data(), "r");

    if(!input) {
        std::cout << "input erreur main" << std::endl;
        return 1;
    }

    

    for(int i=0; i<9; i++) {
        getline(&ligne, &n_char, input);
        for(int j=0; j<9; j++) {
            grille.set_val(i,j, ligne[j] - '0');
        }
    }

    grille_orig = grille;

    std::cout << "Grille chargée" << std::endl;

    //debug : affichage de la grille
    print_grille(grille);

    std::cout << std::endl;

    std::cout << "Résolution...";

    resolve_sudoku(grille);

    std::cout << "ok" << std::endl;

    do {
        int x,y;

        if(nr_occupied(grille_orig) == 81) break;

        std::cout << std::endl << "Que voulez-vous ?" << std::endl;
        std::cout << "1. Un indice" << std::endl;
        std::cout << "2. La grille résolue" << std::endl << "> ";

        std::cin >> choix;
        switch (choix)
        {
        case 1:
choix:
            std::cout <<  std::endl << "D'accord, et :" << std::endl;
            std::cout << "1. Au hasard" << std::endl;
            std::cout << "2. Au choix" << std::endl << "> ";
            std::cin >> choix;

            switch (choix)
            {
            case 1:
                x = rand() % 10;
                y = rand() % 10;
                while(grille_orig.is_cell_occupied(x,y)) {
                    x = rand() % 10;
                    y = rand() % 10;
                }
                break;

            case 2:
                std::cout << "x (entre 0 et 8) : ";
                std::cin >> x;
                std::cout << "y (entre 0 et 8) : ";
                std::cin >> y;
                break;
            
            default:
                std::cout << "Vous avez entré un truc bizarre" << std::endl;
                goto choix;
                break;
            }

            

            std::cout << "Dévoilement de la case (coordonnées de 0 à 8) (" << x << "," << y << ") :" << std::endl;
            grille_orig.get_cell(x,y)->val = grille.get_cell(x,y)->val;
            print_grille(grille_orig);
            choix = 1;
            break;
        
        case 2:
            break;

        default:
            std::cout << "Vous avez entré un truc bizarre" << std::endl;
            break;
        }
    } while(choix != 2);

    print_grille(grille);

    return 0;
}