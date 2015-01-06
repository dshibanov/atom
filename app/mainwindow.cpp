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

//  globalPath = QPainterPath();
//  paths.clear();
//  QRectF rect;
//  int indent = 5;

	
//  splineLength2 = 100;
//  polygonCaching = true;
//  bbox_tolerance = 100;
//  fast = false;
//  tolerance = 0.2;


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
  //if(ui->drawNodes->isChecked())
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
			globalBBoxes.addEllipse(QPointF(p.elementAt(i).x, p.elementAt(i).y), 2,2);
//			if(p.elementAt(i).isLineTo() || p.elementAt(i).isMoveTo())
//			{
//				globalBBoxes.addEllipse(QPointF(p.elementAt(i).x, p.elementAt(i).y), 2,2);
//				i = p.elementCount();
//			}			
		}	
}

void MainWindow::glyphToDraw(fg::Glyph &g)
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
    
  translation.x = -(glyphC.x*2.5*global_scale - dCenter.x);
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
	// getting glyph 
	Glyph* g;		
	int glyphIndex = 0;
	for (fg::Glyphs::const_iterator it = package->font->glyphs.begin(); it != package->font->glyphs.end(); ++it, ++glyphIndex)
	{
		if((*it)->name.compare(ui->glyphsList->currentItem()->text().toStdString()) == 0)//ui->glyphsList->currentRow() == glyphIndex)
			//		if((*it)->name.compare("I") == 0)
		{						
			g = (*it);
			qDebug()<<"name: " << g->name.c_str();	
		}
	}
			
	// cycle by contours
	fg::Layer* layer;
  layer = g->fgData()->findLayer("Body");
	int samples = 0;
	AContours acontours;
	
  for (fg::Contours::iterator it = layer->shapes.front().contours.begin();it != layer->shapes.front().contours.end(); ++it)
  {		
		//		++it;
		// decompose contour on particles
		// cycle by particles		
		//		contourToDebug((*it));
		Atoms atoms;
		Contour ac;
		ac.open = true;
		
		list<Point> ltops;
		Point lastp;
		int j = 0;
		
		for(Nodes::iterator ni = (*it).nodes.begin(); ni != (*it).nodes.end() && samples < 5000; ++ni, ++j)
		{
			if(ni != (*it).nodes.begin() && ((*ni).kind == Node::Move || (*ni).kind == Node::On))
			{
				// new atom beginning	...
				Matrix m;
				Atom *a;											
				ac.nodes.push_back((*ni));				
				lastp = (*std::prev(ac.nodes.end())).p;				
				Rect r = ac.boundingBox(false);
				ltops.push_back(Point(r.left(), r.top()));											
				ac.transform(Matrix(1,0,0,1,-r.left(),-r.top()));	
												
				std::stringstream s1;
				s1<<"#" << samples;								
				coutRect(s1.str().c_str(),r);
				
				int reverse = -1;				
				// check for new item
				if(dict.empty())
				{
										qDebug()<<"first";
					ac.open = true;
					dict.push_back(ac);					
					Contours::iterator ci = dict.end();
					ci--;
					a = new Atom((*ci), Matrix(1, 0, 0, 1, r.left(), r.top()), 0, dict.size()-1);
				}
				else 
				{
					m = Matrix(-1,0,0,-1);
					int n = checkNewAtom(ac, dict, m, reverse);
//					coutMatrix("got m: ", m);
					if(n == -1 && dict.size() < 200000)
					{
//						qDebug()<<" new";
						// add atom to dict
						ac.open = true;
						dict.push_back(ac);
						
						Contours::iterator ci = dict.end();
						ci--;
						a = new Atom((*ci), Matrix(1,0,0,1,r.left(),r.top()), 0, dict.size()-1);																	
					}
					else
					{
						//					qDebug()<<" found existing !!! ";
						Contours::iterator ci = dict.begin();
						std::advance(ci, n);
						//ci--;
						
						std::stringstream s;
						s<<" found existing #" << n << " with matrix: ";												
						coutMatrix(s.str().c_str(), m);
						a = new Atom((*ci), Matrix(m.m11, m.m12, m.m21, m.m22, m.dx + r.left(), m.dy + r.top()), reverse, n);					
					}
				}
				
								
//				a->info("current ");								
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
	// add atoms to list
	glyphIndex = 0;
	for (fg::Contours::const_iterator it = dict.begin(); it != dict.end(); ++it, glyphIndex++)
  {
    ui->atomsList->addItem(QString("#%1 ").arg(glyphIndex));//.arg((*it)->name.c_str()));        
//		coutRect("#", (*it).boundingBox(false));
//		qDebug()<<" open " << (*it).open;
  }	

	// here we need to draw decomposed glyph	
	// need to get contours
	// ну тут поидее должны уже иметь все контура для создания глифа
	Contours cs;	
	AGlyphs::iterator ai = agdict.begin();
	//std::advance(ai, 1);
	qDebug()<<" ================= \n trying to get contours....";
	if((*ai).getContours(cs))
	{
		qDebug()<<" succesfully got contours ";		
	}
	else
	{
		qDebug()<<" got error ";				
	}
	
	// have got contours, draw it	
	QPointF drawCenter = ui->canvasLeft->geometry().center();       
	fg::GlyphsR grs;    
	Rect bbox = g->boundingBox(grs, fg::Matrix(1, 0, 0, -1, 0, 0),false);      
	Point dCenter = Point(ui->canvasLeft->x() + ui->canvasLeft->size().width()/2,ui->canvasLeft->y() + ui->canvasLeft->size().height()/2);    
	fg::Point translation;
	fg::Point center;     
  center = fg::Point(drawCenter.x() - 200 * global_scale, drawCenter.y());
  int bottom_line = center.y + 900*global_scale;
	Point glyphC = Point(bbox.left() + bbox.width() / 2, bbox.top() + bbox.height() / 2);
	translation.x = -(glyphC.x*2.5*global_scale - dCenter.x);
	
	clear();
	qDebug()<<" cs.size() " << cs.size();
	
	for (Contours::iterator it = cs.begin(); it != cs.end(); ++it)
	{
		addToDraw((*it), fg::Point(translation.x,bottom_line + translation.y), 1);		
//		qDebug()<<" #.nodes.size() "<< (*it).nodes.size()<< " " << ctrToString((*it)).str().c_str();
	}
			
	this->update();
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
	double l2 = s2.size() * len;
	
	coutRect("atom rect: ", atom.boundingBox(false));
	
	int n = 0;
	for (Contours::iterator it = dict.begin(); it != dict.end(); ++it, ++n)
	{
		errors.clear();
		mxs.clear();
		list<Splines> representations;
		Rect r = (*it).boundingBox(false);	
		coutRect("source rect: ", r);
		
		for (int i = 0; i < 4; ++i)
		{
			int tX = r.left();
			int tY = r.top();
			
			if(scaleXY[i][0] == -1)
			{
				tX += r.width();										 										 
			}
								
			if(scaleXY[i][1] == -1)
			{
				tY += -r.height();
			}
						
			mxs.push_back(Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY));		
			representations.push_back(contourToSplines((*it), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY), 0));
			representations.push_back(contourToSplines((*it), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY), 1));
		}
				
		int mxsCounter = 0;		
		for (list<Splines>::iterator si = representations.begin(); si != representations.end(); ++si, ++mxsCounter) 
		{			
//			Contour c = (*it);			
//			c.transform(mxs[floor((double)mxsCounter/2)]);
					
			double l1 = (*si).size() * len;			
			double alen = (l2 + l1) / 2;		
			double error = compareSplines((*si), s2, len);			
			errors.push_back(error / alen);
		}				
		
//		Matrix m;				
		
		// cycle by errors		
		int minErrori = 0;
		for (int i = 0; i < errors.size(); ++i) 
		{
			if(errors[minErrori] > errors[i])
			{
				minErrori = i;			
			}
		}
						
		if(errors[minErrori] < ui->toleranceEdit->text().toFloat())
		{			
			qDebug()<<" this is existing atom, set matrix ... ";
			// this is existing atom, set matrix
			m = mxs[floor((double)minErrori/2)];									
			double intpart;
			
			if(modf(minErrori/2, &intpart) > 0)
				reverse = 1;
				
			return n;
		}				
	}
	
	// there is no such atom here
	return -1;
}
		
void MainWindow::on_OneBtn_clicked(){}

void MainWindow::on_TwoBtn_clicked(){}

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
	//   qDebug()<< "len: " << len;
	//	qDebug()<<" nodes.size " << contour.nodes.size();
   std::list<fg::Curve> curves;
   fg::Integers intg;
   Splines splines;	 
	 	 
//	 contour.transform(m);
	 
	 if(reverse)
	 {
		 //qDebug()<<"contour.reverse()";
		 contour.reverse();
	 }
	 
//	 if(contour.area() < 0)
//	 {
//		 contour.reverse();
//	 }
	 
	 	 contour.transform(m);
	 	 
	 
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
    
    int glyphIndex = 0;
    for (fg::Glyphs::const_iterator it = package->font->glyphs.begin(); it != package->font->glyphs.end(); ++it, ++glyphIndex)
    {
			if((*it)->name.compare(item->text().toStdString()) == 0)
			{
        glyphToDraw(*(*it));
				qDebug()<<" name: " << item->text();
			}
    }
		
    this->update();
  }
}

double MainWindow::compareSplines(const Splines &splines1, const Splines &splines2, int len)
{
  double sumError = 0;

  // цикл сравнения
  for (uint i = 0; i < splines1.size() || i < splines2.size(); ++i) 
	{
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
  }

  double S = sumError * len;

  return S;
}

void MainWindow::paintEvent(QPaintEvent *e)
{    
	QPainter painter(this);
	
		
	painter.setRenderHint(QPainter::Antialiasing);		
//	painter.setPen(Qt::NoPen);
//	painter.setBrush(Qt::gray);
	
	painter.setPen(QPen(QColor(0, 0, 0), 0.5, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));
	
//	globalPath.setFillRule(Qt::WindingFill);				
	painter.drawPath(globalPath);
			
	
	
	
	painter.setPen(QPen(QColor(255, 0, 0), 1, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));
	
	// globalBBoxes.setFillRule(Qt::WindingFill);
	//	painter.drawPath(globalBBoxes);
	
	
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
	
	
//	painter.setPen(QPen(QColor(0, 255, 0), 1, Qt::SolidLine,
//											Qt::FlatCap, Qt::MiterJoin));
	
	QPainter atomPainter(this);		
	atomPainter.drawPath(globalAtoms);
	
	
	atomPainter.setPen(QPen(QColor(255, 0, 0), 1, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));
	
	if(ui->drawComparingLabel->isChecked())
	{
		atomPainter.drawPath(redPath);
	}
	
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
	globalAtoms = QPainterPath();
	redPath = QPainterPath();
	this->update();
	
  if(item != NULL )
  {    
		
		fg::Contours::iterator current = dict.begin();
		std::advance (current,item->listWidget()->currentRow());
		
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
		
//		(*it).open = false;
//		qDebug()<<"---------- area closed: " << (*it).area();
//		(*it).open = true;
//		qDebug()<<"---------- area opened: " << (*it).area();		
    //		contourToDebug((*it));				
		
		fg::Matrix mtx(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);
		// draw item
		QPainterPath p = contourToPath((*current), mtx).translated(QPointF(translation.x,bottom_line + translation.y));		
		p.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 2,2);				
//		p.addRect(p.boundingRect());
		globalAtoms.addPath(p);
		
		// here comparation with control atom 
    if(ui->drawComparingLabel->isChecked())
		{						
			//			p.addRect(p.boundingRect());							
			
			qDebug()<<" compareEdit: " << ui->compareEdit->text().toInt();	
			qDebug()<<" item: " << item->listWidget()->currentRow();	
			
			Rect r = (*comparing).boundingBox(false);
			list<Splines> representations;			
			vector<Matrix> mxs;
			mxs.reserve(4);
			
			int scaleXY[4][2] = {{1, 1}, {-1,1}, {1,-1}, {-1,-1}};			
			for (int i = 0; i < 4; ++i) 
			{
				int tX = r.left();
				int tY = r.top();
				
				if(scaleXY[i][0] == -1)
				{
					tX += r.width();										 										 
				}
									
				if(scaleXY[i][1] == -1)
				{
					tY += -r.height();
				}
																	
				mxs.push_back(Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY));						
//				representations.push_back(contourToSplines((*it2), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY)));
				representations.push_back(contourToSplines((*comparing), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY), 0));
				representations.push_back(contourToSplines((*comparing), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY), 1));
				
//				if(i == 2)
//				{
//					Contour cx = (*it2);
//					cx.reverse();
//					mxs.push_back(Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY));						
//					representations.push_back(contourToSplines((*it2), len, Matrix(scaleXY[i][0], 0,0, scaleXY[i][1], tX, tY)));										
//				}
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
			//			errors.clear();			
			//			double sumError;
			//			qDebug()<<"it.open" << (*it).open;
			//			qDebug()<<"it2.open" << (*it2).open;
			
			Splines s2 = contourToSplines((*current), len);
			
			int mxsCounter = 0;
			for (list<Splines>::iterator si = representations.begin(); si != representations.end(); ++si, ++mxsCounter) 
			{
				// give representation of it
				//				(*it2).transform(mxs[mxs_counter]);										
				Contour c = (*comparing);
				c.transform(mxs[floor((double)mxsCounter/2)]);
//				qDebug()<<" ------------ ";
//				coutMatrix("m:", mxs[floor((double)mxsCounter/2)]);
//				double intpart;
//				qDebug()<<" reversed: " << !(modf((double)mxsCounter/2, &intpart) == 0);// //floor((double)mxsCounter/2);
				
//				qDebug()<<" modf(mxsCounter/2, &intpart) " << modf((double)mxsCounter/2, &intpart) << " mxsCounter: " << mxsCounter;
//				qDebug()<<" modf(mxsCounter/2, &intpart) " << modf(2.3, &intpart) << " mxsCounter/2: " << (double)mxsCounter/2;
								
				//				qDebug()<<"    it2   ";
//				contourToDebug(c);
//				qDebug()<<"    ----   ";
//				contourToDebug((*it));								
//				qDebug()<<"------------------";
												
//				qDebug()<<" (*si).size() "<< (*si).size() << " s2.size() " << s2.size();
				double l2 = s2.size() * len;
				double l1 = (*si).size() * len;
				
//				qDebug()<<" l1 " << l1 << " l2 " << l2 ;
				double alen = (l2 + l1) / 2;
				
				double error = compareSplines((*si), s2, len);			//// remove twice calculation!!!!!									
				errors.push_back(error / alen);						
//				qDebug()<<" error: " << error / alen << " alen " << alen;
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
			qDebug()<<" reversed: " << !(modf((double)minErrori/2, &intpart) == 0);
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
