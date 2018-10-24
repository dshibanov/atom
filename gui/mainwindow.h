#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QFont>
#include <QtGui>
#include <QPaintEvent>
#include <QFont>
#include <QListWidgetItem>
#include <QFileDialog>

//#include "basics.h"
//#include "glyph.h"
//#include "options.h"
//#include "font.h"
//#include "curve.h"



//#include "blend.h"
//#include "parser.h"


#include "utils.h"

#include <sstream>



using namespace fg;



namespace Ui {
class MainWindow;
}

typedef vector<int> Ints;

struct Path2Draw
{
  int index;
  QPainterPath path;

  Path2Draw()
  {
    index = -1;
  }

  Path2Draw(int _in, const QPainterPath &_path)
  {
    index = _in;
    path = _path;
  }
};

typedef QList<Path2Draw> Paths;



// тут делаем atomDrawer

class AtomDrawer
{
public:

    Point translation;
    fg::Matrix mtx;//(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);

    AtomDrawer()
    {}

    AtomDrawer(Point translation, fg::Matrix mtx)
    {
        this->translation = translation;
        this->mtx = mtx;
    }

    void drawAtomIntoPath(Atom &atom, QPainterPath &path)
    {
        // потом рассчитать матрицу и получить path
        //fg::Matrix mtx(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);
        QPainterPath p = contourToPath(atom, mtx).translated(QPointF(translation.x, translation.y));
        p.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 2,2);
        // добавить атомы в путь
        path.addPath(p);
    }

    QPainterPath contourToPath(const fg::Contour &contour, const fg::Matrix &layerMatrix)
    {

      QPainterPath path;

      fg::_unused(layerMatrix);

      bool open = false;
      //fg::Matrix shapeMatrix = shape.mtx * layerMatrix;
      //if (contour.nodes.size() < 2)
      //continue;

      if (contour.open)
        open = true;

      int curveCount = 0;
      bool spline = false;
      QPointF curve1, curve2, pathStart;

      for (fg::Nodes::const_iterator node = contour.nodes.begin(); node != contour.nodes.end(); ++node){
        fg::Point nodePoint = node->p.transformed(layerMatrix);
        fg::Point point = nodePoint;

        QPointF p(point.x, point.y);


        if (node == contour.nodes.begin()){
          // начало контура
          (&path)->moveTo(p);
          pathStart = p;
        }
        else{
          switch (node->kind){
          case fg::Node::Move:
            if (!contour.open){
              if (curveCount == 1){
                if (spline)
                  path.quadTo(curve1, pathStart);
                else
                  path.cubicTo(curve1, pathStart, pathStart);
              }
              else if (curveCount >= 2){
                path.cubicTo(curve1, curve2, pathStart);
              }
            }

            path.moveTo(p);
            spline = false;
            curveCount = 0;

            break;

          case fg::Node::On:
            if (curveCount == 0)
              path.lineTo(p);
            else if (curveCount == 1){
              if (spline)
                path.quadTo(curve1, p);
              else
                path.cubicTo(curve1, p, p);
            }
            else
              path.cubicTo(curve1, curve2, p);

            spline = false;
            curveCount = 0;
            break;

          case fg::Node::Off:
            if (spline)
              path.quadTo(curve1.x(), curve1.y(), (curve1.x() + p.x()) / 2.0, (curve1.y() + p.y()) / 2.0);
            spline = true;
            curve1 = p;
            curveCount = 1;
            break;

          case fg::Node::Curve:
            if (curveCount == 0){
              curve1 = p;
              curveCount = 1;
            }
            else{
              curve2 = p;
              curveCount = 2;
            }
            break;
          }
        }
      }


      if (!contour.open){
        if (curveCount == 1){
          if (spline)
            path.quadTo(curve1, pathStart);
          else
            path.cubicTo(curve1, pathStart, pathStart);
        }
        else if (curveCount >= 2){
          path.cubicTo(curve1, curve2, pathStart);
        }
        else
          path.closeSubpath();
      }



      return path;
    }


};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_RunButton_clicked();

    void on_atomsList_currentItemChanged(QListWidgetItem *item);
    void on_glyphsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_statAtomsList_currentItemChanged(QListWidgetItem *item, QListWidgetItem *previous);


private:
    Ui::MainWindow *ui;
    QFile* file;
    QFont font;
    io::Parser parser;
    fg::Package *package;
    bool drawDecomposedGlyph;
    XGlyph *drawedGlyphPointer;
    double toleranceEndPoints;

    int maxGraphemesCount; // ?

    double global_scale;
    Atoms dict;// dict of atoms
    Atoms statDict;// dict of atoms
    XGlyphs aGlyphs;// dict of aglyphs
    list<Representations> rdict;

    vector<Ints> usedIn;

    QPainterPath globalPath;
    QPainterPath globalBBoxes;
    QPainterPath globalAtoms;
    QPainterPath redPath;
    Paths paths;

    list<Atom*> statsLinks;


    void paintEvent(QPaintEvent *e);
    Point getAtomTranslation(Atom &atom);
    void toLog(string message);

    int setUpAtomLib(AtomLib &atomlib);
    void refreshAtomsList(const Atoms &dict, QListWidget &qlist);
    void refreshGlyphsList(const XGlyphs &dict, QListWidget &qlist);
    int readFontFile(QString path, QString outPath);
    void clear();
    Point getTopRightPointOfGlyph(const Glyph &g);
    void contoursToDraw(Contours cs, Point tr, QPainterPath &path);
    void drawNodes(QPainterPath p, Point shift = Point(0,0));

    void fillStatDict();
};





#endif // MAINWINDOW_H
