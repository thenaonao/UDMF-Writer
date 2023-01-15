#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define C_SIZE 1024

struct vertex{
    int id;
    float x;
    float y;
    float z;
};

struct linedef{
    int id;
    int id_V1;
    int id_V2;
    int blocking;
    int special;
    //dont draw flag needed
    int id_S1;//Sidedef front
    int id_S2;
};

struct sidedef{
    int sector;
    char Textmiddle[9]; //Textures name are 8 char length, but +1 for the str end.
};

struct sector{
    int floor;
    int ceil;
    
    char* textfloor;
    char* textceil;
    
    int lightlevel;
    int id;
};



struct vertex vertices[C_SIZE*C_SIZE];

int main(int argc, char *argv[]){
    srand(time(NULL));
    
    int counter=0;
    printf("Creating the vertices..........");
    for(float y=0;y<C_SIZE;y++){
        for(float x=0;x<C_SIZE;x++){
            vertices[(int)y*C_SIZE+(int)x].id=counter;
            vertices[(int)y*C_SIZE+(int)x].x=x;
            vertices[(int)y*C_SIZE+(int)x].y=y;
            vertices[(int)y*C_SIZE+(int)x].z=0.0; //(float) (rand()%64);
            counter++;
        }
    }
    printf("Finished!\n");

    FILE *fptr;
    fptr=fopen("TEXTMAP","w+");
    
    //Writes the header of the file
    printf("Writing Header..........");
    fprintf(fptr,"%s","namespace = \"zdoom\";\n");
    printf("Finished!\n");

    //Writes each linedefs

    //Writes each sidedefs


    //Writes each vertex in the file
    printf("Writing Vertices");
    for(int i=0;i<C_SIZE*C_SIZE;i++){
        fprintf(fptr,"vertex // %d\n{\nx = %.1f;\ny = %.1f;\nzfloor = %.1f;\n}\n\n",vertices[i].id,vertices[i].x,vertices[i].y,vertices[i].z);
        if(i%(C_SIZE*C_SIZE/10)==0 &&i!=0){
            printf(".");
        }
    }
    printf("Finished!\n");

    //Writes each sectors...

    //Things here...

    
    printf("Job Done!\n");
    fclose(fptr);
    return 0;
}