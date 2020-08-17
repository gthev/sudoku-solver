#include "grille.hpp"

Grille::Grille()
{
    for(int x=0; x<9; x++) {
        for(int y=0; y<9; y++) {
            cells[x][y].val = 0;
            cells[x][y].opt = new std::set<unsigned short>();
        }    
    }

    dirty = false;

}

Grille::Grille(const Grille &grille)
{
    for(int x=0; x<9; x++) {
        for(int y=0; y<9; y++) {
            cells[x][y].val = grille.cells[x][y].val;
            cells[x][y].opt = new std::set<unsigned short>(*(grille.cells[x][y].opt)); //on copie les nombres
        }
    
    }

    for(int i=0; i<9; i++) {
        lines[i] = grille.lines[i];
        rows[i] = grille.rows[i];
    }

    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            squares[i][j] = grille.squares[i][j];
        }
    }

    dirty = grille.dirty;

}

Grille::~Grille()
{
    //il faut juste libérer les options des cellules
    for(int x=0; x<9; x++) {
        for(int y=0; y<9; y++) {
            delete cells[x][y].opt;
        }
    }
}

//====================

//renvoie true s'il la valeur déclenche une contradiction
bool Grille::set_val(int x, int y, unsigned short val) {
    //si les coordonnées sont mauvaises...
    if(x > 8 || x < 0 || y > 8 || y < 0) {
        std::cout << "problem set_val : x=" << x << " y=" << y << std::endl; 
    }

    //on vérifie si la valeur est déjà dans la ligne, la colone ou le carré
    if(lines[x].find(val) != lines[x].end() || rows[y].find(val) != rows[y].end() || squares[x/3][y/3].find(val) != squares[x/3][y/3].end()) {
        return true;
    }

    cells[x][y].val = val;
    lines[x].insert(val);
    rows[y].insert(val);
    squares[x/3][y/3].insert(val);

    return false;
}

void Grille::add_opt(int x, int y, unsigned short opt) {
    cells[x][y].opt->insert(opt);
}

//renvoie aussi true en cas de contradiction (ie si la case n'a plus d'option après coup) -> il ne faut pas l'utiliser en l'état si la case contient une valeur !
bool Grille::rem_opt(int x, int y, unsigned short opt) {
    //on pourrait faire ça pour plus de sécurité :
    //if(is_cell_occupied(x,y)) return false;

    //ne vérifie pas si la case contient effectivement l'option
    cells[x][y].opt->erase(opt);
    return cells[x][y].opt->empty();
}



struct _cellule *Grille::get_cell(int x, int y) {
    return &cells[x][y];
}

//on fait appel à clear de partout
void Grille::reinit() {
    for(int i=0; i<9; i++) {
        for(int j=0; j<9; j++) {
            cells[i][j].val = 0;
            cells[i][j].opt->clear();
        }
        lines[i].clear();
        rows[i].clear();
    }

    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            squares[i][j].clear();
        }
    }

    dirty = false;
}

int nr_occupied(Grille &grille) {
    int count = 0;

    for(int x=0; x<9; x++) {
        for(int y=0; y<9; y++) {
            if(grille.is_cell_occupied(x,y)) count++;
        }
    }

    count++;

    return count;
}

void print_grille(Grille &grille) {
    for(int x=0; x<9; x++) {
        for(int y=0; y<9; y++) {
            std::cout << grille.get_cell(x,y)->val << " ";
        }
        std::cout << std::endl;
    }
}