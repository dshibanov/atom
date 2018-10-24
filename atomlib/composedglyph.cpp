#include "basics.h"
#include "options.h"
#include "glyph.h"
#include "math.h"

#include "curve.h"
#include "blend.h"

using namespace fg;

#include "composedglyph.h"

#include <vector>
#include <QtCore>

ComposedGlyph::ComposedGlyph()
{
  originalIndex = -1;
}

ContourData::ContourData(const fg::Contour &_contour) :
  contour(_contour)
{
	if(contour.area() < 0)
		contour.reverse();
	
  updateBoundingBox();
}

void ContourData::updateBoundingBox()
{
  boundingBox = contour.transformed(Matrix()).boundingBox(Matrix(), true);
}


const Splines& ContourData::getSplines(int len, bool remade)
{
  if (splines.empty() || remade == true)
  {		
		Contour tr_contour = contour;		
		
		if(tr_contour.area() < 0){	
			tr_contour.reverse();	
		}
		
    // трансформируем безье в ломаную
    //std::vector<fg::Curve> curves;
		list<fg::Curve> curves;
    fg::Integers intg;

    Grapheme::contourToCurves(curves, tr_contour);
    Grapheme::curvesToSplines(curves, splines, intg, len);
  }

  return splines;
}

void ContourData::getTransformedSplines(Splines &tr_splines, Matrix m, int len)
{
	// трансформируем безье в ломаную
		
	Contour tr_contour = contour;		
	tr_contour.transform(m);
	
	if(tr_contour.area() < 0)
	{
		tr_contour.reverse();
	}
	
  //std::vector<fg::Curve> curves;
	list<fg::Curve> curves;
  fg::Integers intg;	

  Grapheme::contourToCurves(curves, tr_contour);	
  Grapheme::curvesToSplines(curves, tr_splines, intg, len);			
}

PositionedGrapheme::PositionedGrapheme(Grapheme* _g, fg::Matrix _matrix, int _index):
  index(_index),
  grapheme(_g),
  matrix(_matrix)
{
	Matrix m = matrix;
}

PositionedGrapheme::PositionedGrapheme(Grapheme* _g, fg::Point _pos, int _index):
  index(_index),
  grapheme(_g),
	matrix(Matrix(1,0,0,1, _pos.x, _pos.y))
{	
	Matrix m = matrix;	
}

PositionedGrapheme::PositionedGrapheme() :
  index(-1),
  grapheme(NULL)
{
}

Grapheme::Grapheme() :
  graphemeIndex(-1),
  glyphIndex(-1)
{
}

Grapheme::Grapheme(int index) :
  graphemeIndex(index),
  glyphIndex(-1)
{
}


/**
 * @brief Grapheme::sortContours - sort contours by theirs box size
 */
void Grapheme::sortContours()
{
  cdata.sort(cCompare);  
}

/**
 * @brief Grapheme::bbox - return bbox by whole graphema
 * @return
 */
fg::Rect& Grapheme::bbox(bool remade)
{
  if (remade || bboxRect.empty())
  {
    bboxRect.clear();
    for (ContoursData::iterator it = cdata.begin(); it != cdata.end(); ++it)
      bboxRect |= it->boundingBox;
  }

  return bboxRect;
}

bool Grapheme::cCompare(ContourData d1, ContourData d2)
{  
	
  if (d1.boundingBox.width() * d1.boundingBox.height()  > d2.boundingBox.width() * d2.boundingBox.height())
  {		
    return true;
  }

  return false;	
}

bool Grapheme::contourToCurves(list<Curve> &curves, const fg::Contour &contour)
{
  if (contour.empty())
    return false;

  curves.clear();  

  fg::Curve c;
  fg::cNode node0 = contour.nodes.begin();
  fg::cNode node = node0;
  node++;

  bool spline = false;
  bool curve = false;

  fg::Point q0, q1, q2;

  for (; node != contour.nodes.end(); ++node)
  {
    switch (node->kind)
    {
    case fg::Node::Off:
    {
      if (spline)
      {
        q2 = (q1 + node->p) / 2;
        curves.push_back(fg::Curve(q0, q1, q2));
        q0 = q2;
        q1 = node->p;
      }
      else
      {
        q0 = node0->p;
        q1 = node->p;
        spline = true;
      }

      break;
    }

    case fg::Node::On:
    case fg::Node::Move:

      if (spline)
      {
        curves.push_back(fg::Curve(q0, q1, node->p));
        spline = false;
        curve = false;
        break;
      }

      if (curve)
      {
        c.q3 = node->p;
        curves.push_back(c);
        curve = false;
        break;
      }

      curves.push_back(fg::Curve(node0->p, node->p));
      break;

    case fg::Node::Curve:
			
      if (curve)
      {
        c.q2 = node->p;
        break;
      }

      c.q0 = node0->p;
      c.q1 = node->p;
      curve = true;
      break;
    }

    node0 = node;
  }
	
  node = contour.nodes.begin();

  if(contour.open == false && node0->p != node->p)
	{    
    node0 = contour.nodes.begin();
    node = --(contour.nodes.end());
    curves.push_back(fg::Curve(node->p, node0->p));
  }
	
  return true;
}

void Grapheme::curvesToSplines(const list<Curve> &curves, Splines &contour, Integers &indexes, int len)
{
  unsigned int j;

  contour.clear();
  indexes.clear();

  contour.reserve(curves.size() * 5);
  indexes.reserve(curves.size());

  PointSpline sp;

  sp.time = 0;
  sp.assign(curves.front().q0);
  sp.curve = curves.size() - 1;

  indexes.push_back(0);

  j = 0;
  int l,L, restl = 0;
  L = len * BLEND_CURVE_PRECISION;  
	
	for(list<Curve>::const_iterator it = curves.begin(); it != curves.end(); ++it)
	{						    
    const Curve &c = (*it);
    l = (int)fg::round(c.len(10) * BLEND_CURVE_PRECISION);
    // restl -  сколько длинны текущей кривой осталось под сплайны

    if(restl + l > 0)
		{
      // добавляем точку
      Curve cs(c.q0 * BLEND_CURVE_PRECISION, c.q1 * BLEND_CURVE_PRECISION, c.q2 * BLEND_CURVE_PRECISION, c.q3 * BLEND_CURVE_PRECISION);

      if(restl > 0)
			{
        sp.time = (double)(L - restl)/ (double)l;
        restl = l - (L - restl);
      }
      else
			{
        sp.time = (double)(abs(restl)) / (double)l;
        restl = l + restl;
      }

      sp.assign(cs.point(sp.time) * (1.0 / BLEND_CURVE_PRECISION));
      sp.curve = j;
      contour.push_back(sp);

      int q = 1;
      while(restl - q*L > 0)
			{
        sp.time = (double)(L*q + l - restl) / (double)(l);        
        sp.assign(cs.point(sp.time) * (1.0 / BLEND_CURVE_PRECISION));
        sp.curve = j;
        contour.push_back(sp);
        q++;
      }

      restl = restl - q*L;

    }
    else
      restl +=  l;

    j++;
  }

  if((--(contour.end()))->time < 1){

    // добавляем последний сплайн
    const Curve &c = (curves.back());
    l = (int)fg::round(c.len(10) * BLEND_CURVE_PRECISION);
    Curve cs(c.q0 * BLEND_CURVE_PRECISION, c.q1 * BLEND_CURVE_PRECISION, c.q2 * BLEND_CURVE_PRECISION, c.q3 * BLEND_CURVE_PRECISION);
    sp.time = 1;
    sp.assign(cs.point(sp.time) * (1.0 / BLEND_CURVE_PRECISION));
    sp.curve = curves.size() - 1;
    contour.push_back(sp);
  }
}

void Grapheme::printBBox(){
	
	qDebug()<<"		G index: " << this->graphemeIndex << " and rects->"; 
	for (ContoursData::iterator it = cdata.begin(); it != cdata.end(); ++it) {
    Rect rect = it->contour.transformed(Matrix()).boundingBox(Matrix(), false);
		
		
		qDebug()<<"			rect " << rect.left() << " " << rect.top()<<" " << rect.right() << " " << rect.bottom();	
	}	
}

/**
 * @brief Grapheme::normalize - нормализует графему, сдвигая ее контуры таким образом,
 * чтобы (left,top) bbox'а было (0,0)
 */
void Grapheme::normalize()
{  
  sortContours();  	
  Rect bbox = this->bbox();
  fg::Matrix mtx = fg::Matrix(1,0,0,1, -bbox.left(),-bbox.top());
  for(ContoursData::iterator it = cdata.begin(); it != cdata.end(); ++it)
  {
    it->contour.transform(mtx);
    it->updateBoundingBox();
  }
}

void Grapheme::addContours(const Contours &contours)
{
  for (Contours::const_iterator it = contours.begin(); it != contours.end(); ++it)
  {
    cdata.push_back(ContourData(*it));
  }
}
