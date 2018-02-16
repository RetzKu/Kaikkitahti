#include <stdint.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <assert.h>
#include <glut/glut.h>
#include <memory.h>
#include <vector>
#include <deque>
#include <memory>




struct Cell
{
	Cell(int x, int y, int score, int walked, std::shared_ptr<Cell> previous) {
		this->x = x; this->y = y; this->score = score; this->totalwalked = walked; /*this->previous = previous*/;
	}
	Cell() {};
	int x = 0;
	int y = 0;
	int score = 999999;
	int totalwalked = 0;
	
	//std::shared_ptr<Cell> previous;
};

int DistanceScore(Cell a, Cell end);
// STUDENT_TODO: Make implementation for doPathFinding function, which writes found path to outputData
bool once = true;
std::deque<Cell> OpenList;
std::deque<Cell> ClosedList;

Cell BestScore(std::deque<Cell> *list)
{
	Cell best;
	best.score = 999999999;
	int last;
	for(int i = 0; i < list->size(); i++)
	{
		if (list->at(i).score < best.score)
		{
			best = list->at(i);
			last = i;
		}
	}
	list->erase(list->begin()+last);
	return best;
}

Cell end;
bool found = false;
void doPathFinding(const uint8_t* inputData, int width, int height, uint8_t* outputData, int startX, int startY, int endX, int endY)
{
	if(once)
	{
		once = false;
		end.x = endX;
		end.y = endY;
		Cell tmp;
		tmp.x = startX; tmp.y = startY; tmp.totalwalked = 0; tmp.score = DistanceScore(tmp,end);
		OpenList.push_back(tmp);
	}

	if (found)
	{
		//while (last->previous->score != 0)
		//{
		//	last = last->previous;
		//	uint8_t* pix = &outputData[3 * ((last->y)*width + (last->x))]; // get pixel
		//																			   //std::cout << "CurrentX: " << currentx << " CurrentY: " << currenty << std::endl;
		//	pix[0] = 0;
		//	pix[1] = 0;
		//	pix[2] = 255;
		//}
	}
	if (!found)
	{
		Cell current = BestScore(&OpenList);
		int index = 0;
		for (int x = -1; x < 2; x++)
		{
			for (int y = -1; y < 2; y++)
			{
				uint8_t* pix = &outputData[3 * ((y + current.y)*width + (x + current.x))]; // get pixel
				//std::cout << "CurrentX: " << currentx << " CurrentY: " << currenty << std::endl;
				uint8_t r = pix[2];
				uint8_t g = pix[1];
				uint8_t b = pix[0];

				int checkX = current.x + x;
				int checkY = current.y + y;

				if (r == 255 && g == 0 && b == 0)
				{
					std::cout << "voitit pelin";
					found = true;
					//last = std::shared_ptr<Cell>(new Cell(current));
				}
				if (r == 255 && g == 255 && b == 255)
				{
					Cell best;

					//Distance calc
					best.x = current.x + x;
					best.y = current.y + y;
					int distancescore = DistanceScore(best, end);

					//walked Calc
					int movementcost = 0;
					if (x != 0 && y != 0) { movementcost = 14 + current.totalwalked; }
					else if (x != 0 || y != 0) { movementcost = 10 + current.totalwalked; }

					int totalscore = distancescore + movementcost;

					best.score = totalscore; best.totalwalked = movementcost;
					//best.previous = (current.x, current.y, current.score, current.totalwalked, current.previous);

					if (!OpenList.empty())
					{
						if (OpenList.front().score > totalscore)
						{
							OpenList.push_front(best);
							index++;
						}
						else
						{
							OpenList.push_back(best);
						}
					}
					else
					{
						OpenList.push_back(best);
					}

					uint8_t* pix = &outputData[3 * ((current.y)*width + (current.x))];
					pix[0] = 255;
					pix[1] = 0;
					pix[2] = 0;

					pix = &outputData[3 * ((current.y + y)*width + (current.x + x))];
					pix[0] = 255;
					pix[1] = 0;
					pix[2] = 0;
				}
			}
		}
		ClosedList.push_front(current);
	}
}

int DistanceScore(Cell a, Cell end)
{
	int multi = 1000;
	return (std::abs(a.x - end.x) + std::abs(a.y - end.y))*multi;
}

namespace
{
	// Quick and dirty function for reading bmp-files to opengl textures.
	GLuint loadBMPTexture(const char *fileName, int* w, int* h, uint8_t** data)
	{
		assert(w != 0);
		assert(h != 0);
		assert(data != 0);
		FILE *file;
		if ((file = fopen(fileName, "rb")) == NULL)
			return 0;
		fseek(file, 18, SEEK_SET);

		int width = 0;
		fread(&width, 2, 1, file);
		fseek(file, 2, SEEK_CUR);
		int height = 0;
		fread(&height, 2, 1, file);
		printf("Image \"%s\" (%dx%d)\n", fileName, width, height);

		*data = new uint8_t[3 * width * height];
		assert(data != 0);
		fseek(file, 30, SEEK_CUR);
		fread(*data, 3, width * height, file);
		fclose(file);

		GLuint  texId;
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, *data);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (w) *w = width;
		if (h) *h = height;
		return texId;
	}

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	// Global variables

	// OpenGL texture ids for rendering.
	GLuint  inputTexture = 0;
	GLuint  outputTexture = 0;
	// Input and output data in pixels. outputData is updated to outputTexture each frame
	uint8_t* inputData = 0;
	uint8_t* outputData = 0;
	// width and height of the input and output datas
	int width = 0;
	int height = 0;
	// start and end position for path finding. These are found automatically from input file.
	int startX = -1;
	int startY = -1;
	int endX = -1;
	int endY = -1;

	// Initialization
	bool init()
	{
		glMatrixMode(GL_PROJECTION);
		glOrtho(0, 512 + 4, 256 + 2, 0, -1, 1);

		// Load input file
		inputTexture = loadBMPTexture("input.bmp", &width, &height, &inputData);
		if (0 == inputTexture)
		{
			printf("Error! Cannot open file: \"input.bmp\"\n");
			return false;
		}

		// Make outputTexture
		glGenTextures(1, &outputTexture);
		glBindTexture(GL_TEXTURE_2D, outputTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Copy inputData also to outputData
		outputData = new uint8_t[3 * width*height];
		memcpy(outputData, inputData, 3 * width*height);

		// find start and end
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				uint8_t* pix = &inputData[3 * (y*width + x)]; // get pixel
				uint8_t r = pix[0];
				uint8_t g = pix[1];
				uint8_t b = pix[2];
				if (255 == r && g == 0 && b == 0) // Red?
				{
					// Start
					startX = x;
					startY = y;
					printf("Start position: <%d,%d>\n", x, y);
				}
				if (255 == b && r == 0 && g == 0) // Blue?
				{
					// End
					endX = x;
					endY = y;
					printf("End position: <%d,%d>\n", x, y);
				}
			}
		}

		if (startX < 0 || startY < 0)
		{
			printf("Error! Start position not found\n");
			return false;
		}

		if (endX < 0 || endY < 0)
		{
			printf("Error! End position not found\n");
			return false;
		}

		return true;
	}

	// Draw/Render
	void draw() 
	{
		doPathFinding(inputData, width, height, outputData, startX, startY, endX, endY);

		// Copy outputData to outputTexture
		glBindTexture(GL_TEXTURE_2D, outputTexture);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, outputData);
		glBindTexture(GL_TEXTURE_2D, 0);

		glClear(GL_COLOR_BUFFER_BIT);

		// Draw input texture to left half of the screen
		glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, inputTexture);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 1); glVertex2d(1, 1);
		glTexCoord2d(0, 0); glVertex2d(1, 1 + 256);
		glTexCoord2d(1, 0); glVertex2d(1 + 256, 1 + 256);
		glTexCoord2d(1, 1); glVertex2d(1 + 256, 1);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();

		// Draw output texture to right half of the screen
		glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, outputTexture);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 1); glVertex2d(2 + 256, 1);
		glTexCoord2d(0, 0); glVertex2d(2 + 256, 1 + 256);
		glTexCoord2d(1, 0); glVertex2d(2 + 512, 1 + 256);
		glTexCoord2d(1, 1); glVertex2d(2 + 512, 1);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();

		glutSwapBuffers();
		glutPostRedisplay();
	}
} // end - anonymous namespace


// Main
int main(int argc, char ** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(2*(512+4), 2*(256+2));
	glutCreateWindow("Pathfinding Demo");
	glutDisplayFunc(draw);
	if (!init()) return -1;
	glutMainLoop();
	delete inputData;
	delete outputData;
	return 0;
}