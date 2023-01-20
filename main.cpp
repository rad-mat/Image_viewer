/* PRZEGLĄDARKA PLIKÓW GRAFICZNYCH W FORMACIE *.ply
W celu nawigacji między obrazkami użyj klawiszy strzałek. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GL\glew.h"
#include "GL\freeglut.h"
#include "shaderLoader.h"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

int screen_width = 640;
int screen_height = 480;

// perspektywa
double kameraX = 45.0;
double kameraY = 20.0;
double kameraZ = -15.0;

int pozycjaMyszyX;
int pozycjaMyszyY;
int mbutton;
double poprzednie_kameraX;
double poprzednie_kameraY;
double poprzednie_kameraZ;

glm::mat4 MV;
glm::mat4 P;

int nFiles = 0;
char** fileNames;

// wczytywane wierzchołki i ściany objektów
float** vertObj;
GLuint* nVertObj;
GLuint** elemObj;
GLuint* nElemObj;

// atrybuty pomocnicze
float** wallCol;
float** vertAll;

float* maxX;
float* maxY;
float* maxZ;
float* minX;
float* minY;
float* minZ;
float* scale;

float** axesCoords;

// czytanie danych o wierzchołkach z pliku
void readVertexInfo( const char* filename, float** vertArray, GLuint* nVert, GLuint** elemArray, GLuint* nElem );

GLuint programID;
GLuint* VBOvert;
GLuint* VBOaxes;
GLuint* VBOcol;
GLuint* VAOobj;
GLuint* VAOaxes;

void mysz( int button, int state, int x, int y )
{
    mbutton = button;
    switch( state )
    {
    case GLUT_UP:
        break;
    case GLUT_DOWN:
        pozycjaMyszyX = x;
        pozycjaMyszyY = y;
        poprzednie_kameraX = kameraX;
        poprzednie_kameraY = kameraY;
        poprzednie_kameraZ = kameraZ;
        break;
    }
}

void mysz_ruch( int x, int y )
{
    if( mbutton == GLUT_LEFT_BUTTON )
    {
        kameraX = poprzednie_kameraX - ( pozycjaMyszyX - x ) * 0.1;
        kameraY = poprzednie_kameraY - ( pozycjaMyszyY - y ) * 0.1;
    }
    if( mbutton == GLUT_RIGHT_BUTTON )
    {
        kameraZ = poprzednie_kameraZ + ( pozycjaMyszyY - y ) * 0.1;
    }
}

int model = 0;
void klawisz( int key, int x, int y )
{
    switch( key ) {
    case GLUT_KEY_LEFT: model--; break;
    case GLUT_KEY_RIGHT: model++; break;
    }
    model += nFiles;
    model %= nFiles;
}

void rysuj( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glClearBufferfv( GL_COLOR, 0, color );

    glUseProgram( programID );

    MV = glm::mat4( 1.0f );
    MV = glm::translate( MV, glm::vec3( 0, 0, kameraZ ) );
    MV = glm::rotate( MV, (float)glm::radians( kameraY - 90.0f ), glm::vec3( 1, 0, 0 ) );
    MV = glm::rotate( MV, (float)glm::radians( kameraX ), glm::vec3( 0, 0, 1 ) );
    MV = glm::scale( MV, glm::vec3( scale[model], scale[model], scale[model] ) );
    MV = glm::translate( MV, glm::vec3(
        -( minX[model] + maxX[model] ) / 2,
        -( minY[model] + maxY[model] ) / 2,
        -( minZ[model] + maxZ[model] ) / 2 ) );

    glm::mat4 MVP = P * MV;

    GLuint MVP_id = glGetUniformLocation( programID, "MVP" );
    glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );

    // rysowanie osi
    glBindVertexArray( VAOaxes[model] );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glVertexAttrib1f( 2, 1 );
    glLineWidth( 2 );
    glDrawArrays( GL_LINES, 0, 18 );

    glLineStipple( 1, 0x1010 );
    glEnable( GL_LINE_STIPPLE );
    glLineWidth( 2 );

    for( int i = 0; i < 5; i++ ) {
        glm::mat4 MVPA = P * glm::translate( MV, glm::vec3( 0, ( maxY[model] - minY[model] ) / 4 * i, 0 ) );
        glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVPA[0][0] ) );
        glDrawArrays( GL_LINES, 0, 2 );

        MVPA = P * glm::translate( MV, glm::vec3( 0, ( maxY[model] - minY[model] ), ( maxZ[model] - minZ[model] ) / 4 * i ) );
        glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVPA[0][0] ) );
        glDrawArrays( GL_LINES, 0, 2 );

        MVPA = P * glm::translate( MV, glm::vec3( ( maxX[model] - minX[model] ) / 4 * i, 0, 0 ) );
        glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVPA[0][0] ) );
        glDrawArrays( GL_LINES, 2, 2 );

        MVPA = P * glm::translate( MV, glm::vec3( maxX[model] - minX[model], 0, ( maxZ[model] - minZ[model] ) / 4 * i ) );
        glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVPA[0][0] ) );
        glDrawArrays( GL_LINES, 2, 2 );

        MVPA = P * glm::translate( MV, glm::vec3( ( maxX[model] - minX[model] ) / 4 * i, 0, 0 ) );
        glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVPA[0][0] ) );
        glDrawArrays( GL_LINES, 4, 2 );

        MVPA = P * glm::translate( MV, glm::vec3( maxX[model] - minX[model], -( maxY[model] - minY[model] ) / 4 * i, 0 ) );
        glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVPA[0][0] ) );
        glDrawArrays( GL_LINES, 4, 2 );
    }

    // rysowanie obiektu
    glDisable( GL_LINE_STIPPLE );
    glUniformMatrix4fv( MVP_id, 1, GL_FALSE, &( MVP[0][0] ) );
    glBindVertexArray( VAOobj[model] );
    glDrawArrays( GL_TRIANGLES, 0, nElemObj[model] * 9 );

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glVertexAttrib1f( 2, 0 );
    glDrawArrays( GL_TRIANGLES, 0, nElemObj[model] * 9 );

    glutSwapBuffers();
}

void rozmiar( int width, int height )
{
    screen_width = width;
    screen_height = height;
    glViewport( 0, 0, screen_width, screen_height );

    P = glm::perspective( glm::radians( 60.0f ), (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f );

    glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

int main( int argc, char** argv )
{
    system( "dir models /b > models.txt" );
    FILE* fModels = NULL;
    if( fopen_s( &fModels, "models.txt", "r" ) ) return -1;

    char buffer[256];
    while( fscanf_s( fModels, "%s", buffer, 256 ) > 0 ) nFiles++;
    fseek( fModels, 0, SEEK_SET );

    fileNames = (char**)calloc( nFiles, sizeof( char* ) );

    for( int i = 0; i < nFiles; i++ ) {
        fileNames[i] = (char*)calloc( 256, sizeof( char ) );
        strcpy_s( fileNames[i], 256, "models\\" );
        fscanf_s( fModels, "%s", fileNames[i] + 7, 256 - 7 );
    }

    fclose( fModels );
    system( "del models.txt" );

    vertObj = (float**)calloc( nFiles, sizeof( float* ) );
    nVertObj = (GLuint*)calloc( nFiles, sizeof( GLuint ) );
    elemObj = (GLuint**)calloc( nFiles, sizeof( GLuint* ) );
    nElemObj = (GLuint*)calloc( nFiles, sizeof( GLuint ) );

    wallCol = (float**)calloc( nFiles, sizeof( float* ) );
    vertAll = (float**)calloc( nFiles, sizeof( float* ) );

    maxX = (float*)calloc( nFiles, sizeof( float ) );
    maxY = (float*)calloc( nFiles, sizeof( float ) );
    maxZ = (float*)calloc( nFiles, sizeof( float ) );
    minX = (float*)calloc( nFiles, sizeof( float ) );
    minY = (float*)calloc( nFiles, sizeof( float ) );
    minZ = (float*)calloc( nFiles, sizeof( float ) );
    scale = (float*)calloc( nFiles, sizeof( float ) );
    axesCoords = (float**)calloc( nFiles, sizeof( float* ) );

    for( int nF = 0; nF < nFiles; nF++ ) {
        readVertexInfo( fileNames[nF], vertObj + nF, nVertObj + nF, elemObj + nF, nElemObj + nF );
        maxX[nF] = maxY[nF] = maxZ[nF] = -1e8;
        minX[nF] = minY[nF] = minZ[nF] = 1e8;

        for( int i = 0; i < nVertObj[nF]; i++ ) {
            if( vertObj[nF][i * 3] > maxX[nF] ) maxX[nF] = vertObj[nF][i * 3];
            else if( vertObj[nF][i * 3] < minX[nF] ) minX[nF] = vertObj[nF][i * 3];
            if( vertObj[nF][i * 3 + 1] > maxY[nF] ) maxY[nF] = vertObj[nF][i * 3 + 1];
            else if( vertObj[nF][i * 3 + 1] < minY[nF] ) minY[nF] = vertObj[nF][i * 3 + 1];
            if( vertObj[nF][i * 3 + 2] > maxZ[nF] ) maxZ[nF] = vertObj[nF][i * 3 + 2];
            else if( vertObj[nF][i * 3 + 2] < minZ[nF] ) minZ[nF] = vertObj[nF][i * 3 + 2];
        }

        if( maxX[nF] - minX[nF] > scale[nF] ) scale[nF] = maxX[nF] - minX[nF];
        if( maxY[nF] - minY[nF] > scale[nF] ) scale[nF] = maxY[nF] - minY[nF];
        if( maxZ[nF] - minZ[nF] > scale[nF] ) scale[nF] = maxZ[nF] - minZ[nF];
        scale[nF] = 10 / scale[nF];

        wallCol[nF] = (float*)calloc( 3 * nElemObj[nF], sizeof( float ) );
        for( int i = 0; i < nElemObj[nF]; i++ ) {
            wallCol[nF][i * 3] = wallCol[nF][i * 3 + 1] = wallCol[nF][i * 3 + 2] =
                ( ( vertObj[nF][elemObj[nF][i * 3] * 3 + 2] + 
                    vertObj[nF][elemObj[nF][i * 3 + 1] * 3 + 2] + 
                    vertObj[nF][elemObj[nF][i * 3 + 2] * 3 + 2] ) / 3 - minZ[nF] ) / ( maxZ[nF] - minZ[nF] );
        }

        vertAll[nF] = (float*)calloc( 9 * nElemObj[nF], sizeof( float ) );
        for( int i = 0; i < 3 * nElemObj[nF]; i++ ) {
            vertAll[nF][i * 3] = vertObj[nF][elemObj[nF][i] * 3];
            vertAll[nF][i * 3 + 1] = vertObj[nF][elemObj[nF][i] * 3 + 1];
            vertAll[nF][i * 3 + 2] = vertObj[nF][elemObj[nF][i] * 3 + 2];
        }

        axesCoords[nF] = (float*)calloc( 18, sizeof( float ) );
        axesCoords[nF][0] = minX[nF];
        axesCoords[nF][1] = minY[nF];
        axesCoords[nF][2] = minZ[nF];
        axesCoords[nF][3] = maxX[nF];
        axesCoords[nF][4] = minY[nF];
        axesCoords[nF][5] = minZ[nF];
        axesCoords[nF][6] = minX[nF];
        axesCoords[nF][7] = minY[nF];
        axesCoords[nF][8] = minZ[nF];
        axesCoords[nF][9] = minX[nF];
        axesCoords[nF][10] = maxY[nF];
        axesCoords[nF][11] = minZ[nF];
        axesCoords[nF][12] = minX[nF];
        axesCoords[nF][13] = maxY[nF];
        axesCoords[nF][14] = minZ[nF];
        axesCoords[nF][15] = minX[nF];
        axesCoords[nF][16] = maxY[nF];
        axesCoords[nF][17] = maxZ[nF];
    }

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( screen_width, screen_height );
    glutInitWindowPosition( 0, 0 );
    glutCreateWindow( "Przeglądarka plików graficznych w formacie *.ply" );
    glewInit();
    glEnable( GL_DEPTH_TEST );

    glutDisplayFunc( rysuj );			// def. funkcji rysującej
    glutIdleFunc( idle );				// def. funkcji rysującej w czasie wolnym procesora
    glutReshapeFunc( rozmiar );		    // def. obsługi zdarzenia resize (GLUT)
    glutSpecialFunc( klawisz );		    // def. obsługi klawiszy specjalnych
    glutMouseFunc( mysz ); 			    // def. obsługi zdarzenia przycisku myszy (GLUT)
    glutMotionFunc( mysz_ruch );		// def. obsługi zdarzenia ruchu myszy (GLUT)

    programID = loadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    VBOvert = (GLuint*)calloc( nFiles, sizeof( GLuint ) );
    VBOaxes = (GLuint*)calloc( nFiles, sizeof( GLuint ) );
    VBOcol = (GLuint*)calloc( nFiles, sizeof( GLuint ) );
    VAOobj = (GLuint*)calloc( nFiles, sizeof( GLuint ) );
    VAOaxes = (GLuint*)calloc( nFiles, sizeof( GLuint ) );

    glGenVertexArrays( nFiles, VAOobj );
    glGenVertexArrays( nFiles, VAOaxes );
    glGenBuffers( nFiles, VBOvert );
    glGenBuffers( nFiles, VBOaxes );
    glGenBuffers( nFiles, VBOcol );

    for( int nF = 0; nF < nFiles; nF++ ) {
        glBindVertexArray( VAOobj[nF] );

        glBindBuffer( GL_ARRAY_BUFFER, VBOvert[nF] );
        glBufferData( GL_ARRAY_BUFFER, 9 * nElemObj[nF] * sizeof( float ), vertAll[nF], GL_STATIC_DRAW );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

        glBindBuffer( GL_ARRAY_BUFFER, VBOcol[nF] );
        glBufferData( GL_ARRAY_BUFFER, 3 * nElemObj[nF] * sizeof( float ), wallCol[nF], GL_STATIC_DRAW );
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 1, GL_FLOAT, GL_FALSE, 0, (void*)0 );

        glBindVertexArray( VAOaxes[nF] );
        glBindBuffer( GL_ARRAY_BUFFER, VBOaxes[nF] );
        glBufferData( GL_ARRAY_BUFFER, 18 * sizeof( float ), axesCoords[nF], GL_STATIC_DRAW );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    }

    glutMainLoop();

    glDeleteBuffers( nFiles, VBOvert );
    glDeleteBuffers( nFiles, VBOaxes );
    glDeleteBuffers( nFiles, VBOcol );
    glDeleteBuffers( nFiles, VAOobj );
    glDeleteBuffers( nFiles, VAOaxes );
    free( VBOvert );
    free( VBOaxes );
    free( VBOcol );
    free( VAOobj );
    free( VAOaxes );
    for( int nF = 0; nF < nFiles; nF++ ) {
        free( fileNames[nF] );
        free( vertAll[nF] );
        free( elemObj[nF] );
        free( wallCol[nF] );
        free( vertAll[nF] );
        free( axesCoords[nF] );
    }
    free( fileNames );
    free( vertObj );
    free( nVertObj );
    free( elemObj );
    free( nElemObj );
    free( wallCol );
    free( vertAll );
    free( maxX );
    free( maxY );
    free( maxZ );
    free( minX );
    free( minY );
    free( minZ );
    free( scale );
    free( axesCoords );

    return( 0 );
}

void readVertexInfo( const char* filename, float** vertArray, GLuint* nVert, GLuint** elemArray, GLuint* nElem )
{
    FILE* file = NULL;
    //printf( "%s ", filename );
    if( fopen_s( &file, filename, "r" ) ) return;

    /* NAGŁÓWEK */
    char header[256]{};
    while( strcmp( header, "vertex" ) )
        fscanf_s( file, "%s", header, 256 );

    // ilość współrzędnych
    fscanf_s( file, "%u", nVert );

    while( strcmp( header, "face" ) )
        fscanf_s( file, "%s", header, 256 );

    // ilość elementów
    fscanf_s( file, "%u", nElem );

    while( strcmp( header, "end_header" ) )
        fscanf_s( file, "%s", header, 256 );

    /* WSPÓŁRZĘDNE */
    // alokacja pamięci
    *vertArray = (float*)calloc( *nVert * 3, sizeof( float ) );
    // współrzędne
    for( int i = 0; i < *nVert * 3; i++ ) {
        fscanf_s( file, "%f", *vertArray + i );
    }

    int buffer;
    /* ŚCIANY */
    *elemArray = (GLuint*)calloc( *nElem * 3, sizeof( GLuint ) );
    for( int i = 0, j = 0; i < *nElem * 3; j++ ) {
        if( j % 4 == 0 )
            fscanf_s( file, "%u", &buffer );
        else
            fscanf_s( file, "%u", *elemArray + i++ );
    }

    fclose( file );
}