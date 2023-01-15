#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define C_SIZE 32
#define C_MAP_LENGTH 8192.0

struct vertex{
    unsigned int id;
    float x;
    float y;
    float z;
};

struct linedef{
    unsigned int id; //Linedef ID
    unsigned int id_V1;//Vertex ID reference
    unsigned int id_V2;
    unsigned int blocking; //Should be 0 Unless its on the exterior of the square
    unsigned int twosided; //Should be 1 if its sandwiched between 2 sectors
    int special; //Exterior should have =9, else its 0.
    //Do not forget "dont draw" flag
    unsigned int id_S1;//Sidedef front reference
    unsigned int id_S2; //-1 default
};

struct sidedef{
    unsigned int sector; //sector ID reference
    char Textmiddle[9]; //Textures name are 8 char length, but +1 for the str end.
                        // STARTAN2
};

struct sector{
    int floor;//Floor height
    int ceil;//Ceil height Both should be 0, because we use the vertices to draw the terrain.
    
    char* textfloor;
    char* textceil; //texturefloor = "FLOOR0_1"; textureceiling = "CEIL1_1";
    
    int lightlevel; //Can be interesing if we want to cast fixed shadow from terrain... (For the future heh)
    unsigned int id; //Sector ID
};



struct vertex vertices[C_SIZE*C_SIZE];
struct linedef linedefs[(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))]; //Ouch thats a lot!
struct sidedef sidedefs[(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))]; //should be the same as linedefs
struct sector sectors[2*(C_SIZE-1)*(C_SIZE-1)];

int main(int argc, char *argv[]){
    float h=C_MAP_LENGTH/C_SIZE;
    srand(time(NULL));
    
    int counter=0;

    printf("Creating the Vertices");
    for(float y=0;y<C_SIZE;y++){
        for(float x=0;x<C_SIZE;x++){
            //Vertices
            vertices[(int)y*C_SIZE+(int)x].id=counter;
            vertices[(int)y*C_SIZE+(int)x].x=h*x;
            vertices[(int)y*C_SIZE+(int)x].y=h*y;
            vertices[(int)y*C_SIZE+(int)x].z=0.0; //(float) (rand()%64);

            if( ((int)y*C_SIZE+(int)x)%(C_SIZE*C_SIZE/10)==0 &&((int)y*C_SIZE+(int)x)!=0){
                printf(".");
            }
            counter++;
        }
    }
    printf("Finished!\n");

    counter=0;
    printf("Creating the Linedefs & Sidedefs");
    for(float y=0;y<C_SIZE;y++){
        for(float x=0;x<C_SIZE;x++){
            if(x==C_SIZE-1 && y==C_SIZE-1){
                continue;
            }else if(x==C_SIZE-1){
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
            }else if (y==C_SIZE-1){
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
            }else if(x==0 && y==0){
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;                
            }else if(x==0){
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
            }else if(y==0){
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
            }else{
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=counter;
                linedefs[counter].id_S2=-1;
                counter++;
            }
            if( ((int)y*C_SIZE+(int)x)%(C_SIZE*C_SIZE/10)==0 &&((int)y*C_SIZE+(int)x)!=0){
                printf(".");
            }
        }   
    }
    printf("Finished!\n");

    printf("Creating TEXTMAP Lump.\n");
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

    //Writes each linedefs & sidedefs... They should be written in the same loop.(Because we have no backsided sidedefs).
    printf("Writing Linedefs & Sidedefs");
    for(int i=0;i<(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1));i++){
        if(i!=0 && linedefs[i].id==0){
            continue;
        }
        fprintf(fptr,"linedef // %d\n{\nv1 = %d;\nv2 = %d;\nspecial = %d;\nsidefront = %d;\nsideback = %d;\n%s = true;\n}\n\n",linedefs[i].id,linedefs[i].id_V1,linedefs[i].id_V2,linedefs[i].special,linedefs[i].id_S1,linedefs[i].id_S2,(linedefs[i].blocking==1)?"blocking":"twosided");
        if(i%((C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))/10)==0 &&i!=0){
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