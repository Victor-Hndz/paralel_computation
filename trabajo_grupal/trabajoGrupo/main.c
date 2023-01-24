#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

//extern void multiplicarMatriz(int **matrix, int *vector, int *result, int filas, int columnas);

void multiplicarMatrizEnC(int **matrix, int *vector, int *result, int filas, int columnas)
{
    int sum;
    for(int i = 0; i < filas;i++)//calculamos la multiplicación de matrices
		{
			sum = 0;
			for(int e = 0; e<columnas;e++)
			{
				sum += matrix[i][e] * vector[e];
			}
			result[i] = sum;
		}
}
int main (int argc, char *argv[])
{
    //Control de parametros
    if(argc!=5){
        printf("Error: Numero de argumentos incorrecto\n");
        printf("Introducir nombre del fichero, filas, columnas y verbosidad\n");
        printf("Ejemplo: datos.raw 15 15 [si-no]\n");
        return 0;
    }

    //Declaracion de variables
    int filas = atoi(argv[2]);
    int columnas = atoi(argv[3]);
    int i, j;
    int **matriz;
    int *X, *producto;
    char verbose;
    if(strcmp(argv[4],"si")==0)
    {
        verbose = 1;
    }
    else
    {
        verbose = 0;
    }
    //Reserva de la memoria
    matriz = (int **)malloc(sizeof(int *)*filas);
    matriz[0]= (int*)malloc(sizeof(int) * filas *columnas);
    for(i = 1; i < filas; i++){
        matriz[i] = matriz[i-1]+columnas;
    }

    X = (int *)malloc(sizeof(int)*columnas);
    producto = (int *)malloc(sizeof(int)*filas);

    //Comprobamos que se ha reservado memoria
    if(matriz == NULL || X == NULL || producto == NULL){
        printf("Error: No se ha podido reservar memoria\n");
        return -1;
    }

    //Lectura de datos
    FILE *fp = fopen(argv[1], "rb");
    if(fp == NULL){
        printf("Error: No se pudo abrir el archivo");
        return -2;
    }
    //Lemos primero la matriz
    for(i = 0; i < filas; i++){
        fread(matriz[i], sizeof(int), columnas, fp);
    }
    //Lemos la X acontinuacion de leer la matriz
    fread(X, sizeof(int), columnas, fp);

    //Operamos con la matriz en ensamblador
    fflush(stdout);
    clock_t start, end;
    start = clock();
    multiplicarMatrizEnC(matriz,X, producto,filas,columnas);
    end = clock();
    //Imprimimos los resultados
    if(verbose)
    {
        printf("Matriz:\n");
    printf("┌");
    for(i = 0; i < columnas; i++){
        printf("───────");
        if(i == columnas-1){
            printf("┐\n");
        }
        else{
            printf("┬");
        }
    }
    for(i = 0; i < filas; i++){
        printf("│");
        for(j = 0; j < columnas; j++){
            printf(" %5d ", matriz[i][j]);
            if(j == columnas-1){
                printf("│\n");
            }
            else{
                printf("│");
            }
        }
        if(i == filas-1){
            printf("└");
            for(j = 0; j < columnas; j++){
                printf("───────");
                if(j == columnas-1){
                    printf("┘\n");
                }
                else{
                    printf("┴");
                }
            }
        }
        else{
            printf("├");
            for(j = 0; j < columnas; j++){
                printf("───────");
                if(j == columnas-1){
                    printf("┤\n");
                }
                else{
                    printf("┼");
                }
            }
        }
    }
    //Imprmimos el simbolo de multiplicacion ajustado al centro
    printf(" %*s \n", (columnas*6)/2, "×");

    //Mostramos el vector X
    printf("Vector X:\n");
    printf("┌");
    for(i = 0; i < columnas; i++){
        printf("───────");
        if(i == columnas-1){
            printf("┐\n");
        }
        else{
            printf("┬");
        }
    }
    printf("│");
    for(i = 0; i < columnas; i++){
        printf(" %5d ", X[i]);
        if(i == columnas-1){
            printf("│\n");
        }
        else{
            printf("│");
        }
    }
    printf("└");
    for(i = 0; i < columnas; i++){
        printf("───────");
        if(i == columnas-1){
            printf("┘\n");
        }
        else{
            printf("┴");
        }
    }
    //Imprmimos el simbolo de igual ajustado al centro
    printf(" %*s \n", (columnas*6)/2, "=");
    

    //Mostramos el vector producto
    printf("Vector producto:\n");
    printf("┌");
    for(i = 0; i < filas; i++){
        printf("───────");
        if(i == filas-1){
            printf("┐\n");
        }
        else{
            printf("┬");
        }
    }
    printf("│");
    for(i = 0; i < filas; i++){
        printf(" %5d ", producto[i]);
        if(i == filas-1){
            printf("│\n");
        }
        else{
            printf("│");
        }
    }
    printf("└");
    for(i = 0; i < filas; i++){
        printf("───────");
        if(i == filas-1){
            printf("┘\n");
        }
        else{
            printf("┴");
        }
    }
    }
    

    printf("%.16g\n",(double) (end - start)/CLOCKS_PER_SEC);
    //Liberamos la memoria
    free(matriz[0]);
    free(matriz);
    free(X);
    free(producto);

    return 0;
}