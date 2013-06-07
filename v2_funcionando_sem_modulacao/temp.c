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
    int *testedup;
}tnodo;

tnodo *nodo;

main ( int argc, char *argv[]) {
    static int n,
    token, aux_token,
    event;
    int r, s,
    i, j, k, l, p, q,
    max_cluster;
    char fa_name[5];
    node_set* nodes,* aux_nodes;

    if (argc != 2) {
        puts ("Número errado de parametros, uso correto, ./program |número de nodos|");
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

        for (j = 0; j < n; j++) {
            nodo[i].testedup[j] = -1;
        }
        nodo[i].testedup[i] = 0;
    }

    for (i = 0; i< n ; i++) {
        schedule(test, 30.0, i);
    }

    schedule(fault, 40.0, 2);
    schedule(repair, 70.0, 2);

    printf("\n<<<<<<<<<<<<<<<<<<<START SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");

    while (time() < 100.0) {

        cause(&event, &token);

        switch(event){

        case test:

           if ( status(nodo[token].id) != 0) break;

            /* find nodes of the cluster of token*/
            for (i = 1; i <= max_cluster; i++){

                printf("test cluster %d\n", i);

                /* find nodes of the cluster*/
                nodes = cis(token, i);

                /* find node withou fail in the cluster */
                j = 0;
                while ( status(nodo[nodes->nodes[j]].id) != 0 && j < nodes->size ){
                    nodo[token].testedup[nodes->nodes[j]] = status(nodo[nodes->nodes[j]].id);
                    j++;
                }

                /* if all nodes are fail in the cluster */
                if ( status(nodo[nodes->nodes[j - 1]].id) != 0 && j == nodes->size ){
                    j--;
                }
                else{

                    /* update last node tested */
                    nodo[token].testedup[nodes->nodes[j]] = status(nodo[nodes->nodes[j]].id);

                    /* get information of the node without fail */
                    // percorre vetor testedup do token //
                    for (k = 0; k < n; k++){
                        /* disregard past clusters */
                        s = 0;
                        // percorre todos os cluster anteriores //
                        for (l = 1; l <= i; l++){
                            aux_nodes = cis(token, l);
                            for(p = 0; p < aux_nodes->size; p++){
                                // se o nodo já foi atualizado em um cluster anterior //
                                if (aux_nodes->nodes[p] == k)
                                    s = 1;
                            }
                        }
                        // se o token não tem informação do nodo e não é ele mesmo //
                        if ( s != 1 && k != token)
                            nodo[token].testedup[k] = nodo[nodes->nodes[j]].testedup[k];
                    }
                }

                /* print log */
                printf("sou o nodo %d testei por último o nodo %d no tempo %5.1f \n", token, nodes->nodes[j], time());
                printf("[%d] state: ", token);
                for (q = 0; q < n; q++) {
                    printf(" [%d] ", nodo[token].testedup[q]);
                }
                printf("\n\n");

            }

            schedule(test, 30.0, token);

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
