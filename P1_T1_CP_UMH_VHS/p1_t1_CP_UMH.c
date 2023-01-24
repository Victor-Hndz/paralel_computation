#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define MEDIA 0
#define MEDIANA 1
#define SOBEL 2
#define OUT "out.raw"
#define LOG "log.txt"


/// @brief filtrado de pixeles por sobel.
/// @param m matriz a filtrar (ya expandida).
/// @param filtrada matriz donde se almacena la filtrada.
/// @param rows filas.
/// @param cols columnas.
/// @param out archivo de salida.
unsigned char* sobel(unsigned char **m, unsigned char **filtrada, int rows, int cols, int pi)
{
    int sumF, sumC, j;
    unsigned char *ant, *act, *sig, *filt;

    ant = m[pi-1];
    act = m[pi];
    sig = m[pi+1];
    filt = filtrada[pi-1];

    for(j=1;j<=cols;j++)
    {
        sumC = -ant[j-1] -(2*ant[j]) -ant[j+1] +sig[j-1] +2*sig[j] +sig[j+1];
        sumF = -ant[j-1] +ant[j+1] -(2*act[j-1]) +2*act[j+1] -sig[j-1] +sig[j+1];

        filt[j-1] = sqrt(sumC*sumC+sumF*sumF);
    }
    return filt;
}

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


/// @author Víctor Hernández Sánchez
int main(int argc, char **argv)
{
    int i, j, nproces, myrank, mode, rows, cols, tamfilt, *div, resto, extra=2, *offset;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD,&nproces);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

    printf("Soy el numero %d de %d\n", myrank, nproces);

    if(strcmp(argv[2], "media") == 0)
        mode = MEDIA;
    else if(strcmp(argv[2], "mediana")==0)
        mode = MEDIANA;
    else if(strcmp(argv[2], "sobel")==0)
        mode = SOBEL;
    else
    {
        printf("\x1b[31m"
                "ERROR: " 
                "Error en el segundo argumento. Debe ser 'media', 'mediana' o 'sobel'.\n"
                "\x1b[0m");
        exit(-1);
    }

    rows = atoi(argv[3]);
    cols = atoi(argv[4]);
    tamfilt = atoi(argv[5]);

    div = (int*)malloc((nproces)*sizeof(int));
    offset = (int*)malloc((nproces)*sizeof(int));

    //Calculamos la divison de cada proceso y el offset que tendrá.
    resto=rows%nproces;
    for(i=0;i<nproces;i++)
    {
        div[i] = (rows/nproces);
        offset[i]=(div[i]*i)-1;
    }
    //Añadimos el resto.
    if(resto!=0)
        div[nproces-1] += resto;

    if(myrank==0)
    {
        FILE *in, *out, *log;
        char *filename;
        unsigned char **m, **mfilt;
        double secs;
        clock_t t_ini, t_fin;

        filename = argv[1];
        in = fopen(filename, "rb");

        out = fopen(OUT, "wb");
        log = fopen(LOG, "w");

        m = (unsigned char**)malloc((rows+2)*sizeof(unsigned char*));
        mfilt = (unsigned char**)malloc((rows)*sizeof(unsigned char*)); 

        if(mode==2)
        {
            m[0] = (unsigned char*)malloc((cols+2)*sizeof(unsigned char));
            m[rows+1] = (unsigned char*)malloc((cols+2)*sizeof(unsigned char));
        }        
        for(i=0;i<rows;i++)
        {
            m[i+1] = (unsigned char*)malloc((cols+2)*sizeof(unsigned char));
            mfilt[i] = (unsigned char*)malloc((cols)*sizeof(unsigned char));

            m[i+1]++;
            fread(m[i+1], sizeof(unsigned char), cols, in);
            m[i+1]--;
            //lado derecho e izquierdo del borde (sobel)
            m[i+1][0] = m[i+1][2];
            m[i+1][rows+1] = m[i+1][rows-1];
        }
        fclose(in);
        //Comenzamos a contar el tiempo
        t_ini = clock();
        if(mode==2)
        {
            //Fila superior e inferior del borde
            memcpy(m[0], m[2], sizeof(unsigned char)*cols+2);
            memcpy(m[rows+1], m[rows-1], sizeof(unsigned char)*cols+2);

            //esquinas del borde
            m[0][0] = m[2][2];
            m[rows+1][0] = m[rows-1][2];
            m[0][cols+1] = m[2][cols-1];
            m[rows+1][cols+1] = m[rows-1][cols-1];

            for(i=1;i<nproces;i++)
            {
                //printf("Enviando desde fila %d hasta la fila %d al proceso %d\n",offset[i]+1,offset[i]+(div[i]+extra),i);
                for(j=1;j<=div[i]+extra;j++)
                    MPI_Send(m[j+offset[i]],cols+2,MPI_UNSIGNED_CHAR,i,i+1,MPI_COMM_WORLD);
            }
            //Calculo del proceso 0
            for(i=1;i<=div[myrank];i++)
                mfilt[i-1] = sobel(m,mfilt,rows,cols+2,i);

            //Recepciones de los demás procesos, ya hecho el calculo.
            for(i=1;i<nproces;i++)
                for(j=0;j<div[i];j++)  
                    MPI_Recv(mfilt[j+offset[i]+1],cols,MPI_UNSIGNED_CHAR,i,i+1,MPI_COMM_WORLD,&status);
        }
        else
        {
            //Enviamos a cada proceso su parte a procesar.
            for(i=1;i<nproces;i++)
            {   
                //El ultimo proceso no necesita una fila extra superior para procesar.
                if(i==nproces-1)
                    extra--;       
                
                //printf("soy %d y mi divi+extra es %d\n",i,div[i]+extra);
                //printf("Enviando desde fila %d hasta la fila %d al proceso %d\n",offset[i],offset[i]+(div[i]+extra-1),i);
                for(j=0;j<div[i]+extra;j++)
                {
                    m[j+offset[i]+1]++;
                    MPI_Send(m[j+offset[i]+1],cols,MPI_UNSIGNED_CHAR,i,i+1,MPI_COMM_WORLD);
                    m[j+offset[i]+1]--;
                }
            }
            //Procesamos la sección que le toca a 0.
            printf("nproces=%d\n",nproces);
            for(i=0;i<div[myrank];i++)
            {
                for(j=0;j<cols;j++)
                {
                    //printf("i=%d, j=%d\n",i,j);
                    if(nproces==1)
                    {
                        if(j<1 || i<1 || j>(cols-2) || i>(rows-2))
                            mfilt[i][j] = m[i+1][j+1];
                        else
                        {
                            if(mode==0)
                                mfilt[i][j] = avgPixel(i,j,m,tamfilt);
                            else
                                mfilt[i][j] = medianaPixel(i,j,m,tamfilt);
                        }
                    }
                    else
                    {
                        if(j<1 || i<1 || j>(cols-2))
                            mfilt[i][j] = m[i+1][j+1];
                        else
                        {
                            if(mode==0)
                                mfilt[i][j] = avgPixel(i,j,m,tamfilt);
                            else
                                mfilt[i][j] = medianaPixel(i,j,m,tamfilt);
                        }
                    }
                }
            }

            //Recibimos los resultados del resto de procesos.
            for(i=1;i<nproces;i++)
            {   
                //MPI_Recv(mfilt[offset[i]+1],cols,MPI_UNSIGNED_CHAR,i,i+1,MPI_COMM_WORLD,&status);
                //printf("Reciviendo desde fila %d hasta la fila %d del proceso %d\n",offset[i],offset[i]+(div[i]-1),i);
                for(j=0;j<div[i];j++)  
                    MPI_Recv(mfilt[j+offset[i]+1],cols,MPI_UNSIGNED_CHAR,i,i+1,MPI_COMM_WORLD,&status);
                //printf("CERO: %d-pos %d: filtrada0=%d\n",i,offset[i]+1,mfilt[offset[i]+1][248]);
            }

            //La ultima fila y la primera no se procesan, se quedan igual.
            m[rows]++;
            m[1]++;
            memcpy(mfilt[rows-1],m[rows],sizeof(unsigned char)*cols);
            memcpy(mfilt[0],m[1],sizeof(unsigned char)*cols);
            m[rows]--;
            m[1]--;
        }

        //Escribimos la matriz en el fichero resultado.
        for(i=0;i<rows;i++)
            fwrite(mfilt[i], sizeof(unsigned char), cols, out);
        fclose(out);

        t_fin = clock();
        secs = (double)(t_fin - t_ini)/CLOCKS_PER_SEC;

        //Escritura del log.
        fprintf(log, "Fichero de entrada: %s\nTamaño de la imagen: %dx%d\nNombre del fichero de salida: %s\nMétodo usado: %s\nNumero de procesos: %d\nTiempo de ejecucion: %.16g ms.\n", filename, rows, cols, OUT, argv[2], nproces, secs*1000);
        fclose(log);
    
        for(i=0;i<rows;i++)
            free(mfilt[i]);
        
        for(i=1;i<=rows;i++)
            free(m[i]);
        if(mode==2)
        {
            free(m[0]);
            free(m[rows+1]);
        }
        free(m);
        free(mfilt);
    }
    //Procesos 1 a n-1
    else
    {
        unsigned char **mproc, **mfiltproc;

        //El ultimo proceso no necesita una fila extra superior para procesar.
        if(myrank==nproces-1 && mode != 2)
            extra--;

        mproc = (unsigned char**)malloc((div[myrank]+extra)*sizeof(unsigned char*));
        mfiltproc = (unsigned char**)malloc((div[myrank])*sizeof(unsigned char*));
    
        if(mode==2)
        {
            for(i=0;i<div[myrank];i++)
                mfiltproc[i] = (unsigned char*)malloc((cols+2)*sizeof(unsigned char));

            //Recibimos de 0.
            for(j=0;j<div[myrank]+extra;j++)   
            {
                mproc[j] = (unsigned char*)malloc((cols+2)*sizeof(unsigned char));
                MPI_Recv(mproc[j],cols+2,MPI_UNSIGNED_CHAR,0,myrank+1,MPI_COMM_WORLD,&status);
            }

            //Procesamos.
            for(i=1;i<=div[myrank];i++)
                mfiltproc[i-1] = sobel(mproc,mfiltproc,rows,cols+2,i);
        }
        else
        {
            for(i=0;i<div[myrank];i++)
                mfiltproc[i] = (unsigned char*)malloc(cols*sizeof(unsigned char));

            //Recibimos las filas a procesar + las extras necesarias para calcular correctamente.
            for(j=0;j<div[myrank]+extra;j++)
            {
                mproc[j] = (unsigned char*)malloc(cols*sizeof(unsigned char));
                MPI_Recv(mproc[j],cols,MPI_UNSIGNED_CHAR,0,myrank+1,MPI_COMM_WORLD,&status);
            }

            //Procesamos la media
            if(myrank==nproces-1)
                div[myrank]--;
            for(i=0;i<div[myrank];i++)
            {
                for(j=0;j<cols;j++)
                {      
                    if(j<1 || j>(cols-2))
                        mfiltproc[i][j]=mproc[i+1][j];
                    else
                    {
                        if(mode==0)
                            mfiltproc[i][j]=avgPixel(i,j-1,mproc,tamfilt);
                        else
                            mfiltproc[i][j]=medianaPixel(i,j-1,mproc,tamfilt);
                    }
                }
            }
            if(myrank==nproces-1)
                div[myrank]++;
        }

        //printf("holi desde %d a 0.\n", myrank);
        //Enviamos al proceso 0 el resultado.
        for(j=0;j<div[myrank];j++)  
            MPI_Send(mfiltproc[j],cols,MPI_UNSIGNED_CHAR,0,myrank+1,MPI_COMM_WORLD);

        for(i=0;i<div[myrank];i++)
            free(mfiltproc[i]);

        for(i=0;i<div[myrank]+extra;i++)
            free(mproc[i]);
    
        free(mfiltproc);
        free(mproc);
    }
    
    free(div);
    free(offset);
    MPI_Finalize();

    printf("FIN DEL PROGRAMA %d\n\n", myrank);
    //fflush(stdout);

    return 0;
}