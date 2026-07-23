//#include "C:/Wichtig/System/Static/Library/WindowEngine.h"
#include "/home/codeleaded/System/Static/Library/WindowEngine.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/PerlinNoise.h"
#include "/home/codeleaded/System/Static/Library/RayCast.h"

#include "./Math3D.h"
#include "./Hitbox3D.h"


typedef struct Camera {
	M4x4D matProj;
	Vec3D vCamera;
	Vec3D vVelocity;
	Vec3D vLookDir;
	float fYaw;	
	float fPitch;
	Vec3D vLength;
} Camera;

Camera Camera_New(Vec3D p){
	Camera c;
	c.matProj = Matrix_MakeProjection(90.0f,(float)GetHeight() / (float)GetWidth(),0.1f,1000.0f);
	c.vCamera = p;
	c.vVelocity = (Vec3D){ 0.0f,0.0f,0.0f,1.0f };
	c.vLookDir = (Vec3D){ 0.0f,0.0f,0.0f,1.0f };
	c.fYaw = 0.0f;
	c.fPitch = 0.0f;
	c.vLength = (Vec3D){ 0.0f,0.0f,0.0f,1.0f };
	return c;
}


mesh meshCube;
M4x4D matProj;
Vec3D vCamera = { 10.0f,50.0f,10.0f,1.0f };
Vec3D vVelocity = { 0.0f,0.0f,0.0f,1.0f };
Vec3D vLookDir;
float fYaw;	
float fPitch;	
float fTheta;

Vec3D vLength = { 0.5f,1.8f,0.5f,1.0f };
Vector Cubes;
Vec2 MouseBefore = { 0.0f,0.0f };

M4x4D matView;

char OnGround = 0;
int Mode = 0;
int Menu = 0;

typedef unsigned char Block;

#define BLOCK_ERROR	255

#define BLOCK_VOID	0
#define BLOCK_DIRT	1
#define BLOCK_GRAS	2
#define BLOCK_LEAF	3
#define BLOCK_LOG	4

#define WORLD_DX	100
#define WORLD_DY	40
#define WORLD_DZ	100

Block World[WORLD_DX * WORLD_DY * WORLD_DZ];

Block World_GetX(Block* World,float x,float y,float z){
	if(x<0.0f || x>=WORLD_DX) return BLOCK_ERROR;
	if(y<0.0f || y>=WORLD_DY) return BLOCK_ERROR;
	if(z<0.0f || z>=WORLD_DZ) return BLOCK_ERROR;
	return World[(int)x + (int)y * WORLD_DX + (int)z * WORLD_DX * WORLD_DY];
}
void World_SetX(Block* World,float x,float y,float z,Block b){
	if(x<0.0f || x>=WORLD_DX) return;
	if(y<0.0f || y>=WORLD_DY) return;
	if(z<0.0f || z>=WORLD_DZ) return;
	World[(int)x + (int)y * WORLD_DX + (int)z * WORLD_DX * WORLD_DY] = b;
}
Block World_Get(Block* World,Vec3D p){
	return World_GetX(World,p.x,p.y,p.z);
}
void World_Set(Block* World,Vec3D p,Block b){
	World_SetX(World,p.x,p.y,p.z,b);
}

int World_Height(Block* World,float x,float z){
	int h = WORLD_DY - 1;
	for(;h>=0;h--){
		if(World_GetX(World,x,h,z)!=BLOCK_VOID){
			return h;
		}
	}
	return h;
}

#define CUBE_SIDE_SOUTH		0
#define CUBE_SIDE_EAST		1
#define CUBE_SIDE_NORTH		2
#define CUBE_SIDE_WEST		3
#define CUBE_SIDE_TOP		4
#define CUBE_SIDE_BOTTOM		5


Vec3D Neighbour_Side(int s){
	switch (s){
	case CUBE_SIDE_SOUTH: 	return (Vec3D){ 0.0f, 0.0f,-1.0f,1.0f };
	case CUBE_SIDE_EAST: 	return (Vec3D){ 1.0f, 0.0f, 0.0f,1.0f };
	case CUBE_SIDE_NORTH: 	return (Vec3D){ 0.0f, 0.0f, 1.0f,1.0f };
	case CUBE_SIDE_WEST: 	return (Vec3D){-1.0f, 0.0f, 0.0f,1.0f };
	case CUBE_SIDE_TOP: 		return (Vec3D){ 0.0f, 1.0f, 0.0f,1.0f };
	case CUBE_SIDE_BOTTOM: 	return (Vec3D){ 0.0f,-1.0f, 0.0f,1.0f };
	}
	return (Vec3D){ 0.0f,0.0f,0.0f,1.0f };
}
Pixel Block_Pixel(Block b,int s){
	switch (b){
	case BLOCK_VOID: 	return BLACK;
	case BLOCK_DIRT: 	return BROWN;
	case BLOCK_GRAS:	return s==CUBE_SIDE_TOP ? GREEN : BROWN;
	case BLOCK_LEAF: 	return DARK_GREEN;
	case BLOCK_LOG: 	return DARK_BROWN;
	}
	return BLACK;
}

void World_Tree(Block* World,int x,int y,int z){
	World_SetX(World,x,y,z,BLOCK_LOG);
	World_SetX(World,x,y+1,z,BLOCK_LOG);
	World_SetX(World,x,y+2,z,BLOCK_LOG);

	World_SetX(World,x,y+3,z,BLOCK_LEAF);

	World_SetX(World,x+1,y+3,z,BLOCK_LEAF);
	World_SetX(World,x+1,y+3,z+1,BLOCK_LEAF);
	World_SetX(World,x+1,y+3,z-1,BLOCK_LEAF);
	World_SetX(World,x-1,y+3,z,BLOCK_LEAF);
	World_SetX(World,x-1,y+3,z+1,BLOCK_LEAF);
	World_SetX(World,x-1,y+3,z-1,BLOCK_LEAF);
	World_SetX(World,x,y+3,z+1,BLOCK_LEAF);
	World_SetX(World,x,y+3,z-1,BLOCK_LEAF);

	World_SetX(World,x,y+4,z,BLOCK_LEAF);
	World_SetX(World,x+1,y+4,z,BLOCK_LEAF);
	World_SetX(World,x-1,y+4,z,BLOCK_LEAF);
	World_SetX(World,x,y+4,z+1,BLOCK_LEAF);
	World_SetX(World,x,y+4,z-1,BLOCK_LEAF);
}
void World_Generate(Block* World,int dx,int dy,int dz){
	memset(World,0,dx * dy * dz);

	float Seed[dx * dz];
	for(int i = 0;i<dx * dz;i++){
		Seed[i] = (float)Random_f64_New();
	}

	float Out[dx * dz];
	PerlinNoise_2D_Buffer(dx,dz,Seed,15,0.9f,Out);

	for(int i = 0;i<dz;i++){
		for(int j = 0;j<dx;j++){
			int h = 0;
			for(;h<Out[j + i * dx] * ((float)dy * 0.8f);h++){
				World_SetX(World,j,h,i,BLOCK_DIRT);
			}
			World_SetX(World,j,h,i,BLOCK_GRAS);
		}
	}


	for(int i = 2;i<dz-2;i++){
		for(int j = 2;j<dx-2;j++){
			int h = World_Height(World,j,i) + 1;
			
			if(Random_i32_MinMax(0,40)==0){
				World_Tree(World,j,h,i);
			}
		}
	}
}

void Triangle_CalcNorm(Tri3D* t){
	Vec3D normal, line1, line2;

	line1 = Vec3D_Sub(t->p[1],t->p[0]);
	line2 = Vec3D_Sub(t->p[2],t->p[0]);

	normal = Vec3D_CrossProduct(line1,line2);

	t->n = Vec3D_Normalise(normal);
}

void Cube_Set(Tri3D* trisout,Vec3D p,Vec3D d,Pixel c){
	Tri3D tris[12] = {
	// SOUTH
	{ 0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	{ 0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	// EAST                                                      
	{ 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	{ 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,	c },
	// NORTH                                                     
	{ 1.0f, 0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	// WEST                                                      
	{ 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	{ 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	// TOP                                                       
	{ 0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	{ 0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	// BOTTOM                                                    
	{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,	    0.0f, 0.0f, 0.0f, 1.0f,	c },
	};

	for(int i = 0;i<12;i++){
		for(int j = 0;j<3;j++){
			tris[i].p[j] = Vec3D_Add(p,Vec3D_new(tris[i].p[j].x * d.x,tris[i].p[j].y * d.y,tris[i].p[j].z * d.z));
		}
		Triangle_CalcNorm(&tris[i]);
		
		trisout[i] = tris[i];
	}
}
void MakeCube(Vec3D p,Vec3D d,Pixel c){
	Tri3D tris[12];
	Cube_Set(tris,p,d,c);

	for(int i = 0;i<12;i++){
		Vector_Push(&meshCube.tris,&tris[i]);
	}
}
void MakePlane(Vec3D p,Vec3D d,int Plane,Pixel c){
	Tri3D tris[12];
	Cube_Set(tris,p,d,c);

	for(int i = Plane*2;i<(Plane+1)*2;i++){
		Vector_Push(&meshCube.tris,&tris[i]);
	}
}
void BuildCube(Vec3D p,Vec3D d,Pixel c){
	Vector_Push(&Cubes,(Rect3[]){ { p,d } });
	MakeCube(p,d,c);
}

void Mesh_Reload(){
	Vector_Clear(&meshCube.tris);

	for(int i = 0;i<WORLD_DZ;i++){
		for(int j = 0;j<WORLD_DY;j++){
			for(int k = 0;k<WORLD_DX;k++){
				Block b = World_GetX(World,k,j,i);

				if(b!=BLOCK_VOID){
					//MakeCube((Vec3D){ k,j,i,1.0f },(Vec3D){ 1.0f,1.0f,1.0f },GREEN);

					for(int s = 0;s<6;s++){
						Vec3D p = { k,j,i,1.0f };
						Vec3D n = Vec3D_Add(p,Neighbour_Side(s));
						
						if(World_Get(World,n)==BLOCK_VOID){
							Pixel c = Block_Pixel(b,s);
							MakePlane(p,(Vec3D){ 1.0f,1.0f,1.0f },s,c);
						}
					}
				}
			}
		}
	}
}

int Cubes_Compare(const void* e1,const void* e2) {
	Rect3 r1 = *(Rect3*)e1;
	Rect3 r2 = *(Rect3*)e2;
	
	Vec3D pos = Vec3D_Add(vCamera,(Vec3D){ vLength.x * 0.5f,vLength.y * 0.9f,vLength.z * 0.5f });
	Vec3D d1 = Vec3D_Sub(r1.p,pos);
    Vec3D d2 = Vec3D_Sub(r2.p,pos);
	return Vec3D_Length(d1) == Vec3D_Length(d2) ? 0 : (Vec3D_Length(d1) < Vec3D_Length(d2) ? 1 : -1);
}
void Cubes_Reload(Block* World){
	Vector_Clear(&Cubes);

	Vec3D f = { (int)vCamera.x,(int)vCamera.y,(int)vCamera.z };
	for(int i = -2;i<2;i++){
		for(int j = -2;j<2;j++){
			for(int k = -2;k<2;k++){
				Vec3D n = { k,j,i };
				Vec3D r = Vec3D_Add(f,n);

				Block b = World_Get(World,r);

				if(b!=BLOCK_VOID && b!=BLOCK_ERROR){
					Vector_Push(&Cubes,(Rect3[]){ { r,(Vec3D){ 1.0f,1.0f,1.0f } } });
				}
			}
		}
	}

	qsort(Cubes.Memory,Cubes.size,Cubes.ELEMENT_SIZE,Cubes_Compare);
}

void World_Edit(Block* World,Vec3D p,Block b){
	World_SetX(World,p.x,p.y,p.z,b);
	Mesh_Reload();
}

int compare(const void* e1,const void* e2) {
	Tri3D t1 = *(Tri3D*)e1;
	Tri3D t2 = *(Tri3D*)e2;
	float z1 = (t1.p[0].z+t1.p[1].z+t1.p[2].z)/3;
    float z2 = (t2.p[0].z+t2.p[1].z+t2.p[2].z)/3;
    return z1 == z2 ? 0 : (z1 < z2 ? 1 : -1);
}

void Triangles_Project(){
	Vector vecTrianglesToRaster = Vector_New(sizeof(Tri3D));

	for (int i = 0;i<meshCube.tris.size;i++){
		Tri3D tri = *(Tri3D*)Vector_Get(&meshCube.tris,i);
		
		Vec3D vCameraRay = Vec3D_Sub(tri.p[0], vCamera);

		if (Vec3D_DotProduct(tri.n,vCameraRay) < 0.0f){
			Vec3D light_direction = Vec3D_new(0.4f,0.5f,-0.6f);
			light_direction = Vec3D_Normalise(light_direction);

			float dp = fmaxf(0.1f, Vec3D_DotProduct(light_direction,tri.n));

			tri.c = Pixel_Mul(tri.c,Pixel_toRGBA(dp,dp,dp,1.0f));
			//tri.c = Pixel_toRGBA(dp,dp,dp,1.0f);

			tri.p[0] = Matrix_MultiplyVector(matView,tri.p[0]);
			tri.p[1] = Matrix_MultiplyVector(matView,tri.p[1]);
			tri.p[2] = Matrix_MultiplyVector(matView,tri.p[2]);

			int nClippedTriangles = 0;
			Tri3D clipped[2];
			nClippedTriangles = Triangle_ClipAgainstPlane(Vec3D_new(0.0f,0.0f,0.1f),Vec3D_new(0.0f,0.0f,1.0f),tri,&clipped[0],&clipped[1]);

			for (int n = 0; n < nClippedTriangles; n++){
				clipped[n].p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
				clipped[n].p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
				clipped[n].p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);

				clipped[n].p[0] = Vec3D_Div(clipped[n].p[0], clipped[n].p[0].w);
				clipped[n].p[1] = Vec3D_Div(clipped[n].p[1], clipped[n].p[1].w);
				clipped[n].p[2] = Vec3D_Div(clipped[n].p[2], clipped[n].p[2].w);

				clipped[n].p[0].x *= -1.0f;
				clipped[n].p[1].x *= -1.0f;
				clipped[n].p[2].x *= -1.0f;
				clipped[n].p[0].y *= -1.0f;
				clipped[n].p[1].y *= -1.0f;
				clipped[n].p[2].y *= -1.0f;

				Vec3D vOffsetView = Vec3D_new( 1,1,0 );
				clipped[n].p[0] = Vec3D_Add(clipped[n].p[0], vOffsetView);
				clipped[n].p[1] = Vec3D_Add(clipped[n].p[1], vOffsetView);
				clipped[n].p[2] = Vec3D_Add(clipped[n].p[2], vOffsetView);
				clipped[n].p[0].x *= 0.5f * (float)GetWidth();
				clipped[n].p[0].y *= 0.5f * (float)GetHeight();
				clipped[n].p[1].x *= 0.5f * (float)GetWidth();
				clipped[n].p[1].y *= 0.5f * (float)GetHeight();
				clipped[n].p[2].x *= 0.5f * (float)GetWidth();
				clipped[n].p[2].y *= 0.5f * (float)GetHeight();

				Vector_Push(&vecTrianglesToRaster,&clipped[n]);
			}			
		}
	}

	qsort(vecTrianglesToRaster.Memory,vecTrianglesToRaster.size,vecTrianglesToRaster.ELEMENT_SIZE,compare);

	for (int i = 0;i<vecTrianglesToRaster.size;i++)
	{
		Tri3D triToRaster = *(Tri3D*)Vector_Get(&vecTrianglesToRaster,i);

		Tri3D clipped[2];
		Vector listTriangles = Vector_New(sizeof(Tri3D));

		Vector_Push(&listTriangles,&triToRaster);
		int nNewTriangles = 1;

		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				Tri3D test = *(Tri3D*)Vector_Get(&listTriangles,0);
				Vector_Remove(&listTriangles,0);
				nNewTriangles--;

				switch (p)
				{
				case 0:	nTrisToAdd = Triangle_ClipAgainstPlane(Vec3D_new( 0.0f, 0.0f, 0.0f ), 					Vec3D_new( 0.0f, 1.0f, 0.0f ), 	test, &clipped[0], &clipped[1]); break;
				case 1:	nTrisToAdd = Triangle_ClipAgainstPlane(Vec3D_new( 0.0f, (float)GetHeight() - 1, 0.0f ), Vec3D_new( 0.0f, -1.0f, 0.0f ), test, &clipped[0], &clipped[1]); break;
				case 2:	nTrisToAdd = Triangle_ClipAgainstPlane(Vec3D_new( 0.0f, 0.0f, 0.0f ), 					Vec3D_new( 1.0f, 0.0f, 0.0f ), 	test, &clipped[0], &clipped[1]); break;
				case 3:	nTrisToAdd = Triangle_ClipAgainstPlane(Vec3D_new( (float)GetWidth() - 1, 0.0f, 0.0f ), 	Vec3D_new( -1.0f, 0.0f, 0.0f ), test, &clipped[0], &clipped[1]); break;
				}

				for (int w = 0; w < nTrisToAdd; w++)
					Vector_Push(&listTriangles,&clipped[w]);
			}
			nNewTriangles = listTriangles.size;
		}

		for (int j = 0;j<listTriangles.size;j++){
			Tri3D t = *(Tri3D*)Vector_Get(&listTriangles,j);
			//RenderTriangle(((Vec2){ t.p[0].x, t.p[0].y }),((Vec2){ t.p[1].x, t.p[1].y }),((Vec2){ t.p[2].x, t.p[2].y }),t.c);
			//RenderTriangleWire(((Vec2){ t.p[0].x, t.p[0].y }),((Vec2){ t.p[1].x, t.p[1].y }),((Vec2){ t.p[2].x, t.p[2].y }),WHITE,1.0f);

			if(Mode==0) RenderTriangle(((Vec2){ t.p[0].x, t.p[0].y }),((Vec2){ t.p[1].x, t.p[1].y }),((Vec2){ t.p[2].x, t.p[2].y }),t.c);
			if(Mode==1) RenderTriangleWire(((Vec2){ t.p[0].x, t.p[0].y }),((Vec2){ t.p[1].x, t.p[1].y }),((Vec2){ t.p[2].x, t.p[2].y }),t.c,1.0f);
			if(Mode==2){
				RenderTriangle(((Vec2){ t.p[0].x, t.p[0].y }),((Vec2){ t.p[1].x, t.p[1].y }),((Vec2){ t.p[2].x, t.p[2].y }),t.c);
				RenderTriangleWire(((Vec2){ t.p[0].x, t.p[0].y }),((Vec2){ t.p[1].x, t.p[1].y }),((Vec2){ t.p[2].x, t.p[2].y }),WHITE,1.0f);
			}
		}

		Vector_Free(&listTriangles);
	}
	Vector_Free(&vecTrianglesToRaster);
}

void Stand(Vec3D* Data){
	Data->y = 0.0f;
	OnGround = 1;
}

char World_Void(Block* World,Vec3 p){
	Block b = World_GetX(World,p.x,p.y,p.z);
	return b==BLOCK_VOID || b==BLOCK_ERROR;
}

void Menu_Set(int m){
	if(Menu==0 && m==1){
		AlxWindow_Mouse_SetInvisible(&window);
		SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
	}
	if(Menu==1 && m==0){
		AlxWindow_Mouse_SetVisible(&window);
	}
	
	MouseBefore = GetMouse();
	Menu = m;
}

void Setup(AlxWindow* w){
	Menu_Set(1);
	Random_Get(6969);

	meshCube = (mesh){ Vector_New(sizeof(Tri3D)) };
	Cubes = Vector_New(sizeof(Rect3));

	matProj = Matrix_MakeProjection(90.0f, (float)GetHeight() / (float)GetWidth(), 0.1f, 1000.0f);

	World_Generate(World,WORLD_DX,WORLD_DY,WORLD_DZ);

	Mesh_Reload();
}

void Update(AlxWindow* w){
	//w->ElapsedTime = 0.05;
	
	/*if(Stroke(ALX_MOUSE_L).PRESSED){
		MouseBefore = GetMouse();
	}
	if(Stroke(ALX_MOUSE_L).DOWN){
		if(GetMouse().x!=MouseBefore.x || GetMouse().y!=MouseBefore.y){
			Vec2 d = Vec2_Sub(GetMouse(),MouseBefore);
			Vec2 a = Vec2_Mulf(Vec2_Div(d,(Vec2){ window.Width,window.Height }),2 * PI);
	
			fYaw += a.x;
			fPitch += a.y;
	
			MouseBefore = GetMouse();
		}
	}*/
	if(Menu==1){
		if(GetMouse().x!=MouseBefore.x || GetMouse().y!=MouseBefore.y){
			Vec2 d = Vec2_Sub(GetMouse(),MouseBefore);
			Vec2 a = Vec2_Mulf(Vec2_Div(d,(Vec2){ window.Width,window.Height }),2 * F32_PI);
	
			fYaw += a.x;
			fPitch += a.y;
	
			SetMouse((Vec2){ GetWidth() / 2,GetHeight() / 2 });
			MouseBefore = GetMouse();
		}
	}
	
	if(Stroke(ALX_KEY_ESC).PRESSED)
		Menu_Set(!Menu);

	if(Stroke(ALX_KEY_UP).DOWN)
		fPitch -= 2.0f * w->ElapsedTime;

	if(Stroke(ALX_KEY_DOWN).DOWN)
		fPitch += 2.0f * w->ElapsedTime;

	if(Stroke(ALX_KEY_LEFT).DOWN)
		fYaw -= 2.0f * w->ElapsedTime;

	if(Stroke(ALX_KEY_RIGHT).DOWN)
		fYaw += 2.0f * w->ElapsedTime;

	if(Stroke(ALX_KEY_Z).PRESSED)
		Mode = Mode < 3 ? Mode+1 : 0;

	if(Stroke(ALX_KEY_R).DOWN)
		//if(OnGround) 
			vVelocity.y = 5.0f;
	
	//if(Stroke(ALX_KEY_F).DOWN)
	//	vVelocity.y = -5.0f;

	//if(Stroke(ALX_KEY_R).RELEASED || Stroke(ALX_KEY_F).RELEASED)
	//	vVelocity.y = 0.0f;

	M4x4D matCameraRot = Matrix_MakeRotationY(fYaw);
	Vec3D vForward = Matrix_MultiplyVector(matCameraRot,Vec3D_new(0.0f,0.0f,1.0f));
	Vec3D vLeft = Vec3D_Perp(vForward);
	
	if(Stroke(ALX_KEY_W).DOWN){
		vVelocity.x += vForward.x * 20.0f * w->ElapsedTime;
		vVelocity.z += vForward.z * 20.0f * w->ElapsedTime;
	}
	if(Stroke(ALX_KEY_S).DOWN){
		vVelocity.x -= vForward.x * 20.0f * w->ElapsedTime;
		vVelocity.z -= vForward.z * 20.0f * w->ElapsedTime;
	}
	if(Stroke(ALX_KEY_A).DOWN){
		vVelocity.x -= vLeft.x * 20.0f * w->ElapsedTime;
		vVelocity.z -= vLeft.z * 20.0f * w->ElapsedTime;
	}
	if (Stroke(ALX_KEY_D).DOWN){
		vVelocity.x += vLeft.x * 20.0f * w->ElapsedTime;
		vVelocity.z += vLeft.z * 20.0f * w->ElapsedTime;
	}

	Vec2 v = { vVelocity.x,vVelocity.z };
	Vec2 d = Vec2_Norm(v);

	float drag = OnGround ? 12.0f : 10.0f;
	Vec2 da = Vec2_Norm(v);
	v = Vec2_Sub(v,Vec2_Mulf(d,drag * w->ElapsedTime));

	if(F32_Sign(v.x)!=F32_Sign(da.x) || F32_Sign(v.y)!=F32_Sign(da.y)){
		v.x = 0.0f;
		v.y = 0.0f;
	}

	float maxVelo = OnGround ? 4.0f : 6.0f;
	if(Vec2_Mag(v)>maxVelo){
		v = Vec2_Mulf(d,maxVelo);
	}

	vVelocity.x = v.x;
	vVelocity.z = v.y;

	vVelocity = Vec3D_Add(vVelocity,Vec3D_Mul((Vec3D){ 0.0f,-10.0f,0.0f,1.0f },w->ElapsedTime));
	vCamera = Vec3D_Add(vCamera,Vec3D_Mul(vVelocity,w->ElapsedTime));

	Cubes_Reload(World);
	OnGround = 0;
	for(int i = 0;i<Cubes.size;i++){
		Vec3D pos = { vLength.x * 0.5f,vLength.y * 0.9f,vLength.z * 0.5f };

		Rect3 r1 = *(Rect3*)Vector_Get(&Cubes,i);
		Rect3 r2 = (Rect3){ Vec3D_Sub(vCamera,pos),vLength };
		Rect3_Static(&r2,r1,&vVelocity,(void (*[])(void*)){ NULL,NULL,NULL,NULL,NULL,(void*)Stand });
		vCamera = Vec3D_Add(r2.p,pos);
	}

	float Border = F32_PI * 0.5f - 0.00001;
	if(fPitch<-Border) fPitch = -Border;
	if(fPitch>Border) fPitch = Border;

	Vec3D vUp = Vec3D_new( 0.0f,1.0f,0.0f );
	Vec3D vTarget = Vec3D_new( 0.0f,0.0f,1.0f );
	M4x4D matCameraRotX = Matrix_MakeRotationX(fPitch);
	vLookDir = Matrix_MultiplyVector(matCameraRotX,vTarget);
	vLookDir = Matrix_MultiplyVector(matCameraRot,vLookDir);
	
	vTarget = Vec3D_Add(vCamera, vLookDir);
	M4x4D matCamera = Matrix_PointAt(vCamera, vTarget, vUp);
	matView = Matrix_QuickInverse(matCamera);


	if(Stroke(ALX_MOUSE_L).PRESSED){
		Vec3 c = (Vec3){ vCamera.x,vCamera.y,vCamera.z };
		RayCast_TileMap(World,(void*)World_Void,c,(Vec3){ vLookDir.x,vLookDir.y,vLookDir.z },0.01f,4.0f,&c);
		if(c.x!=vCamera.x || c.y!=vCamera.y || c.z!=vCamera.z)
			World_Edit(World,(Vec3D){ c.x,c.y,c.z },BLOCK_VOID);
	}
	if(Stroke(ALX_MOUSE_R).PRESSED){
		Vec3 c = (Vec3){ vCamera.x,vCamera.y,vCamera.z };
		RayCast_TileMap_N(World,(void*)World_Void,c,(Vec3){ vLookDir.x,vLookDir.y,vLookDir.z },0.01f,4.0f,&c);
		if(c.x!=vCamera.x || c.y!=vCamera.y || c.z!=vCamera.z)
			World_Edit(World,(Vec3D){ c.x,c.y,c.z },BLOCK_DIRT);
	}

	Clear(LIGHT_BLUE);
	Triangles_Project();

	String str = String_Format("X: %f, Y: %f, Z: %f, Size: %d",vCamera.x,vCamera.y,vCamera.z,meshCube.tris.size);
	RenderCStrSize(str.Memory,str.size,0,0,RED);
	String_Free(&str);
}

void Delete(AlxWindow* w){
	Vector_Free(&meshCube.tris);
	Vector_Free(&Cubes);

	AlxWindow_Mouse_SetVisible(&window);
}

int main(){
    if(Create("3D Test no Tex",2500,1200,1,1,Setup,Update,Delete))
        Start();
    return 0;
}