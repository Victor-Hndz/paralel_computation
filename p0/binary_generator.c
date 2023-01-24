#include <stdio.h>
#include <stdlib.h>

#define ROW 5
#define COL 5

void readBin()
{
    FILE* f = fopen("out.bin", "rb");

    for(int i = 0; i < ROW; i++)
    {
        for(int j = 0; j < COL; j++)
        {
            int num;
            fread(&num, sizeof(int), 1, f);
            printf("%d ", num);
        }
        printf("\n");
    }

    fclose(f);
}


int main() 
{
    /* Create the file */
    int** x = (int**)malloc(ROW * sizeof(int*));
    for (int i = 0; i < COL; i++)
        x[i] = (int*)malloc(COL * sizeof(int));

    
    FILE* f = fopen("file.bin", "wb");
    if (f != NULL) 
    {
        for(int i = 0; i < ROW; i++)
        {
            for(int j = 0; j < COL; j++)
            {
                x[i][j] = rand()%1000;
                fwrite(&x[i][j], sizeof(int), 1, f);
            }
        }
        fclose (f);
    }
    
    readBin();


    for (int i = 0; i < COL; i++)
        free(x[i]);
    free(x);

    return 0;
}