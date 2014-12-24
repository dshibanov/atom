#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QFont>
#include <QtGui>
#include <QPaintEvent>
#include <QFont>
#include <QListWidgetItem>

#include "basics.h"
#include "glyph.h"
#include "options.h"
#include "font.h"
#include "curve.h"
#include "blend.h"
#include "parser.h"

#include "composedglyph.h"


namespace Ui {
class MainWindow;
}


class Atom 
{
public:	
	Contour *contour;
	Matrix m;
	bool reverse;
	
	Atom(Contour &_contour, Matrix _m, bool _reverse = false):
		contour(&_contour),		
	  m(_m)	,
		reverse(_reverse)
	{	}	
	
	Atom(){}
};


typedef list<Atom> Atoms;

class AGlyph
{
public:		
	string name;
	int index;	
	Atoms atoms;
	
	AGlyph() 
	{
		index = -1;						
	}
	
	AGlyph(const Atoms &_atoms,string _name, int _index = -1) :
		atoms(_atoms),
		name(_name),
	  index(_index)	
	{					
	}
				
};

typedef list<AGlyph> AGlyphs;

class ContourGlyph
{
public:
  ContourGlyph() {}
  //fg::Points ContourGlyph::getControlPoints(int pointsCount);
//  double distanceToLine(fg::Point p, double a, double b = 0, double c = 0);
  string name;
  fg::Shape shape;
  fg::Shape auxiliaryShape;
};

typedef std::vector<ContourGlyph> ContourGlyphs;

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

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
	
	
private slots:
	void on_DecomposeBtn_clicked();
	
	void on_OneBtn_clicked();
	
	void on_TwoBtn_clicked();
	
	void on_glyphsList_itemClicked(QListWidgetItem *item);
	
	
	
	void on_atomsList_itemChanged(QListWidgetItem *item);
	
	void on_atomsList_itemClicked(QListWidgetItem *item);
	
	//void on_atomsList_currentRowChanged(int currentRow);
	
	void on_atomsList_currentItemChanged(QListWidgetItem *item, QListWidgetItem *previous);
	
private:
	Ui::MainWindow *ui;
	QFile* file;
	QFont font;
	io::Parser parser;
	fg::Package *package;
	QPainterPath globalPath;
	QPainterPath globalBBoxes;
	QPainterPath globalAtoms;
	QPainterPath redPath;
	Paths paths;
	int maxGraphemesCount;
	double global_scale;
	Contours dict;// dict of atoms
	AGlyphs agdict;// dict of aglyphs
	int len;
	
	
	
	// -------------------------
	void MainWindow::clear();
	int MainWindow::readFontFile(QString path, QString outPath);
	Contour MainWindow::contourToPoligone(Contour contour, int len);
	Contour MainWindow::splinesToContour(Splines splines);
	Splines MainWindow::contourToSplines(Contour contour, int len, Matrix m = Matrix(1,0,0,1), bool reverse = false);
	void MainWindow::fillList();
	void MainWindow::glyphToDraw(fg::Glyph &g);
	void addToDraw(Contour &c, const fg::Point &translation, int index);
	void addToDraw(fg::Contour &c, const fg::Matrix &m, int index);
	QPainterPath MainWindow::contourToPath(/*QPainterPath &path, */const fg::Contour &contour, const fg::Matrix &layerMatrix);
	void MainWindow::drawNodes(QPainterPath p);
	int MainWindow::isNewAtom(const Contour &atom, Contours &dict, Matrix &m);
	double MainWindow::compareSplines(const Splines &splines1, const Splines &splines2, int len);		
	void MainWindow::contourToDebug(const Contour &c) const;
	void MainWindow::coutRect(const string &header, const Rect &r);
	void MainWindow::coutMatrix(const string &header, const Matrix &m);
	void MainWindow::coutSplines(const Splines &s);
	// ---
	
	void paintEvent(QPaintEvent *e);
	
};

#endif // MAINWINDOW_H
