#ifndef OSGCAMERAFORM_H
#define OSGCAMERAFORM_H

#include <QWidget>

namespace Ui {
class OsgCameraForm;
}

class OsgCameraForm : public QWidget
{
    Q_OBJECT

public:
    explicit OsgCameraForm(QWidget *parent = 0);
    ~OsgCameraForm();
public slots:
    void updateFromCamera();

private:
    Ui::OsgCameraForm *ui;
};

#endif // OSGCAMERAFORM_H
