#include "mainwindow.h"

#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFontDialog>
#include <QFont>

#include <QGraphicsTextItem>
#include <QDebug>
#include <math.h>
#include <sstream>

static MainWindow *mainWindow;


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
	
//  mainWindow = this;
//  gCounter = 0;
//  maxGraphemesCount = 0;  
//  bool ok;  
//  ui->progressBar->hide();
//  show();

  file = new QFile(QFileDialog::getOpenFileName());
  ui->fontPath->setText(file->fileName());
  readFontFile(file->fileName(), QString("D://"));
	
	len = ui->lenEdit->text().toInt();

//  globalPath = QPainterPath();
//  paths.clear();
//  QRectF rect;
//  int indent = 5;

	
//  splineLength2 = 100;
//  polygonCaching = true;
//  bbox_tolerance = 100;
//  fast = false;
//  tolerance = 0.2;
	
	globalCtr = 0;


  this->update();
}

MainWindow::~MainWindow()
{
  delete ui;
  //delete file;
  //delete package;

  //qDebug()<<"delete decFont";
  //delete decFontPointer;
}

QPainterPath MainWindow::contourToPath(/*QPainterPath &path, */const fg::Contour &contour, const fg::Matrix &layerMatrix)
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


  /*
    if(drawbbox){
        // тут оси еще надо бы подрисовать
        path.addRect((*it).boundingBox(false).left(),(*it).boundingBox(false).top(),(*it).boundingBox(false).width(), - (*it).boundingBox(false).height());
    }*/

  return path;
}

void MainWindow::clear()
{
  paths.clear();
  globalPath = QPainterPath();
  globalBBoxes = QPainterPath();	
  this->update();
}

int MainWindow::readFontFile(QString path, QString outPath)
{
 //ui->progressBar->show();

	try 
	{
		parser = io::Parser(path.toStdString());
    parser.tempPath = outPath.toStdString();//outName.fPath;
    fg::Options options;

    if (parser.go(&options, false)) 
    {
      if (parser.packages.empty()) 
      {
        cout << "font loading error";//Source font file \"";// << errorFileName <<  "\" does not contain any fonts." << endl;
        return 0;
      }

      parser.filterDuplicates();

      QApplication::processEvents();

      if (parser.packages.size() > 0)
      {       
       }
     }
   }
   catch(...)
   {

   }


  package = new fg::Package;
  *package = parser.packages.front();

  global_scale = 180/package->style->metrics.upm;
  fg::Font* font = package->font;


  if(font->glyphs.empty() == true)
    return 0;
  
  int glyphIndex = 0;

  maxGraphemesCount = 0;

  ui->glyphsList->clear();
  ui->atomsList->clear();

  
  for (fg::Glyphs::const_iterator it = font->glyphs.begin(); it != font->glyphs.end(); ++it, glyphIndex++)
  {

		fg::Layer* layer = (*it)->bodyLayer();
    if (layer && layer->countNodes() > 0)
		{	
//			ui->glyphsList->addItem(QString("#%1  %2  ").arg(glyphIndex).arg((*it)->name.c_str()));
			ui->glyphsList->addItem(QString("%1").arg((*it)->name.c_str()));
			
			if((*it)->name.compare("I") == 0)
				qDebug()<<"I found gi = " << glyphIndex;    
		}
  }

	
	return 1;
 
 }

void MainWindow::addToDraw(fg::Contour &c, const fg::Point &translation, int index)
{
  fg::Matrix mtx(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);
  QPainterPath p = contourToPath(c, mtx).translated(QPointF(translation.x, translation.y));
  globalPath.addPath(p);
  paths.append(Path2Draw(index, p));
	
	//if(ui->drawNodes->isChecked())
    drawNodes(p);	
}

void MainWindow::addToDraw(fg::Contour &c, const fg::Matrix &m, int index)
{
  
  qDebug()<<"---addToDraw + matrix";
  QPainterPath p = contourToPath(c, m);//.translated(QPointF(m.dx, m.dy));

  globalPath.addPath(p);
  paths.append(Path2Draw(index, p));
  if(ui->drawNodes->isChecked())
    drawNodes(p);     
}

void MainWindow::drawNodes(QPainterPath p)
{
	//	  qDebug()<<"--- drawNodes";				
	
	if(p.isEmpty())
		return;
	
		globalBBoxes.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 3,3);
		// ищем следующий isLine Node
		
		for(int i = 1; i < p.elementCount(); ++i)
		{			
			globalBBoxes.addEllipse(QPointF(p.elementAt(i).x, p.elementAt(i).y), 1,1);
//			if(p.elementAt(i).isLineTo() || p.elementAt(i).isMoveTo())
//			{
//				globalBBoxes.addEllipse(QPointF(p.elementAt(i).x, p.elementAt(i).y), 2,2);
//				i = p.elementCount();
//			}			
		}	
}

void MainWindow::glyphToDraw(fg::Glyph &g, Point &tr)
{ 
  //  int SHIFT = 0;
  fg::Point translation;
  
  QPointF drawCenter = ui->canvasLeft->geometry().center();       
  fg::GlyphsR grs;    
  Rect bbox = g.boundingBox(grs, fg::Matrix(1, 0, 0, -1, 0, 0),false);      
  Point dCenter = Point(ui->canvasLeft->x() + ui->canvasLeft->size().width()/2, 
  ui->canvasLeft->y() + ui->canvasLeft->size().height()/2);   
  
  fg::Point center;     
  center = fg::Point(drawCenter.x() - 200 * global_scale, drawCenter.y());
  int bottom_line = center.y + 900*global_scale;
  Point glyphC = Point(bbox.left() + bbox.width() / 2, bbox.top() + bbox.height() / 2);
    
  translation.x = -(glyphC.x*2.5*global_scale - dCenter.x) + tr.x;
  //  qDebug()<<"gscale " << global_scale;
  
  fg::Layer* layer; 
  layer = g.fgData()->findLayer("Body");
  for (fg::Contours::iterator it = layer->shapes.front().contours.begin();it != layer->shapes.front().contours.end(); ++it)
  {
    addToDraw((*it), fg::Point(translation.x,bottom_line + translation.y), 1);
		
		
		
  } 
}



void MainWindow::on_DecomposeBtn_clicked()
{				
	clock_t tStart = clock();
	agdict.clear();
	dict.clear();
	// getting glyph 
	Glyph* g;		
	int glyphIndex = 0;
	for (fg::Glyphs::const_iterator it = package->font->glyphs.begin(); it != package->font->glyphs.end() && glyphIndex < ui->toEdit->text().toInt(); ++it, ++glyphIndex)
	{
		fg::Layer* layer = (*it)->bodyLayer();
		if (layer && layer->countNodes() > 0 && ui->fromEdit->text().toInt() <= glyphIndex && (glyphIndex == 8 || glyphIndex == 13))
		{
			g = (*it);
			qDebug()<<"name: " << g->name.c_str() << " # "<<glyphIndex;									
			
			// cycle by contours
			fg::Layer* layer;
		  layer = g->fgData()->findLayer("Body");
			int samples = 0;
			AContours acontours;
			int count = 0;
			
		  for (fg::Contours::iterator it = layer->shapes.front().contours.begin(); it != layer->shapes.front().contours.end(); ++it, ++count)
		  {		
				Atoms atoms;
				Contour ac;
				ac.open = true;
				list<Point> zeroPoints;
				Point lastp;
				int j = 0;
				
				for(Nodes::iterator ni = (*it).nodes.begin(); ni != (*it).nodes.end() && samples < 200000; ++ni, ++j)
				{
					if(ni != (*it).nodes.begin() && ((*ni).kind == Node::Move || (*ni).kind == Node::On))
					{
						// new atom beginning	...
						Matrix m;
						Atom *a;											
						ac.nodes.push_back((*ni));				
						lastp = (*std::prev(ac.nodes.end())).p;				
//						Rect r = ac.boundingBox(false);				
						Point pfirst = (*(ac.nodes.begin())).p;
						zeroPoints.push_back(pfirst);
						ac.transform(Matrix(1,0,0,1,-pfirst.x,-pfirst.y));																			
						int reverse = 0;
						// check for new item
						if(dict.empty())
						{										
							ac.open = true;
							dict.push_back(ac);					
							usedIn.push_back(Ints());
//							(*prev(usedIn.end())).push_back(glyphIndex);
							usedIn[0].push_back(glyphIndex);
							Contours::iterator ci = dict.end();
							//prev(ci);
							ci--;							
							a = new Atom((*ci), Matrix(1, 0, 0, 1, pfirst.x, pfirst.y), 0, dict.size()-1);
						}
						else 
						{		
							int n = checkNewAtom(ac, dict, m, reverse);		
							if(n == -1 && dict.size() < 200000)
							{		
								ac.open = true;
								dict.push_back(ac);
								
								Contours::iterator ci = dict.end();
								ci--;
								a = new Atom((*ci), Matrix(1,0,0,1, pfirst.x, pfirst.y), 0, dict.size()-1);
								usedIn.push_back(Ints());
								(*prev(usedIn.end())).push_back(glyphIndex);
							}
							else
							{								
								Contours::iterator ci = dict.begin();
								std::advance(ci, n);		
								a = new Atom((*ci), Matrix(m.m11, m.m12, m.m21, m.m22, m.dx + pfirst.x, m.dy + pfirst.y), reverse, n);					
//								usedIn.push_back(Ints());
								usedIn[n].push_back(glyphIndex);
							}
						}
														
		//				a->info("current ");	
						a->usedIn.push_back(glyphIndex);
						atoms.push_back(*a);
										
						// and here start to new atom
						ac.clear();
						ac.open = true;
						if((*ni).kind == Node::NodeType::On)
						{
							// add Move to the beginning of atom
							ac.nodes.push_back(Node(Node::Move, lastp));
						}
						samples++;	
					}
					ac.nodes.push_back((*ni));
				}	
				acontours.push_back(atoms);
			}
			
			agdict.push_back(AGlyph(acontours,g->name.c_str(), g->index));
		}			
	}
	
	ui->atomsList->clear();
	
	// add atoms to list
	int atomCount = 0;//dict.size()-1;
	for (fg::Contours::const_iterator it = dict.begin(); it != dict.end(); ++it, atomCount++)
  {
    ui->atomsList->addItem(QString("#%1 ").arg(atomCount));        
  }

	qDebug()<<" time: " << (double)(clock() - tStart)/CLOCKS_PER_SEC;	
}

void MainWindow::coutMatrix(const string &header, const Matrix &m)
{
	qDebug()<<header.c_str() << m.m11<< " "<< m.m12<< " "<< m.m21<< " "<< m.m22<< " "<< m.dx<< " " << m.dy; 			
}

void MainWindow::contourToDebug(const Contour &c) const
{
  for (Nodes::const_iterator it = c.nodes.begin();  it != c.nodes.end(); ++it)
    qDebug() << "Node::" << (*it).kind << ",Point(" << (*it).p.x << "," <<  (*it).p.y << ");";
}

void MainWindow::coutSplines(const Splines &s)
{	
	for (Splines::const_iterator it = s.begin(); it != s.end(); ++it) 
	{		
		PointSpline ps;		
		qDebug()<<" curve: " << (*it).curve << " time: "<< (*it).time;
		
	}			
}

/**
 * @brief MainWindow::isExistedAtom - return index of existed atom, or -1 if there is 
 * no such atom in dictionary..
 * @param atom
 * @param dict
 * @param m
 * @param reverse
 * @return 
 */
int MainWindow::checkNewAtom(const Contour &atom, Contours &dict, Matrix &m, int &reverse)
{		
	// getting splines from atom		
	len = ui->lenEdit->text().toInt();		
	int scaleXY[4][2] = {{1, 1}, {-1,1}, {1,-1}, {-1,-1}};
	vector<Matrix> mxs;
	mxs.reserve(4);
	vector<double> errors;
	errors.reserve(8);
	Splines s2 = contourToSplines(atom, len);
	double l2 = (s2.size()-2) * len + + sqrt(pow((*prev(prev(s2.end()))).x - (*prev(s2.end())).x, 2) + 
																					 pow((*prev(prev(s2.end()))).y - (*prev(s2.end())).y, 2));	;
	
	
//	Node &afirst = (*(atom.nodes.begin()));	
//	coutRect("atom rect: ", atom.boundingBox(false));
	
	int n = 0;
	for (Contours::iterator it = dict.begin(); it != dict.end(); ++it, ++n)
	{		
		errors.clear();
		mxs.clear();
		list<Splines> representations;		
		
		for (int i = 0; i < 4; ++i)
		{
			int dX = 0;
			int dY = 0;							
			Nodes::iterator afirst = (*it).nodes.begin();	
			std::advance(afirst, (*it).nodes.size() - 1);			
			dX = (scaleXY[i][0] == -1) ? (*afirst).p.x : -(*afirst).p.x; 				
			dY = (scaleXY[i][1] == -1) ? (*afirst).p.y : -(*afirst).p.y;			
			mxs.push_back(Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], 0, 0));
			mxs.push_back(Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], dX, dY));
			representations.push_back(contourToSplines((*it), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], 0, 0), 0));
			representations.push_back(contourToSplines((*it), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], dX, dY), 1));				
			
			
		}
				
		int mxsCounter = 0;		
		for (list<Splines>::iterator si = representations.begin(); si != representations.end(); ++si, ++mxsCounter) 
		{			
//			Contour c = (*it);			
//			c.transform(mxs[floor((double)mxsCounter/2)]);

			double l1 = ((*si).size()-2) * len  + sqrt(pow((*prev(prev((*si).end()))).x - (*prev((*si).end())).x, 2) + 
																								 pow((*prev(prev((*si).end()))).y - (*prev((*si).end())).y, 2));	
			double alen = (l2 + l1) / 2;		
//			double bboxError;
//			if(ui->compareBBox->isChecked())
//			{
//				bboxError = 2*edist() / (edist() + edist());
//				if(bboxError < )
//			}
			
			
			
			double error = compareSplines((*si), s2, len);			
			errors.push_back(error / alen);
		}
		
		// cycle by errors		
		int minErrori = 0;
		for (int i = 0; i < errors.size(); ++i) 
		{
//			qDebug()<<"error #" << i << " : " << errors[i];
			if(errors[minErrori] > errors[i])
			{
				minErrori = i;			
			}
		}
						
		if(errors[minErrori] < ui->toleranceEdit->text().toFloat())
		{
			// this is existing atom, set matrix
//			m = mxs[floor((double)minErrori/2)];									
			m = mxs[minErrori];
			double intpart;
//			qDebug()<<" this is existing atom, set matrix ... ";// << "minErrori: " << minErrori << " modf(minErrori/2, &intpart) " << modf((double)minErrori/2, &intpart);						
			
			if(modf((double)minErrori/2, &intpart) > 0)
				reverse = 1;
				
			return n;
		}
	}
	
	// there is no such atom here
	return -1;
}
		
void MainWindow::on_OneBtn_clicked()
{
	len = ui->lenEdit->text().toInt();
	
//	bool yeah = true;	
//	int dY = (!yeah) ? 10 : -3;	
//	qDebug()<<" dY " << dY;
	
	
	// existed
//	[0 : (0, 0)]; 
//	 [3 : (-8, -5)]; 
//	 [3 : (-8, -18)]; 
//	 [1 : (-2, -24)];
	
	// own (original)
//	[0 : (0, 0)]; 
//	 [1 : (0, 0)]; 
//	 [3 : (-5, -7)]; 
//	 [3 : (-1, -17)]; 
//	 [1 : (7, -21)]; 
	
	/*
	if(globalCtr == 0)
	{
		Point p0 = Point(0,0);
		Point p1 = Point(200,100);		
		cc.nodes.push_back(Node(Node::Move, p0));
		cc.nodes.push_back(Node(Node::On, p1));		
		addToDraw(cc, Matrix(1,0,0,1, 300, 200), 0);
		this->update();
		
		globAngle = angle(p0,p1);
		qDebug()<<" angle(btwp0p1) = " << globAngle;
		globalCtr = 1;
	}
	else if(globalCtr = 1)
	{
		clear();
		Matrix m = Matrix(cos(globAngle), -sin(globAngle), sin(globAngle),cos(globAngle));						
		
//		Matrix()
//		m.slant(-globAngle);
		cc.transform(m);
						
		addToDraw(cc, Matrix(1,0,0,1, 300, 200), 0);
		this->update();
		
	}
	
	*/
	
	
	if(globalCtr == 0)
	{		
		clear();
		c1.clear();
		c2.clear();									
							
//		c2.nodes.push_back(Node(Node::Move, Point(0, 0)));
//		c2.nodes.push_back(Node(Node::On, Point(0, 0)));
//		c2.nodes.push_back(Node(Node::Curve, Point(0, 32)));
//		c2.nodes.push_back(Node(Node::Curve, Point(-22, 59)));
//		c2.nodes.push_back(Node(Node::On, Point(-56,59)));		
								
//		c1.nodes.push_back(Node(Node::Move, Point(0,0)));
////		c2.nodes.push_back(Node(Node::On, Point(0,0)));
//		c1.nodes.push_back(Node(Node::Curve, Point(34, 0)));
//		c1.nodes.push_back(Node(Node::Curve, Point(56, 23)));
//		c1.nodes.push_back(Node(Node::On, Point(56, 55)));			
		
		// own (original)
			c2.nodes.push_back(Node(Node::Move, Point(0, 0)));
			c2.nodes.push_back(Node(Node::On, Point(0, 0)));
			c2.nodes.push_back(Node(Node::Curve, Point(-5, -7)));
			c2.nodes.push_back(Node(Node::Curve, Point(-1, -17)));
			c2.nodes.push_back(Node(Node::On, Point(7,-21)));
			
			// existed
			c1.nodes.push_back(Node(Node::Move, Point(0,0)));
			c1.nodes.push_back(Node(Node::Curve, Point(-8, -5)));
			c1.nodes.push_back(Node(Node::Curve, Point(-8, -18)));
			c1.nodes.push_back(Node(Node::On, Point(-2, -24)));			
			
		
		
//		c1.nodes.push_back(Node(Node::Move, Point(0, 0)));
//		c1.nodes.push_back(Node(Node::On, Point(20, 20)));		
//		c1.nodes.push_back(Node(Node::On, Point(100,20)));		
		
//		c2.nodes.push_back(Node(Node::Move, Point(0,0)));
//		c2.nodes.push_back(Node(Node::On, Point(80,0)));		
//		c2.nodes.push_back(Node(Node::On, Point(100, -20)));			
		
		c2.open = true;
		c1.open = true;		
		
		// getting norm representations 						
//		Contours cs;
//		getRepres(c1, cs);
//		c1 = *(cs.begin());
//		cs.clear();
//		getRepres(c2, cs);
//		c2 = *(cs.begin());			
		
		clear();
		Contours cs;
		cs.push_back(c1);
		cs.push_back(c2);
		
		
//		contoursToDraw(cs, Point(300, 200), globalPath);
//		contoursToDraw(c2, Point(300, 200), globalPath);
		
		addToDraw(c1, Matrix(1,0,0,-1, 300, 200), 0);
		addToDraw(c2, Matrix(1,0,0,-1, 300, 200), 0);
		this->update();				
		
//		Splines s1, s2;		
//		s1 = contourToSplines(c1,len, Matrix(), false);
//		s2 = contourToSplines(c2,len, Matrix(), false);		
//		double er = compareSplines(s1,s2, len);		
//		double l2 = s2.size() * len;
//		double l1 = s1.size() * len;				
//		double alen = (l2 + l1) / 2;	
//		qDebug()<<" er " << er << " er / alen " << er / alen << " alen " << alen  ;					
//		globalCtr = 2;	
				globalCtr = 1;
	}
	else if(globalCtr >= 1 && globalCtr < 9)
	{			
				
		// надо так тут, делаем репрезентации и потом каждый раз по нажатию 
		// прибавляем итератор
		
		
		// ок, создаем репрезентации тут 
		vector<Splines> representations;			
		vector<Matrix> mxs;
		mxs.reserve(4);
		
		int scaleXY[4][2] = {{1, 1}, {-1,1}, {1,-1}, {-1,-1}};			
		for (int i = 0; i < 4; ++i) 
		{								
			int dX = 0;
			int dY = 0;				
			
			Nodes::iterator afirst = c1.nodes.begin();	
			std::advance(afirst, c1.nodes.size() - 1);			
			dX = (scaleXY[i][0] == -1) ? (*afirst).p.x : -(*afirst).p.x; 				
			dY = (scaleXY[i][1] == -1) ? (*afirst).p.y : -(*afirst).p.y;			
			mxs.push_back(Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], 0, 0));
			representations.push_back(contourToSplines(c1, len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], 0, 0), 0));			
			representations.push_back(contourToSplines(c1, len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], dX, dY), 1));							
			// --------------------
		}
		
		
		// цикл сравнения
		vector<double> errors;
		errors.reserve(representations.size());	
//		list<Splines>::iterator si;
		Splines s1;
		Splines s2 = contourToSplines(c2, len);
		
		
		if(globalCtr > 0 && globalCtr < representations.size() + 1)
		{			
			double l2 = s2.size() * len;
			double l1 = representations[globalCtr - 1].size() * len;				
			double alen = (l2 + l1) / 2;	
			double error = compareSplines(representations[globalCtr - 1], s2, len);
			qDebug()<<" error: " << error / alen << " alen " << alen;
			coutMatrix("m:", mxs[floor((double)(globalCtr - 1)/2)]);
//			errors.push_back(error / alen);			
			globalCtr++;
		}
			 
		
		
		
		
//		Contour c1n = c1; 
//		// reverse
//		Nodes::iterator afirst = c1n.nodes.begin();	
//		std::advance(afirst, c1n.nodes.size() - 1);
//		c1n.transform(Matrix(-1,0,0,1, (*afirst).p.x, -(*afirst).p.y));		
//		c1n.reverse();
		
//		clear();
//		addToDraw(c1n, Matrix(1,0,0,1, 300, 200), 0);
//		addToDraw(c2, Matrix(1,0,0,1, 300, 210), 0);
//		this->update();
		
//		Splines s1, s2;		
//		s1 = contourToSplines(c1n,len, Matrix(), false);
//		s2 = contourToSplines(c2,len, Matrix(), false);		
//		double er = compareSplines(s1,s2, len);		
//		double l2 = s2.size() * len;
//		double l1 = s1.size() * len;				
//		double alen = (l2 + l1) / 2;	
//		qDebug()<<" er " << er << " er / alen " << er / alen << " alen " << alen  ;									
//		globalCtr = 3;	
	}
	else if(globalCtr == 3)
	{
		Contour c1n = c2; 
		// reverse
		Nodes::iterator afirst = c1n.nodes.begin();
		std::advance(afirst, c1n.nodes.size() - 1);
		c1n.transform(Matrix(-1,0,0,1, (*afirst).p.x, -(*afirst).p.y));		
		c1n.reverse();
		
		clear();
		addToDraw(c1, Matrix(1,0,0,1, 300, 200), 0);
		addToDraw(c1n, Matrix(1,0,0,1, 300, 210), 0);
		this->update();
		
		Splines s1, s2;		
		s1 = contourToSplines(c1,len, Matrix(), false);
		s2 = contourToSplines(c1n,len, Matrix(), false);		
		double er = compareSplines(s1,s2, len);		
		double l2 = s2.size() * len;
		double l1 = s1.size() * len;				
		double alen = (l2 + l1) / 2;	
		qDebug()<<" er " << er << " er / alen " << er / alen << " alen " << alen  ;									
		globalCtr = 3;			
	}
}

int MainWindow::getRepres(Contour source, Contours &cs)
{		
	Nodes::iterator it = source.nodes.begin();
	Point p0 = (*it).p;
	std::advance(it, source.nodes.size() - 1);
	Point p1 = (*it).p;											 
	double ang = angle(p0,p1);
	Matrix m = Matrix(cos(ang), -sin(ang), sin(ang),cos(ang), -p0.x, -p0.y);
	source.transform(m);			
	
	
	qDebug()<<" source.area() " << source.area();
	source.open = false;
	if(source.area() < 0)
	{
		
		qDebug()<<" LESS ";
		source.transform(Matrix(1,0,0,-1));					
		source.open = true;
		
//		source.reverse();	
		//		// translate
		//		Nodes::iterator afirst = tr.nodes.begin();	
		//		std::advance(afirst, tr.nodes.size() - 1);
		//		qDebug()<<" last.p " << (*afirst).p.x<<" " << (*afirst).p.y;
		
	}
	
	cs.push_back(source);
	
	return 1;
}

double MainWindow::angle(Point &p0, Point &p1)
{	
	double dy = p1.y - p0.y;
	double dx = p1.x - p0.x;
	return std::atan(dy/dx);								
}

void MainWindow::on_TwoBtn_clicked()
{	
	
	if(globalCtr == 0)
	{		
		Rect r1 = c1.boundingBox(false);
		Rect r2 = c2.boundingBox(false);
		
		Node &afirst1 = (*(c1.nodes.begin()));
		Node &afirst2 = (*(c2.nodes.begin()));
		
		int tX = r1.left();
		int tY = r1.top();			
//		tX += r1.width();
//		tY += -r1.height();		
		tX += afirst1.p.x;
		tY += -afirst1.p.y;		
		
		qDebug()<<" " << r1.width() << " " << r1.height();	
		Contour c1tr = c1;
		
		
		
//		c1tr.transform(Matrix(-1,0,0,-1, tX, tY));	
//		c1tr.transform(Matrix(-1,0,0,1, 0, 0));	
		c1.transform(Matrix(-1,0,0,1, 0, 0));
		
		Node &n10 = (*(c1tr.nodes.begin()));
		Node &n20 = (*(c2.nodes.begin()));
						
		// совмещаем первые узлы контуров для сравнения
//		Point trnsl  = Point(n20.p.x - n10.p.x, n20.p.y - n10.p.y);														
//		c1tr.transform(Matrix(1,0,0,1, trnsl.x, trnsl.y));	
		
		
		clear();
		
		addToDraw(c1, Matrix(1,0,0,1, 300, 200), 0);
		addToDraw(c2, Matrix(1,0,0,1, 300, 200), 0);
		this->update();
		
		qDebug()<<" c1 - transformed, c2 - original ";
		qDebug()<<ctrToString(c1tr).str().c_str() <<"\n" << ctrToString(c2).str().c_str();		
		
		Splines s1, s2;		
		s1 = contourToSplines(c1,20, Matrix(), false);
		s2 = contourToSplines(c2,20, Matrix(), false);
		
		double er = compareSplines(s1,s2, 20);
		qDebug()<<" er " << er;				
		globalCtr = 1;
	}
	
	else if(globalCtr == 1)
	{
		
//		Rect r1 = c1.boundingBox(false);
//		Rect r2 = c2.boundingBox(false);
		
//		int tX = r2.left();
//		int tY = r2.top();			
//		tX += r2.width();
//		tY += -r2.height();		
		
//		Contour c1tr = c2;
		
//		c1tr.transform(Matrix(-1,0,0,-1, tX, tY));	
//		c1tr.transform(Matrix(-1,0,0,-1, 0, 0));	
		clear();						
		
//		Node &n20 = (*(c1tr.nodes.begin()));
//		Node &n10 = (*(c1.nodes.begin()));
						
		// совмещаем первые узлы контуров для сравнения
//		Point trnsl  = Point(n10.p.x - n20.p.x, n10.p.y - n20.p.y);														
//		c1tr.transform(Matrix(1,0,0,1, trnsl.x, trnsl.y));	
		Nodes::iterator afirst = c2.nodes.begin();	
		std::advance(afirst, c2.nodes.size() - 1);
		
//		dX = (scaleXY[i][0] == -1) ? (*afirst).p.x : -(*afirst).p.x; 				
//		dY = (scaleXY[i][1] == -1) ? (*afirst).p.y : -(*afirst).p.y;
		double dX = -(*afirst).p.x; 				
		double dY =  (*afirst).p.y;
		
		c2.transform(Matrix(1,0,0,-1, dX, dY));
		c2.reverse();
		
//		c1.reverse();
		
//		Node &n10 = (*(c1.nodes.begin()));
		
		addToDraw(c1, Matrix(1,0,0,-1, 300, 200), 0);
		addToDraw(c2, Matrix(1,0,0,-1, 300, 200), 0);
		this->update();
		
		qDebug()<<" c1 - original, c2 - transformed ";
		qDebug()<<ctrToString(c1).str().c_str() <<"\n" << ctrToString(c2).str().c_str();		
		
		Splines s1, s2;		
		s1 = contourToSplines(c1,20, Matrix(), false);
		s2 = contourToSplines(c2,20, Matrix(), false);
		
//		double er = compareSplines(s1,s2, 20);
		
		double er = compareSplines(s1,s2, len);		
		double l2 = s2.size() * len;
		double l1 = s1.size() * len;				
		double alen = (l2 + l1) / 2;	
		qDebug()<<" er " << er << " er / alen " << er / alen << " alen " << alen  ;									
//		qDebug()<<" er " << er;				
		globalCtr = 2;		
	}
	else if(globalCtr == 2)
	{
		
//		Contour c1tr = c1;
//		Nodes::iterator n1itr = c1.nodes.begin();
//		n1itr--;
//		qDebug()<<" lastp " << (*n1itr).p.x <<" " << (*n1itr).p.y;		
//		c1tr.reverse();
//		n1itr = c1tr.nodes.end();
//		n1itr--;
//		qDebug()<<"reversed lastp " << (*n1itr).p.x <<" " << (*n1itr).p.y;
		
		Nodes::iterator afirst = c2.nodes.begin();	
		std::advance(afirst, c2.nodes.size() - 1);
		
//		dX = (scaleXY[i][0] == -1) ? (*afirst).p.x : -(*afirst).p.x; 				
//		dY = (scaleXY[i][1] == -1) ? (*afirst).p.y : -(*afirst).p.y;
		double dX = -(*afirst).p.x; 				
		double dY =  (*afirst).p.y;
		c2.transform(Matrix(1,0,0,1, dX, dY));	
																
				

		//c1tr.transform(Matrix(-1,0,0,-1, 0, 0));	
		clear();										
		
//		c1.transform(Matrix(1,0,0,1, -(*n1itr).p.x,  -(*n1itr).p.y));
						
		// совмещаем первые узлы контуров для сравнения
//		Point trnsl  = Point(n10.p.x - n20.p.x, n10.p.y - n20.p.y);														
//		c1tr.transform(Matrix(1,0,0,1, trnsl.x, trnsl.y));	
		
		addToDraw(c1, Matrix(1,0,0,-1, 310, 200), 0);
		addToDraw(c2, Matrix(1,0,0,-1, 300, 200), 0);
		this->update();
		
//		qDebug()<<" c1 - reversed, c2 - original ";
////		qDebug()<<ctrToString(c1).str().c_str() <<"\n" << ctrToString(c1tr).str().c_str();		
		
		
		
//		Splines s1, s2;		
//		s1 = contourToSplines(c1,20, Matrix(), false);
//		s2 = contourToSplines(c2,20, Matrix(), false);
		
//		double er = compareSplines(s1,s2, 20);
//		qDebug()<<" er " << er;				
		globalCtr = 3;								
	}
	
		
}

void MainWindow::coutRect(const string &header, const Rect &r)
{
		qDebug()<<header.c_str() <<": " << r.left() << ","<< r.top() << "   " << r.right() << "," << r.bottom() << " width: " <<  r.width() << " height: " << r.height();
}

Contour MainWindow::contourToPoligone(Contour contour, int len)
{
   qDebug()<< "len: " << len;
   std::list<fg::Curve> curves;
   fg::Integers intg;
   Splines splines;

   Grapheme::contourToCurves(curves, contour);   
   return splinesToContour(splines);
}

Splines MainWindow::contourToSplines(Contour contour, int len, Matrix m, bool reverse)
{
   std::list<fg::Curve> curves;
   fg::Integers intg;
   Splines splines;	 	 	 
	 
	 contour.transform(m);	
	 
	 if(reverse)
	 {		 
		 contour.reverse();
		 // and new transform		 
	 }
	 	  	 	 
//	 qDebug()<<" FIRST.P " << (*(contour.nodes.begin())).p.x << " " <<(*(contour.nodes.begin())).p.y;
	 
   Grapheme::contourToCurves(curves, contour); 
	 Grapheme::curvesToSplines(curves, splines, intg, len);
   return splines;
}

Contour MainWindow::splinesToContour(Splines splines)
{
   Contour c;
   c.addNode(Node::Move, Point(splines[0].x, splines[0].y));
   for (int i = 1; i < splines.size(); ++i)
       c.addNode(Node::On, Point(splines[i].x, splines[i].y));
   return c;
}

void MainWindow::on_glyphsList_itemClicked(QListWidgetItem *item)
{
  clear();
	
  if(item != NULL )
  {   
    // тут отрисовываем оригинальный глиф
    // находим нужный глиф по имени    
    fg::Point center;
    center = fg::Point(ui->canvasLeft->geometry().x() + ui->canvasLeft->geometry().width()/2, ui->canvasLeft->geometry().y() + ui->canvasLeft->geometry().height()/2);
		
		qDebug()<<" name: " << item->text();
		
		Point tr;
    
    int glyphIndex = 0;
    for (fg::Glyphs::iterator it = package->font->glyphs.begin(); it != package->font->glyphs.end(); ++it, ++glyphIndex)
    {
			if((*it)->name.compare(item->text().toStdString()) == 0)
			{
				
				qDebug()<<"have found original...";
				Glyph &g = *(*it);												
				fg::GlyphsR grs;    
				Rect bbox = g.boundingBox(grs, fg::Matrix(1, 0, 0, -1, 0, 0),false);      				
				tr = Point((int)bbox.width(), 0);										
				fg::Layer* layer; 
			  layer = g.fgData()->findLayer("Body");
				Contours cs;
			  for (fg::Contours::iterator it2 = layer->shapes.front().contours.begin();it2 != layer->shapes.front().contours.end(); ++it2)
				{
					cs.push_back((*it2));
				}
				
				contoursToDraw(cs, Point(-700,0), globalPath);
			}
    }
		
		
		for (AGlyphs::iterator it = agdict.begin(); it != agdict.end(); ++it)
    {
			if((*it).name.compare(item->text().toStdString()) == 0)
			{				
				drawedGlyphIndex = &(*it);
				qDebug()<<"have found transformed...";
				// получаем контура
				Contours cs;
				(*it).getContours(cs);
				// и отрисовываем их				
				contoursToDraw(cs, Point(-700 + 20 + (int)tr.x,0), globalPath);								
			}
    }
		
    this->update();
  }
}


void MainWindow::contoursToDraw(Contours cs, Point tr, QPainterPath &path)
{   
  fg::Point translation;  
  QPointF drawCenter = ui->canvasLeft->geometry().center();       
  Point dCenter = Point(ui->canvasLeft->x() + ui->canvasLeft->size().width()/2, 
  ui->canvasLeft->y() + ui->canvasLeft->size().height()/2);   
  
  fg::Point center;     
  center = fg::Point(drawCenter.x() - 200 * global_scale, drawCenter.y());
  int bottom_line = center.y + 900*global_scale;
	translation.x = dCenter.x + 2.5*global_scale*tr.x;
	translation.y += bottom_line + tr.y;  
    
  for (fg::Contours::iterator it = cs.begin();it != cs.end(); ++it)
  {		
		fg::Matrix mtx(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);
	  QPainterPath p = contourToPath((*it), mtx).translated(QPointF(translation.x, translation.y));
//	  globalPath.addPath(p);
		path.addPath(p);
//	  paths.append(Path2Draw(index, p));
		
		if(ui->drawNodes->isChecked())
	    drawNodes(p);
  } 
}


double MainWindow::compareSplines(const Splines &splines1, const Splines &splines2, int len)
{
  double sumError = 0;

  // цикл сравнения
  for (uint i = 0; i < splines1.size() || i < splines2.size(); ++i) 
	{
		
//		if(i < splines1.size() && i < splines2.size())
//		qDebug()<<" [1] " << splines1[i].x << " "<< splines1[i].y 
//					 << " [2] " << splines2[i].x << " " <<  splines2[i].y 
//					 << " DELTA.X " << splines1[i].x - splines2[i].x <<  " DELTA.Y " 
//							<< splines1[i].y - splines2[i].y;
		
    if(i < splines1.size() && i < splines2.size())
      sumError += sqrt(pow(splines1[i].x - splines2[i].x,2) + pow(splines1[i].y - splines2[i].y,2));

    /** если в одном из полигонов сплайнов больше чем в другом,
        то длинна "лишних" полигонов добавляется к общей ошибке
    **/

    else if(i < splines1.size())
		{
      sumError += sqrt(pow(splines1[i].x - (*prev(splines2.end())).x,2) + pow(splines1[i].y - (*prev(splines2.end())).y,2));
    }
    else if(i < splines2.size())
		{						
      sumError += sqrt(pow(splines2[i].x - (*prev(splines1.end())).x,2) + pow(splines2[i].y - (*prev(splines1.end())).y,2));
    }
//		qDebug()<<" summError : " << sumError;
  }

//  double S = sumError * len;
//	qDebug()<<" E = " << S;

  return sumError;//S;
}

void MainWindow::paintEvent(QPaintEvent *e)
{    
	QPainter painter(this);
	
		
	painter.setRenderHint(QPainter::Antialiasing);		
	painter.setPen(QPen(QColor(0, 0, 0), 0.5, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));	
	painter.drawPath(globalPath);						
	painter.setPen(QPen(QColor(255, 0, 0), 1, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));
	
	painter.setBrush(Qt::NoBrush);
	
	
	foreach (const Path2Draw &path, paths)
	{
		if (path.index <= 1)
		{
			painter.setPen(QPen(Qt::black, 0.5, Qt::SolidLine,
													Qt::FlatCap, Qt::MiterJoin));
		}
		else
		{
			QColor c = QColor::fromHsv(path.index * 255 / maxGraphemesCount, 255, 200);
			//      c.setAlpha();
			
			painter.setPen(QPen(c, 1.5, Qt::SolidLine,
													Qt::FlatCap, Qt::MiterJoin));
		}
		
		painter.drawPath(path.path);
	}
	
		
	painter.setPen(QPen(QColor(255, 0, 0), 1, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));
			
			
	painter.drawPath(globalBBoxes);		
	
	QPainter atomPainter(this);		
	atomPainter.drawPath(globalAtoms);
	
	
	atomPainter.setPen(QPen(QColor(255, 0, 0), 1, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));
	
	atomPainter.setRenderHint(QPainter::Antialiasing);		
	
	if(ui->drawComparingLabel->isChecked())
	{
		atomPainter.drawPath(redPath);
	}
	
	atomPainter.drawPath(redPath);
	
//	QPainterPath pptest;
//	pptest.addEllipse(QPointF(50,50), 10, 15);
//	atomPainter.drawPath(pptest);	
//	qDebug()<<"atomPainter.begin(ui->canvasRight)  " << atomPainter.begin(ui->canvasRight);		
}

void MainWindow::on_atomsList_itemChanged(QListWidgetItem *item)
{
//    on_atomsList_itemClicked(item);
}

//void MainWindow::draw

void MainWindow::on_atomsList_itemClicked(QListWidgetItem *item)
{
}

std::stringstream MainWindow:: ctrToString(const Contour &contour)
{	
	std::stringstream s_result;
	for (Nodes::const_iterator it = contour.nodes.begin(); it != contour.nodes.end(); ++it) 
	{
		s_result << " ["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")]; \n";
	}
	
 return	s_result;
}

//std::stringstream MainWindow:: atomsToString(const Contour &contour)
//{	
//	std::stringstream s_result;
//	for (Nodes::const_iterator it = contour.nodes.begin(); it != contour.nodes.end(); ++it) 
//	{
//		s_result << " ["<< (*it).kind << " : (" << (*it).p.x << ", " << (*it).p.y << ")]; \n";
//	}
	
// return	s_result;
//}

void MainWindow::on_atomsList_currentItemChanged(QListWidgetItem *item, QListWidgetItem *previous)
{		
	
	len = ui->lenEdit->text().toInt();
	globalAtoms = QPainterPath();
	redPath = QPainterPath();
	this->update();
	
	qDebug()<<" dict.size " << dict.size() << " item->listWidget()->currentRow() " << item->listWidget()->currentRow() ;
  if(item != NULL && dict.size() > item->listWidget()->currentRow())
  {    				
		fg::Contours::iterator current = dict.begin();
		std::advance (current,item->listWidget()->currentRow());
		stringstream ss;
		
		Ints &c = usedIn[item->listWidget()->currentRow()];		
		for (Ints::iterator it = c.begin(); it != c.end(); ++it) 
		{
			ss << (*it) <<", ";
		}  
		qDebug()<<" usedIn: " << ss.str().c_str();
		
		// second atom
		fg::Contours::iterator comparing = dict.begin();
		std::advance (comparing, ui->compareEdit->text().toInt());
		
		Point translation;
    fg::Point center;				  
		QPoint pp = QPoint(700,0);		
		Rect bbox = (*current).boundingBox(false);
		Point dCenter = Point(pp.x() + ui->canvasRight->size().width()/2, ui->canvasRight->y() + ui->canvasRight->size().height()/2);
    center = fg::Point(pp.x() + ui->canvasRight->geometry().width()/2, ui->canvasRight->geometry().y() + ui->canvasRight->geometry().height()/2);
		Point glyphC = Point(bbox.left() + bbox.width() / 2, bbox.top() + bbox.height() / 2);
		translation.x = -(glyphC.x*2.5*global_scale - dCenter.x); 					      	  	  	  	  
	  int bottom_line = center.y + 100*global_scale;	  	   
		
		qDebug()<<" current raw " << ctrToString((*current)).str().c_str();
		qDebug()<<" comparing raw " << ctrToString((*comparing)).str().c_str();
				
		
		fg::Matrix mtx(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);
		// draw item
		QPainterPath p = contourToPath((*current), mtx).translated(QPointF(translation.x,bottom_line + translation.y));		
		p.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 2,2);				
		//		p.addRect(p.boundingRect());
		globalAtoms.addPath(p);
		
		if(ui->ShowSampleCheckBox->isChecked())
		{					
			for (AGlyphs::iterator it = agdict.begin(); it != agdict.end(); ++it) 
			{				
				if(&(*it) == drawedGlyphIndex)								
				{
					qDebug()<<" & : " << &(*it);
				
					Contours cs;
					(*it).getContours(cs, &(*current));
					
					for (Contours::iterator it2 = cs.begin(); it2 != cs.end(); ++it2)
					{
						(*it2).open = true;
					}					
					contoursToDraw(cs, Point(-700,0), redPath);																	
				}
					
			}																							
		}
		
		// here comparation with control atom 
    if(ui->drawComparingLabel->isChecked())
		{	
			//			p.addRect(p.boundingRect());										
			qDebug()<<" compareEdit: " << ui->compareEdit->text().toInt();	
			qDebug()<<" item: " << item->listWidget()->currentRow();	
						
			list<Splines> representations;			
			vector<Matrix> mxs;
			mxs.reserve(4);
			
			int scaleXY[4][2] = {{1, 1}, {-1,1}, {1,-1}, {-1,-1}};			
			for (int i = 0; i < 4; ++i) 
			{								
				int dX = 0;
				int dY = 0;								
				Nodes::iterator afirst = (*comparing).nodes.begin();	
				std::advance(afirst, (*comparing).nodes.size() - 1);				
				dX = (scaleXY[i][0] == -1) ? (*afirst).p.x : -(*afirst).p.x; 				
				dY = (scaleXY[i][1] == -1) ? (*afirst).p.y : -(*afirst).p.y;				
				mxs.push_back(Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], 0, 0));	
				representations.push_back(contourToSplines((*comparing), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], 0, 0), 0));				
				representations.push_back(contourToSplines((*comparing), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], dX, dY), 1));				
			}
			
			if(ui->compareEdit->text().toInt() != item->listWidget()->currentRow())
			{
				//(*it2).transform(mxs[2]);
				p = contourToPath((*comparing), mtx).translated(QPointF(translation.x,bottom_line + translation.y));		
				p.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 2,2);				
				redPath.addPath(p);						  					
			}
				
			vector<double> errors;
			errors.reserve(representations.size());																			
			Splines s2 = contourToSplines((*current), len);
			
			int mxsCounter = 0;
			for (list<Splines>::iterator si = representations.begin(); si != representations.end(); ++si, ++mxsCounter) 
			{
				// give representation of it
				//				(*it2).transform(mxs[mxs_counter]);										
				Contour c = (*comparing);
				c.transform(mxs[floor((double)mxsCounter/2)]);
				
				double l2 = (s2.size()-2) * len + sqrt(pow((*prev(prev(s2.end()))).x - (*prev(s2.end())).x, 2) + 
																					 pow((*prev(prev(s2.end()))).y - (*prev(s2.end())).y, 2));
				
				if(mxsCounter == 0 || mxsCounter == 1)
				{
					stringstream ss;
					foreach (const PointSpline &var, s2) 
					{
						ss <<"(" << var.x << "," << var.y <<")";
					} 
					qDebug()<<"s2: " << ss.str().c_str();				
					
					ss.clear();
					stringstream ss1;					
					foreach (const PointSpline &var, (*si)) 
					{						
						ss1 <<"(" << var.x << "," << var.y <<")";
					} 
					qDebug()<<"s1: " << ss1.str().c_str();														
					
					// compare
//					Splines::iterator it1 = (*si).begin();
//					for (Splines::iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++it) 
//					{
						
						
//					}					
				}
				
				
				
//				qDebug()<<" (PointSpline)(*prev(s2.end())).len() " << (*prev(s2.end())).len();							
				
//				double lastSplineLength = sqrt(pow((*prev(prev(s2.end()))).x - (*prev(s2.end())).x, 2) + 
//																 pow((*prev(prev(s2.end()))).y - (*prev(s2.end())).y, 2));
				
//				qDebug()<<" lastSplineLength fo s2 " << lastSplineLength;

//				double l1 = ((*si).size()-1) * len + ;				
				double l1 = ((*si).size()-2) * len  + sqrt(pow((*prev(prev((*si).end()))).x - (*prev((*si).end())).x, 2) + 
																									 pow((*prev(prev((*si).end()))).y - (*prev((*si).end())).y, 2));				
				
				qDebug()<<" l1 " << l1 << " l2 " << l2;
				
//				qDebug()<<" (PointSpline)(*prev((*si).end())).len() " << (*prev((*si).end())).len();
				
				double alen = (l2 + l1) / 2;
				
				qDebug()<<"si.size" << (*si).size() << " s2.size() " << s2.size() << " alen " << alen;
				double error = compareSplines((*si), s2, len);			
//				double error2 = compareSplines(s2, (*si), len);
//				qDebug()<<" error " << error << " error2 " << error2;
				errors.push_back(error / alen);
			}
			
			
			// cycle by errors		
			int minErrori = 0;
			for (int i = 0; i < errors.size(); ++i) 
			{
				if(errors[minErrori] > errors[i])
				{
					minErrori = i;				
				}
			}
			
			qDebug()<<"error: " << errors[minErrori];
			coutMatrix("m:", mxs[floor((double)minErrori/2)]);
			double intpart;
			qDebug()<<" reversed: " << !(modf((double)minErrori/2, &intpart) == 0) << " \n";
		}
		
		// here drawing current atom in the glyph canvas
		if(ui->ShowSampleCheckBox->isChecked())
		{
			// need to draw this atom or atoms in the glyph contours
			
			// search atom in glyphs
//			for()
//			{
				
				
//			}
			
			
			// but we need to have glyph represented as sequence of atoms... 
			// it means we need new class for this....									
		}

		
//		qDebug()<<" contours in dictionary ...  " ;
//		for (Contours::iterator it2 = dict.begin(); it2 != dict.end(); ++it2) 
//		{
//			qDebug()<<"$$ " << ctrToString((*it2)).str().c_str();			
//		}
		

    this->update();
  }
}

double MainWindow::edist(const Point &p1, const Point &p2)
{	
	return sqrt(pow(p1.x - p2.x,2) + pow(p1.y - p2.y,2));
}
