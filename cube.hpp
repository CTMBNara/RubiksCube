//
//  cube.hpp
//  RubiksCube
//
//  Created by Danil on 02.10.2020.
//

#ifndef cube_hpp
#define cube_hpp

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <cmath>

class Cube : protected QOpenGLFunctions
{
private:
    QVector3D center;
    QVector3D vertices[8];
    QOpenGLBuffer indicesBuffer;
    QOpenGLBuffer verticesBuffer;

    void initCube();

public:
    Cube(const QVector3D &v);
    
    void drawCube(QOpenGLShaderProgram* program, const QVector3D& rotation);

    ~Cube();
};

#endif /* cube_hpp */
