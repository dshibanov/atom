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
  //  globalBBoxes = QPainterPath();
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
  ui->splinesList->clear();

  
  for (fg::Glyphs::const_iterator it = font->glyphs.begin(); it != font->glyphs.end(); ++it, glyphIndex++)
  {

    ui->glyphsList->addItem(QString("#%1  %2  ").arg(glyphIndex).arg((*it)->name.c_str()));
    
    if((*it)->name.compare("I") == 0)
      qDebug()<<"I found gi = " << glyphIndex;    
  }

	
	return 1;
 
 }


void MainWindow::addToDraw(fg::Contour &c, const fg::Point &translation, int index)
{
  fg::Matrix mtx(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);
  QPainterPath p = contourToPath(c, mtx).translated(QPointF(translation.x, translation.y));
  globalPath.addPath(p);
  paths.append(Path2Draw(index, p));
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
	
	/*
		globalBBoxes.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 3,3);						
		// ищем следующий isLine Node
		
		for(int i = 1; i < p.elementCount(); ++i)
		{
			if(p.elementAt(i).isLineTo() || p.elementAt(i).isMoveTo())
			{
				globalBBoxes.addEllipse(QPointF(p.elementAt(i).x, p.elementAt(i).y), 2,2);
				i = p.elementCount();
			}			
		}
		*/
	
	
	
//	globalBBoxes.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 4,4);				
//	globalBBoxes.addEllipse(QPointF(p.elementAt(1).x, p.elementAt(1).y), 2,2);		
	
	//		globalBBoxes.addEllipse(QPointF(p.elementAt(2).x, p.elementAt(2).y), 1,1);		
	//		globalBBoxes.addEllipse(QPointF(p.elementAt(3).x, p.elementAt(3).y), 1,1);		
	//		globalBBoxes.addEllipse(QPointF(p.elementAt(4).x, p.elementAt(4).y), 1,1);		
	//		globalBBoxes.addEllipse(QPointF(p.elementAt(5).x, p.elementAt(5).y), 1,1);		
	
	/*
		for (int i = 2; i < p.elementCount(); ++i) 
		{
				globalBBoxes.addEllipse(QPointF(p.elementAt(i).x, p.elementAt(i).y), 1,1);
		}
		*/
	
	
	//    qDebug()<<"nodes drawn" << p.elementCount();
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
	
	//	list<Sample> samplesDict;
	//	list<int> samplesDict;
	
	
	// decompose current glyph's contours on samples	
	
	
	// getting glyph 
	Glyph* g;		
	int glyphIndex = 0;
	for (fg::Glyphs::const_iterator it = package->font->glyphs.begin(); it != package->font->glyphs.end(); ++it, ++glyphIndex)
	{
		if(ui->glyphsList->currentRow() == glyphIndex)
		{
			g = (*it);		
		}
	}
	
	// cycle by contours
	fg::Layer* layer; 
  layer = g->fgData()->findLayer("Body");
  for (fg::Contours::iterator it = layer->shapes.front().contours.begin();it != layer->shapes.front().contours.end(); ++it)
  {
		
		// decompose contour on particles
		// cycle by particles		  		
		
		for(Nodes::iterator ni = (*it).nodes.begin(); ni != (*it).nodes.end(); ++ni)
		{
		
		}
		
//    addToDraw((*it), fg::Point(translation.x,bottom_line + translation.y), 1);
		
  } 
	

}

void MainWindow::on_OneBtn_clicked(){}

void MainWindow::on_TwoBtn_clicked(){}


Contour MainWindow::contourToPoligone(Contour contour, int len)
{
   qDebug()<< "len: " << len;
   std::list<fg::Curve> curves;
   fg::Integers intg;
   Splines splines;

   Grapheme::contourToCurves(curves, contour);

   //qDebug()<<"curves.size(): " << curves.size();
   //Grapheme::curvesToSplines(curves, splines, intg, len);
   //Grapheme::curvesToSplines(curves, splines, intg, len);
    
    //static void curvesToSplines(const Curves &curves, Splines &contour, Integers &indexes, int len);

   return splinesToContour(splines);
}

Splines MainWindow::contourToSplines(Contour contour, int len)
{
   qDebug()<< "len: " << len;
   std::list<fg::Curve> curves;
   Splines splines;


   Grapheme::contourToCurves(curves, contour);
   //qDebug()<<"curves.size(): " << curves.size();
   //curves.erase(--curves.end());
   //Grapheme::curvesToSplines(curves, splines, intg, len);
   //Grapheme::curvesToSplines(curves, splines, intg, len);

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
//  stringstream log;
//  ComposedGlyph cg;
  if(item != NULL )
  {   
    // тут отрисовываем оригинальный глиф
    // находим нужный глиф по имени
    
    fg::Point center;
    center = fg::Point(ui->canvasLeft->geometry().x() + ui->canvasLeft->geometry().width()/2, ui->canvasLeft->geometry().y() + ui->canvasLeft->geometry().height()/2);
    
    int glyphIndex = 0;
    for (fg::Glyphs::const_iterator it = package->font->glyphs.begin(); it != package->font->glyphs.end(); ++it, ++glyphIndex)
    {
      if(item->listWidget()->currentRow() == glyphIndex)
        glyphToDraw(*(*it));
    }
    this->update();      
  }
}






//void MainWindow::shiftContour(double a, Contour &c){
//	for(Nodes::iterator it = c.nodes.begin(); it != c.nodes.end(); ++it){

		
//		int xN =(*it).p.x*cos(a) + (*it).p.y*sin(a);
//		int yN = (*it).p.x*sin(a) + (*it).p.y*cos(a);
		
//		//(*it).p.x = xN;// + (*it).p.x;				
//		//(*it).p.y = yN;

//	}	
//	fg::Matrix mtx(1, 0, a, 1, 0, 0);
//	c.transform(mtx);
	
//}


void MainWindow::paintEvent(QPaintEvent *e)
{    
	QPainter painter(this);
	
	painter.setRenderHint(QPainter::Antialiasing);		
	painter.setPen(Qt::NoPen);
		
	painter.setRenderHint(QPainter::Antialiasing);		
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::gray);
	painter.setPen(QPen(QColor(0, 0, 0), 0.5, Qt::SolidLine,
											Qt::FlatCap, Qt::MiterJoin));
	
	globalPath.setFillRule(Qt::WindingFill);				
	painter.drawPath(globalPath);
		
	//	QPainterPath p = contourToPath(c, mtx).translated(QPointF(translation.x, translation.y));
	//  globalPath.addPath(p);
	//  paths.append(Path2Draw(index, p));
	
	
	
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
}
