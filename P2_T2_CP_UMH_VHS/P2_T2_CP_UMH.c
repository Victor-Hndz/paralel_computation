#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define LOG "log.txt"
#define N 15000
#define LIMSUP 49
#define LIMINF -49
#define LIMVAL 25


/// @brief Multiplicación de dos vectores, fila x columna de dos matrices o de matriz y vector.
/// @param p1 
/// @param p2 
/// @param tam longitud de la fila.
/// @return valor del elemento resultado de la operación.
double multRow(double* p1, double* p2, int tam)
{
    double sum=0;
    for (int i=0;i<tam;i++)
        sum += p1[i]*p2[i];

    return sum;
}


/// @author Víctor Hernández Sánchez
int main(int argc, char **argv)
{
    FILE *in, *out;
    double **m, *xi, *abs, *temp, *aux, vi[N], t_ini, t_fin;
    char *fmatrix, *fout;
    int nit, n_proc;

    srand(time(NULL));

    if(argc != 5)
    {
        printf("Error en el numero de argumentos.\n");
        exit(-1);
    }
    else
    {
        fmatrix=argv[1];
        fout=argv[2];
        nit= atoi(argv[3]);
        n_proc=atoi(argv[4]);
    }

    in = fopen(fmatrix, "rb");
    out = fopen(fout, "wb");

    m = (double**)malloc(N*sizeof(double*));
    xi = (double*)malloc(N*sizeof(double));
    temp = (double*)malloc(N*sizeof(double));
    abs = (double*)malloc((nit-1)*sizeof(double));

    //Inicializamos vector unidad.
    for(int i=0;i<N;i++)
        vi[i]=1;

    if(in == NULL) //Generación de matriz aleatoria si el fichero no existe.
    {
        for(int i=0;i<N;i++)
        {
            m[i] = (double*)malloc(N*sizeof(double));

            for(int j=0;j<N;j++)
            {
                if(i==j)
                    m[i][j] = 1.0;
                else if(i>j) 
                    m[i][j] = rand()%(LIMSUP+1); //Forma aleatoria
                    //(double)50*(i+1)*(j+1)/((double)N*N*10000); //Triangular inferior

                else
                    m[i][j] = (rand()%(-LIMINF+1))+LIMINF; //Forma aleatoria
                    //(double)-50*(i+1)*(j+1)/((double)N*N*10000); //Triangular superior
            }
            temp[i] = multRow(m[i], vi, N);
        }
    }
    else //Lectura del archivo si existe.
    {
        for(int i=0;i<N;i++)
        {
            m[i] = (double*)malloc(N*sizeof(double));
            fread(m[i], sizeof(unsigned char), N, in);
            temp[i] = multRow(m[i], vi, N); //Primera iteración M*vector unitario
        }
        fclose(in);
    }

    t_ini = omp_get_wtime();

    int np, iam, i, fragmento;
    double vabs_min, vabs_max;
    #pragma omp parallel num_threads(n_proc) private(iam, i, fragmento) shared(np, nit, xi, m, temp, abs, aux, vabs_min, vabs_max) default(none)
    {
        #pragma omp single //Num procesos.
            np = omp_get_num_threads();
        
        iam = omp_get_thread_num();  //Identificador de hilo.
        printf("soy %d de %d\n", iam, np);

        fragmento = N/np;

        #pragma omp master //Determina si hay resto y si lo hay, el 0 se queda las iteraciones restantes.
        {
            if(N%np != 0)
                fragmento += N&np;
        }

        for(int x=0;x<nit-1;x++) //Ejecución principal de las m iteraciones.
        {
            vabs_max=0, vabs_min=0;
            //Se reducen el min y el max de cada hilo.
            #pragma omp for schedule(dynamic, fragmento) reduction(max : vabs_max) reduction(min : vabs_min)
                for(i=0;i<N;i++)
                {
                    xi[i] = multRow(m[i], temp, N); //M iteraciones

                    //Obtención de max y min.
                    if(xi[i] < vabs_min)
                        vabs_min = xi[i];
                    if(xi[i] > vabs_max)
                        vabs_max = xi[i];
                }

            #pragma omp single //El primer hilo que llega obtiene el max_abs.
            {
                if(fabs(vabs_max) > fabs(vabs_min))
                    abs[x] = vabs_max;
                else
                    abs[x] = vabs_min;
            }

            #pragma omp single //Intercambio de punteros.
            {
                aux = temp;
                temp = xi;
                xi = aux;
            }

            if(fabs(abs[x]) > LIMVAL) //Si el max_abs es mayor que el limite, entra.
            {
                #pragma omp for schedule(dynamic, fragmento) //división de cada elemento del vector entre el max_abs.
                    for(i=0;i<N;i++)
                        temp[i] = temp[i]/abs[x];
            }
            #pragma omp barrier
        }
    }
    t_fin = omp_get_wtime();

    //Escritura del log.
    fprintf(out, "Fichero de entrada: %s\nElementos de mayor valor absoluto obtenidos: ", fmatrix);
    for(int i=0;i<nit-1;i++)
        fprintf(out, "%.1f ", abs[i]);
    fprintf(out, "\nNumero de hilos usados: %d\nTiempo de ejecucion: %.16g ms.\n", np, (t_fin-t_ini)*1000);
    
    fclose(out);

    for(int i=0;i<N;i++)
        free(m[i]);
    free(m);
    free(xi);
    free(abs);
    free(temp);

    printf("FIN DEL PROGRAMA\n\n");
    return 0;
}