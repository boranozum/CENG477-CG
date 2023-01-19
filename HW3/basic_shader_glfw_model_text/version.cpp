#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include <ctime>

#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[3];
GLint gIntensityLoc;
float gIntensity = 200;
int gWidth = 640, gHeight = 600;

typedef enum {EXPLODE, TRANSITION, IDLE} STATE;

float y_coord = 0.0;
int transition_column = 0;
int transition_head = 0;
int grid_w, grid_h;
int score=0, moves=0;
int explosion_count = 0;
int column_count = 0;
STATE state = IDLE;

GLuint modelMatrixBuffer;

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<vector<glm::vec3>> colors = {
    {
        glm::vec3(0.5, 0.0, 0.0),
        glm::vec3(0.6, 0.6, 0.6),
        glm::vec3(0.1, 0.1, 0.1),
    },
    {
        glm::vec3(0.0, 0.15, 0.0),
        glm::vec3(0.6, 0.6, 0.0),
        glm::vec3(0.1, 0.1, 0.2),
    },
    {
        glm::vec3(0.0, 0.0, 0.2),
        glm::vec3(0.6, 0.6, 0.6),
        glm::vec3(0.2, 0.2, 0.0),
    },
    {
        glm::vec3(0.3, 0.15, 0.0),
        glm::vec3(0.6, 0.6, 0.6),
        glm::vec3(0.1, 0.1, 0.1),
    },
    {
        glm::vec3(0.5, 0.15, 0.4),
        glm::vec3(0.6, 0.6, 0.6),
        glm::vec3(0.01, 0.0, 0.01),
    },
};

vector<Vertex> gVertices;
vector<Texture> gTextures;
vector<Normal> gNormals;
vector<Face> gFaces;

GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
bool refresh = false;

vector<GLfloat> y_coords;
vector<int> transition_heads;
vector<int> transition_columns;

class Cell
{
    public:
        GLint color_index;
        GLfloat x, y;
        float size;
        float explode_size;
        bool will_explode;
        bool exploded;
        Cell(GLfloat inX, GLfloat inY, GLint inColor_index) : x(inX), y(inY), color_index(inColor_index){ 
            size = 25.0/(grid_h*grid_w);
            explode_size = size*1.5;
            will_explode = false;
            exploded = false;
        }
        // do assignment operator overloading
        Cell& operator=(const Cell& rhs) {
            x = rhs.x;
            y = rhs.y;
            color_index = rhs.color_index;
            size = rhs.size;
            explode_size = rhs.explode_size;
            will_explode = rhs.will_explode;
            exploded = rhs.exploded;
            return *this;
        }   
};

vector<vector<Cell>> grid;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


bool ParseObj(const string& fileName)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    gFaces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x, 
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x, 
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x, 
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices.size() == gNormals.size());

    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram();
    gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();

    createVS(gProgram[0], "vert0.glsl");
    createFS(gProgram[0], "frag0.glsl");

    createVS(gProgram[1], "vert1.glsl");
    createFS(gProgram[1], "frag1.glsl");

    createVS(gProgram[2], "vert_text.glsl");
    createFS(gProgram[2], "frag_text.glsl");

    glBindAttribLocation(gProgram[0], 0, "inVertex");
    glBindAttribLocation(gProgram[0], 1, "inNormal");
    glBindAttribLocation(gProgram[1], 0, "inVertex");
    glBindAttribLocation(gProgram[1], 1, "inNormal");
    glBindAttribLocation(gProgram[2], 2, "vertex");

    glLinkProgram(gProgram[0]);
    glLinkProgram(gProgram[1]);
    glLinkProgram(gProgram[2]);
    glUseProgram(gProgram[0]);

    gIntensityLoc = glGetUniformLocation(gProgram[0], "intensity");
    cout << "gIntensityLoc = " << gIntensityLoc << endl;
    glUniform1f(gIntensityLoc, gIntensity);
}

void initVBO()
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [gVertices.size() * 3];
    GLfloat* normalData = new GLfloat [gNormals.size() * 3];
    GLuint* indexData = new GLuint [gFaces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3*i] = gVertices[i].x;
        vertexData[3*i+1] = gVertices[i].y;
        vertexData[3*i+2] = gVertices[i].z;

        minX = std::min(minX, gVertices[i].x);
        maxX = std::max(maxX, gVertices[i].x);
        minY = std::min(minY, gVertices[i].y);
        maxY = std::max(maxY, gVertices[i].y);
        minZ = std::min(minZ, gVertices[i].z);
        maxZ = std::max(maxZ, gVertices[i].z);
    }

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3*i] = gNormals[i].x;
        normalData[3*i+1] = gNormals[i].y;
        normalData[3*i+2] = gNormals[i].z;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3*i] = gFaces[i].vIndex[0];
        indexData[3*i+1] = gFaces[i].vIndex[1];
        indexData[3*i+2] = gFaces[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init(string objFile) 
{
	//ParseObj("armadillo.obj");
	ParseObj(objFile.c_str());

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initFonts(gWidth, gHeight);
    initVBO();

    for (float i = 0; i < grid_w; i++)
    {

        vector<Cell> row;
        
        for (float j = 0; j < grid_h; j++)
        {
            GLint index = rand() % 5;
            Cell cell = Cell(-10+(20.0/(grid_w+1))+20*i/(grid_w+1), -10+(20.0/(grid_h+1))+20*j/(grid_h+1), index);
            if(grid_h*grid_w < 25){
                cell.size = 1;
                cell.explode_size = 1.5;
            }
            cout << "cell coords: " << cell.x << ", " << cell.y << endl;
            row.push_back(cell);
        }

        grid.push_back(row);
        transition_columns.push_back(0);
        transition_heads.push_back(grid_h);
        y_coords.push_back(grid[i][grid_h-1].y+20);
    }
}

int updateExplosion(int i, int j){
    
    if (!grid[i][j].will_explode) {
        grid[i][j].will_explode = true;
        return 1;
    }
    return 0;
}

int findMatches()
{
    bool found = false;
    int count = 0;
    for (int i = 0; i < grid_w; i++) {
        for (int j = 0; j < grid_h - 2; j++) {
            if (grid[i][j].color_index == grid[i][j+1].color_index && grid[i][j+1].color_index == grid[i][j+2].color_index) {
                // state = EXPLODE;
                count += updateExplosion(i, j);
                count += updateExplosion(i, j+1);
                count += updateExplosion(i, j+2);

                found = true;

                y_coords[i] = min(grid[i][j].y, y_coords[i]);
                transition_heads[i] = min(j, transition_heads[i]);
            }
        }
    }
    for (int j = 0; j < grid_h; j++) {
        for (int i = 0; i < grid_w - 2; i++) {
            if (grid[i][j].color_index == grid[i+1][j].color_index && grid[i+1][j].color_index == grid[i+2][j].color_index) {
                // state = EXPLODE;
                
                count += updateExplosion(i, j);
                count += updateExplosion(i+1, j);
                count += updateExplosion(i+2, j);

                found = true;

                y_coords[i] = min(grid[i][j].y, y_coords[i]);
                y_coords[i+1] = min(grid[i][j].y, y_coords[i+1]);
                y_coords[i+2] = min(grid[i][j].y, y_coords[i+2]);

                transition_heads[i] = min(j, transition_heads[i]);
                transition_heads[i+1] = min(j, transition_heads[i+1]);
                transition_heads[i+2] = min(j, transition_heads[i+2]);

                
            }
        }
    }

    for(int i = 0; i < grid_w; i++){
        transition_columns[i] = grid_h - transition_heads[i];
    }

    if (found) {
        state = EXPLODE;
        for(int i = 0; i < grid_w; i++){
            cout << "transition column: " << transition_columns[i] << endl;
            cout << "transition head: " << transition_heads[i] << endl;
        }

        cout << "============================" << endl;
    }

    return count;
}


void drawModel()
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, gFaces.size() * 3, GL_UNSIGNED_INT, 0);
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[2]);
    glUniform3f(glGetUniformLocation(gProgram[2], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

int getColumnCount(){
    int count = 0;
    for (int i = 0; i < grid_w; i++)
    {
        if(transition_columns[i] > 0){
            count++;
        }
    }
    
    return count;
}

void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;

    if(refresh){
        angle = 0;
        refresh = false;
    }

    // glUseProgram(gProgram[0]);
	// //glLoadIdentity();
	// //glTranslatef(-2, 0, -10);
	// //glRotatef(angle, 0, 1, 0);

    // glm::mat4 T = glm::translate(glm::mat4(1.f), glm::vec3(-2.f, 0.f, -10.f));
    // glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0, 1, 0));
    // glm::mat4 modelMat = T * R;
    // glm::mat4 modelMatInv = glm::transpose(glm::inverse(modelMat));
    // glm::mat4 perspMat = glm::perspective(glm::radians(45.0f), 1.f, 1.0f, 100.0f);

    // glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelfindMat));
    // glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
    // glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

    // drawModel();
 
    // glUseProgram(gProgram[1]);
	// //glLoadIdentity();
	// //glTranslatef(2, 0, -10);
	// //glRotatef(-angle, 0, 1, 0);

    // T = glm::translate(glm::mat4(1.f), glm::vec3(2.f, 0.f, -10.f));
    // R = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0, 1, 0));
    // modelMat = T * R;
    // modelMatInv = glm::transpose(glm::inverse(modelMat));

    // glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    // glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
    //glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

    // drawModel();

    vector<glm::mat4> modelMats;    

    bool inPlace = true;

    for(float i = 0; i < grid.size(); i++){
        
        for(float j = 0; j < grid[i].size(); j++){

            bool render = true;

            glm::mat4 T;
            glm::mat4 R;
            glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(grid[i][j].size, grid[i][j].size, grid[i][j].size));
            glm::mat4 modelMat;
            glm::mat4 modelMatInv;
            glm::mat4 perspMat = glm::ortho(-10.f, 10.f, -10.f, 10.f, -20.f, 20.f);
            glm::vec3 lightPos;

            glUseProgram(gProgram[0]);
            // GLint color_index = glGetUniformLocation(gProgram[1], "color_index");
            // glUniform1i(color_index, grid[i][j].color_index);
            glUniform3fv(glGetUniformLocation(gProgram[0], "kd"), 1, glm::value_ptr(colors[grid[i][j].color_index][0]));
            glUniform3fv(glGetUniformLocation(gProgram[0], "ks"), 1, glm::value_ptr(colors[grid[i][j].color_index][1]));
            glUniform3fv(glGetUniformLocation(gProgram[0], "ka"), 1, glm::value_ptr(colors[grid[i][j].color_index][2]));

            

            switch (state)
            {
            case IDLE:

                T = glm::translate(glm::mat4(1.f), glm::vec3(grid[i][j].x, grid[i][j].y, -5.f));
                R = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0, 1, 0));
                modelMat = T * R * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                lightPos = glm::vec3(grid[i][j].x, grid[i][j].y, 1);
                glUniform3fv(glGetUniformLocation(gProgram[0], "lightPos"), 1, glm::value_ptr(lightPos));

                explosion_count = findMatches();
                score += explosion_count;
                column_count = getColumnCount();

                break;

            case EXPLODE:
                    
                T = glm::translate(glm::mat4(1.f), glm::vec3(grid[i][j].x, grid[i][j].y, -5.f));
                R = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0, 1, 0));
                modelMat = T * R * S;
                modelMatInv = glm::transpose(glm::inverse(modelMat));

                lightPos = glm::vec3(grid[i][j].x, grid[i][j].y, 1);
                glUniform3fv(glGetUniformLocation(gProgram[0], "lightPos"), 1, glm::value_ptr(lightPos));

                if(grid[i][j].will_explode){

                    grid[i][j].size += 0.01f;
                    if(grid[i][j].size > grid[i][j].explode_size - 0.01f && grid[i][j].size < grid[i][j].explode_size + 0.01f){
                        grid[i][j].will_explode = false;
                        grid[i][j].exploded = true;


                        grid[i].erase(grid[i].begin() + j);
                        // Cell cell = Cell(-10+(20.0/(grid_w+1))+20*i/(grid_w+1), -10+(20.0/(grid_h+1))+20*j/(grid_h+1), index);
                        Cell cell = Cell(-10+(20.0/(grid_w+1))+i*20.0/(grid_w+1), grid[i][grid_h-1].y+20.0/(grid_h+1), rand() % 5);
                        if(grid_h*grid_w < 25){
                            cell.size = 1;
                            cell.explode_size = 1.5;
                        }
                        grid[i].push_back(cell);

                        explosion_count--;
                        if(explosion_count == 0){
                            cout << "column_count: " << column_count << endl;
                            state = TRANSITION;
                        }

                        render = false;
                    }
                }

                break;

            case TRANSITION:
                render = true;
                if(!transition_columns[i]){
                    T = glm::translate(glm::mat4(1.f), glm::vec3(grid[i][j].x, grid[i][j].y, -5.f));
                    R = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0, 1, 0));
                    modelMat = T * R * S;
                    modelMatInv = glm::transpose(glm::inverse(modelMat));
                }
                else{
                    // if(j < transition_heads[i]){
                    //     T = glm::translate(glm::mat4(1.f), glm::vec3(grid[i][j].x, grid[i][j].y, -5.f));
                    //     R = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0, 1, 0));
                    //     modelMat = T * R * S;
                    //     modelMatInv = glm::transpose(glm::inverse(modelMat));
                    // }
                    // else{
                    //     T = glm::translate(glm::mat4(1.f), glm::vec3(grid[i][j].x, grid[i][j].y, -5.f));
                    //     R = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0, 1, 0));
                    //     modelMat = T * R * S;
                    //     modelMatInv = glm::transpose(glm::inverse(modelMat));
                    //     grid[i][j].y -= 0.05f;
                    //     if(grid[i][transition_heads[i]].y >= y_coords[i] - 0.05 && grid[i][transition_heads[i]].y <= y_coords[i] + 0.05){


                    //         // SIKINTIIII
                    //         column_count--;
                    //         if(column_count == 0){
                    //             state = IDLE;
                    //         }

                    //         if(j == transition_heads[i]){
                    //             grid[i][j].y = y_coords[i];
                    //         }
                    //         else{
                    //             grid[i][j].y = grid[i][j-1].y;
                    //         } 
                    //         transition_columns[i] = false;  
                    //         y_coords[i] = grid[i][grid_h-1].y+20;
                    //         transition_heads[i] = grid_h;
                                        
                    //     }

                    // }

                    T = glm::translate(glm::mat4(1.f), glm::vec3(grid[i][j].x, grid[i][j].y, -5.f));
                    R = glm::rotate(glm::mat4(1.f), glm::radians(-angle), glm::vec3(0, 1, 0));
                    modelMat = T * R * S;
                    modelMatInv = glm::transpose(glm::inverse(modelMat));

                    if(j >= transition_heads[i]){
                        grid[i][j].y -= 0.05f;
                        if(grid[i][j].y >= -10+(20.0/(grid_h+1))+j*20.0/(grid_h+1) - 0.05 && grid[i][j].y <= -10+(20.0/(grid_h+1))+j*20.0/(grid_h+1) + 0.05){
                            grid[i][j].y = -10+(20.0/(grid_h+1))+j*20.0/(grid_h+1);
                            inPlace = true;
                        }
                        else{
                            inPlace = false;
                        }
                    } 

                    
                }

                lightPos = glm::vec3(grid[i][j].x, grid[i][j].y, 1);
                glUniform3fv(glGetUniformLocation(gProgram[0], "lightPos"), 1, glm::value_ptr(lightPos));

                break;
            
            default:
                break;
            }

            if(render){
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
                glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

                drawModel();
            }
        }
    }

    if(state == TRANSITION && inPlace){
        state = IDLE;
    }

    assert(glGetError() == GL_NO_ERROR);

    // convert moves and score to string
    string text = "Moves: " + to_string(moves) + " Score: " + to_string(score);

    renderText(text, 0, 0, 1, glm::vec3(1, 1, 0));

    assert(glGetError() == GL_NO_ERROR);

	angle += 0.5;
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);


}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS && state == IDLE)
    {

        moves = 0;
        score = 0;
        for (float i = 0; i < grid_w; i++)
        {
            for (float j = 0; j < grid_h; j++)
            {
                GLint index = rand() % 5;
                grid[i][j].color_index = index;
            }
        }

        refresh = true;
    }
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
    cout << button << endl;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && state == IDLE)
    {
        moves++;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = xpos / gWidth * 20 - 10;
        ypos = -ypos / gHeight * 20 + 10;
        printf("xpos: %f, ypos: %f\n", xpos, ypos);
        // find the grid cell that was clicked
        for (int i = 0; i < grid_w; i++)
        {
            for (int j = 0; j < grid_h; j++)
            {
                if (xpos > grid[i][j].x - (9.0/grid_w) && xpos < grid[i][j].x + (9.0/grid_w) && ypos > grid[i][j].y - (9.0/grid_h) && ypos < grid[i][j].y + (9.0/grid_h))
                {
                    grid[i][j].will_explode = true;
                    y_coords[i] = grid[i][j].y;
                    transition_heads[i] = j;
                    transition_columns[i] = grid_h - j;
                    cout << i << " " << j << endl;
                }
            }
        }

        state = EXPLODE;
        explosion_count = 1;
    }
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    srand(time(NULL));

    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(gWidth, gHeight, "Simple Example", NULL, NULL);

    if(argc < 1){
        cout << "Usage: ./main grid_w grid_h .obj" << endl;
        exit(-1);
    }

    grid_w = atoi(argv[1]);
    grid_h = atoi(argv[2]);
    string objfile = argv[3];

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init(objfile);

    glfwSetKeyCallback(window, keyboard);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

