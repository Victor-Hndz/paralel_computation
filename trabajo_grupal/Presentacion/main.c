#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

extern void multiplicarMatriz(int **matrix, int *vector, int *result, int filas, int columnas);

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
    if(argc!=6){
        printf("Error: Numero de argumentos incorrecto\n");
        printf("Introducir nombre del fichero, filas, columnas, verbosidad y si deseamos obtener resultados para csv\n");
        printf("Ejemplo: datos.raw 15 15 [si-no] [si-no]\n");
        return 0;
    }

    //Declaracion de variables
    int filas = atoi(argv[2]);
    int columnas = atoi(argv[3]);
    int i, j;
    int **matriz;
    int *X, *producto;
    char verbose;
    char csv;
    if(strcmp(argv[4],"si")==0)
        verbose = 1;
    else
        verbose = 0;

    if(strcmp(argv[5],"si")==0)
        csv = 1;
    else
        csv = 0;

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
    clock_t start, end;
    double cpu_time_used_asm, cpu_time_used_c;
    start = clock();
    multiplicarMatriz(matriz,X, producto,filas,columnas);
    end = clock();
    cpu_time_used_asm = ((double) (end - start)) / CLOCKS_PER_SEC;
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

    //Operamos con la matriz en C
    start = clock();
    multiplicarMatrizEnC(matriz,X, producto,filas,columnas);
    end = clock();
    cpu_time_used_c = ((double) (end - start)) / CLOCKS_PER_SEC;

    if(csv){
        printf("%d,%d,%.6g,%.6g,%.6g,%.4g,%.3g\n", filas, columnas, cpu_time_used_asm*1000, cpu_time_used_c*1000, (cpu_time_used_c - cpu_time_used_asm)*1000, (cpu_time_used_c - cpu_time_used_asm)/cpu_time_used_c*100, cpu_time_used_c/cpu_time_used_asm);
    }else{
        //Imprimimos los resultados
        printf("\n----------------------------------------------------\n");
        printf("Resltado con %d filas y %d columnas\n", filas, columnas);
        printf("Tiempo ensamblador en milisegundos: %.6g\n", cpu_time_used_asm*1000);
        printf("Tiempo en C en milisegundos: %.6g\n", cpu_time_used_c*1000);
        printf("Diferencia de tiempo en milisegundos: %.6g\n", (cpu_time_used_c - cpu_time_used_asm)*1000);
        printf("Porcentaje de mejora con respecto a C: %.4g\% \n", (cpu_time_used_c - cpu_time_used_asm)/cpu_time_used_c*100);
        printf("Más rápido que c: %.3g veces\n", cpu_time_used_c/cpu_time_used_asm);
        printf("----------------------------------------------------\n");
    }
    
    //Liberamos la memoria
    free(matriz[0]);
    free(matriz);
    free(X);
    free(producto);

    return 0;
}