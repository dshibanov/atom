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
#include <sstream>

#include "composedglyph.h"


namespace Ui {
class MainWindow;
}


class Atom 
{
public:	
	Contour *contour;
	Matrix m;
	int reverse;
	int contourIndex;
	
	Atom(Contour &_contour, Matrix _m, int _reverse = -1, int _contourIndex = 0):
		contour(&_contour),		
	  m(_m)	,
		reverse(_reverse),
		contourIndex(_contourIndex)		
	{	}	
	
	Atom(){}
	
	void info(string header = " ")
	{
				
		//							<< " reverse: " << reverse;		
		//		qDebug()<<header.c_str() << m.m11<< " "<< m.m12<< " "<< m.m21<< " "<< m.m22<< " "<< m.dx<< " " << m.dy; 					
		//		string ctr = "ajsdhjad";
		
		std::stringstream s_result;
		for (Nodes::iterator it = contour->nodes.begin(); it != contour->nodes.end(); ++it) 
		{
			s_result << " ["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")];";
		}
		
		qDebug()<<header.c_str() <<"#"<< contourIndex<< " m: "<< m.m11<< " "<< m.m12<< " "<< m.m21<< " "<< m.m22<< " "<< m.dx<< " " << m.dy << s_result.str().c_str();				
	}
};


typedef list<Atom> Atoms;
typedef list<Atoms> AContours;

class AGlyph
{
public:		
	string name;
	int index;	
	AContours contours;
	
	AGlyph() 
	{
		index = -1;						
	}
	
	AGlyph(const AContours &_contours,string _name, int _index = -1) :
		contours(_contours),
		name(_name),
	  index(_index)
	{
		
//		qDebug()<<" ------- init ---------  " ;												
//		for (AContours::iterator it = contours.begin(); it != contours.end(); ++it) 
//		{
//			for (Atoms::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2) 
//			{
//				qDebug()<<" nodes.size() " << (*it2).contour->nodes.size();												
//			}					
//		}
		
		
	}
	
	int getContours(Contours &c)
	{
		if(contours.empty())
			return 0;
		
		for (AContours::iterator it = contours.begin(); it != contours.end(); ++it) 
		{
			Contour contour;
			// getting contour from atoms
			for (Atoms::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
			{
				// connecting atoms to contour				
				Contour atom = (*(*it2).contour);
				//	qDebug()<<"atom nodes size: " << atom.nodes.size() ;
				//	qDebug()<<" atom #"<<(*it2).contourIndex;
				//	qDebug()<<" nodes.size: " << (*it2).contour->nodes.size();
				
//				qDebug()<<" before: " <<ctrToString(atom).str().c_str();
				atom.transform((*it2).m);
				
				coutMatrix("m: ", (*it2).m);
				if((*it2).reverse)
					atom.reverse();				
				//	qDebug()<<" after: " <<ctrToString(atom).str().c_str();				
				//	qDebug()<<"atom nodes size: " << atom.nodes.size();
								
				for (Nodes::iterator ni = atom.nodes.begin(); ni != atom.nodes.end(); ++ni) 
				{
					Node n = (*ni); 
					contour.addNode(n.kind, n.p, false);					
				}
			}
			c.push_back(contour);
		}
		
		return 1;
	}			
	
	void coutMatrix(const string &header, const Matrix &m)
	{
		qDebug()<<header.c_str() << m.m11<< " "<< m.m12<< " "<< m.m21<< " "<< m.m22<< " "<< m.dx<< " " << m.dy; 			
	}

	std::stringstream ctrToString(const Contour &contour)
	{	
		std::stringstream s_result;
		for (Nodes::const_iterator it = contour.nodes.begin(); it != contour.nodes.end(); ++it) 
		{
			s_result << " ["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")];";
		}
		
		s_result<<"\n";		
	 return	s_result;
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
	int MainWindow::checkNewAtom(const Contour &atom, Contours &dict, Matrix &m, int &reverse);
	double MainWindow::compareSplines(const Splines &splines1, const Splines &splines2, int len);		
	void MainWindow::contourToDebug(const Contour &c) const;
	void MainWindow::coutRect(const string &header, const Rect &r);
	void MainWindow::coutMatrix(const string &header, const Matrix &m);
	void MainWindow::coutSplines(const Splines &s);
	std::stringstream MainWindow:: ctrToString(const Contour &contour);
	// ---
	
	void paintEvent(QPaintEvent *e);
	
};

#endif // MAINWINDOW_H
