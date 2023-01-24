#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define LOG "log.txt"
#define N 15000
#define LIMSUP 49
#define LIMINF -49
#define LIMVAL 25


/// @brief Operación de MPI nueva para calcular el máximo en valor absoluto.
/// @param in 
/// @param inout 
/// @param len 
/// @param dptr 
void maxvabs(double *in, double *inout, int* len, MPI_Datatype *dptr)
{
    int i;
    for(i=0;i<*len;++i)
        if(fabs(in[i]) > fabs(inout[i]))
            inout[i] = in[i];
}


/// @brief Multiplicación de dos vectores, fila x columna de dos matrices o de matriz y vector.
/// @param p1 
/// @param p2 
/// @param tam longitud de la fila.
/// @return valor del elemento resultado de la operación.
double multRow(double* p1, double* p2, int tam)
{
    double sum=0;
    int i;

    for (i=0;i<tam;i++)
        sum += p1[i]*p2[i];
    return sum;
}


/// @author Víctor Hernández Sánchez
int main(int argc, char **argv)
{
    FILE *in, *out;
    char *fmatrix;
    int i, j, nproces, myrank, nit, resto, *div, *scount, *despcount;
    double vabs, *temp, *temp_p, **m, *abs, secs; 
    clock_t t_ini, t_fin;
    MPI_Op mpi_max_abs; //Nueva operación de mpi para reduce.

    srand(time(NULL));

    MPI_Init(&argc, &argv);

    MPI_Op_create((MPI_User_function*)maxvabs,1,&mpi_max_abs); //Creación de la Operación nueva.

    MPI_Comm_size(MPI_COMM_WORLD,&nproces);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

    printf("Soy el numero %d de %d\n", myrank, nproces);

    fmatrix=argv[1];
    nit = atoi(argv[3]);

    temp = (double*)malloc(N*sizeof(double));
    div = (int*)malloc((nproces)*sizeof(int));
    scount = (int*)malloc((nproces)*sizeof(int));
    despcount = (int*)malloc((nproces)*sizeof(int));

    //Calculo de filas de cada proceso y del resto.
    resto=N%nproces;
    for(i=0;i<nproces;i++)
    {
        div[i] = (N/nproces);

        //Añadimos el resto.
        if(resto!=0)
        {
            div[i]++;
            resto--;
        }
        //calculo del despalazamiento y el tamaño a asignar en el scatter a cada proceso.
        scount[i] = div[i]*N;
        despcount[i] = scount[i]*i;
    }

    temp_p = (double*)malloc(div[myrank]*sizeof(double));
    abs = (double*)malloc((nit-1)*sizeof(double));

    if(myrank==0) //Proceso 0 inicialización y lectura.
    {
        in = fopen(fmatrix, "rb");
        m = (double**)malloc(N*sizeof(double*));
        m[0] = (double*)malloc(N*N*sizeof(double));

        for(int i=1;i<N;i++)
            m[i] = m[i-1] + N;

        if(in == NULL) //Generación de matriz aleatoria si el fichero no existe.
        {
            for(int i=0;i<N;i++)
            {
                for(int j=0;j<N;j++)
                {
                    if(i==j)
                        m[i][j] = 1.0;
                    else if(i>j) 
                        m[i][j] = (double)50*(i+1)*(j+1)/((double)N*N*10000); //Triangular inferior
                        //(rand()%(-LIMINF+1))+LIMINF; //Forma aleatoria
                    else
                        m[i][j] = (double)-50*(i+1)*(j+1)/((double)N*N*10000); //Triangular superior
                        //(rand()%(LIMSUP+1)); //Forma aleatoria
                }
            }
        }
        else //Si existe, le el fichero.
        {
            for(int i=0;i<N;i++)
                fread(m[i], sizeof(unsigned char), N, in);

            fclose(in);
        }
    }
    else //Resto de procesos, inicialización.
    {
        m = (double**)malloc(div[myrank]*sizeof(double*));
        m[0] = (double*)malloc(div[myrank]*N*sizeof(double));

        for(i=1;i<div[myrank];i++)
            m[i] = m[i-1] + N;
    }

    t_ini = clock();
    //Dispersión de la matriz desde 0 al resto con scatterv.
    MPI_Scatterv(m[0],scount,despcount,MPI_DOUBLE,m[0],N*N,MPI_DOUBLE,0,MPI_COMM_WORLD);

    //Inicializamos vector a 1.
    for(i=0;i<N;i++)
        temp[i]=1;

    //Modificamos desplazamiento y recepción a cada proceso para el Allgatherv.
    for(i=0;i<nproces;i++)
    {
        scount[i] = div[i];
        despcount[i] = scount[i]*i;
    }

    //printf("Soy %d y voy de la fila %d a la fila %d\n", myrank,offset[myrank],offset[myrank]+div[myrank]-1);
    //Calculo de cada proceso de su fragmento de vector.
    for(i=0;i<div[myrank];i++)
        temp_p[i] = multRow(m[i],temp,N);
    
    for(int x=0;x<nit-1;x++) //Ejecución principal de las m iteraciones.
    {
        vabs=0;
        //Unificación y dispersión de todos los fragmentos de del vector en uno de tmaño N. Todos obtienen una copia del vector entero.
        MPI_Allgatherv(temp_p,div[myrank],MPI_DOUBLE,temp,scount,despcount,MPI_DOUBLE,MPI_COMM_WORLD);
        for(i=0;i<div[myrank];i++)
        {
            //Multiplicación de cada trozo de matriz por el vector.
            temp_p[i] = multRow(m[i], temp, N);

            if(fabs(temp_p[i]) > fabs(vabs)) //Calculo del máximo valor absoluto.
                vabs = temp_p[i];
        }
        //Reduce y Broadcast a todos los procesos. Todos aportan su máximo, se calcula el máximo de todos ellos y se envía a todos el resultado.
        MPI_Allreduce(&vabs,&abs[x],1,MPI_DOUBLE,mpi_max_abs,MPI_COMM_WORLD);

        //Si el máximo es mayor que el limite, cada proceso divide su fragmento de matriz entre ese mismo valor.
        if(fabs(abs[x]) > LIMVAL)
            for(int i=0;i<div[myrank];i++)
                temp_p[i] = temp_p[i]/abs[x];
    }
    t_fin = clock();

    if(myrank==0) //Escritura del output por el proceso 0.
    {
        out = fopen(argv[2], "w");
        secs = (double)(t_fin - t_ini)/CLOCKS_PER_SEC;

        //Escritura del log.
        fprintf(out, "Fichero de entrada: %s\nElementos de mayor valor absoluto obtenidos: ", fmatrix);
        for(int i=0;i<nit-1;i++)
            fprintf(out, "%.1f ", abs[i]);
        fprintf(out, "\nNumero de procesos usados: %d\nTiempo de ejecucion: %.16g ms.\n", nproces, secs*1000);
        
        fclose(out);
    }

    //Liberación de memoria.
    free(temp);
    free(div);
    free(m[0]);
    free(m);
    free(temp_p);
    free(abs);
    free(scount);
    free(despcount);
    MPI_Op_free(&mpi_max_abs);
    
    MPI_Finalize();

    printf("EXIT: %d\n", myrank);

    return 0;
}
