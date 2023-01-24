#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define MEDIA 0
#define MEDIANA 1
#define SOBEL 2
#define SOBEL_M 3
#define OUT "out.raw"
#define LOG "log.txt"


/// @brief Algoritmo Bubblesort.
/// @param a Vector a ordenar.
/// @param n Tamaño del vector.
void bubblesort(unsigned char *a, int n)
{
    int i, j, temp;

    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (a[j] > a[j + 1])
            {
                temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}


/// @brief Filtrado de pixel mediante mediana, ordenando con bubblesort y devolviendo el pixel en el centro del vector (pos 4).
/// @param pi Índice del pixel a filtrar -1 (x).
/// @param pj Índice del pixel a filtrar -1 (y).
/// @param m Matriz a filtrar.
/// @param tamfilt Tamaño del lado de la matriz de filtración (3x3).
/// @return Pixel filtrado.
unsigned char medianaPixel(int pi, int pj, unsigned char **m, int tamfilt)
{
    int tam = tamfilt*tamfilt;
    unsigned char p[tam];
    int in=0;

    for (int i=0;i<tamfilt;i++)
    {
        for (int j=0;j<tamfilt;j++)
        {
            p[in] = m[pi+i][pj+j];
            in++;
        }
    }
    
    bubblesort(p, tam);
    return p[(tam-1)/2];
}


/// @brief Filtrado de pixel por media.
/// @param pi Índice del pixel a filtrar -1 (x).
/// @param pj Índice del pixel a filtrar -1 (y).
/// @param m Matriz a filtrar.
/// @param tamfilt Tamaño del lado de la matriz de filtración (3x3).
/// @return Pixel filtrado.
unsigned char avgPixel(int pi, int pj, unsigned char **m, int tamfilt)
{
    uint16_t avg=0;

    for(int i=0;i<tamfilt;i++)
        for (int j=0;j<tamfilt;j++)
            avg += m[pi+i][pj+j];
    
    return (avg/(tamfilt*tamfilt));
}


/// @brief Función que sirve para seleccionar en qué tipo borde de pixel se está.
/// @param m Matriz a filtrar.
/// @param pi Índice del pixel a filtrar -1 (x).
/// @param pj Índice del pixel a filtrar -1 (y).
/// @param tamfilt Tamaño del lado de la matriz de filtración (3x3).
/// @param rows Total de filas de la matriz.
/// @param cols Total de columnas de la matriz.
/// @return El pixel correcto del borde a que corresponda.
unsigned char selectPixel(unsigned char **m, int pi, int pj, int tamfilt, int rows, int cols)
{
    //esquinas 
    if(pi==-1 && pj==-1)
        return m[pi+2][pj+2];
    else if(pi==-1 && pj==cols)
        return m[pi+2][pj-2];
    else if(pi==rows && pj==-1)
        return m[pi-2][pj+2];
    else if(pi==rows && pj==cols)
        return m[pi-2][pj-2];
    else //bordes
    {
        if(pi==-1)
            return m[pi+2][pj];
        else if(pi==rows)
            return m[pi-2][pj];
        else if(pj==-1)
            return m[pi][pj+2];
        else
            return m[pi][pj-2];
    }
}


/// @brief Filtrado de Pixel por Sobel.
/// @param pi Índice del pixel a filtrar -1 (x).
/// @param pj Índice del pixel a filtrar -1 (y).
/// @param m Matriz a filtrar.
/// @param tamfilt Tamaño del lado de la matriz de filtración (3x3).
/// @param rows Total de filas de la matriz.
/// @param cols Total de columnas de la matriz.
/// @return Pixel filtrado.
unsigned char sobel(int pi, int pj, unsigned char **m, int tamfilt, int rows, int cols)
{
    int F[][SOBEL_M] = {-1,0,1,-2,0,2,-1,0,1}, C[][SOBEL_M] = {-1,-2,-1,0,0,0,1,2,1}, sumF=0, sumC=0;

    for(int i=0;i<tamfilt;i++)
    {
        for(int j=0;j<tamfilt;j++)
        {
            //Si la matriz a filtrar tiene algún pixel en el borde, se selecciona, se corrige y luego se suma.
            if(pi+i==-1 || pj+j==-1 || pi+i==rows || pj+j==cols)
            {
                sumC += (selectPixel(m, pi+i, pj+j, tamfilt, rows, cols)*C[i][j]);
                sumF += (selectPixel(m, pi+i, pj+j, tamfilt, rows, cols)*F[i][j]);
            }
            else //si no, se suma el pixel y se sigue.
            {
                sumC += (m[i+pi][j+pj]*C[i][j]);
                sumF += (m[i+pi][j+pj]*F[i][j]);
            }
        }
    }
    return sqrt((pow(sumC,2))+pow(sumF,2));
}


/// @author Víctor Hernández Sánchez
int main(int argc, char **argv)
{
    FILE *in, *out, *log;
    unsigned char **m, **filtrada;
    int rows, cols, mode, tamfilt, N;
    char* filename;
    double secs;
    struct timespec start, end;

    if(argc != 7) //Comprobación de los parámetros.
    {
        printf("Error en el numero de argumentos.\n");
        exit(-1);
    }
    else //Control de argumentos.
    {
        filename = argv[1];
        in = fopen(filename, "rb");

        if(strcmp(argv[2], "media") == 0)
            mode = MEDIA;
        else if(strcmp(argv[2], "mediana")==0)
            mode = MEDIANA;
        else if(strcmp(argv[2], "sobel")==0)
            mode = SOBEL;
        else
        {
            printf("Error en el segundo argumento. Debe ser 'media', 'mediana' o 'sobel'.\n");
            exit(-1);
        }

        rows = atoi(argv[3]);
        cols = atoi(argv[4]);
        tamfilt = atoi(argv[5]);
        N = atoi(argv[6]);
    }

    out = fopen(OUT, "wb");
    log = fopen(LOG, "w");
    m = (unsigned char**)malloc((rows)*sizeof(unsigned char*));
    filtrada = (unsigned char**)malloc((rows)*sizeof(unsigned char*));

    for (int i=0;i<rows;i++) //Lectura del fichero de entrada.
    {
        m[i] = (unsigned char*)malloc((cols)*sizeof(unsigned char));
        filtrada[i] = (unsigned char*)malloc((cols)*sizeof(unsigned char));
        fread(m[i], sizeof(unsigned char), cols, in);
    }
    fclose(in);
    int np, iam, ini, final, i, j, resto;

    //Sección paralela
    clock_gettime(CLOCK_REALTIME, &start);
    #pragma omp parallel num_threads(N) private(iam, i, j, ini, final) shared(np, cols, rows, tamfilt, m, filtrada, resto, mode) default(none)
    {
        #pragma omp single //Num procesos y el resto de repartir las filas.
        {
            np = omp_get_num_threads();
            resto=rows%np;
        }
        iam = omp_get_thread_num();

        printf("soy %d de %d\n", iam, np);

        //Nº de filas para cada proceso
        ini = iam*(rows/np);
        final = ((iam+1)*(rows/np)-1);
       
       //Al ultimo proceso se le suman las del resto.
        if(iam == np-1)
        {
            final += resto;
            resto=0;
        }
        //printf("%d - %d\n",ini,final);

        for(i=ini;i<=final;i++) //Bucle principal de filtrado pixel a pixel.
        {
            //printf("soy %d - %d\n",iam,i);
            for (j=0;j<cols;j++)
            {
                if(mode==2)
                    filtrada[i][j] = sobel(i-1, j-1, m, tamfilt, rows, cols);
                else
                {
                    if(j<1 || i<1 || j>(cols-2) || i>(rows-2))
                    {
                        filtrada[i][j] = m[i][j];
                    }
                    else
                    {
                        if(mode==0)
                            filtrada[i][j] = avgPixel(i-1, j-1, m, tamfilt);
                        else
                            filtrada[i][j] = medianaPixel(i-1, j-1, m, tamfilt);
                    }
                }
            }  
        }
    }
    
    clock_gettime(CLOCK_REALTIME, &end);
    //Escritura del archivo.
    for(int i=0;i<rows;i++)
        fwrite(filtrada[i], sizeof(unsigned char), cols, out);
    fclose(out);

    secs = (double)(end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1000000000.0;

    //Escritura del log.
    fprintf(log, "Fichero de entrada: %s\nTamaño de la imagen: %dx%d\nModo de ejecucion: %s\nNombre del fichero de salida: %s\nNumero de hilos usados: %d\nTiempo de ejecucion: %.16g ms.\n", filename, rows, cols, argv[2], OUT, np, secs*1000);
    fclose(log);

    for (int i=0;i<rows;i++)
    {
        free(m[i]);
        free(filtrada[i]);
    }
    free(m);
    free(filtrada);


    printf("FIN DEL PROGRAMA\n\n");
    return 0;
}
