#include <iostream>
#include <set>
#include <iterator>

/*
On décide que soit val = 0, soit opt est vide par exemple
*/
struct _cellule {
    unsigned short val;                 /* la valeur dans la case */
    std::set<unsigned short> *opt;      /* les candidats dans la case */
};

class Grille
{
private:
    struct _cellule             cells[9][9];    //matrices des cases
    std::set<unsigned short>    lines[9];       //liste ordonnée des valeurs dans une ligne
    std::set<unsigned short>    rows[9];        //idem pour les colonnes
    std::set<unsigned short>    squares[3][3];  //idem pour les carrés

    

// x : ligne, y : colonne

public:

    bool                        dirty;                  //si la grille a été modifiée depuis la dernière réinitialisation

    Grille();                                           //constructeur par défaut
    Grille(const Grille &grille);                       //constructeur de copie
    ~Grille();                               

    void reinit();                                      //réinitialise la grille

    bool set_val(int x, int y, unsigned short val);      //mettre une valeur, renvoie >0 si une contradiction
    void add_opt(int x, int y, unsigned short opt);     //ajouter une option
    bool rem_opt(int x, int y, unsigned short opt);      //enlever une option, renvoie >0 si une contradiction

    struct _cellule *get_cell(int x, int y);            //renvoie un pointeur vers une cellule de la grille

    bool is_in_line(int x, int val) {return (lines[x].find(val) != lines[x].end());};           //si une valeur est dans une ligne
    bool is_in_row(int y, int val) {return (rows[y].find(val) != rows[y].end());};
    bool is_in_square(int x, int y, int val) {return (squares[x/3][y/3].find(val) != squares[x/3][y/3].end());};

    bool is_cell_occupied(int x, int y) {return (cells[x][y].val != 0);};                       //si la case contient une valeur


//peut-être à enlever plus tard
/*     std::set<unsigned short> *getlines();
    std::set<unsigned short> *getrows();
    std::set<unsigned short> *getsquares(); */
};



struct hypothese {
    unsigned short opt;             //le nombre qui a été supposé dans une cellule
    int x, y;                       //les coordonnées de ladite cellule
    Grille *bakup;                  //pointeur vers la grille dans l'état où elle était au moment de l'hypothèse
};

//nombre de cases occupées dans la grille
int nr_occupied(Grille &grille);
//affiche la grille
void print_grille(Grille &grille);