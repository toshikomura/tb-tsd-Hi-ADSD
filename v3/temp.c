#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"smpl.h"
#include"cisj.h"

#define test 1
#define fault 2
#define repair 3

typedef struct {
    int id;
    int cluster;
    int *testedup;
}tnodo;

tnodo *nodo;

main ( int argc, char *argv[]) {
    static int n,
    token, aux_token,
    event;
    int r, s,
    i, j, k, l, p, q,
    max_cluster, num_cluster = 0;
    char fa_name[5];
    node_set* nodes,* aux_nodes;

    if (argc != 2) {
        puts ("Número errado de parametros, uso correto, ./temp |número de nodos|");
        exit(1);
    }

    n = atoi(argv[1]);
    max_cluster =(int) ceil( log2(n) );

    smpl(0, "Simulação");
    reset();
    stream(1);
    nodo = (tnodo *) malloc (sizeof(tnodo)*n);

    for ( i = 0; i < n ; i++) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);
        nodo[i].id = facility(fa_name, 1);
        nodo[i].testedup = (int *) malloc (sizeof(int)*n);
        /* inicia no cluster 1 */
        nodo[i].cluster = 1;

        /* seta vetor de teste para -1 */
        for (j = 0; j < n; j++) {
            nodo[i].testedup[j] = -1;
        }
        /* valor de teste dele mesmo */
        nodo[i].testedup[i] = 0;
    }

    /* primeiro escalonamento de todos os nodos */
    for (i = 0; i< n ; i++) {
        schedule(test, 30.0, i);
    }

    /* definição de tempo de erro e reparo */
    schedule(fault, 60.0, 0);
    schedule(fault, 60.0, 3);
    schedule(fault, 60.0, 4);
    schedule(fault, 60.0, 6);

    printf("\n<<<<<<<<<<<<<<<<<<<START SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");

    while (time() < 100.0) {

        cause(&event, &token);

        switch(event){

        case test:

            if ( status(nodo[token].id) != 0) break;

            nodes = cis(token, nodo[token].cluster);

            /* procura nodos sem falha no cluster */
            j = 0;
            while ( status(nodo[nodes->nodes[j]].id) != 0 && j < nodes->size ){
                nodo[token].testedup[nodes->nodes[j]] = status(nodo[nodes->nodes[j]].id);
                j++;
            }

            /* se todos os nodos do cluster estão falhos */
            if ( status(nodo[nodes->nodes[j - 1]].id) != 0 && j == nodes->size ){
                j--;
            }
            else{
                /* atualiza o nodo testado */
                nodo[token].testedup[nodes->nodes[j]] = status(nodo[nodes->nodes[j]].id);
                for (k = 0; k < n; k++){
                    /* desconsiderando clusters anteriores */
                    s = 0;
                    // percorre todos os cluster anteriores //
                    for (l = 1; l <= nodo[token].cluster - 1; l++){
                        aux_nodes = cis(token, l);
                        for(p = 0; p < aux_nodes->size; p++){
                            // se o nodo já foi atualizado em um cluster anterior //
                            if (aux_nodes->nodes[p] == k)
                                s = 1;
                        }
                    }
                    // se o token não tem informação do nodo e não é ele mesmo e não é o nodo testado //
                    if ( s != 1 && k != token & k != nodes->nodes[j]){
                        nodo[token].testedup[k] = nodo[nodes->nodes[j]].testedup[k];
                    }
                }

            }

            /* imprimi log */

            /* checa cluster */
            if (num_cluster != nodo[token].cluster){
                printf("CLUSTER %d\n", nodo[token].cluster);
                num_cluster = nodo[token].cluster;
            }

            /* checa se o nodo testado não é maior que a quantidade de nodos */
            if (nodes->nodes[j] < n) {
                printf("sou o nodo %d testei por último o nodo %d no tempo %5.1f \n", token, nodes->nodes[j], time());
                printf("[%d] state: ", token);
                for (q = 0; q < n; q++) {
                    printf(" [%d] ", nodo[token].testedup[q]);
                }
                printf("\n\n");
            }

            /* checa se esta no último cluster ou não */
            if ( nodo[token].cluster < max_cluster ) {
                nodo[token].cluster++;
                schedule(test, 0.0, token);
            }
            else {
                nodo[token].cluster = 1;
                schedule(test, 30.0, token);
            }

            break;

        case fault:
            r = request( nodo[token].id, token, 0);
            if (r != 0) {
                printf ("Não consegui falhar nodo %d\n", token);
                exit(1);
            }
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("O nodo %d falha no tempo %5.1f \n", token, time());
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
            break;

        case repair:
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("o nodo %d reparou no tempo %5.1f \n", token, time());
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
            release(nodo[token].id, token);
            schedule(test, 30.0, token);
            break;

        }
    }

    printf("<<<<<<<<<<<<<<<<<<<FINISH SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");

}
