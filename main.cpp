#include <vector>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <set>

#define INIT_VERTICES 12
#define LOD_TARGET 3

static const double S5  = 2.2360679775;
static const double phi = (1.0 + S5) / 2.0;
static const double a   = 1.0 / S5;
static const double b   = 2.0 / S5;

static const int ICOSAHEDRON_FACES[20][3] = {
    {0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 1},
    {1, 6, 2}, {2, 7, 3}, {3, 8, 4}, {4, 9, 5}, {5,10, 1},
    {6, 7, 2}, {7, 8, 3}, {8, 9, 4}, {9,10, 5}, {10,6, 1},
    {11,7, 6}, {11,8, 7}, {11,9, 8}, {11,10,9}, {11,6,10}
};

struct Vertex {
    double x;
    double y;
    double z;
};

//The face of a triangle of the sphere. Its actually the indices inside the vector.
struct Face {
    int v0;
    int v1;
    int v2;
};

//=======
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
    int tag;
    int lightlevel;
    unsigned int id; //Sector ID
};



Vertex v_ComputeMidPoint(Vertex *v1, Vertex *v2){
	Vertex res;
	res.x = (v1->x + v2->x) / 2;
	res.y = (v1->y + v2->y) / 2;
	res.z = (v1->z + v2->z) / 2;
	return res;
}

//This take 2 vertices indices, and create an unique edge key. A linedef but in the 3D world.
std::pair<int, int> makeEdgeKey(int a, int b) {
    return a < b ? std::make_pair(a, b) : std::make_pair(b, a);
}

/**
 * The idea is that we build a regular Icosahedron.
 * Then we subdivide it in sub-triangles like a fractal.
 * Then we can project the vertices on the sphere plan.
 */
int main()
{
    const double R = 256.0;
    int LOD = 1;

	int v_count = INIT_VERTICES;
    std::vector<Vertex> v_initial = {
        { 0.0,  0.0,  1.0 },
        {  b,              0.0,            a },
        {  b*(phi-1)/2.0,  b*std::sqrt(10.0+2.0*S5)/4.0,  a },
        { -b*phi/2.0,      b*std::sqrt(10.0-2.0*S5)/4.0,  a },
        { -b*phi/2.0,     -b*std::sqrt(10.0-2.0*S5)/4.0,  a },
        {  b*(phi-1)/2.0, -b*std::sqrt(10.0+2.0*S5)/4.0,  a },
        {  b*phi/2.0,      b*std::sqrt(10.0-2.0*S5)/4.0, -a },
        { -b*(phi-1)/2.0,  b*std::sqrt(10.0+2.0*S5)/4.0, -a },
        { -b,              0.0,           -a },
        { -b*(phi-1)/2.0, -b*std::sqrt(10.0+2.0*S5)/4.0, -a },
        {  b*phi/2.0,     -b*std::sqrt(10.0-2.0*S5)/4.0, -a },
        { 0.0,  0.0, -1.0 }
    };

	std::vector<Face> f_initial;

	//Pushback vertices position.
	//eg we have {0,1,2} where 0 is the vertex at (0,0,1)
	for (int i = 0; i < 20; ++i) {
		f_initial.push_back({
			ICOSAHEDRON_FACES[i][0],
			ICOSAHEDRON_FACES[i][1],
			ICOSAHEDRON_FACES[i][2]
		});
	}

	std::vector<Vertex> v_duplicated = v_initial;
	std::vector<Face> f_duplicated;
	
    while (LOD != LOD_TARGET) {
		std::map<std::pair<int, int>, int> midpointCache;
    
		for(auto& f : f_initial){ // For each edge, get or create midpoint
			
			auto edge01 = makeEdgeKey(f.v0, f.v1);
			auto edge12 = makeEdgeKey(f.v1, f.v2);
			auto edge20 = makeEdgeKey(f.v2, f.v0);
			
			int mid01, mid12, mid20;
			
			// Check if midpoint already exists, otherwise create it
			if (midpointCache.find(edge01) != midpointCache.end()) {
				mid01 = midpointCache[edge01];
			} else {
				v_duplicated.push_back(v_ComputeMidPoint(&v_initial[f.v0], &v_initial[f.v1]));
				mid01 = v_count++;
				midpointCache[edge01] = mid01;
			}
			
			if (midpointCache.find(edge12) != midpointCache.end()) {
				mid12 = midpointCache[edge12];
			} else {
				v_duplicated.push_back(v_ComputeMidPoint(&v_initial[f.v1], &v_initial[f.v2]));
				mid12 = v_count++;
				midpointCache[edge12] = mid12;
			}
			
			if (midpointCache.find(edge20) != midpointCache.end()) {
				mid20 = midpointCache[edge20];
			} else {
				v_duplicated.push_back(v_ComputeMidPoint(&v_initial[f.v2], &v_initial[f.v0]));
				mid20 = v_count++;
				midpointCache[edge20] = mid20;
			}
			
			// Create 4 new faces using the midpoint indices
			f_duplicated.push_back({ f.v0, mid01, mid20 });
			f_duplicated.push_back({ mid01, f.v1, mid12 });
			f_duplicated.push_back({ mid20, mid12, f.v2 });
			f_duplicated.push_back({ mid01, mid12, mid20 });
		}
		
		// Prepare for next iteration
		v_initial = v_duplicated;
		v_duplicated.clear();
		f_initial = f_duplicated;
		f_duplicated.clear();
		
		LOD++;
    }
	
	std::vector<Vertex> v_projected = v_initial;
    // Projection to sphere
    for (auto& v : v_projected) {
        const double n = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
        v.x = R * v.x / n;
        v.y = R * v.y / n;
        v.z = R * v.z / n;
    }

    std::cout << "Gud\n";
	
	//So each face should be a sector.
	//An edge is a linedef/sidedef.
	//We already have the vertices.
	
	std::vector<sector> sectors;
	std::vector<sidedef> sidedefs;
	std::vector<linedef> linedefs;

	//Ok, first we need to iterate over all the edges.
	//So for that, we iterate over all the faces.
	std::set<std::pair<int, int>> uniqueEdges;

	for (const Face& f : f_initial) {
		uniqueEdges.insert(makeEdgeKey(f.v0, f.v1));
		uniqueEdges.insert(makeEdgeKey(f.v1, f.v2));
		uniqueEdges.insert(makeEdgeKey(f.v0, f.v2));
	}


    //Write to TEXTMAP
    std::ofstream f("TEXTMAP");
    f << "namespace = \"zdoom\";\n";
	
	int id = 0;
    for (const auto& v : v_projected) {
		if(v.z < 0) continue;
        double z = v.z < 0.0 ? 0.0 : v.z;
        f <<
            "vertex //" <<  id++ << "\n"
            "{\n"
            "  x = " << std::fixed << std::setprecision(1) << v.x << ";\n"
            "  y = " << std::fixed << std::setprecision(1) << v.y << ";\n"
			"  zfloor = " << std::fixed << std::setprecision(1) << v.z << ";\n"
            "}\n\n";
    }

    return 0;
}
