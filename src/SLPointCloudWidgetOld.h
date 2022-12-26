#ifndef SLPOINTCLOUDWIDGET_H
#define SLPOINTCLOUDWIDGET_H

#include <QtGui>
#include <QOpenGLWidget>
#include <opencv2/opencv.hpp>

class SLPointCloudWidget : public QOpenGLWidget {
    Q_OBJECT
    public:
        SLPointCloudWidget(QWidget * parent = 0) : QOpenGLWidget(parent), rotationX(0), rotationY(0), rotationZ(0){}
    public slots:
        void updatePointCloud(cv::Mat _pointCloud, cv::Mat _shading);
    protected:
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent*);
    private:
        cv::Mat pointCloud;
        cv::Mat shading;
        QPoint lastMousePos;
        float rotationX, rotationY, rotationZ;
    signals:
        void newPointCloudDisplayed();
};

#endif // SLPOINTCLOU_H

