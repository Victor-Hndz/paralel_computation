//progama para generar un fichero binario con datos aleatorios de interos de 4 bytes
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    //Control de parametros
    if(argc!=4){
        printf("Error: Numero de argumentos incorrecto\nº");
        printf("Introducir nombre del fichero, filas y columnas\n");
        printf("Ejemplo: datos.raw 15 15\n");
        return 0;
    }

    //Declaracion de variables
    int filas = atoi(argv[2]);
    int columnas = atoi(argv[3]);
    int i, j;

    //Reserva de la memoria
    int **matriz = (int **)malloc(sizeof(int *)*filas);
    for(i = 0; i < filas; i++){
        matriz[i] = (int *)malloc(sizeof(int)*columnas);
    }
    int *X = (int *)malloc(sizeof(int)*columnas);

    //Comprobamos que se ha reservado memoria
    if(matriz == NULL || X == NULL){
        printf("Error: No se ha podido reservar memoria\n");
        return -1;
    }
     
    //Generamos los datos aleatorios entre -50 y 50
    srand(time(NULL));
    for(i = 0; i < filas; i++){
        for(j = 0; j < columnas; j++){
            matriz[i][j] = rand()%101 - 50;
        }
    }
    for(i = 0; i < columnas; i++){
        X[i] = rand()%101 - 50;
    }

    //Escribimos los datos en el fichero
    FILE *fp = fopen(argv[1], "wb");
    if(fp == NULL){
        printf("Error: No se pudo abrir el archivo");
        return -2;
    }
    //Escribimos primero la matriz
    for(i = 0; i < filas; i++){
        fwrite(matriz[i], sizeof(int), columnas, fp);
    }
    //Escribimos la X acontinuacion de escribir la matriz
    fwrite(X, sizeof(int), columnas, fp);

    //Liberamos la memoria
    for(i = 0; i < filas; i++){
        free(matriz[i]);
    }
    free(matriz);
    free(X);

    //Cerramos el fichero
    fclose(fp);

    return 0;
}


