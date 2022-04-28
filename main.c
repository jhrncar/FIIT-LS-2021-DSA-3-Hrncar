#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>

typedef struct uzol {
    char vstup; //v liste predstavuje 0,1 a v obyčajnom uzly predstavuje písmeno a,b,c,...
    struct uzol *nula; //ukazovateľ na ďalší uzol, ak je ohodnotenie tejto vrstvy 0
    struct uzol *jedna;//ukazovateľ na ďalší uzol, ak je ohodnotenie tejto vrstvy 1
} UZOL;


typedef struct bdd {
    UZOL *pointer; //ukazovateľ na koreň diagramu
    int pocet;  //počet premennych, z ktorých bol zostavený strom
    int velkost;   //počet uzlov v celom strome
} BDD;

char abeceda[17] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
                    'm', 'n', 'o', 'p', 'q'}; //abeceda, z ktorej sa vkladaju do nodov
int poc = 0;

BDD *BDD_create(char *vektor);

char BDD_use(BDD *bdd, char *vstupy);

UZOL *bdd_novy(int vyska, int pocet, const char *vektor);

UZOL *bdd_insert_rekurz(UZOL *koren, int cislo, int max, char *vektor);

UZOL *bdd_insert_rekurz(UZOL *koren, int cislo, int max,
                        char *vektor) {//koren je node o vrstvu vyssie, cislo je momentalne cislo vrstvy, max je maximalny pocet vrstiev, vektor je vstupny vektor ktory sa ulozi do listov
    if (cislo == max) { //ak som v poslednej vrstve, rekurzia sa zacne vracat
        return koren;
    }
    koren = bdd_novy(cislo + 1, max, vektor); //vytvorenie noveho nodu
    koren->nula = bdd_insert_rekurz(koren->nula, cislo + 1, max, vektor); //volanie rekurzie s vrstvou o 1 hlbsou
    koren->jedna = bdd_insert_rekurz(koren->jedna, cislo + 1, max, vektor);

    return koren;
}

BDD *BDD_create(char *vektor) {
    int pocet = (int) log2((double) strlen(vektor)); //zistenie nutneho poctu vrstiev (premennych)
    UZOL *koren;
    koren = bdd_novy(0, pocet, vektor); //vytvorenie korena
    if (pocet > 1) { //ak toto nestaci, vytvorim rekurzivne cely strom
        koren->nula = bdd_insert_rekurz(koren->nula, 0, pocet, vektor); //zaciatok rekurzie
        koren->jedna = bdd_insert_rekurz(koren->jedna, 0, pocet, vektor);
    }
    BDD *p = (BDD *) malloc(sizeof(BDD)); //vytvorenie struktury BDD v spravnom formate a jej vratenie
    p->pointer = koren;
    p->pocet = pocet;
    p->velkost = 0;
    for (int i = 0; i <= pocet; i++)
        p->velkost += (int) pow(2, i);


    return p;
}


UZOL *bdd_novy(int vyska, int pocet, const char *vektor) {
    UZOL *uzol = (UZOL *) malloc(sizeof(UZOL));//vytvorenie noveho nodu
    if (vyska == pocet) {//zapisujem uz do listu?
        if (vektor[poc] == '0' || vektor[poc] == '1') {//ak ano, zapis
            uzol->vstup = vektor[poc];
            poc++;
        } else { //zapisovat sa mozu len cisla 0 a 1
            exit(poc);
        }
    } else if (vyska > pocet) {
        return NULL;
    } else {
        uzol->vstup = abeceda[vyska];//ak nezapisujem do listu, pisem dalsiu premennu
    }
    uzol->jedna = NULL;
    uzol->nula = NULL;

    return uzol;
}

char BDD_use(BDD *bdd, char *vstupy) {
    int dlzka = (int) strlen(vstupy);//zistim velkost celeho stromu
    UZOL *strom = bdd->pointer; //skopirujem si koren stromu
    for (int i = 0; i < dlzka; i++) { //iterativne prechadzam cez strom
        if (!strom) return -1; //ak nastane problem, vratim -1
        if (vstupy[i] == '0') {
            strom = strom->nula;
        } else {
            strom = strom->jedna;
        }
    }
    if (!strom) return -1;//ak nastane problem, vratim -1
    return strom->vstup;
}

void bdd_free(UZOL *koren, int pocet) {//rekurzivne uvolnenie stromu zdola nahor
    if (koren->jedna != NULL && pocet > 0) {
        bdd_free(koren->jedna, pocet - 1);
    }
    if (koren->nula != NULL && pocet > 0) {
        bdd_free(koren->nula, pocet - 1);
    }
    free(koren);
}

int main(void) {
    FILE *vektor;
    FILE *hodnota;
    clock_t startc, finishc, startu, finishu, notdone = 0;
    double doneu = 0, donec = 0;
    vektor = fopen("vektory.txt", "r");
    hodnota = fopen("vstupy.txt", "r");

    char *vector;
    char *var;
    int pocet = 17; //pocet premennych, treba vzdy zmenit na aktualny
    unsigned long k = (int) pow(2, pocet);
    vector = (char *) calloc(k + 2, sizeof(char));
    var = (char *) calloc(pocet + 1, sizeof(char));
    BDD *strom;
    fseek(hodnota, 0, SEEK_SET);
    fseek(vektor, 0, SEEK_SET);


    for (int i = 0; i < 2000; i++) { //vykonam 2000 jedinecnych moznosti create
        fgets(vector, k + 1, vektor);
        startc = clock();
        strom = BDD_create(vector);
        finishc = clock();
        donec += finishc - startc;
        for (int y = 0; y < k; y++) { // pre kazdy create vykonam vsetkych 8192 nastaveni 13tich premennych
            fgets(var, pocet + 1, hodnota);
            startu = clock();
            char x = BDD_use(strom, var);
            if (x == -1) {
                printf("%d-%d: %s, %d : %c", i, y, var, var[pocet - 1],
                       var[pocet - 1]); //ak nastane problem, program sam diagnostikuje a vrati kde nastal problem
                exit(var[0]);
            }
            finishu = clock();
            notdone += finishu - startu;
        }
        doneu += notdone;
        notdone = 0;
        fseek(hodnota, 0, SEEK_SET);
        poc = 0;
        bdd_free(strom->pointer, pocet); //strom treba po pouziti uvolnit
        free(strom);
        fgetc(vektor);
        //fgetc(vektor);//musi tu byt 2x fgetc kvoli kompilovaniu na Linuxe, v pripade kompilacie na Windows treba zakomentovat 1x fgetc

    }
    printf("Cas po vytvoreni: %g s\n", (donec / CLOCKS_PER_SEC));
    printf("Cas po pouziti: %g s\n", (doneu / CLOCKS_PER_SEC));
    fclose(vektor);
    fclose(hodnota);
    free(var);
    free(vector);
    return 0;
}

