#pragma comment(lib, "glew32.lib")

#include <iostream>

#define GL3_PROTOTYPES 1 
#include <math.h>  
#include <GL/glew.h>
#include <SDL.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define PI 3.14
#define WIDTH 800
#define HEIGHT 600

//------------------------------------------------------------------------------------------------------------------------------
//Globalne zmienne 

// kod zrodlowy shadera wierzcholkow
const GLchar* vertexSource =
"#version 150 core\n"
"in vec3 position;"
"in vec3 color;"
"in vec2 texcoord;"
"out vec3 Color;"
"out vec2 Texcoord;"
"uniform mat4 transformMatrix;"
"void main()"
"{"
"    gl_Position =  transformMatrix * vec4(position, 1.0);"
"    Color = color;"
"	 Texcoord = texcoord;"
"}";

// kod zrodlowy shadera fragmentow
const GLchar* fragmentSource =
"#version 150 core\n"
"in vec3 Color;"
"in vec2 Texcoord;"
"out vec4 outColor;"
"uniform sampler2D tex;"
"void main()"
"{"
"    outColor = texture(tex, Texcoord) * vec4(Color, 1.0);"
"}";

using namespace std;
//------------------------------------------------------------------------------------------------------------------------------

GLint posAttrib, colAttrib, texAttrib;					//wskazniki atrybutow wierzcholkow
GLuint vertexShader, fragmentShader, shaderProgram;		//shadery


GLuint vao[5], vbo[16], ebo, tex, tex2, tex3, tex4;;	// identyfikatory poszczegolnych obiektow (obiekty tablic wierzcholkow, buforow wierzcholkow, elementow, tekstury)

//------------------------------------------------------------------------------------------------------------------------------

GLfloat ver_floor[] = { //wspolrzedne wierzcholkow podlogi
	-5.0f,  0.0f, -5.0f,
	5.0f,  0.0f, -5.0f,
	5.0f,  0.0f,  5.0f,
	-5.0f,  0.0f,  5.0f,
};

GLfloat col_floor[] = { //kolory wierzcholkow podlogi
		1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	11.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
};

GLfloat tex_floor[] = {	//wspolrzedne tekstury dla podlogi, 
						//jesli wykraczaja poza przedzial <0,1> nastapi nakladanie zalezne od parametrow ustawionych za pomoca funkcji glTexParameteri, np. GL_REPEAT
	0.0f, 0.0f,	// lewy dolny rog tekstury
	5.0f, 0.0f,
	5.0f, 5.0f,
	0.0f, 5.0f, // prawy gorny rog tekstury
};

GLuint elements[] = { // pogrupowanie wierzcholkow w trojkaty, wykorzystane zarowno dla sciany jaki dla podlogi
	0,1,2,
	2,3,0,
};

float pixels_floor[] = { //tekstura o wymiarach 2x2; dla kazdego punktu okreslone skladowe RGB koloru 
	1.0f, 1.0f, 1.0f,   0.1f, 0.1f, 0.1f,
	0.1f, 0.1f, 0.1f,   1.0f, 1.0f, 1.0f,
};

GLfloat ver_triangle[] = { //wspolrzedne wierzcholkow trojkata okreslajacego polozenie obserwatora (kamery)
	-0.2f,  0.1f, 0.0f, //left bottom
	 0.2f,  0.1f, 0.0f, //right bottom
	 0.0f,  0.1f, -1.0f,//center
};

GLfloat ver_triangle_old[9];

GLfloat col_triangle[] = { //kolory wierzcholkow trojkata okreslajacego polozenie obserwatora (kamery)
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,
};



GLfloat ver_wall[] = { //wspolrzedne wierzcholkow sciany
   5.0f,  0.0f, -5.0f,
	-5.0f,  0.0f, -5.0f,
	-5.0f,  10.0f, -5.0f,
	5.0f,  10.0f, -5.0f,
};
GLfloat tex_wall[] = {	//wspolrzedne tekstury dla podlogi, 
						//jesli wykraczaja poza przedzial <0,1> nastapi nakladanie zalezne od parametrow ustawionych za pomoca funkcji glTexParameteri, np. GL_REPEAT
	0.0f, 0.0f,	// lewy dolny rog tekstury
	5.0f, 0.0f,
	5.0f, 5.0f,
	0.0f, 5.0f, // prawy gorny rog tekstury
};
GLfloat col_wall[] = { //kolory wierzcholkow podlogi
	-1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-11.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
};
//change wall position
GLfloat ver_wall3[] = { //wspolrzedne wierzcholkow sciany
		-3.0f,  0.0f, 5.0f,
	-3.0f,  0.0f, 0.0f,
	-3.0f,  10.0f, 0.0f,
	-3.0f, 10.0f, 5.0f,
};


//sciana lewo
GLfloat ver_wall4[] = { //wspolrzedne wierzcholkow sciany
0.0f,  0.0f, -1.0f,
	5.0f,  0.0f, -1.0f,
	5.0f,  10.0f, -1.0f,
	0.0f, 10.0f, -1.0f,
};



GLfloat tex_wall2[] = {    //wspolrzedne tekstury dla sciany, 
						//jesli wykraczaja poza przedzial <0,1> nastapi nakladanie zalezne od parametrow ustawionych za pomoca funkcji glTexParameteri, np. GL_REPEAT
	0.0f, 0.0f,    // lewy dolny rog tekstury
	0.0f, 0.5f,
	1.0f, 0.5f,
	1.0f, 0.0f, // prawy gorny rog tekstury
};

GLfloat tex_wall3[] = {    //wspolrzedne tekstury dla sciany, 
						//jesli wykraczaja poza przedzial <0,1> nastapi nakladanie zalezne od parametrow ustawionych za pomoca funkcji glTexParameteri, np. GL_REPEAT
	0.0f, 0.0f,    // lewy dolny rog tekstury
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f, // prawy gorny rog tekstury
};

//color change
float pixels2[] = {			//tekstura o wymiarach 3x3; dla kazdego punktu okreslone skladowe RGB koloru 
	1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 0.0f,

};

//------------------------------------------------------------------------------------------------------------------------------

int init_shaders()
{
	// tworzenie i kompilacja shadera wierzcholkow
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		std::cout << "Kompilacja shadera wierzcholkow NIE powiodla sie!\n";
		return 0;
	}

	// tworzenie i kompilacja shadera fragmentow
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		std::cout << "Kompilacja shadera fragmentow NIE powiodla sie!\n";
		return 0;
	}

	// dolaczenie programow przetwarzajacych wierzcholki i fragmenty do programu cieniujacego
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// wskazniki atrybutow wierzcholkow
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);

	return 1;

}

//------------------------------------------------------------------------------------------------------------------------------

void create_objects()
{
	// generowanie obiektow
	glGenVertexArrays(5, vao);  // obiekt tablicy wierzcholkow, dla kazdego obiektu (np. dla podlogi) mamy jedna tablice
	glGenBuffers(14, vbo);		// obiekty buforow wierzcholkow, dla kazdego typu atrubutow kazdego obiektu mamy jeden bufor (np. bufor dla kolorow podlogi, bufor dla wspolrzednych podlogi itd.)
	glGenBuffers(1, &ebo);		// obiekt bufora elementow (ten sam bufor mozna wykorzystac zarowno dla podlogi jak i sciany)
	glGenTextures(1, &tex2);        // obiekt tekstury
	glGenTextures(1, &tex3);        // obiekt tekstury
	glGenTextures(1, &tex4);        // obiekt tekstury



	// podloga vao[0]

	glBindVertexArray(vao[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);	// bufor wspolrzednych wierzcholkow podlogi
	glBufferData(GL_ARRAY_BUFFER, sizeof(ver_floor), ver_floor, GL_STATIC_DRAW);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);	// bufor kolorow wierzcholkow podlogi
	glBufferData(GL_ARRAY_BUFFER, sizeof(col_floor), col_floor, GL_STATIC_DRAW);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);	// bufor wspolrzednych tekstury podlogi
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex_floor), tex_floor, GL_STATIC_DRAW);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texAttrib);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// trojkat vao[1]

	glBindVertexArray(vao[1]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);	// bufor wspolrzednych wierzcholkow trojkata
	glBufferData(GL_ARRAY_BUFFER, sizeof(ver_triangle), ver_triangle, GL_STATIC_DRAW);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);	// bufor kolorow wierzcholkow trojkata
	glBufferData(GL_ARRAY_BUFFER, sizeof(col_triangle), col_triangle, GL_STATIC_DRAW);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colAttrib);


	// sciana vao[2]
	glBindVertexArray(vao[2]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);    // bufor wspolrzednych wierzcholkow sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(ver_wall), ver_wall, GL_STATIC_DRAW);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);	// bufor kolorow wierzcholkow sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(col_wall), col_wall, GL_STATIC_DRAW);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);	// bufor wspolrzednych tekstury sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex_wall), tex_wall, GL_STATIC_DRAW);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texAttrib);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// sciana vao[3]

	glBindVertexArray(vao[3]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);    // bufor wspolrzednych wierzcholkow sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(ver_wall3), ver_wall3, GL_STATIC_DRAW);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);	// bufor kolorow wierzcholkow sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(col_wall), col_wall, GL_STATIC_DRAW);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);	// bufor wspolrzednych tekstury sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex_wall), tex_wall, GL_STATIC_DRAW);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texAttrib);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// sciana vao[4]

	glBindVertexArray(vao[4]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);    // bufor wspolrzednych wierzcholkow sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(ver_wall4), ver_wall4, GL_STATIC_DRAW);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);	// bufor kolorow wierzcholkow sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(col_wall), col_wall, GL_STATIC_DRAW);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(colAttrib);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);	// bufor wspolrzednych tekstury sciany
	glBufferData(GL_ARRAY_BUFFER, sizeof(tex_wall), tex_wall, GL_STATIC_DRAW);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texAttrib);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

}

//------------------------------------------------------------------------------------------------------------------------------

void configure_texture1()
{
	glGenTextures(1, &tex);		// obiekt tekstury
	glBindTexture(GL_TEXTURE_2D, tex);		// powiazanie tekstury z obiektem (wybor tekstury)

	// ustawienia parametrow tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// sposob nakladania tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // sposob filtrowania tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels_floor); // ladowanie do tekstury tablicy pikseli
}


void configure_texture2()
{
	glBindTexture(GL_TEXTURE_2D, tex2);        // powiazanie tekstury z obiektem (wybor tekstury)

	// ustawienia parametrow tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // sposob nakladania tekstury ||||||GL_CLAMP
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // sposob filtrowania tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 3, 3, 0, GL_RGB, GL_FLOAT, pixels2); // ladowanie do tekstury tablicy pikseli
}

void configure_texture3()
{
	glBindTexture(GL_TEXTURE_2D, tex3);        // powiazanie tekstury z obiektem (wybor tekstury)

	// ustawienia parametrow tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // sposob nakladania tekstury ||||||GL_CLAMP
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // sposob filtrowania tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 3, 3, 0, GL_RGB, GL_FLOAT, pixels2); // ladowanie do tekstury tablicy pikseli
}

void configure_texture4()
{
	glBindTexture(GL_TEXTURE_2D, tex4);        // powiazanie tekstury z obiektem (wybor tekstury)

	// ustawienia parametrow tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);    // sposob nakladania tekstury ||||||GL_CLAMP
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // sposob filtrowania tekstury
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 3, 3, 0, GL_RGB, GL_FLOAT, pixels2); // ladowanie do tekstury tablicy pikseli
}

void configure_texture(float angle_rotate_arrow = 0.1)
{
	//newAngle -= i;
	configure_texture1();

	glBindVertexArray(vao[0]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	//rysujemy podloge

	//glutSwapBuffers();
	glBindVertexArray(vao[1]);
	glDisable(GL_TEXTURE_2D);
	glDrawArrays(GL_TRIANGLES, 0, 3);	//rysujemy trojkat przedstawiajacy polozenie kamery

	configure_texture2();
	glBindVertexArray(vao[2]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    //rysujemy sciane

	configure_texture3();
	glBindVertexArray(vao[3]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    //rysujemy sciane

	configure_texture4();

	glBindVertexArray(vao[4]);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    //rysujemy sciane


}

//------------------------------------------------------------------------------------------------------------------------------


int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_Window* window = SDL_CreateWindow("OpenGL", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	SDL_Event windowEvent;
	float speed = 0.1;
	float change_x = 0.0f;
	float change_y = 0;
	float change_z = -1.0f;
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "ERROR" << std::endl;
	}
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // GL_ALWAYS)

	if (!init_shaders())
		return 0;


	create_objects();


	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 20.0f);		 //macierz rzutowania perspektywicznego
	glm::mat4 viewMatrix;  //macierz widoku
	glm::mat4 transformMatrix; //macierz wynikowa //model matrix

	GLint transformMatrixUniformLocation = glGetUniformLocation(shaderProgram, "transformMatrix");

	int top_view = false; //zmienna okreslajaca czy patrzymy na scene z gory

	glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f); //poczatkowe polozenie kamery
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f); //poczatkowy kierunek, w ktorym kamera jest skierowana
	bool change_arrow = false;
	int check_valid_next_pos = 0;
	bool ch_test = true;
	float angle = 1.0f;
	int speed_circle_rotation = 7;
	bool left_ev = false;
	bool right_ev = false;
	double cos_full = 0.0f;
	double sin_full = 0.0f;
	double cos_res_minus = round(cos(-angle * PI / 180.0) * 1000) / 1000;
	double sin_res_minus = round(sin(-angle * PI / 180.0) * 1000) / 1000;
	double cos_res_plus = round(cos(angle * PI / 180.0) * 1000) / 1000;
	double sin_res_plus = round(sin(angle * PI / 180.0) * 1000) / 1000;
	float angle_of_triangle = 90.0f;
	float angle_for_rot_camera = 0.0f;
	float angle_for_rot_camera_rad = 0.0f;
	float angle_of_triangle_old = 90.0f;
	float angle_for_rot_camera_old = 0.0f;
	int count_of_walls = 3;
	GLdouble ox = 0.0f;
	GLdouble oz = 0.0f;
	GLfloat change_x_pos = 0.0f;
	while (true)
	{
		const Uint8* keystates = SDL_GetKeyboardState(0);
		if (keystates[SDL_SCANCODE_ESCAPE]) break;
		if (keystates[SDL_SCANCODE_SPACE]) {
			//top_view = !top_view;
		}
		if (keystates[SDL_SCANCODE_N]) {

		}

		if (SDL_PollEvent(&windowEvent))
		{

			if (windowEvent.type == SDL_KEYUP &&
				windowEvent.key.keysym.sym == SDLK_ESCAPE) break;
			if (windowEvent.type == SDL_QUIT) break;
			if (windowEvent.type == SDL_KEYDOWN)
			{
				switch (windowEvent.key.keysym.sym)
				{
				case SDLK_SPACE:
					top_view = !top_view;
					break;

				case SDLK_UP:
					for (int j = 0; j < 9; j++)
					{
						ver_triangle_old[j] = ver_triangle[j];
					}
					cos_full = round(cos(angle_of_triangle * PI / 180.0) * 1000) / 1000;
					sin_full = round(sin(angle_of_triangle * PI / 180.0) * 1000) / 1000;
					for (int i = 0; i <= 6; i += 3)
					{
						if (angle_of_triangle >= 0 && angle_of_triangle <= 90)
						{
							ver_triangle[i] -= speed * cos_full;
							ver_triangle[i + 2] -= speed * sin_full;
						}
						else if (angle_of_triangle >= 90 && angle_of_triangle <= 185)
						{
							ver_triangle[i] -= speed * cos_full;
							ver_triangle[i + 2] += speed * (-sin_full);
						}
						else if (angle_of_triangle >= 185 && angle_of_triangle <= 270)
						{
							ver_triangle[i] += speed * (-cos_full);
							ver_triangle[i + 2] += speed * (-sin_full);
						}
						else if (angle_of_triangle >= 270 && angle_of_triangle <= 370)
						{
							ver_triangle[i] += speed * (-cos_full);
							ver_triangle[i + 2] -= speed * sin_full;
						}
					}

					if ((ver_triangle[6] > ver_wall[6] + 0.1 || ver_triangle[6] < ver_wall[0] - 0.1) || ver_triangle[8] < ver_wall[5] - 0.2 || ver_triangle[8] > ver_wall[11] + 0.2) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[6] > ver_wall3[6] + 0.1 || ver_triangle[6] < ver_wall3[0] - 0.1) || ver_triangle[8] < ver_wall3[5] - 0.2 || ver_triangle[8] > ver_wall3[11] + 0.2) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[6] > ver_wall4[6] + 0.1 || ver_triangle[6] < ver_wall4[0] - 0.1) || ver_triangle[8] < ver_wall4[5] - 0.2 || ver_triangle[8] > ver_wall4[11] + 0.2) {
						check_valid_next_pos++;
					}
					if (check_valid_next_pos != count_of_walls) {
						for (int j = 0; j < 9; j++)
						{
							ver_triangle[j] = ver_triangle_old[j];
						}
					}
					else {
						change_arrow = true;
					}
					check_valid_next_pos = 0;
					break;
				case SDLK_DOWN:
					for (int j = 0; j < 9; j++)
					{
						ver_triangle_old[j] = ver_triangle[j];
					}
					cos_full = round(cos(angle_of_triangle * PI / 180.0) * 1000) / 1000;
					sin_full = round(sin(angle_of_triangle * PI / 180.0) * 1000) / 1000;
					for (int i = 0; i <= 6; i += 3)
					{
						if (angle_of_triangle >= 0 && angle_of_triangle <= 90)
						{
							ver_triangle[i] += speed * cos_full;
							ver_triangle[i + 2] += speed * sin_full;
						}
						else if (angle_of_triangle >= 90 && angle_of_triangle <= 185)
						{
							ver_triangle[i] += speed * cos_full;
							ver_triangle[i + 2] -= speed * (-sin_full);
						}
						else if (angle_of_triangle >= 185 && angle_of_triangle <= 270)
						{
							ver_triangle[i] -= speed * (-cos_full);
							ver_triangle[i + 2] -= speed * (-sin_full);
						}
						else if (angle_of_triangle >= 270 && angle_of_triangle <= 370)
						{
							ver_triangle[i] -= speed * (-cos_full);
							ver_triangle[i + 2] += speed * sin_full;
						}
					}
					if ((ver_triangle[0] > ver_wall[6] + 0.1 || ver_triangle[0] < ver_wall[0] - 0.1) &&
						(ver_triangle[3] > ver_wall[6] + 0.1 || ver_triangle[3] < ver_wall[0] - 0.1) ||
						(ver_triangle[2] < ver_wall[5] - 0.2 || ver_triangle[2] > ver_wall[11] + 0.2) &&
						(ver_triangle[5] < ver_wall[5] - 0.2 || ver_triangle[5] > ver_wall[11] + 0.2)) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[0] > ver_wall3[6] + 0.1 || ver_triangle[0] < ver_wall3[0] - 0.1) &&
						(ver_triangle[3] > ver_wall3[6] + 0.1 || ver_triangle[3] < ver_wall3[0] - 0.1) ||
						(ver_triangle[2] < ver_wall3[5] - 0.2 || ver_triangle[2] > ver_wall3[11] + 0.2) &&
						(ver_triangle[5] < ver_wall3[5] - 0.2 || ver_triangle[5] > ver_wall3[11] + 0.2)) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[0] > ver_wall4[6] + 0.1 || ver_triangle[0] < ver_wall4[0] - 0.1) &&
						(ver_triangle[3] > ver_wall4[6] + 0.1 || ver_triangle[3] < ver_wall4[0] - 0.1) ||
						(ver_triangle[2] < ver_wall4[5] - 0.2 || ver_triangle[2] > ver_wall4[11] + 0.2) &&
						(ver_triangle[5] < ver_wall4[5] - 0.2 || ver_triangle[5] > ver_wall4[11] + 0.2)) {
						check_valid_next_pos++;
					}
					if (check_valid_next_pos != count_of_walls) {
						for (int j = 0; j < 9; j++)
						{
							ver_triangle[j] = ver_triangle_old[j];
						}
					}
					else {
						change_arrow = true;
					}
					check_valid_next_pos = 0;
					break;
				case SDLK_LEFT:
					for (int j = 0; j < 9; j++)
					{
						ver_triangle_old[j] = ver_triangle[j];
					}
					angle_of_triangle_old = angle_of_triangle;
					angle_for_rot_camera_old = angle_for_rot_camera;
					for (int i = 0; i < speed_circle_rotation; i++)
					{

						//cos_full = round(cos(angle_of_triangle * PI / 180.0) * 1000) / 1000;
						//sin_full = round(sin(angle_of_triangle * PI / 180.0) * 1000) / 1000;
						ox = (ver_triangle[0] + ver_triangle[3] + ver_triangle[6]) / 3;
						oz = (ver_triangle[2] + ver_triangle[5] + ver_triangle[8]) / 3;
						ver_triangle[0] = ox + (ver_triangle[0] - ox) * cos_res_minus - (ver_triangle[2] - oz) * sin_res_minus;
						ver_triangle[2] = oz + (ver_triangle[0] - ox) * sin_res_minus + (ver_triangle[2] - oz) * cos_res_minus;
						ver_triangle[3] = ox + (ver_triangle[3] - ox) * cos_res_minus - (ver_triangle[5] - oz) * sin_res_minus;
						ver_triangle[5] = oz + (ver_triangle[3] - ox) * sin_res_minus + (ver_triangle[5] - oz) * cos_res_minus;
						ver_triangle[6] = ox + (ver_triangle[6] - ox) * cos_res_minus - (ver_triangle[8] - oz) * sin_res_minus;
						ver_triangle[8] = oz + (ver_triangle[6] - ox) * sin_res_minus + (ver_triangle[8] - oz) * cos_res_minus;

						if (angle_of_triangle == 0) {
							angle_of_triangle = 360;
						}
						angle_for_rot_camera--;
						angle_of_triangle--;
					}

					if ((ver_triangle[6] > ver_wall[6] + 0.1 || ver_triangle[6] < ver_wall[0] - 0.1) || ver_triangle[8] < ver_wall[5] - 0.2 || ver_triangle[8] > ver_wall[11] + 0.2) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[6] > ver_wall3[6] + 0.1 || ver_triangle[6] < ver_wall3[0] - 0.1) || ver_triangle[8] < ver_wall3[5] - 0.2 || ver_triangle[8] > ver_wall3[11] + 0.2) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[6] > ver_wall4[6] + 0.1 || ver_triangle[6] < ver_wall4[0] - 0.1) || ver_triangle[8] < ver_wall4[5] - 0.2 || ver_triangle[8] > ver_wall4[11] + 0.2) {
						check_valid_next_pos++;
					}
					if (check_valid_next_pos != count_of_walls) {
						for (int j = 0; j < 9; j++)
						{
							ver_triangle[j] = ver_triangle_old[j];
						}
						angle_for_rot_camera = angle_for_rot_camera_old;
						angle_of_triangle = angle_of_triangle_old;
					}
					else {
						change_arrow = true;
					}
					check_valid_next_pos = 0;
					break;
				case SDLK_RIGHT:
					for (int j = 0; j < 9; j++)
					{
						ver_triangle_old[j] = ver_triangle[j];
					}
					angle_of_triangle_old = angle_of_triangle;
					angle_for_rot_camera_old = angle_for_rot_camera;
					for (int i = 0; i < speed_circle_rotation; i++)
					{
						ox = (ver_triangle[0] + ver_triangle[3] + ver_triangle[6]) / 3;
						oz = (ver_triangle[2] + ver_triangle[5] + ver_triangle[8]) / 3;
						ver_triangle[0] = ox + (ver_triangle[0] - ox) * cos_res_plus - (ver_triangle[2] - oz) * sin_res_plus;
						ver_triangle[2] = oz + (ver_triangle[0] - ox) * sin_res_plus + (ver_triangle[2] - oz) * cos_res_plus;
						ver_triangle[3] = ox + (ver_triangle[3] - ox) * cos_res_plus - (ver_triangle[5] - oz) * sin_res_plus;
						ver_triangle[5] = oz + (ver_triangle[3] - ox) * sin_res_plus + (ver_triangle[5] - oz) * cos_res_plus;
						ver_triangle[6] = ox + (ver_triangle[6] - ox) * cos_res_plus - (ver_triangle[8] - oz) * sin_res_plus;
						ver_triangle[8] = oz + (ver_triangle[6] - ox) * sin_res_plus + (ver_triangle[8] - oz) * cos_res_plus;
						if (angle_of_triangle == 360) {
							angle_of_triangle = 0;
						}
						angle_for_rot_camera++;
						angle_of_triangle++;
					}
					if ((ver_triangle[6] > ver_wall[6] + 0.1 || ver_triangle[6] < ver_wall[0] - 0.1) || ver_triangle[8] < ver_wall[5] - 0.2 || ver_triangle[8] > ver_wall[11] + 0.2) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[6] > ver_wall3[6] + 0.1 || ver_triangle[6] < ver_wall3[0] - 0.1) || ver_triangle[8] < ver_wall3[5] - 0.2 || ver_triangle[8] > ver_wall3[11] + 0.2) {
						check_valid_next_pos++;
					}
					if ((ver_triangle[6] > ver_wall4[6] + 0.1 || ver_triangle[6] < ver_wall4[0] - 0.1) || ver_triangle[8] < ver_wall4[5] - 0.2 || ver_triangle[8] > ver_wall4[11] + 0.2) {
						check_valid_next_pos++;
					}
					if (check_valid_next_pos != count_of_walls) {
						for (int j = 0; j < 9; j++)
						{
							ver_triangle[j] = ver_triangle_old[j];
						}
						angle_for_rot_camera = angle_for_rot_camera_old;
						angle_of_triangle = angle_of_triangle_old;
					}
					else {
						change_arrow = true;
					}
					check_valid_next_pos = 0;
					break;
				}
			}
		}
		if (change_arrow) {
			glBindVertexArray(vao[1]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);	// bufor wspolrzednych wierzcholkow trojkata
			glBufferData(GL_ARRAY_BUFFER, sizeof(ver_triangle), ver_triangle, GL_STATIC_DRAW);
			glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(posAttrib);
			change_arrow = false;
		}
		//cout << change_x << endl;
		direction = glm::vec3(change_x, change_y, change_z);
		position = glm::vec3(0.0f + ver_triangle[6], 1.0f + ver_triangle[7], 5.0f + ver_triangle[8]);
		//glm::mat4 myMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
		if (top_view) //patrzymy z gory
			viewMatrix = glm::lookAt(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

		else { //patrzymy z miejsca, w ktorym jest obserwator 

			viewMatrix = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
			viewMatrix = glm::translate(viewMatrix, glm::vec3(ver_triangle[6], ver_triangle[7], ver_triangle[8]));
			viewMatrix = glm::rotate(viewMatrix, glm::radians(angle_for_rot_camera), glm::vec3(0.0f, 1.0f, 0.0f));
			viewMatrix = glm::translate(viewMatrix, glm::vec3(-ver_triangle[6], -ver_triangle[7], -ver_triangle[8]));
		}

		//glm::mat4 rotateMatrix = glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f));;
		transformMatrix = projectionMatrix * viewMatrix;				// wynikowa macierz transformacji
		//transformMatrix = glm::rotate(transformMatrix, glm::radians(20.0f * 2), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(transformMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(transformMatrix));	// macierz jako wejściowa zmienna dla shadera wierzcholkow


		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);	// szare tlo
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		configure_texture();

		SDL_GL_SwapWindow(window);
	}

	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	glDeleteBuffers(16, vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteTextures(1, &tex);
	glDeleteVertexArrays(5, vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}