#ifndef RUBIKSCUBE_H
#define RUBIKSCUBE_H

#include "cube.hpp"

#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QBasicTimer>
#include <QQuaternion>
#include <QMouseEvent>
#include <QElapsedTimer>
#include <QLabel>
#include <QPushButton>
#include <stack>
#include <vector>
#include <thread>

#define PROJECTION_Z_NEAR 7.0
#define PROJECTION_Z_FAR 12.0
#define PROJECTION_FOV 45.0

#define VIEWER_X 0
#define VIEWER_Y 0
#define VIEWER_Z -10

#define BACKGROUN_COLOR_R 0
#define BACKGROUN_COLOR_B 255
#define BACKGROUN_COLOR_G 255
#define BACKGROUN_COLOR_A 1

#define CUBE_COUNT 27

#define ROTATION_SPEED 50

struct Input {
    char key;
    bool modifier;
};

class RubiksCube : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

private:
    QOpenGLShaderProgram program;
    QBasicTimer timer;
    
    QOpenGLTexture *texture;
    Cube **cubes;
    
    QVector2D mousePosition;
    QVector3D rotationAxis;
    double angularSpeed;
    QQuaternion rotation;
    QMatrix4x4 projection;
    QMatrix4x4 viewer;
    
    ushort indices[27];
    char key;
    bool modifier;
    ushort count;
    QVector3D rotate;
    
    QElapsedTimer frameTime;
    
    QWidget *help;
    QPushButton *refreshButton;
    std::stack<Input, std::vector<Input>> stack;
    bool bRefresh;
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initShaders();
    void initTextures();
    
    void timerEvent(QTimerEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    
    void keyPressEvent(QKeyEvent *e) override;

    bool cull_index(ushort i);
    void updateIndices();
    
    void refreshThread();
    
protected slots:
    void refresh();

public:
    RubiksCube(QWidget *parent = nullptr);
    ~RubiksCube();
};

#endif // RUBIKSCUBE_H
