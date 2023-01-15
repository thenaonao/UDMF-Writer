#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define C_SIZE 256

struct vertex{
    int id;
    float x;
    float y;
    float z;
};

struct linedef{
    int id; //Linedef ID
    int id_V1;//Vertex ID reference
    int id_V2;
    int blocking; //Should be 0 Unless its on the exterior of the square
    int special; //Exterior should have =9, else its 0.
    //Do not forget "dont draw" flag
    int id_S1;//Sidedef front reference
    int id_S2; //-1 default
};

struct sidedef{
    int sector; //sector ID reference
    char Textmiddle[9]; //Textures name are 8 char length, but +1 for the str end.
};

struct sector{
    int floor;//Floor height
    int ceil;//Ceil height Both should be 0, because we use the vertices to draw the terrain.
    
    char* textfloor;
    char* textceil; //texturefloor = "FLOOR0_1"; textureceiling = "CEIL1_1";
    
    int lightlevel; //Can be interesing if we want to cast fixed shadow from terrain... (For the future heh)
    int id; //Sector ID
};



struct vertex vertices[C_SIZE*C_SIZE];
struct linedef linedefs[(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))]; //Ouch thats a lot!
struct sidedef sidedefs[(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))]; //should be the same as linedefs
struct sector sectors[2*(C_SIZE-1)*(C_SIZE-1)];

int main(int argc, char *argv[]){
    float h=8192.0/C_SIZE;
    srand(time(NULL));
    
    int counter=0;
    printf("Creating the vertices..........");
    for(float y=0;y<C_SIZE;y++){
        for(float x=0;x<C_SIZE;x++){
            vertices[(int)y*C_SIZE+(int)x].id=counter;
            vertices[(int)y*C_SIZE+(int)x].x=h*x;
            vertices[(int)y*C_SIZE+(int)x].y=h*y;
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