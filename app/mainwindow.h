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


//class UnpAtom :public Contour
//{
	
	
	
//}


class Atom 
{
public:	
	Contour *contour;
	Matrix m;
	int reverse;
	int contourIndex;
	bool straightLine;
	vector<int> usedIn;
	
	Atom(Contour &_contour, Matrix _m, int _reverse = -1, int _contourIndex = 0, bool _straightLine = false):
		contour(&_contour),		
	  m(_m)	,
		reverse(_reverse),
		contourIndex(_contourIndex), 
		straightLine(_straightLine)		
	{	
	
//		qDebug()<<"dx dy " << m.dx << " " << m.dy;
	
	}	
	
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
	
	int getContours(Contours &c, Contour *useOnly = NULL)
	{
		if(contours.empty())
			return 0;
				
		// anyway we need to remove zeros from constructed contour !!!
		// it looks we need special rule for remove zeros
		
		for (AContours::iterator it = contours.begin(); it != contours.end(); ++it) 
		{
			
			Contour contour;
			Contour contour2;
			int n = 0;
			
			// getting contour from atoms
			for (Atoms::iterator it2 = (*it).begin(); it2 != (*it).end() && n < 500; ++it2, ++n)
			{
				// connecting atoms to contour				
				std::stringstream ss;
				Contour atom = (*(*it2).contour);				
				
//				ss<<"#" << n << " aa.contourIndex " << (*it2).contourIndex << " m11: " << (*it2).m.m11 << " m22: " << (*it2).m.m22 << " r: " << (*it2).reverse; 
//				qDebug()<<ss.str().c_str();
											
				if((*it2).reverse)
				{
					atom.reverse();									
				}
				
						
				atom.transform((*it2).m);
							
				
				for (Nodes::iterator ni = atom.nodes.begin(); ni != atom.nodes.end(); ++ni) 
				{
					std::stringstream s_result;
					Node &n = (*ni); 					
					if(useOnly == NULL)
					{
						if(contour.empty() || n.kind != Node::Move)
						{
							contour.addNode(n.kind, n.p, false);			
							s_result << "	["<< n.kind << " : (" << n.p.x << ", " << n.p.y << ")];   | ";						
						}
						else
							s_result << "	";
						
						contour2.addNode(n.kind, n.p, false);										
						//						s_result << "	["<< n.kind << " : (" << n.p.x << ", " << n.p.y << ")]; \n";										
						//					qDebug()<<s_result.str().c_str();												
					}
					else if((*it2).contour == useOnly)
					{						
						contour.addNode(n.kind, n.p, false);															
					}
						
				}
			}
//			qDebug()<<" * " << ctrToString(contour).str().c_str();
//			contour.open = true;
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
			if((*it).kind == 0)
				s_result << "		["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")]; \n";
			else
				s_result << " ["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")]; \n";
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
	vector<Ints> usedIn;
	AGlyph *drawedGlyphIndex;
	
		
	Contour c1;
	Contour c2;
	Contour cc;
	int globalCtr;
	double globAngle;
				
	// -------------------------
	void MainWindow::clear();
	int MainWindow::readFontFile(QString path, QString outPath);
	Contour MainWindow::contourToPoligone(Contour contour, int len);
	Contour MainWindow::splinesToContour(Splines splines);
	Splines MainWindow::contourToSplines(Contour contour, int len, Matrix m = Matrix(1,0,0,1), bool reverse = false);
	void MainWindow::fillList();
	void MainWindow::glyphToDraw(fg::Glyph &g, Point &tr = Point (0,0));
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
	double MainWindow::edist(const Point &p1, const Point &p2 = Point(0,0));
	// ---
	
	double MainWindow::angle(Point &p0, Point &p1);
	int MainWindow::getRepres(Contour source, Contours &cs);
	void MainWindow::contoursToDraw(Contours cs, Point tr, QPainterPath &path);	
	void paintEvent(QPaintEvent *e);
	
};

#endif // MAINWINDOW_H
