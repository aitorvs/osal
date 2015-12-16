/**
 *  \file   quicksort.c
 *  \brief  This file implements the quicksort algorithm for RTEMS
 *
 *  Detailed description starts here.
 *
 *  \author  Aitor Viana Sanchez (avs), Aitor.Viana.Sanchez@esa.int
 *
 *  \internal
 *    Created:  12/03/09
 *   Revision:  $Id: quicksort.c 1.4 12/03/09 avs Exp $
 *   Compiler:  gcc/g++
 *    Company:  European Space Agency (ESA-ESTEC)
 *  Copyright:  Copyright (c) 2009, Aitor Viana Sanchez
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>

void quicksort(int lista[], int inf, int sup)
{
    int izq, der, mitad, x;

    izq = inf;
    der = sup;

    mitad = lista[(izq + der)/2];

    do
    {
        while (lista[izq] < mitad && izq < sup) izq++;
        while (mitad < lista[der] && der > inf) der--;
        if (izq <= der)
        {
            x = lista[izq];
            lista[izq] = lista[der];
            lista[der] = x;
            izq++; der--;
        }
    } while (izq <= der);
    if (inf < der) quicksort(lista, inf, der);
    if (izq < sup) quicksort(lista, izq, sup);
}

int main(void)
{
    int lista[] = { 12, 3, 7, 5, 10, 32, 67, 39, 25, 1};
    int i = 0;

    printf("Original list:");
    for (i = 0; i < 10; i++)
        printf(" %d", lista[i]);
    printf("\n");

    quicksort(lista, 0, 9);

    printf("Ordered list:");
    for (i = 0; i < 10; i++)
        printf(" %d", lista[i]);
    printf("\n");

    return 0;
}
