#include "utils.h"



XGlyph* findGlyphByIndex(XGlyphs &glyphs, int index)
{
    for(auto it = glyphs.begin(); it != glyphs.end(); it++)
    {
        XGlyph *g = &(*it);
        if(g->index == index)
            return g;
    }

    return NULL;
}

int getRepresentationsFromContour(const Contour &c, Representations &rs, int len, bool reverse)
{
    if(c.empty())
        return 0;

    for (int i = 0; i < 4; ++i)
    {

        int dX = 0;
        int dY = 0;
        Node lastn = (*prev(c.nodes.end()));
        dX = (AtomLib::SCALE_XY[i][0] == -1) ? lastn.p.x : -lastn.p.x;
        dY = (AtomLib::SCALE_XY[i][1] == -1) ? lastn.p.y : -lastn.p.y;


        if(!reverse)
            rs.push_back(Representation(Matrix(AtomLib::SCALE_XY[i][0], 0,0, AtomLib::SCALE_XY[i][1], 0, 0), false,
                    contourToSplines(c, len, Matrix(AtomLib::SCALE_XY[i][0], 0,0, AtomLib::SCALE_XY[i][1], 0, 0), 0), len));

        else
            rs.push_back(Representation(Matrix(AtomLib::SCALE_XY[i][0], 0,0, AtomLib::SCALE_XY[i][1], dX, dY), true,
                    contourToSplines(c, len, Matrix(AtomLib::SCALE_XY[i][0], 0,0, AtomLib::SCALE_XY[i][1], dX, dY), 1), len));

    }

    return 1;
}

std::stringstream intsToStr(const Ints &ints)
{
        std::stringstream s_result;
        Ints::const_iterator it = ints.begin();
        for(;it != ints.end(); ++it)
        {
                if(it != ints.begin())
                        s_result << ",";
                s_result << (*it);
        }

        return s_result;
}

InsertPoints getIpsForGlyph(InsertPoints &source, string glyphName)
{
    InsertPoints ips;

    for(auto it = source.begin(); it != source.end(); it++)
    {
        InsertPoint &ip = (*it);
        if(glyphName.compare(ip.glyphName) == 0)
        {
            ips.push_back((*it));
        }
    }
    return ips;
}

InsertPoints getIpsForContourNum(InsertPoints &source, int cnum)
{
    InsertPoints ips;

    for(auto it = source.begin(); it != source.end(); it++)
    {
        InsertPoint &ip = (*it);
        if(cnum == ip.contourNum)
        {
            ips.push_back((*it));
        }
    }
    return ips;
}


/*
inline double splinesSize(const Splines &s, double len)
{
        const PointSpline &last = *prev(s.end());
        const PointSpline &preLast = *prev(prev(s.end()));
        return (s.size()-2) * len + last.dist(preLast);
}


/*
double compareSplines(const Splines &splines1, const Splines &splines2, int len)
{
        double sumError = 0;

        for (uint i = 0; i < splines1.size() || i < splines2.size(); ++i)
        {

                if(i < splines1.size() && i < splines2.size())
                sumError += sqrt(pow(splines1[i].x - splines2[i].x,2) + pow(splines1[i].y - splines2[i].y,2));

                else if(i < splines1.size())
                {
                        sumError += sqrt(pow(splines1[i].x - (*prev(splines2.end())).x,2) + pow(splines1[i].y - (*prev(splines2.end())).y,2));
                }
                else if(i < splines2.size())
                {
                        sumError += sqrt(pow(splines2[i].x - (*prev(splines1.end())).x,2) + pow(splines2[i].y - (*prev(splines1.end())).y,2));
                }

        }

        // теперь нормализуем внутри функции
        double alen = (splinesSize(splines1, len) + splinesSize(splines2, len)) / 2;

        return sumError / alen;
}

*/

Splines contourToSplines(const Contour &contour, int len, Matrix m, bool reverse)
{
   std::list<fg::Curve> curves;
   fg::Integers intg;
   Splines splines;

         Contour copy = contour;
         copy.transform(m);

         if(reverse)
         {
                 copy.reverse();
                 // and new transform
         }

//	 qDebug()<<" FIRST.P " << (*(contour.nodes.begin())).p.x << " " <<(*(contour.nodes.begin())).p.y;

   Grapheme::contourToCurves(curves, copy);
         Grapheme::curvesToSplines(curves, splines, intg, len);
   return splines;
}

int findGlyphByIndex(fg::Package *package, Glyph &g, int index)
{
        for (fg::Glyphs::iterator it = package->font->glyphs.begin(); it != package->font->glyphs.end(); ++it)
        {
                if((*it)->index == index)
                {
                        g = *(*it);
                        return 1;
                }
        }

        qDebug() << "there is no such glyph";
        return 0;
}

Contours getContoursOfGlyph(const Glyph &g)
{
        Contours contours;
        const fg::Layer *layer = g.fgData()->findLayer("Body");
        for (fg::Contours::const_iterator it2 = layer->shapes.front().contours.begin();it2 != layer->shapes.front().contours.end(); ++it2)
        {
                contours.push_back((*it2));
        }
        return contours;
}

string getName(const XGlyphs &xg, int i)
{
    for(auto it = xg.begin(); it != xg.end(); it++)
    {
        if((*it).index == i)
            return (*it).name;
    }

    string s = "";
    return s;
}

string namesFromUsedIn(const XGlyphs &xglyphs, const Integers &usedIn)
{
    stringstream ss;
    for(auto it = usedIn.begin(); it != usedIn.end(); it++)
    {
        ss << "'" << getName(xglyphs, (*it)).c_str() << "',";
    }

//    if(std::find(usedIn.begin(), usedIn.end(), (*it).index) != usedIn.end()) // temp. remove me

    return ss.str();
}

bool compareNodes(Node &n1, Node &n2)
{
    if(n1.kind != n2.kind)
    {
        qDebug() << " n1.kind != n2.kind ";
        return false;
    }

    if(n1.p.x != n2.p.x)
    {
        qDebug() << " n1.p.x != n2.p.x ";
        return false;
    }

    if(n1.p.y != n2.p.y)
    {
        qDebug() << " n1.p.y != n2.p.y ";
        return false;
    }
    return true;
}

bool compareContoursByNodes(Contour &c1, Contour &c2) // true - the same, false - different
{
    if(c1.nodes.size() != c2.nodes.size())
    {
        qDebug() << "Contours are different..";
        return false;
    }

    auto it1 = c1.nodes.begin();
    auto it2 = c2.nodes.begin();
    int i = 0;
    for(; it1 != c1.nodes.end(); it1++, it2++, i++)
    {
        Node &n1 = (*it1);
        Node &n2 = (*it2);
        if(compareNodes(n1,n2))
        {
            qDebug() << "#"<< i << " are diff";
            return false;
        }
    }

    return true;
}

bool compareGlyphs(Glyph &g, XGlyph &xg) // 1 - the same
{
    Contours cs = getContoursOfGlyph(g);
    Contours xcs;// = xg.contours;
    getContoursFromXGlyph(xg, xcs);
    if(cs.size() != xcs.size())
        return 0;

    auto it = cs.begin();
    auto xit = xcs.begin();
    int i = 0;
    for(;it != cs.end(); it++, xit++, i++)
    {
        Contour &c  = (*it);
        Contour &xc = (*xit);

        qDebug() << "contours # " << i;

        printNodes(c);
        printNodes(xc);


//        if(compareContoursByNodes(c,xc))
//            return false;
    }
    return true;
}

void findGlyphByName(Glyphs &gs, Glyph &g, string &name)
{
    for(auto it = gs.begin(); it != gs.end(); it++)
    {
        Glyph &currentGlyph = *(*it);
        if(currentGlyph.name().compare(name) == 0)
            g = currentGlyph;
    }
}

int getContoursOnlyWithAtom(XGlyph &g, Atom *atom, Contours &result)
{
    if(g.contours.empty())
    {
        qDebug()<<" getContours::return 0";
        return 0;
    }


    for (auto it = g.contours.begin(); it != g.contours.end(); ++it)
    {
        Contour contour;

        getContourFromPositionedAtoms((*it), contour, atom);

        result.push_back(contour);
    }

    return 1;
}

int getContoursFromXGlyph(XGlyph &g, Contours &c)
{
    if(g.contours.empty())
    {
        qDebug()<<" getContours::return 0";
        return 0;
    }

    // anyway we need to remove zeros from constructed contour !!!
    // it looks like we need special rule for remove zeros

    for (auto it = g.contours.begin(); it != g.contours.end(); ++it)
    {
        Contour contour;
        getContourFromPositionedAtoms((*it), contour);

        qDebug()<<" contour pushed... size: " << contour.nodes.size();
        c.push_back(contour);
    }

    return 1;
}

void getAtoms(XContour &c)
{
    for (auto it = c.nodes.begin(); it != c.nodes.end(); ++it)
    {
        XNode &a = (*it);
        qDebug() << a.atom->contourIndex;
    }
}

int nodesToContourWithBreaks(Nodes &nodes, Contour &contour)
{
    for (Nodes::iterator ni = nodes.begin(); ni != nodes.end(); ++ni)
    {
        Node &n = (*ni);
        if(ni == nodes.begin())
        {
            n = *(ni);
            if (n.kind == Node::Move)
            qDebug()<<"Node::Move";
            else
            n.kind = Node::Move;
        }

        contour.addNode(n.kind, n.p, false);
    }
    return 1;
}

int nodesToContour(Nodes &nodes, Contour &contour)
{
    for (Nodes::iterator ni = nodes.begin(); ni != nodes.end(); ++ni)
    {
        Node &n = (*ni);

        if(contour.empty() || n.kind != Node::Move)
            // REFACTOR CONDITION?
            // bool contourIsEmptyOrNodeKindIsNotMove()
        {

            Node &lastNode = (*prev(contour.nodes.end()));
            Node newNode(n.kind, n.p);
            if(!compareNodes(lastNode, newNode))
                contour.addNode(n.kind, n.p, false);
        }
    }

    return 1;
}

int getSubNodesOfXNode(XNode &xn, Nodes &nodes)
{
    Atom transformedAtom = *xn.atom;
    if(xn.atom->nodes.empty())
    {
        return -1;
    }

    if(xn.reverse)
    {
        transformedAtom.reverse();
    }

    if((*transformedAtom.nodes.begin()).kind == Node::Move)
    (*transformedAtom.nodes.begin()).kind = Node::On;
    transformedAtom.transform(xn.m);
    nodes = transformedAtom.nodes;
    return 1;
}

InsertPoints findIn(XGlyph &g, Atom *a)
{
    InsertPoints ip;
    for(auto it = g.contours.begin(); it != g.contours.end(); it++)
    {
        XContour &c = (*it);        
        for(auto it2 = c.nodes.begin(); it2 != c.nodes.end(); it2++)
        {
            XNode &n = (*it2);

            if(n.atom == a)
                ip.push_back({g.name,
                              distance(g.contours.begin(), it),
                              distance(c.nodes.begin(), it2),
                              0});
        }
    }
    return ip;
}

int indexIn(Atom *atom, Atoms &dict)
{
    int i = 0;
    for(auto it = dict.begin(); it != dict.end(); it++)
    {
        Atom &a = (*it);
        if(&a == atom)
            return distance(dict.begin(), it);
    }

return -1;
}

int getContourFromPositionedAtoms(XContour &positionedAtoms, Contour &contour, Atom *useOnly)
{
    for (auto posAtomIt = positionedAtoms.nodes.begin(); posAtomIt != positionedAtoms.nodes.end(); ++posAtomIt)
    {
        XNode &currentNode = (*posAtomIt);
        Atom *currentAtom = currentNode.atom;
        Nodes nodes;


        getSubNodesOfXNode(currentNode, nodes);
        //currentNode.getNodes(nodes);


        if(currentAtom == useOnly) // RFC?
            nodesToContourWithBreaks(nodes, contour);
        else if(useOnly == NULL) // RFC?
            nodesToContour(nodes, contour);

    }
    return 1;
}

string nodeKind(int k)
{
    if (k == 0)
        return "Move";
    else if(k==1)
        return "On";
    else if(k==2)
        return "Off";
    else if(k==3)
        return "Curve";

    return "Nan";
}

void printNodes(fg::Contour &c)
{
    stringstream ss;
    for (auto it = c.nodes.begin(); it != c.nodes.end(); ++it)
    {
        fg::Node &n = (*it);
        ss << nodeKind(n.kind).c_str() << ":" << n.p.x << "," << n.p.y << "; ";
    }

    string s = ss.str();
    qDebug() << s.c_str(); //ss.c_str();
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

/*

void drawNodes(QPainterPath p, Point shift)
{				
	if(p.isEmpty())
		return;
	
		globalBBoxes.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 3,3);
		// ищем следующий isLine Node
		
		for(int i = 1; i < p.elementCount(); ++i)
		{			
			globalBBoxes.addEllipse(QPointF(p.elementAt(i).x + shift.x, p.elementAt(i).y + shift.y), 1,1);
//			if(p.elementAt(i).isLineTo() || p.elementAt(i).isMoveTo())
//			{
//				globalBBoxes.addEllipse(QPointF(p.elementAt(i).x, p.elementAt(i).y), 2,2);
//				i = p.elementCount();
//			}			
		}	
}

void contoursToDraw(Contours cs, Point tr, QPainterPath &path)
{     
	//qDebug()<<"contoursToDraw ";
	
	Point translation;
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
		path.addPath(p);
		
		if(ui->drawNodes->isChecked())
		drawNodes(p);
	} 
}
*/
