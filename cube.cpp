#include "cube.hpp"

#include <array>

QMatrix3x3 rotationMatrix(const QVector3D& rotation)
{
    QMatrix3x3 matrix;
    matrix.setToIdentity();
    
    QMatrix3x3 rotate;
    if (rotation.x() != 0)
    {
        rotate.setToIdentity();
        float value = cos(rotation.x());
        rotate(1, 1) = value;
        rotate(2, 2) = value;
        value = sin(rotation.x());
        rotate(2, 1) = value;
        rotate(1, 2) = -value;
        matrix = rotate * matrix;
    }
    if (rotation.y() != 0)
    {
        rotate.setToIdentity();
        float value = cos(rotation.y());
        rotate(0, 0) = value;
        rotate(2, 2) = value;
        value = sin(rotation.y());
        rotate(2, 0) = -value;
        rotate(0, 2) = value;
        matrix = rotate * matrix;
    }
    if (rotation.z() != 0)
    {
        rotate.setToIdentity();
        float value = cos(rotation.z());
        rotate(0, 0) = value;
        rotate(1, 1) = value;
        value = sin(rotation.z());
        rotate(1, 0) = value;
        rotate(0, 1) = -value;
        matrix = rotate * matrix;
    }
    
    return matrix;
}

QVector3D operator *(const QMatrix3x3 &matrix, const QVector3D &vector)
{
    QVector3D result;
    
    for (ushort i = 0; i < 3; i++)
        result[i] = vector[0] * matrix(i, 0) + vector[1] * matrix(i, 1) + vector[2] * matrix(i, 2);
    
    return result;
}

struct VertexData {
    QVector3D vertex;
    QVector2D texture;
};

Cube::Cube(const QVector3D &center) : center(center), indicesBuffer(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    verticesBuffer.create();
    indicesBuffer.create();

    vertices[0] = QVector3D(-1./2 + center.x(), -1./2 + center.y(),   1./2 + center.z());
    vertices[1] = QVector3D( 1./2 + center.x(), -1./2 + center.y(),   1./2 + center.z());
    vertices[2] = QVector3D(-1./2 + center.x(),  1./2 + center.y(),   1./2 + center.z());
    vertices[3] = QVector3D( 1./2 + center.x(),  1./2 + center.y(),   1./2 + center.z());
    vertices[4] = QVector3D( 1./2 + center.x(), -1./2 + center.y(),  -1./2 + center.z());
    vertices[5] = QVector3D( 1./2 + center.x(),  1./2 + center.y(),  -1./2 + center.z());
    vertices[6] = QVector3D(-1./2 + center.x(), -1./2 + center.y(),  -1./2 + center.z());
    vertices[7] = QVector3D(-1./2 + center.x(),  1./2 + center.y(),  -1./2 + center.z());
    
    initCube();
}

void Cube::initCube()
{
    VertexData data[] = {
        //Green
        { vertices[0], QVector2D(  0., 0.)},
        { vertices[1], QVector2D(1./6, 0.)},
        { vertices[2], QVector2D(  0., 1.)},
        { vertices[3], QVector2D(1./6, 1.)},

        //Red
        { vertices[1], QVector2D(1./6, 0.)},
        { vertices[4], QVector2D(2./6, 0.)},
        { vertices[3], QVector2D(1./6, 1.)},
        { vertices[5], QVector2D(2./6, 1.)},

        //Blue
        { vertices[4], QVector2D(2./6, 0.)},
        { vertices[6], QVector2D(3./6, 0.)},
        { vertices[5], QVector2D(2./6, 1.)},
        { vertices[7], QVector2D(3./6, 1.)},

        //Orange
        { vertices[6], QVector2D(3./6, 0.)},
        { vertices[0], QVector2D(4./6, 0.)},
        { vertices[7], QVector2D(3./6, 1.)},
        { vertices[2], QVector2D(4./6, 1.)},

        //White
        { vertices[6], QVector2D(4./6, 0.)},
        { vertices[4], QVector2D(5./6, 0.)},
        { vertices[0], QVector2D(4./6, 1.)},
        { vertices[1], QVector2D(5./6, 1.)},

        //Yelow
        { vertices[2], QVector2D(5./6, 0.)},
        { vertices[3], QVector2D(  1., 0.)},
        { vertices[7], QVector2D(5./6, 1.)},
        { vertices[5], QVector2D(  1., 1.)}
    };

    ushort indices[] = {
        0, 1, 2,     1, 3, 2,
        4, 5, 6,     5, 7, 6,
        8, 9, 10,    9, 11, 10,
        12, 13, 14,  13, 15, 14,
        16, 17, 18,  17, 19, 18,
        20, 21, 22,  21, 23, 22
    };

    verticesBuffer.bind();
    verticesBuffer.allocate(data, 24 * sizeof(VertexData));
    
    indicesBuffer.bind();
    indicesBuffer.allocate(indices, 36 * sizeof(ushort));
}

void Cube::drawCube(QOpenGLShaderProgram* program, const QVector3D& rotation)
{
    verticesBuffer.bind();
    indicesBuffer.bind();
    int offset = 0;
    
    if (!(rotation.x() == 0. && rotation.y() == 0. && rotation.z() == 0.))
    {
        QMatrix3x3 rotate = rotationMatrix(rotation);
        center = rotate * center;
        for (ushort i = 0; i < 8; i++)
            vertices[i] = rotate * vertices[i];
        
        std::array<ushort, 24> indices = {
            0, 1, 2, 3,
            1, 4, 3, 5,
            4, 6, 5, 7,
            6, 0, 7, 2,
            6, 4, 0, 1,
            2, 3, 7, 5
        };
        
        for (auto i : indices)
        {
            verticesBuffer.write(offset, &vertices[i], sizeof(QVector3D));
            offset += sizeof(VertexData);
        }
        
        offset = 0;
    }
    
    program->enableAttributeArray("a_position");
    program->setAttributeBuffer("a_position", GL_FLOAT, offset, 3, sizeof(VertexData));
    
    offset += sizeof(QVector3D);
    
    program->enableAttributeArray("a_texcoord");
    program->setAttributeBuffer("a_texcoord", GL_FLOAT, offset, 2, sizeof(VertexData));
    
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
}

Cube::~Cube()
{
    verticesBuffer.destroy();
    indicesBuffer.destroy();
}
