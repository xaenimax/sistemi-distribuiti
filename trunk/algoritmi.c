#include "basic.h"



//Author: Alessandro Pacca
//ritorna il numero del server da contattare e tiene traccia di quale server è stato contattato precedentemente. Da richiamare da qualche parte nel codice, appena posso vedo dove :)
int RoundRobin() {

        if(numeroServerDaContattare == numeroDiServerPresenti)
                numeroServerDaContattare = 1;
        else
                numeroServerDaContattare++;

        return numeroServerDaContattare;
}

