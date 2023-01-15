#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define C_SIZE 768

struct vertex{
    int id;
    float x;
    float y;
    float z;
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


    //Writes each vertex in the file
    printf("Writing Vertices");
    for(int i=0;i<C_SIZE*C_SIZE;i++){
        fprintf(fptr,"vertex // %d\n{\nx = %.1f;\ny = %.1f;\nzfloor = %.1f;\n}\n\n",vertices[i].id,vertices[i].x,vertices[i].y,vertices[i].z);
        if(i%(C_SIZE*C_SIZE/10)==0 &&i!=0){
            printf(".");
        }
    }
    printf("Finished!\n");

    fclose(fptr);
    return 0;
}