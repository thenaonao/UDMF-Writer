#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define C_SIZE 136 //width of vertices & height
#define C_MAP_LENGTH 16384.0 //32768.0
#define C_MAP_MAX_HEIGHT 2048.0 //1536.0

#define C_MountainThreshold 500.0
#define C_DirtThreshold 150.0
#define DEBUG





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
    unsigned int id;
    unsigned int sector; //sector ID reference
    char Textmiddle[9]; //Textures name are 8 char length, but +1 for the str end.
                        // STARTAN2
};

struct sector{
    int floor;//Floor height
    int ceil;//Ceil height Both should be 0, because we use the vertices to draw the terrain.
    
    char textfloor[9];
    char textceil[9]; //texturefloor = "FLOOR0_1"; textureceiling = "CEIL1_1";
    int special;
    int lightlevel; //Can be interesing if we want to cast fixed shadow from terrain... (For the future heh)
    unsigned int id; //Sector ID
};

//Global vars
struct vertex vertices[C_SIZE*C_SIZE];
struct linedef linedefs[(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))];
struct sidedef sidedefs[2*(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))];
struct sector sectors[2*(C_SIZE-1)*(C_SIZE-1)];
float zfloor[C_SIZE*C_SIZE];


float interpolate(float a0, float a1, float w);
float dotproduct(int ix,int iy, float x, float y);
void generateRandomVector(int ix, int iy,float* vx, float* vy);
float perlin(float x,float y);
float AQWABS(float a);

void generateZMap();
void generateVertices(float h);
int generateLineAndSidedefs(unsigned int c);
void generateSectors();
void injectThings();
int getVertexIdFromSectorID(int id);

float AQWConvolution(float *tab,int x,int y);

int main(int argc, char *argv[]){

    //Todo creates input arguments to set C_SIZE and C_MAP_LENGTH
    //Also if we can enable or not Debug flag.


    float h=C_MAP_LENGTH/C_SIZE;
    srand(time(NULL));
    

    

    

    #ifdef DEBUG
    printf("Theorical number max of vertices: %d\n",C_SIZE*C_SIZE);
    printf("Theorical number max of linedefs: %d\n",(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1)));
    printf("Theorical number max of sidedefs: %d\n",2*(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1)));
    printf("Theorical number max of sectors : %d\n",2*(C_SIZE-1)*(C_SIZE-1));
    #endif

    generateZMap();
    generateVertices(h);
    unsigned int backsidecounter=(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1));
    backsidecounter=generateLineAndSidedefs(backsidecounter);
    generateSectors();
    
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
        //printf("%f\n",vertices[i].z);
        if(C_SIZE>=4){
            if(i%(C_SIZE*C_SIZE/10)==0 &&i!=0){
                printf(".");
            }
        }
    }
    printf("Finished!\n");

    //Writes each linedefs...
    printf("Writing Linedefs");
    for(int i=0;i<(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1));i++){
        if(i!=0 && linedefs[i].id==0){
            continue;
        }
        if(linedefs[i].special==0){
            fprintf(fptr,"linedef // %d\n{\nv1 = %d;\nv2 = %d;\nsidefront = %d;\nsideback = %d;\n%s = true;\ndontdraw = true;\n}\n\n",linedefs[i].id,linedefs[i].id_V1,linedefs[i].id_V2,linedefs[i].id_S1,linedefs[i].id_S2,(linedefs[i].blocking==1)?"blocking":"twosided");
        }else{
            fprintf(fptr,"linedef // %d\n{\nv1 = %d;\nv2 = %d;\nspecial = %d;\nsidefront = %d;\nsideback = %d;\n%s = true;\ndontdraw = true;\n}\n\n",linedefs[i].id,linedefs[i].id_V1,linedefs[i].id_V2,linedefs[i].special,linedefs[i].id_S1,linedefs[i].id_S2,(linedefs[i].blocking==1)?"blocking":"twosided");
        }
        if(C_SIZE>=4){
            if(i%((C_SIZE-1)*(C_SIZE+(2*C_SIZE-1))/10)==0 &&i!=0){
                printf(".");
            }
        }
    }
    printf("Finished!\n");

    //Writes each sidedefs...
    printf("Writing Sidedefs");
    for(int i=0;i<backsidecounter;i++){
        /*if( i>=sidecounter && sidedefs[i].sector==0 ){
            continue;
        }*/
    
        //fprintf(fptr,"sidedef // %d\n{\noffsetx = 0;\noffsety = 0;\ntexturetop = \"-\";\ntexturebottom = \"-\";\ntexturemiddle = \"%s\";\nsector = %d;\n}\n\n",sidedefs[i].id,sidedefs[i].Textmiddle,sidedefs[i].sector);
        fprintf(fptr,"sidedef // %d\n{\nsector = %d;\n}\n\n",sidedefs[i].id,sidedefs[i].sector);
        if(i%((2*(C_SIZE-1)*(C_SIZE+(2*C_SIZE-1)))/10)==0 &&i!=0){
            printf(".");
        }
    }
    printf("Finished!\n");

    //Writes each sectors...
    printf("Writing Sectors");
    for(int i=0;i<2*(C_SIZE-1)*(C_SIZE-1);i++){
        /*if(i!=0 && sectors[i].id==0){
            continue;
        }*/
        //fprintf(fptr,"sector // %d\n{\nheightfloor = %d;\nheightceiling = %d;\ntexturefloor = \"%s\";\ntextureceiling = \"%s\";\nlightlevel = %d;\nspecial = %d;\nid = %d;\n}\n\n",sectors[i].id,sectors[i].floor,sectors[i].ceil,sectors[i].textfloor,sectors[i].textceil,160,0,sectors[i].id);
        if(sectors[i].special==0){
            fprintf(fptr,"sector // %d\n{\nheightfloor = %d;\nheightceiling = %d;\ntexturefloor = \"%s\";\ntextureceiling = \"%s\";\nlightlevel = %d;\n}\n\n",sectors[i].id,sectors[i].floor,sectors[i].ceil,sectors[i].textfloor,sectors[i].textceil,sectors[i].lightlevel);
        }else{
            fprintf(fptr,"sector // %d\n{\nheightfloor = %d;\nheightceiling = %d;\ntexturefloor = \"%s\";\ntextureceiling = \"%s\";\nlightlevel = %d;\nspecial = %d;\n}\n\n",sectors[i].id,sectors[i].floor,sectors[i].ceil,sectors[i].textfloor,sectors[i].textceil,sectors[i].lightlevel,sectors[i].special);
        }
        if(C_SIZE>=4){
            if(i%(2*(C_SIZE-1)*(C_SIZE-1)/10)==0 &&i!=0){
                printf(".");
            }
        }
    }
    printf("Finished!\n");

    //Things here...
    printf("Injecting Things...\n");
    fprintf(fptr,"thing // 0\n{\nx = 128.0;\ny = 128.0;\ntype = 1;\n}");
    printf("Finished!\n");
    printf("Job Done!\n");
    printf("Nummber of sidedefs: %d\n%s\n",backsidecounter,(backsidecounter>65535*2)?"Generated too many sidedefs, please retry!":"Ok!");
    fclose(fptr);
    return 0;
}

//From wikipedia, "https://en.wikipedia.org/wiki/Perlin_noise"
//Adapted a bit for my purpose.
float interpolate(float a0, float a1, float w){
    if (0.0 > w) return a0;
    if (1.0 < w) return a1;
    return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

float dotproduct(int ix,int iy, float x, float y){
    float vx,vy;
    generateRandomVector(ix, iy,&vx,&vy);
    float dx = x - (float)ix;
    float dy = y - (float)iy;
    return (dx*vx+ dy*vy);
}
void generateRandomVector(int ix, int iy,float* vx, float* vy){
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; // rotation width
    unsigned a = ix, b = iy;
    a *= 3284157443; b ^= a << s | a >> w-s;
    b *= 1911520717; a ^= b << s | b >> w-s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    *vx = cos(random);
    *vy = sin(random);
}
float perlin(float x,float y){
    int x0 = (int)floor(x);
    int x1 = x0 + 1;
    int y0 = (int)floor(y);
    int y1 = y0 + 1;
    float sx = x - (float)x0;
    float sy = y - (float)y0;
    float n0, n1, ix0, ix1, value;

    n0 = dotproduct(x0, y0, x, y);
    n1 = dotproduct(x1, y0, x, y);
    ix0 = interpolate(n0, n1, sx);
    n0 = dotproduct(x0, y1, x, y);
    n1 = dotproduct(x1, y1, x, y);
    ix1 = interpolate(n0, n1, sx);
    value = interpolate(ix0, ix1, sy);
    return value;
}

void generateZMap(){
    printf("Generating vertices height...");
    int zcount=0;
    float zbuffer[C_SIZE*C_SIZE];
    for(float y=0;y<C_SIZE/8;y+=0.125){
        for (float x=0;x<C_SIZE/8;x+=0.125)
        {
            if(x==0 || y==0 || x==C_SIZE/8-0.125 || y==C_SIZE/8-0.125){
                zbuffer[zcount]=0;//We tie it to 0
            }else{
                zbuffer[zcount]=perlin(x,y)*C_MAP_MAX_HEIGHT+perlin(4*x,4*y)*C_MAP_MAX_HEIGHT/4+perlin(16*x,16*y)*C_MAP_MAX_HEIGHT/16;
            }
            zcount++;
        }
    }
    zcount=0;
    for(float y=0;y<C_SIZE;y++){
        for (float x=0;x<C_SIZE;x++)
        {
            if(x==0 || y==0 || x==C_SIZE-1 || y==C_SIZE-1){
                zfloor[zcount]=0;
            }else{
                zfloor[zcount]=zbuffer[zcount];//AQWConvolution(zbuffer,(int)x,(int)y);
            }
            zcount++;
        }
        
    }
    printf("Finished!\n");
}

void generateVertices(float h){
    printf("Creating the Vertices");
    int counter=0;
    for(float y=0;y<C_SIZE;y++){
        for(float x=0;x<C_SIZE;x++){
            //Vertices
            vertices[(int)y*C_SIZE+(int)x].id=counter;
            vertices[(int)y*C_SIZE+(int)x].x=h*x;
            vertices[(int)y*C_SIZE+(int)x].y=h*y;
            vertices[(int)y*C_SIZE+(int)x].z=zfloor[(int)y*C_SIZE+(int)x];
            if(C_SIZE>=4){
                if( ((int)y*C_SIZE+(int)x)%(C_SIZE*C_SIZE/10)==0 &&((int)y*C_SIZE+(int)x)!=0){
                    printf(".");
                }
            }
            counter++;
        }
    }
    printf("Finished!\n");
}

int generateLineAndSidedefs(unsigned int backsidecounter){
    unsigned int counter=0;
    unsigned int sidecounter=0;
    printf("Creating the Linedefs & Sidedefs & Sectors");
    for(float y=0;y<C_SIZE;y++){
        for(float x=0;x<C_SIZE;x++){
            if(x==C_SIZE-1 && y==C_SIZE-1){
                continue;
            }else if(x==C_SIZE-1){
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE); //So we inverted the order  because we want the linedef to be facing the other side!
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=sidecounter;
                linedefs[counter].id_S2=-1;
                counter++;

                //SL2
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)-1;
                sidecounter++;

            }else if (y==C_SIZE-1){
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=sidecounter;
                linedefs[counter].id_S2=-1;
                counter++;

                //SL1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)-2*(C_SIZE-1)+1;
                sidecounter++;
            }else if(x==0 && y==0){
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1; //Lets not forget to change the order!
                linedefs[counter].id_V2=(int)(x+y*C_SIZE); //Should be the vertex on the right.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=sidecounter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=sidecounter+1;
                linedefs[counter].id_S2=-1;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter+2;
                linedefs[counter].id_S2=backsidecounter;
                counter++;
                

                //SL1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;

                //SL2
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;

                //SL3-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)+1;
                //SL3-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1);

                sidecounter++;
                backsidecounter++;
            }else if(x==0){
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter;
                linedefs[counter].id_S2=backsidecounter;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=sidecounter+1;
                linedefs[counter].id_S2=-1;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter+2;
                linedefs[counter].id_S2=backsidecounter+1;
                counter++;

                //SL1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)-2*(C_SIZE-1)+1;
                //SL1-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;
                backsidecounter++;

                //SL2-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;

                //SL3-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)+1;
                //SL3-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;
                backsidecounter++;

            }else if(y==0){
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1; //We change the order 
                linedefs[counter].id_V2=(int)(x+y*C_SIZE); //Should be the vertex on the right.
                linedefs[counter].blocking=1;
                linedefs[counter].twosided=0;
                linedefs[counter].special=9;
                linedefs[counter].id_S1=sidecounter;
                linedefs[counter].id_S2=-1;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter+1;
                linedefs[counter].id_S2=backsidecounter;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter+2;
                linedefs[counter].id_S2=backsidecounter+1;
                counter++;

                //SL1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;

                //SL2-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                //SL2-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1)-1;
                sidecounter++;
                backsidecounter++;

                //SL3-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)+1;
                //SL3-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;
                backsidecounter++;
            }else{
                //L1
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+1; //Should be the vertex on the right.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter;
                linedefs[counter].id_S2=backsidecounter;
                counter++;
                //L2
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE);
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter+1;
                linedefs[counter].id_S2=backsidecounter+1;
                counter++;
                //L3
                linedefs[counter].id=counter;
                linedefs[counter].id_V1=(int)(x+y*C_SIZE)+1;  //Should be the vertex on the right.
                linedefs[counter].id_V2=(int)(x+y*C_SIZE)+C_SIZE; //Should be the vertex on the same column but +1 line higher.
                linedefs[counter].blocking=0;
                linedefs[counter].twosided=1;
                linedefs[counter].special=0;
                linedefs[counter].id_S1=sidecounter+2;
                linedefs[counter].id_S2=backsidecounter+2;
                counter++;

                //SL1-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)-2*(C_SIZE-1)+1;
                //SL1-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;
                backsidecounter++;

                //SL2-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                //SL2-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1)-1;
                sidecounter++;
                backsidecounter++;

                //SL3-1
                sidedefs[sidecounter].id=sidecounter;
                strcpy(sidedefs[sidecounter].Textmiddle,"-");
                sidedefs[sidecounter].sector=(x*2)+y*2*(C_SIZE-1)+1;
                //SL3-2
                sidedefs[backsidecounter].id=backsidecounter;
                strcpy(sidedefs[backsidecounter].Textmiddle,"-");
                sidedefs[backsidecounter].sector=(x*2)+y*2*(C_SIZE-1);
                sidecounter++;
                backsidecounter++;
            }
            if(C_SIZE>=4){
                if( ((int)y*C_SIZE+(int)x)%(C_SIZE*C_SIZE/10)==0 &&((int)y*C_SIZE+(int)x)!=0){
                    printf(".");
                }
            }
        }   
    }
    printf("Finished!\n");
    return backsidecounter;//sloppy implementation, but im lazy
}

void generateSectors(){
    unsigned int sectorcounter=0;
    float V1,V2,V3,V4;
    int id;
    printf("Creating the Sectors");
    for(float y=0;y<C_SIZE-1;y++){
        for(float x=0;x<C_SIZE-1;x++){
            id = getVertexIdFromSectorID(sectorcounter);
            V1 = vertices[id].z;
            V2 = vertices[id+1].z;
            V3 = vertices[id+1+C_SIZE].z;
            V4 = vertices[id+C_SIZE].z;
            //Sector1
            sectors[sectorcounter].floor=0;
            sectors[sectorcounter].ceil=4096;
            if(V1 > C_MountainThreshold || V2 > C_MountainThreshold || V4 > C_MountainThreshold){
                strcpy(sectors[sectorcounter].textfloor,"FLAT5_7");
            }else if(V1 > C_DirtThreshold || V2 > C_DirtThreshold || V4 > C_DirtThreshold){
                strcpy(sectors[sectorcounter].textfloor,"FLAT10");
            }else{
                strcpy(sectors[sectorcounter].textfloor,"GRASS1");
            }
            
            strcpy(sectors[sectorcounter].textceil,"F_SKY1");                
            sectors[sectorcounter].lightlevel=160; 
            sectors[sectorcounter].id=sectorcounter;
            sectorcounter++;
            
            //Sector2
            sectors[sectorcounter].floor=0;
            sectors[sectorcounter].ceil=4096;
            if(V2 > C_MountainThreshold || V3 > C_MountainThreshold || V4 > C_MountainThreshold){
                strcpy(sectors[sectorcounter].textfloor,"FLAT5_7");
            }else if(V2 > C_DirtThreshold || V3 > C_DirtThreshold || V4 > C_DirtThreshold){
                strcpy(sectors[sectorcounter].textfloor,"FLAT10");
            }else{
                strcpy(sectors[sectorcounter].textfloor,"GRASS1");
            }
            strcpy(sectors[sectorcounter].textceil,"F_SKY1");                
            sectors[sectorcounter].lightlevel=160; 
            sectors[sectorcounter].id=sectorcounter;
            sectorcounter++;

            if(C_SIZE>=4){
                if( ((int)y*C_SIZE+(int)x)%(C_SIZE*C_SIZE/10)==0 &&((int)y*C_SIZE+(int)x)!=0){
                    printf(".");
                }
            }
        }
        
    }
    printf("Finished!\n");
}

//I dont do the matrix flip, because the chosen matrix is squared inversible.
float AQWConvolution(float *tab,int x,int y){
    float ret=0;
    float Kernel[5][5]={{0.04,0.04,0.04,0.04,0.04},{0.04,0.04,0.04,0.04,0.04},{0.04,0.04,0.04,0.04,0.04},{0.04,0.04,0.04,0.04,0.04},{0.04,0.04,0.04,0.04,0.04}};
    int xval=0;
    int yval=0;
    for(int xi=0;xi<5;xi++){
        for(int yi=0;yi<5;yi++){
            xval=x+xi-2;
            yval=y+yi-2;
            if((xval<0 && yval<0)|| (xval>=C_SIZE && yval>=C_SIZE)){
                continue;//So its like adding 0
            }else if(xval<0 || xval>=C_SIZE){
                continue;//So its like adding 0
            }else if(yval<0 || yval>=C_SIZE){
                continue;//So its like adding 0
            }else{
                ret+=(tab[xval+yval*C_SIZE]*Kernel[xi][yi]);
            }
        }
    }
    return ret;
}

float AQWABS(float a){
    return a<0?-a:a;
}

int getVertexIdFromSectorID(int id){
    int i = (id/2)%(C_SIZE-1);
    int j = id/(2*(C_SIZE-1));
    return i+j*(C_SIZE);
}