// sous mac
// >g++ -I/u/usr/local/include/ -lglfw -lGLEW mainprojo.cpp -framework OpenGL  -omainprojo
// >./mainprojo

// sous linux
// >g++ -I/usr/local/include/ -I/public/ig/glm/ -c mainprojo1.cpp  -omainprojo1.o
// >g++ -I/usr/local mainprojo1.o -lglfw  -lGLEW  -lGL  -omainprojo1
// >./mainprojo1

// Inclut les en-têtes standards
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
//#include <unistd.h>
#include <time.h>
#include <cmath>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
/*#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>*/

using namespace glm;

#define GLM_FORCE_RADIANS
//float const M_PI = 3.1415;

const int N = 10;
//const int N = (N);


// stocke les variables uniformes qui seront communes a tous les vertex dessines
// dans une variable globale CAR
// C'est dans la fonction loadShaders que nous pouvos recupere les bonnes valeurs de pointeur (une fois le shader compile/linke)
// c'est dans le main que nous pouvons donne les bonnes valeurs "au bout du pointeur" pour que les shaders les recoivent
GLint uniform_proj, uniform_view, uniform_model;

//Ajouté pour la rotation
GLint uniform_rotation;

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	uniform_proj = glGetUniformLocation(ProgramID, "projectionMatrix");
	uniform_view = glGetUniformLocation(ProgramID, "viewMatrix");
	uniform_model = glGetUniformLocation(ProgramID, "modelMatrix");
	uniform_rotation = glGetUniformLocation(ProgramID, "rotation");

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

//Calcul de la moyenne pour un tableau
float moyenneTableau (float T[N][N]){
	float total = 0.0;
	float moyenne = 0.0;
	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			total += T[j][i];
		}
	}
	moyenne = total/(N*N);
	return moyenne;
}

//Calcul de l'écart-type pour un tableau
float ecartTypeTableau(float T[N][N], float moyenne){
	float ecartType = 0.0;
	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			ecartType += ((T[j][i] - moyenne)*(T[j][i] - moyenne));
		}
	}
	return pow((ecartType/N), 0.5);
}

//calcul de l'écart-type gloabal de 2 tableau
float ecartType2Tableau(float T1[N][N], float moyenne1, float T2[N][N], float moyenne2){
	float ecartType2 = 0.0;
	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			ecartType2 += ((T1[j][i] - moyenne1) * (T2[j][i] - moyenne2));
		}
	}
	return pow((ecartType2/ N),0.5);
}

//Calcul de la corrélation de 2 tableaux
float correlation (float ecartypet1t2, float ecartypet1, float ecartypet2){
	float corr = 0.0;
	corr = (ecartypet1t2)/(ecartypet1 * ecartypet2);
	return corr;
}

float generateGaussianNoise(float mu, float sigma) {
	float epsilon = 0.0000001;
	float two_pi = 2.0*M_PI;
	float z0, z1;
	float u1, u2;
	do {
		u1 = rand()/(float)RAND_MAX;
		u2 = rand() /(float)RAND_MAX;
	} while (u1 <= epsilon);
	z0 = pow((-2.0 * log(u1)),0.5) * cos(two_pi * u2);
	z1 = pow((-2.0 * log(u1)),0.5) * sin(two_pi * u2);
	return z0 * sigma + mu;
	// return  z1 * sigma + mu;
	// On aurait pu generer un autre nombre aleatoire comme ceci
}

//Création des tableaux à 2 dimensions de taille N
float T1[N][N];

float T1b[N][N];

float T2[N][N];

float T2b[N][N];

float T[N][N];

float Tb[N][N];

int main() {
	
	//Initialise random
	//srand(time(NULL));

	//Remplit les 4 tableaux T1 T2 T1b et T2b
	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			float r = (rand() % 1000)/1000.0;
			T1[j][i] = r;
		}
	}

	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			float r = (rand() % 1000) / 1000.0;
			T2[j][i] = r;
		}
	}

	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			float r = (rand() % 1000) / 1000.0;
			T1b[j][i] = r;
		}
	}

	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			float r = (rand() % 1000) / 1000.0;
			T2b[j][i] = r;
		}
	}

	//Déclaration des variables pour le terrain 1
	double corr = 0.7;
	float total_t1 = 0;
	float total_t2 = 0;
	float ecart_type_t1 = 0.0;
	float ecart_type_t2 = 0.0;
	float moyenne_t1 = 0.0;
	float moyenne_t2 = 0.0;
	float ecart_type_t1t2 = 0.0;
	float maxT = 0.0;

	//Déclaration des variables pour le terrain 2
	double corrb = 0.2;
	float total_t1b = 0;
	float total_t2b = 0;
	float ecart_type_t1b = 0.0;
	float ecart_type_t2b = 0.0;
	float moyenne_t1b = 0.0;
	float moyenne_t2b = 0.0;
	float ecart_type_t1t2b = 0.0;
	float maxTb = 0.0;

	moyenne_t1 = moyenneTableau(T1);
	moyenne_t2 = moyenneTableau(T2);
	moyenne_t1b = moyenneTableau(T1b);
	moyenne_t2b = moyenneTableau(T2b);

	ecart_type_t1 = ecartTypeTableau(T1, moyenne_t1);
	ecart_type_t2 = ecartTypeTableau(T2, moyenne_t2);
	ecart_type_t1b = ecartTypeTableau(T1b, moyenne_t1b);
	ecart_type_t2b = ecartTypeTableau(T2b, moyenne_t2b);

	ecart_type_t1t2 = ecartType2Tableau(T1, moyenne_t1, T2, moyenne_t2);
	ecart_type_t1t2b = ecartType2Tableau(T1b, moyenne_t1b, T2b, moyenne_t2b);

	//Tentative de calcul de corrélation
	//corr = correlation(ecart_type_t1t2, ecart_type_t1, ecart_type_t2);
	//corrb = correlation(ecart_type_t1t2b, ecart_type_t1b, ecart_type_t2b);

	//Remplissage des Tableaux T et Tb
	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {
			T[j][i] = (T1[j][i] * corr) + (T2[j][i] * pow((1 - corr)*corr, 0.5));
			Tb[j][i] = (T1b[j][i] * corrb) + (T2b[j][i] * pow((1 - corrb)*corrb, 0.5));
		}
	}

	//Détermine le maximum de chaque tableau
	maxT = T[0][0];
	maxTb = Tb[0][0];
	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {
			if (T[j][i] > maxT) maxT = T[j][i];
			if (Tb[j][i] > maxTb) maxTb = Tb[j][i];
		}
	}

	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	//Terrain 1
	GLfloat g_vertex_buffer_data[N*N * 18];
	GLfloat g_vertex_color_data[N*N * 18];

	//Contour du Terrain 1
	GLfloat g_vertex_buffer_data_c1[(N)*(N) * 18];
	GLfloat g_vertex_color_data_c1[(N) *(N) * 18];

	//Terrain 2
	GLfloat g_vertex_buffer_data_1[N*N * 18];
	GLfloat g_vertex_color_data_1[N*N * 18];

	//Contour du Terrain 2
	GLfloat g_vertex_buffer_data_c2[(N)*(N) * 18];
	GLfloat g_vertex_color_data_c2[(N) *(N) * 18];

	float M = (float)N;

	float mid = M / 2;
	float midc = (float)N / 2;

	//DEssin du terrain T
	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {

			//terrain
			//Triangle1
			//Sommet 1

			g_vertex_buffer_data[i*N * 18 + j * 18 + 0] = (i - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 1] = (j - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 2] = T[j][i];

			//Sommet 2

			g_vertex_buffer_data[i*N * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 4] = (j - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 5] = T[j][i + 1];

			//SOmmet 3

			g_vertex_buffer_data[i*N * 18 + j * 18 + 6] = (i - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 8] = T[j + 1][i];


			//Triangle2

			//Sommmet 4

			g_vertex_buffer_data[i*N * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 11] = T[j + 1][i + 1];


			//Sommet 2

			g_vertex_buffer_data[i*N * 18 + j * 18 + 12] = ((i + 1) - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 13] = (j - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 14] = T[j][i + 1];


			//Sommet 3
			g_vertex_buffer_data[i*N * 18 + j * 18 + 15] = (i - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
			g_vertex_buffer_data[i*N * 18 + j * 18 + 17] = T[j + 1][i];


			//colors

			g_vertex_color_data[i*N * 18 + j * 18 + 0] = T[j][i];
			g_vertex_color_data[i*N * 18 + j * 18 + 1] = T[j][i];
			g_vertex_color_data[i*N * 18 + j * 18 + 2] = T[j][i];
		
			g_vertex_color_data[i*N * 18 + j * 18 + 3] = T[j][i+1];
			g_vertex_color_data[i*N * 18 + j * 18 + 4] = T[j][i+1];
			g_vertex_color_data[i*N * 18 + j * 18 + 5] = T[j][i+1];
			
			g_vertex_color_data[i*N * 18 + j * 18 + 6] = T[j+1][i];
			g_vertex_color_data[i*N * 18 + j * 18 + 7] = T[j+1][i];
			g_vertex_color_data[i*N * 18 + j * 18 + 8] = T[j+1][i];
			
			g_vertex_color_data[i*N * 18 + j * 18 + 9] = T[j+1][i+1];
			g_vertex_color_data[i*N * 18 + j * 18 + 10] = T[j+1][i+1];
			g_vertex_color_data[i*N * 18 + j * 18 + 11] = T[j+1][i+1];
			
			g_vertex_color_data[i*N * 18 + j * 18 + 12] = T[j][i+1];
			g_vertex_color_data[i*N * 18 + j * 18 + 13] = T[j][i+1];
			g_vertex_color_data[i*N * 18 + j * 18 + 14] = T[j][i+1];
			
			g_vertex_color_data[i*N * 18 + j * 18 + 15] = T[j+1][i];
			g_vertex_color_data[i*N * 18 + j * 18 + 16] = T[j+1][i];
			g_vertex_color_data[i*N * 18 + j * 18 + 17] = T[j+1][i];

		}
	}

	//Dessin du contour du premier terrain
	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {

			//Contour
			//Triangle1


			if (i == 0) {
	
				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = ((i)-mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j][i];
				
				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i)-mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+1][i];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = ((i)-mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j][i];

				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j+1][i];


				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+1][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j+1][i];

			}   else  if (i == N - 1) {

				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j][i + 1];
				

				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+1][i+1];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j][i + 1];
		
				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j+1][i+1];


				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j+1][i+1];

			}	else if (j == 0) {
				
				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j][i + 1];
				
				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j][i+1];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = ((i)-mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j][i];

				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = ((i)-mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = (j - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j][i+1];


				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j][i];

			} else if (j == N - 1) {
				
				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j+1][i+1];

				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+1][i + 1];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = ((i)-mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j+1][i];

				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = ((i)-mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j+1][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j+1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j+1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j+1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j+1][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j+1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j+1][i];

			}	else if (j == 1 && i == 1) {
				
				//Sommet 1
				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j - 1][i];

				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j - 1][i-1];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j - 1][i];

				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j-1][i-1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j-1][i-1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j-1][i-1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j-1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j-1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j-1][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j-1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j-1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j-1][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j-1][i-1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j-1][i-1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j-1][i-1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j-1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j-1][i];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j-1][i];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j-1][i-1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j-1][i-1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j-1][i-1];
			
			} else if (j == 1 && i == (N - 2)) {
				
				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j-1][i+2];

				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j-1][i + 1];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j-1 ][i+2];

				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j-1][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j-1][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j-1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j-1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j-1][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j-1][i+1];

			} else if (j == N - 2 && i == 1) {
				
				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j + 2][i];
				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+2][i-1];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = (i - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j+2][i];

				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				
				//Couleurs
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j-1][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j-1][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j-1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j-1][i+1];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j-1][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j-1][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j-1][i+1];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j-1][i+1];

			}else if (j == N - 2 && i == N - 3) {
				
				//Sommet 1
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 0] = ((i + 3) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 1] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 3] = ((i + 3) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 4] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 5] = T[j+2][i+3];

				//Sommet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 6] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 7] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 9] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 10] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+2][i+2];

				//Sommet 2
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 12] = ((i + 3) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 13] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 14] = T[j+2][i+3];

				//SOmmet 3
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 15] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 16] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c1[i*(N) * 18 + j * 18 + 17] = 0.0;
			
				//Couleurs
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 0] = T[j+2][i+3];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 1] = T[j+2][i+3];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 2] = T[j+2][i+3];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 3] = T[j+2][i+3];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 4] = T[j+2][i+3];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 5] = T[j+2][i+3];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 6] = T[j+2][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 7] = T[j+2][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 8] = T[j+2][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 9] = T[j+2][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 10] = T[j+2][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 11] = T[j+2][i+2];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 12] = T[j+2][i+3];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 13] = T[j+2][i+3];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 14] = T[j+2][i+3];

				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 15] = T[j+2][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 16] = T[j+2][i+2];
				g_vertex_color_data_c1[i*(N) * 18 + j * 18 + 17] = T[j+2][i+2];
			
			}


		}
	}

	//Dessin du terrain 2
	for (int j = 0; j<N; j++) {
		for (int i = 0; i<N; i++) {

			//terrain
			//Triangle1
			//Sommet 1

			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 0] = (i - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 1] = (j - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 2] = Tb[j][i];

			//Sommet 2

			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 4] = (j - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 5] = Tb[j][i + 1];

			//SOmmet 3

			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 6] = (i - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 8] = Tb[j + 1][i];


			//Triangle2

			//Sommmet 4

			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 11] = Tb[j + 1][i + 1];


			//Sommet 2

			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 12] = ((i + 1) - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 13] = (j - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 14] = Tb[j][i + 1];


			//Sommet 3
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 15] = (i - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
			g_vertex_buffer_data_1[i*N * 18 + j * 18 + 17] = Tb[j + 1][i];

			//colors

			
			g_vertex_color_data_1[i*N * 18 + j * 18 + 0] = Tb[j][i];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 1] = Tb[j][i];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 2] = Tb[j][i];
		
			g_vertex_color_data_1[i*N * 18 + j * 18 + 3] = Tb[j][i+1];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 4] = Tb[j][i+1];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 5] = Tb[j][i+1];
			
			g_vertex_color_data_1[i*N * 18 + j * 18 + 6] = Tb[j+1][i];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 7] = Tb[j+1][i];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 8] = Tb[j+1][i];
			
			g_vertex_color_data_1[i*N * 18 + j * 18 + 9] = Tb[j+1][i+1];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 10] = Tb[j+1][i+1];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 11] = Tb[j+1][i+1];
			
			g_vertex_color_data_1[i*N * 18 + j * 18 + 12] = Tb[j][i+1];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 13] = Tb[j][i+1];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 14] = Tb[j][i+1];
			
			g_vertex_color_data_1[i*N * 18 + j * 18 + 15] = Tb[j+1][i];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 16] = Tb[j+1][i];
			g_vertex_color_data_1[i*N * 18 + j * 18 + 17] = Tb[j+1][i];
			

		}
	}

	//Dessin du contour du Second terrain
	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {

			//Contour
			//Triangle1

			if (i == 0) {

				//Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = ((i)-mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j][i];

				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i)-mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j + 1][i];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = ((i)-mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j][i];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j+1][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j+1][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j+1][i];

			} else  if (i == N - 1) {

				//Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j][i + 1];


				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j + 1][i + 1];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j][i + 1];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j+1][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j+1][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j+1][i+1];

			} else if (j == 0) {
				
				//Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j][i + 1];

				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j][i + 1];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = ((i)-mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j][i];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = ((i)-mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = (j - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j][i];

			} else if (j == N - 1) {
			
				//Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j + 1][i + 1];

				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j + 1][i + 1];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = ((i)-mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j + 1][i];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = ((i)-mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = ((j + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j+1][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j+1][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j+1][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j+1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j+1][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j+1][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j+1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j+1][i];

			} else if (j == 1 && i == 1) {
			
				//Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j - 1][i];

				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j - 1][i-1];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j - 1][i];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j-1][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j-1][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j-1][i-1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j-1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j-1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j-1][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j-1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j-1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j-1][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j-1][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j-1][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j-1][i-1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j-1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j-1][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j-1][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j-1][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j-1][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j-1][i-1];

			}  else if (j == 1 && i == (N - 2)) {
				
				//Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j - 1][i + 2];

				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j - 1][i + 1];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j - 1][i + 2];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = ((i + 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = ((j - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j-1][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j-1][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j-1][i+2];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j-1][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j-1][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j-1][i+2];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j-1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j-1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j-1][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j-1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j-1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j-1][i+1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j-1][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j-1][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j-1][i+2];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j-1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j-1][i+1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j-1][i+1];

			}  else if (j == N - 2 && i == 1) {
			    
			    //Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j + 2][i];
				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j + 2][i - 1];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = (i - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j + 2][i];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = ((i - 1) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j+2][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j+2][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j+2][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j+2][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j+2][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j+2][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j+2][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j+2][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j+2][i-1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j+2][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j+2][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j+2][i-1];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j+2][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j+2][i];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j+2][i];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j+2][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j+2][i-1];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j+2][i-1];

			}  else if (j == N - 2 && i == N - 3) {
				
				//Sommet 1
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 0] = ((i + 3) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 1] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 2] = 0.0;

				//SOmmet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 3] = ((i + 3) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 4] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j + 2][i + 3];

				//Sommet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 6] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 7] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 8] = 0.0;

				//SOmmet 4
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 9] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 10] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j + 2][i + 2];

				//Sommet 2
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 12] = ((i + 3) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 13] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j + 2][i + 3];

				//SOmmet 3
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 15] = ((i + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 16] = ((j + 2) - mid) / mid;
				g_vertex_buffer_data_c2[i*(N) * 18 + j * 18 + 17] = 0.0;
				
				//Couleurs
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 0] = Tb[j+2][i+3];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 1] = Tb[j+2][i+3];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 2] = Tb[j+2][i+3];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 3] = Tb[j+2][i+3];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 4] = Tb[j+2][i+3];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 5] = Tb[j+2][i+3];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 6] = Tb[j+2][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 7] = Tb[j+2][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 8] = Tb[j+2][i+2];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 9] = Tb[j+2][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 10] = Tb[j+2][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 11] = Tb[j+2][i+2];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 12] = Tb[j+2][i+3];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 13] = Tb[j+2][i+3];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 14] = Tb[j+2][i+3];

				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 15] = Tb[j+2][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 16] = Tb[j+2][i+2];
				g_vertex_color_data_c2[i*(N) * 18 + j * 18 + 17] = Tb[j+2][i+2];
			}

			
		}
	}


	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // On veut OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Pour rendre MacOS heureux ; ne devrait pas être nécessaire
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // On ne veut pas l'ancien OpenGL
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	// Ouvre une fenêtre et crée son contexte OpenGl
	GLFWwindow* window; // (Dans le code source qui accompagne, cette variable est globale)
	window = glfwCreateWindow(1024, 768, "Tutorial 01", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); // Initialise GLEW
	glewExperimental = true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glDepthRange(-1, 1);

	// modern OpenGL do not have a default VAO anymore. Even if we don't want to use it
	// we have a create and bind one before playing with buffers !
	GLuint VertexArrayID;

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// This will identify our vertex buffer
	GLuint vertexbuffer;

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);

	//The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	// Only allocqte memory. Do not send yet our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data) + sizeof(g_vertex_color_data), 0, GL_STATIC_DRAW);

	// send vertices in the first part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);

	// send vertices in the second part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), sizeof(g_vertex_color_data), g_vertex_color_data);

	// ici les commandes stockees "une fois pour toute" dans le VAO
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer( // same thing for the colors
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)sizeof(g_vertex_buffer_data));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// on desactive le VAO a la fin de l'initialisation
	glBindVertexArray(0);

	//Contour du Terrain 1
	GLuint VertexArrayID_1;

	glGenVertexArrays(1, &VertexArrayID_1);
	glBindVertexArray(VertexArrayID_1);

	// This will identify our vertex buffer
	GLuint vertexbuffer_1;

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer_1);

	//The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_1);

	// Only allocqte memory. Do not send yet our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_c1) + sizeof(g_vertex_color_data_c1), 0, GL_STATIC_DRAW);

	// send vertices in the first part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data_c1), g_vertex_buffer_data_c1);

	// send vertices in the second part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_c1), sizeof(g_vertex_color_data_c1), g_vertex_color_data_c1);

	// ici les commandes stockees "une fois pour toute" dans le VAO
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer( // same thing for the colors
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)sizeof(g_vertex_buffer_data));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// on desactive le VAO a la fin de l'initialisation
	glBindVertexArray(0);

	//Terrain 2
	GLuint VertexArrayID_2;

	glGenVertexArrays(1, &VertexArrayID_2);
	glBindVertexArray(VertexArrayID_2);

	// This will identify our vertex buffer
	GLuint vertexbuffer_2;

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer_2);

	//The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_2);

	// Only allocqte memory. Do not send yet our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_1) + sizeof(g_vertex_color_data_1), 0, GL_STATIC_DRAW);

	// send vertices in the first part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data_1), g_vertex_buffer_data_1);

	// send vertices in the second part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_1), sizeof(g_vertex_color_data_1), g_vertex_color_data_1);

	// ici les commandes stockees "une fois pour toute" dans le VAO
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer( // same thing for the colors
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)sizeof(g_vertex_buffer_data_1));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// on desactive le VAO a la fin de l'initialisation
	glBindVertexArray(0);

	//Contour du terrain 2
	GLuint VertexArrayID_3;

	glGenVertexArrays(1, &VertexArrayID_3);
	glBindVertexArray(VertexArrayID_3);

	// This will identify our vertex buffer
	GLuint vertexbuffer_3;

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer_3);

	//The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_3);

	// Only allocqte memory. Do not send yet our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_c2) + sizeof(g_vertex_color_data_c2), 0, GL_STATIC_DRAW);

	// send vertices in the first part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data_c2), g_vertex_buffer_data_c2);

	// send vertices in the second part of the buffer
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data_c2), sizeof(g_vertex_color_data_c2), g_vertex_color_data_c2);

	// ici les commandes stockees "une fois pour toute" dans le VAO
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer( // same thing for the colors
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)sizeof(g_vertex_buffer_data_c2));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// on desactive le VAO a la fin de l'initialisation
	glBindVertexArray(0);

	// Assure que l'on peut capturer la touche d'échappement enfoncée ci-dessous
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	GLuint programID = LoadShaders("SimpleVertexShader5.vertexshader", "SimpleFragmentShader5.fragmentshader");

	float angle = 0.0f;

	do {
		angle = (angle + M_PI / 200);
		float camangle = 0.0;

		// clear before every draw 1
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// onchange de matrice de projection : la projection orthogonale est plus propice a la visualization !
		//glm::mat4 projectionMatrix = glm::perspective(45.0f, 1024.0f / 768.0f, 0.0f, 200.0f);
		glm::mat4 projectionMatrix = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, -12.f, 12.f);
		glm::mat4 viewMatrix = glm::lookAt(
			//vec3(1.5*cos(angle / 1000), 1.5*sin(angle / 1000), -0.5), // where is the camera
			vec3(1.5, 1.5, -0.5), // where is the camera
			vec3(0, 0, 0.5), //where it looks
			vec3(0, 0, 1) // head is up
		);
		mat4 modelMatrix = glm::mat4(1.0);

		//modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		//cout << glm::mat4:: rotation << endl;


		glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));


		// on re-active le VAO avant d'envoyer les buffers
		glBindVertexArray(VertexArrayID);

		// Draw the triangle(s) !

		glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		//Translation du terrain 1
		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -3.7f, 0.0f));
		//Rotation du terrain 1
		modelMatrix = glm::rotate(modelMatrix, 10*angle, glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		//Dessin du terrain 1
		glDrawArrays(GL_TRIANGLES, 0, sizeof(g_vertex_buffer_data) / (3 * sizeof(float))); // Starting from vertex 0; 6 vertices total -> 2 triangles
		glBindVertexArray(0);

		//Dessin du contour du terrain 1
		glBindVertexArray(VertexArrayID_1);
		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glDrawArrays(GL_TRIANGLES, 0, sizeof(g_vertex_buffer_data_c1) / (3 * sizeof(float))); // Starting from vertex 0; 6 vertices total -> 2 triangles
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayID_2);
		glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		//Translation du contour du terrain 1
		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 0.5f, 0.0f));
		//Rotation du contour du terrain 1
		modelMatrix = glm::rotate(modelMatrix, 10*angle, glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		//Dessin du terrain 2
		glDrawArrays(GL_TRIANGLES, 0, sizeof(g_vertex_buffer_data_1) / (3 * sizeof(float))); // Starting from vertex 0; 6 vertices total -> 2 triangles
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayID_3);
		glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		//Translation du second terrain
		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 0.5f, 0.0f));
		//modification de modelMatrix pour permettre la rotation du second terrain
		modelMatrix = glm::rotate(modelMatrix, 10*angle , glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		//Dessin du contour du terrain 2
		glDrawArrays(GL_TRIANGLES, 0, sizeof(g_vertex_buffer_data_c2) / (3 * sizeof(float))); // Starting from vertex 0; 6 vertices total -> 2 triangles
		glBindVertexArray(0);

		// on desactive les shaders
		glUseProgram(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();		
		
		
		//Accélération de la vitesse de rotation, maintenir Q
		if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS){
			angle += 10;
		}	
		
		//Accélération de la vitesse de rotation, maintenir A
		if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS){
			angle += 0.5;
		}	
		
		//Rotation dans le sens inverse, maintenir W
		if (glfwGetKey(window, GLFW_KEY_Z ) == GLFW_PRESS){
			angle -= 10;
		}
		
		//Rotation dans le sens inverse, en moins rapide, maintenir Z
		if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS){
			angle -= 0.5;
		}

		//Met la rotation en pause, maintenir
		if (glfwGetKey(window, GLFW_KEY_P ) == GLFW_PRESS){
			angle = 0.0;
		} 
			
		//Tentative de faire bouger la caméra
		if (glfwGetKey(window, GLFW_KEY_E ) == GLFW_PRESS){
			viewMatrix = glm::lookAt(vec3(1.5, 1.5, 1.5), vec3(0, 0, 0.5), vec3(0, 0, 1));
		}
		glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		

		// Vérifie si on a appuyé sur la touche échap (ESC) ou si la fenêtre a été fermée
	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);
}
