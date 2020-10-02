#include "rubikscube.h"

#define QUICK_HELP  "Quick help:\n\n" \
                    "To rotate different parts\n" \
                    "of the rubik's cube, use\n" \
                    "the following keys:\n" \
                    "   Q\tW\tE\n" \
                    "   A\tS\tD\n" \
                    "   Z\tX\tC\n" \
                    "Use the Shift+Key\n" \
                    "to reverse rotation."

RubiksCube::RubiksCube(QWidget *parent):
QOpenGLWidget(parent), angularSpeed(0), modifier(false), count(0), bRefresh(false)
{
    setMinimumSize(640, 480);
    
    help = new QWidget(this);
    QLabel *_help = new QLabel("\u003F\xe2\x83\x9d", help);
    _help->setStyleSheet("font-size: 35pt; color: black");
    _help->setFixedSize(42, 42);
    _help->setAlignment(Qt::AlignCenter);
    help->setMouseTracking(true);
    help->setToolTip(QUICK_HELP);
    help->setStyleSheet("font-size: 16pt;");
    
    refreshButton = new QPushButton("\xe2\x9f\xb2", this);
    refreshButton->setStyleSheet("font-size: 35pt; color: black; border: none;");
    refreshButton->setFixedSize(60, 42);
    RubiksCube::connect(refreshButton, SIGNAL(clicked()), this, SLOT(refresh()));
}

void RubiksCube::initializeGL()
{
    setMinimumSize(200, 200);
    
    initializeOpenGLFunctions();

    glClearColor(BACKGROUN_COLOR_R, BACKGROUN_COLOR_B, BACKGROUN_COLOR_G, BACKGROUN_COLOR_A);

    initShaders();
    initTextures();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    cubes = new Cube*[CUBE_COUNT];

    for (ushort indx = 0; indx < CUBE_COUNT; indx++)
    {
        if (indx != 13)
        {
            int i = indx;
            float z = i / 9 - 1; i %= 9;
            float y = i / 3 - 1; i %= 3;
            float x = i - 1;
            cubes[indx] = new Cube({x, y, z});
        }
        indices[indx] = indx;
    }
    
    timer.start(10, this);
}

void RubiksCube::resizeGL(int w, int h)
{
    double aspect = (double) w / (h ? h : 1);

    projection.setToIdentity();
    projection.perspective(PROJECTION_FOV, aspect, PROJECTION_Z_NEAR, PROJECTION_Z_FAR);
    
    help->move(w - 51, 0);
    refreshButton->move(w - 60, 42);
}

void RubiksCube::paintGL()
{
    this->setWindowTitle("FPS is " + QString::number((ushort) (1E9 / (frameTime.nsecsElapsed()))));
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    texture->bind();

    viewer.setToIdentity();
    viewer.translate(VIEWER_X, VIEWER_Y, VIEWER_Z);
    viewer.rotate(rotation);

    program.setUniformValue("mvp_matrix", projection * viewer);
    program.setUniformValue("texture", 0);
    
    if (!count)
    {
        
        for (ushort i = 0; i < CUBE_COUNT; i++)
            if (i != 13)
                cubes[i]->drawCube(&program, {0, 0, 0});
    } else {
        for (ushort i = 0; i < CUBE_COUNT; i++)
            if (i != 13 && (!cull_index(i)))    // (% 3), (>9, ...), (% 9 > 2, ...)
                cubes[indices[i]]->drawCube(&program, {0., 0., 0.});
            else if (i != 13)
                cubes[indices[i]]->drawCube(&program, modifier ? -rotate : rotate);
        count--;
        if (count == 0)
        {
            updateIndices();
            key = ' ';
            modifier = false;
        }
    }
    
    frameTime.restart();
}

void RubiksCube::initShaders()
{
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();
    
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    if (!program.link())
        close();

    if (!program.bind())
        close();
}

void RubiksCube::initTextures()
{
    texture = new QOpenGLTexture(QImage(":/cube_black.png"));
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
}

void RubiksCube::mousePressEvent(QMouseEvent *e)
{
    mousePosition = QVector2D(e->localPos());
}

void RubiksCube::mouseMoveEvent(QMouseEvent *e)
{
    QVector2D diff = QVector2D(e->localPos()) - mousePosition;
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
       
    qreal acc = diff.length() / 2;
    
    rotationAxis = (n * acc).normalized();
    angularSpeed = acc;
    
    mousePosition += diff;
}

void RubiksCube::mouseReleaseEvent(QMouseEvent *)
{
    angularSpeed = 0;
}

char checkKey(int key) {
    char k = ' ';
    
    switch (key) {
        case Qt::Key_Q:
            k = 'q';
            break;
        case Qt::Key_W:
            k = 'w';
            break;
        case Qt::Key_E:
            k = 'e';
            break;
        case Qt::Key_A:
            k = 'a';
            break;
        case Qt::Key_S:
            k = 's';
            break;
        case Qt::Key_D:
            k = 'd';
            break;
        case Qt::Key_Z:
            k = 'z';
            break;
        case Qt::Key_X:
            k = 'x';
            break;
        case Qt::Key_C:
            k = 'c';
            break;
    }
    
    return k;
}

void RubiksCube::keyPressEvent(QKeyEvent *e)
{
    if (!bRefresh && count == 0)
    {
        key = checkKey(e->key());
        
        if (key != ' ')
        {
            if (e->modifiers() == Qt::ShiftModifier)
                modifier = true;
            
            count += ROTATION_SPEED / 2;
            stack.push({key, modifier});
        }
    }
}

void RubiksCube::timerEvent(QTimerEvent *)
{
    rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
    update();
}

bool RubiksCube::cull_index(ushort index)
{
    switch (key) {
        case 'q':
            if (index % 3 == 0)
            {
                rotate = {M_PI/ROTATION_SPEED, 0., 0.};
                return true;
            }
            break;
        case 'w':
            if (index % 3 == 1)
            {
                rotate = {M_PI/ROTATION_SPEED, 0., 0.};
                return true;
            }
            break;
        case 'e':
            if (index % 3 == 2)
            {
                rotate = {M_PI/ROTATION_SPEED, 0., 0.};
                return true;
            }
            break;
        case 'a':
            if (index < 9)
            {
                rotate = {0., 0., M_PI/ROTATION_SPEED};
                return true;
            }
            break;
        case 's':
            if (index >= 9 && index < 18)
            {
                rotate = {0., 0., M_PI/ROTATION_SPEED};
                return true;
            }
            break;
        case 'd':
            if (index >= 18)
            {
                rotate = {0., 0., M_PI/ROTATION_SPEED};
                return true;
            }
            break;
        case 'z':
            if (index % 9 < 3)
            {
                rotate = {0., -M_PI/ROTATION_SPEED, 0.};
                return true;
            }
            break;
        case 'x':
            if (index % 9 >= 3 && index % 9 < 6)
            {
                rotate = {0., -M_PI/ROTATION_SPEED, 0.};
                return true;
            }
            break;
        case 'c':
            if (index % 9 >= 6)
            {
                rotate = {0., -M_PI/ROTATION_SPEED, 0.};
                return true;
            }
            break;
    }
    
    return false;
}

void RubiksCube::updateIndices()
{
    ushort _indices[9];
    
    ushort k = 0;
    for (ushort i = 0; i < 27; i ++)
        switch (key) {
            case 'q':
                if (i % 3 == 0) _indices[k++] = indices[i];
                break;
            case 'w':
                if (i % 3 == 1) _indices[k++] = indices[i];
                break;
            case 'e':
                if (i % 3 == 2) _indices[k++] = indices[i];
                break;
            case 'a':
                if (i < 9) _indices[k++] = indices[i];
                break;
            case 's':
                if (i >= 9 && i < 18) _indices[k++] = indices[i];
                break;
            case 'd':
                if (i >= 18) _indices[k++] = indices[i];
                break;
            case 'z':
                if (i % 9 < 3) _indices[k++] = indices[i];
                break;
            case 'x':
                if (i % 9 >= 3 && i % 9 < 6) _indices[k++] = indices[i];
                break;
            case 'c':
                if (i % 9 >= 6) _indices[k++] = indices[i];
                break;
        }
    
    for (k = 0; k < 2; k++)
    {
        if (!modifier)
        {
            std::swap(_indices[3], _indices[0]);
            std::swap(_indices[6], _indices[3]);
            std::swap(_indices[7], _indices[6]);
            std::swap(_indices[8], _indices[7]);
            std::swap(_indices[5], _indices[8]);
            std::swap(_indices[2], _indices[5]);
            std::swap(_indices[1], _indices[2]);
        } else {
            std::swap(_indices[1], _indices[0]);
            std::swap(_indices[2], _indices[1]);
            std::swap(_indices[5], _indices[2]);
            std::swap(_indices[8], _indices[5]);
            std::swap(_indices[7], _indices[8]);
            std::swap(_indices[6], _indices[7]);
            std::swap(_indices[3], _indices[6]);
        }
    }
    
    k = 0;
    for (ushort i = 0; i < 27; i ++)
        switch (key) {
            case 'q':
                if (i % 3 == 0) indices[i] = _indices[k++];
                break;
            case 'w':
                if (i % 3 == 1) indices[i] = _indices[k++];
                break;
            case 'e':
                if (i % 3 == 2) indices[i] = _indices[k++];
                break;
            case 'a':
                if (i < 9) indices[i] = _indices[k++];
                break;
            case 's':
                if (i >= 9 && i < 18) indices[i] = _indices[k++];
                break;
            case 'd':
                if (i >= 18) indices[i] = _indices[k++];
                break;
            case 'z':
                if (i % 9 < 3) indices[i] = _indices[k++];
                break;
            case 'x':
                if (i % 9 >= 3 && i % 9 < 6) indices[i] = _indices[k++];
                break;
            case 'c':
                if (i % 9 >= 6) indices[i] = _indices[k++];
                break;
        }
}

void RubiksCube::refreshThread()
{
    bRefresh = true;
    
    while (!stack.empty())
    {
        if (count == 0)
        {
            auto input = stack.top();
            stack.pop();
            key = input.key;
            modifier = !input.modifier;
            count += ROTATION_SPEED / 2;
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(frameTime.elapsed() + 1000000));
    }
    
    bRefresh = false;
}

void RubiksCube::refresh()
{
    if (!bRefresh)
    {
        std::thread t(&RubiksCube::refreshThread, this);
        t.detach();
    }
}

RubiksCube::~RubiksCube()
{
    makeCurrent();
    delete help;
    delete refreshButton;
    delete texture;
    for (ushort i = 0; i < CUBE_COUNT; i++)
        if (i != 13)
            delete cubes[i];
    delete cubes;
    doneCurrent();
}
