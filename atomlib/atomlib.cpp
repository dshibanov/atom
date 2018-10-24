#include "atomlib.h"
#include <QDebug>

//const int SCALEXY[4][2] = {{1, 1}, {-1,1}, {1,-1}, {-1,-1}};

const int AtomLib::SCALE_XY[4][2] = {{1, 1}, {-1,1}, {1,-1}, {-1,-1}};

AtomLib::AtomLib()
{
    compareBBox = true;
    minAtomsCount = 2;
    minUsedCount = 1;
    splineLength = 1;
    tolerance = 2;
    toleranceBBox = 0.2;
    toleranceEndPoints = 0;
    allGlyphs = true;
}

// This function calculates how many raw atoms
// (raw - consisted from only one minimal contour, raw atom has rawAtomsCount == 1)
// between (it1, it2)
inline int rawAtomsCountInSequence(XNodes::const_iterator it1, XNodes::const_iterator it2)
{	
    int count = 0;
    for(; it1 != it2; ++it1)
    {
        count += (*it1).atom->rawAtomsCount;
    }

    return count;
}

inline double splinesSize(const Splines &s, double len)
{			
    const PointSpline &last = *prev(s.end());
    const PointSpline &preLast = *prev(prev(s.end()));
    return (s.size()-2) * len + last.dist(preLast);
}

inline double bboxerror(const Point &p1, const Point &p2)
{
    return 2*p1.dist(p2) / (p1.len() + p2.len());
}

inline double average(double a, double b)
{
    return (a + b) / 2;
}

int replaceNodesSequenceOnNewNode(XContour &atoms, XNodes::iterator &start, XNodes::iterator &finish, XNode newNode)
{
    XNodes::iterator last = atoms.nodes.erase(start, finish);
    atoms.nodes.insert(last, newNode);
    start = prev(last);
    return 1;
}

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

    double alen = (splinesSize(splines1, len) + splinesSize(splines2, len)) / 2;

    return sumError / alen;
}

// return 1 if sequences the same, 0 - if differrent, -1 if error
inline bool AtomLib::isSameSequences(Ints seq1, Ints seq2, bool &reverse)
{
    if(seq1 == seq2)
    {
        reverse = false;
        return true;
    }
    else
    {
        std::reverse(seq1.begin(),seq1.end());
        if(seq1 == seq2)
        {
            reverse = true;
            return true;
        }
    }

    return false;
}

// this function returns list of indexes of atoms in range from it1 to it2
inline Ints getAtomsSequence(XNodes::const_iterator it1, XNodes::const_iterator it2)
{		
    Ints sequence;
    sequence.reserve(distance(it1,it2));
    for(;it1 != it2; ++it1)
    {
        const XNode &xn = (*it1);
        sequence.push_back(xn.atom->contourIndex);
    }

    return sequence;
}

int atomsToContour(const XContour &xcontour, XNodes::const_iterator xNodesIt, int count, Atom &c, bool open = true)
{
    c.components.reserve(count);
    for (; xNodesIt != xcontour.nodes.end() && count > 0; ++xNodesIt, --count)
    {
        const XNode &xn = (*xNodesIt);
        Contour atom = *xn.atom;

        if(xn.atom->components.empty())
            c.components.push_back(xn.atom->contourIndex);
        else
            c.components.insert(c.components.end(), xn.atom->components.begin(), xn.atom->components.end());

        if((*xNodesIt).reverse)
            atom.reverse();

        if((*atom.nodes.begin()).kind == Node::Move)
            (*atom.nodes.begin()).kind = Node::On;

        //Matrix me = (*xNodesIt).m;
        atom.transform((*xNodesIt).m);
        std::stringstream s_result;

        for (Nodes::iterator ni = atom.nodes.begin(); ni != atom.nodes.end(); ++ni)
        {
            Node &n = (*ni);
            if(c.empty() || n.kind != Node::Move)
                c.addNode(n.kind, n.p, false);
        }
    }
    c.open = open;
    return 1;
}

int AtomLib::findComposition(XContour &source, XNodes::iterator &sourceStart,
                             XContour &analyzed, XNodes::iterator &analyzedStart,
                             Atoms &dict, bool _addedToDict)


{
    bool addedToDict = _addedToDict;	// added if was succesful comparation (c1,c2)
    Atom c1;
    bool compositionFound = false;
    int combCount = 0;
    XNodes::iterator sourceFinish = sourceStart;
    combCount = getSequenceFinishNode(source, sourceFinish);//rawAtomsCountInSequence(sourceStart, sourceFinish);
    Ints sourceAtomsSequence = getAtomsSequence(sourceStart, sourceFinish);
    if(combCount < minAtomsCount)
        return 0;

    for (; analyzedStart != analyzed.nodes.end(); ++analyzedStart)
    {
        XNodes::iterator analyzedFinish = analyzedStart;
        getSequenceFinishNode(analyzed, analyzedFinish);
        Ints analizedAtomsSequence = getAtomsSequence(analyzedStart, analyzedFinish);

        if(sourceAtomsSequence.size() == analizedAtomsSequence.size())
        {
            bool reverse;
            if(isSameSequences(sourceAtomsSequence,analizedAtomsSequence,reverse))
            {
                if(c1.empty())
                {
                    atomsToContour(source, sourceStart, distance(sourceStart, sourceFinish), c1);
                }

                Atom c2;
                atomsToContour(analyzed, analyzedStart, distance(analyzedStart, analyzedFinish), c2);
                Point pfirst1 = (*c1.nodes.begin()).p;
                Point pfirst2 = (*(c2.nodes.begin())).p;
                c1.transform(Matrix(1,0,0,1,-pfirst1.x,-pfirst1.y));
                c2.transform(Matrix(1,0,0,1,-pfirst2.x,-pfirst2.y));

                Representations rs;
                getRepresentationsFromContour(c1, rs, splineLength, reverse);// with reverse
                Splines s2 = contourToSplines(c2, splineLength);

                Doubles errors;
                for (Representations::iterator r = rs.begin(); r != rs.end(); ++r)
                {
                    double error = compareSplines((*r).s, s2, splineLength);
                    errors.push_back(error);
                }

                int best = distance(errors.begin(), min_element(errors.begin(), errors.end()));

                if (errors[best] < tolerance) // compare result
                {
                    if(!compositionFound)
                        compositionFound = true;

                    if(!addedToDict)
                    {
                        addToDict(dict, c1);
                        addedToDict = true;
                    }

                    Representations::iterator ri = rs.begin();
                    advance(ri, best);
                    //Point joiningPoint = (*analyzedStart).firstJoiningPoint();
                    Point joiningPoint = getFirstJoiningPointOfXNode((*analyzedStart));
                    Matrix m = (*ri).m;
                    m.dx += joiningPoint.x;
                    m.dy += joiningPoint.y;
                    XNode newXNode = XNode((*prev(dict.end())), m, (*ri).reverse);
                    newXNode.atom->rawAtomsCount = combCount;

                    Ints seqX = sourceAtomsSequence;
                    if((*ri).reverse)
                        std::reverse(seqX.begin(), seqX.end());

                    if(!replaceNodesSequenceOnNewNode(analyzed, analyzedStart, analyzedFinish, newXNode))
                        qDebug()<<" Error occured...";
                }
            }
        }
    }


    if(compositionFound)
        return combCount;
    else
        return 0;
}

int AtomLib::addToDict(Atoms &dict, Atom &a)
{
    a.contourIndex = dict.size();
    dict.push_back(a);
    return dict.size();
}

int AtomLib::getSequenceFinishNode(const XContour &contour, XNodes::iterator &finishNode)
{
    int atomsCount = 0;
    int rawAtomsCount = 0;

    for (; finishNode != contour.nodes.end() && (rawAtomsCount < minAtomsCount || atomsCount < 2);
         finishNode++, ++atomsCount)
    {
        rawAtomsCount += (*finishNode).atom->rawAtomsCount;
    }

    return rawAtomsCount;
}

int AtomLib::getRepresentationsFromContour(const Contour &c, Representations &rs, int len, bool reverse)
{
    if(c.empty())
        return 0;

    for (int i = 0; i < 4; ++i)
    {
        int dX = 0;
        int dY = 0;
        Node lastn = (*prev(c.nodes.end()));
        dX = (SCALE_XY[i][0] == -1) ? lastn.p.x : -lastn.p.x;
        dY = (SCALE_XY[i][1] == -1) ? lastn.p.y : -lastn.p.y;
        if(!reverse)
            rs.push_back(Representation(Matrix(SCALE_XY[i][0], 0,0, SCALE_XY[i][1], 0, 0), false,
                    contourToSplines(c, len, Matrix(SCALE_XY[i][0], 0,0, SCALE_XY[i][1], 0, 0), 0), len));
        else
            rs.push_back(Representation(Matrix(SCALE_XY[i][0], 0,0, SCALE_XY[i][1], dX, dY), true,
                    contourToSplines(c, len, Matrix(SCALE_XY[i][0], 0,0, SCALE_XY[i][1], dX, dY), 1), len));
    }

    return 1;
}

Splines AtomLib::contourToSplines(const Contour &contour, int len, Matrix m, bool reverse)
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

// return index of atom in dict
// or -1 if there is no such atom in dict
int AtomLib::getDictIndexOfAtom(const Contour &atom, Atoms &dict, Matrix &m, int &reverse)
{		
    // getting splines from atom
    vector<double> errors;
    errors.reserve(8);
    Splines s2 = contourToSplines(atom, splineLength);
    Splines::iterator sF = s2.begin();
    std::advance(sF, 1);
    Rect r2 = atom.transformed(Matrix()).boundingBox(Matrix(), false);
    Point p2 = Point(abs(r2.left() - r2.right()), abs(r2.bottom() - r2.top()));
    //double l2 = splinesSize(s2, splineLength);

    //qDebug()<<" dict.size: " << dict.size();
    int n = 0;
    for (Atoms::iterator it = dict.begin(); it != dict.end(); ++it, ++n)
    {
        //qDebug()<<"....next";
        errors.clear();

        double bboxError = 0;
        if(compareBBox)
        {
            Rect r1 = (*it).transformed(Matrix()).boundingBox(Matrix(), false);
            Point p1 = Point(abs(r1.left() - r1.right()), abs(r1.bottom() - r1.top()));
            bboxError = bboxerror(p1,p2);
        }


        if(bboxError < toleranceBBox)
        {
            list<Representations>::iterator ri = representationsDict.begin();
            std::advance(ri,n);
            Representations &rprs =  (*ri);
            for (Representations::iterator si = rprs.begin(); si != rprs.end(); ++si)
            {
                double endPointsError = (*prev(atom.nodes.end())).p.dist((*prev((*si).s.end())));

                if(endPointsError <= toleranceEndPoints)
                    errors.push_back(compareSplines((*si).s, s2, splineLength));
                else
                    errors.push_back(tolerance * 2);
            }

            if(!errors.empty())
            {
                // find minimal error
                int minErrori = 0;
                for (int i = 0; i < errors.size(); ++i)
                {
                    if(errors[minErrori] > errors[i])
                        minErrori = i;
                }

                if(errors[minErrori] < tolerance)
                {
                    Representations::iterator  ri = rprs.begin();
                    std::advance(ri, minErrori);
                    m = (*ri).m;
                    reverse = (*ri).reverse;
                    return n;
                }
            }
        }
    }


    // there is no such atom here
    return -1;
}

// ------

inline bool layerIsNotEmpty(const fg::Layer *layer)
{
    return (layer && layer->countNodes() > 0);
}

void fixClosure(Contour &c)
{
    qDebug() << "fixClosure..";
    Node &firstNode = (*c.nodes.begin());
    Node &lastNode = (*prev(c.nodes.end()));
    if(firstNode.p != lastNode.p)
    {
        qDebug() << " CLOSURE IS BAD";
        c.addNode(Node::NodeType::On, firstNode.p);
    }
}

// FIXME: correct is.. const fg::Font* font..
int AtomLib::decompose(fg::Font* font, XGlyphs &aglyphs, Atoms &cdict)
{
    atomsDict = &cdict;
    agGlyphs = &aglyphs;
    agGlyphs->clear();
    atomsDict->clear();
    representationsDict.clear();

    //cout << "in decompose.. font->glyphs.size() " << font->glyphs.size() << endl;
    //cout << "settings.from " << settings.from << " settings.to " << settings.to << endl;

    fg::Glyphs::iterator glyphsIt = font->glyphs.begin();
    int currentGlyphIndex = 0;
    if(!allGlyphs)
    {
        advance(glyphsIt, from);
        currentGlyphIndex = from;
    }

    Integers needFul = {37,39,40,43,44}; // temp. remove me

    // decompose --
    for (; glyphsIt != font->glyphs.end() && (allGlyphs == true || currentGlyphIndex < to); ++glyphsIt, ++currentGlyphIndex)
    {
     //if(std::find(needFul.begin(), needFul.end(), currentGlyphIndex) != needFul.end()) // temp. remove me
        if (layerIsNotEmpty((*glyphsIt)->bodyLayer()))
        {
            XContours contours;

            Glyph *currentGlyph = (*glyphsIt);
            qDebug()<<"name: " << currentGlyph->name().c_str() << " # "<< currentGlyphIndex;
            Contours &currentGlyphsContours = currentGlyph->fgData()->findLayer("Body")->shapes.front().contours;

            for (fg::Contours::iterator contourIt = currentGlyphsContours.begin(); contourIt != currentGlyphsContours.end(); ++contourIt)
            {
                // так вот тут нужно сделать проверку..
                // ну если у нас нет замыкания то нужно  его добавить..

                //Contour &currentContour = (*contourIt);
                Contour currentContour = (*contourIt);
                fixClosure(currentContour);

                XContour contour;
                Atom atom;
                atom.open = true;

                for(Nodes::iterator currentNode = currentContour.nodes.begin(); currentNode != currentContour.nodes.end(); ++currentNode)
                {
                    if(currentNode != currentContour.nodes.begin() && ((*currentNode).kind == Node::Move || (*currentNode).kind == Node::On))
                    {
                        Matrix m;
                        XNode xnode;
                        atom.nodes.push_back((*currentNode));
                        Point lastPoint = (*prev(atom.nodes.end())).p;
                        Point firstPoint = (*(atom.nodes.begin())).p;
                        atom.transform(Matrix(1,0,0,1,-firstPoint.x,-firstPoint.y));
                        int reverse = 0;

                        // check for new item
                        if(atomsDict->empty())
                        {
                            atom.open = true;
                            atom.usedIn.push_back(currentGlyphIndex);
                            addToDict(*atomsDict, atom);
                            Atom &lastAtom = *prev(atomsDict->end());
                            xnode = XNode(lastAtom, Matrix(1, 0, 0, 1, firstPoint.x, firstPoint.y), 0);

                            Representations rs = Representations();
                            getRepresentationsFromContour(lastAtom, rs, splineLength, false);
                            getRepresentationsFromContour(lastAtom, rs, splineLength, true);
                            representationsDict.push_back(rs);
                        }
                        else
                        {
                            int atomIndex = getDictIndexOfAtom(atom, *atomsDict, m, reverse);

                            if(atomIndex == -1)
                            {
                                atom.open = true;
                                addToDict(*atomsDict, atom);
                                Atom &lastAtom = *prev(atomsDict->end());
                                xnode = XNode(lastAtom, Matrix(1,0,0,1, firstPoint.x, firstPoint.y), 0);

                                Representations rs = Representations();
                                getRepresentationsFromContour(lastAtom, rs, splineLength, false);
                                getRepresentationsFromContour(lastAtom, rs, splineLength, true);
                                representationsDict.push_back(rs);
                            }
                            else
                            {
                                Atoms::iterator ci = atomsDict->begin();
                                std::advance(ci, atomIndex);
                                Matrix mx = m;
                                mx.dx += firstPoint.x; mx.dy += firstPoint.y;
                                xnode = XNode((*ci), mx, reverse);
                                //(*ci).usedIn.push_back(currentGlyphIndex);
                            }
                        }

                        contour.nodes.push_back(xnode);

                        // and here start to new atom
                        atom.clear();
                        atom.usedIn.clear();
                        atom.open = true;
                        if((*currentNode).kind == Node::NodeType::On)
                        {
                            // add Move to the beginning of atom
                            atom.nodes.push_back(Node(Node::On, lastPoint));
                        }
                    }

                    atom.addNewNode((*currentNode));
                }
                contours.push_back(contour);
            }

            // agdict - it is dict with AGlyphs, contains source glyphs but represented by different container
            agGlyphs->push_back(XGlyph(contours,currentGlyph->name().c_str(), currentGlyph->index));
        }
    }

    return atomsDict->size();
}

int AtomLib::compressXGlyphs(XGlyphs &aglyphs, Atoms &cdict)//, AtomSettings *as)
{	
    for (XGlyphs::iterator g = aglyphs.begin(); g != aglyphs.end(); ++g)
    {
        for (auto currentContour = (*g).contours.begin(); currentContour != (*g).contours.end(); ++currentContour)
        {
            XContour &source = (*currentContour);
            auto sourceStartNode = source.nodes.begin();

            while(sourceStartNode != prev(source.nodes.end()))
            {
                bool compositionFound = false;
                auto analyzedStartNode = sourceStartNode;
                int itemsCount; // количество элементов! в найденной исходной комбинации

                auto sourceFinishNode = sourceStartNode;

                if (getSequenceFinishNode(source, sourceFinishNode) >= minAtomsCount)
                {
                    // ----------------- for current contour -----------------------------------
                    getSequenceFinishNode(source, analyzedStartNode);

                    int result = findComposition(source, sourceStartNode, source, analyzedStartNode, cdict);
                    if(result > 0)
                    {
                        compositionFound = true;
                    }

                    // ------- for another contours of the glyph ------
                    for (auto ni = next(currentContour); ni != (*g).contours.end(); ++ni)
                    {
                        XContour &atoms = (*ni);
                        analyzedStartNode = atoms.nodes.begin();
                        auto analizedFinishNode = analyzedStartNode;

                        int result = findComposition(source, sourceStartNode, atoms, analyzedStartNode, cdict, compositionFound);
                        if(result > 0)
                        {
                            compositionFound = true;
                        }
                    }

                    // ------- for another glyphs --------
                    if(g != prev(aglyphs.end()))
                    {
                        //qDebug()<<"----------------- for another glyphs";
                        for (XGlyphs::iterator g2 = next(g); g2 != aglyphs.end(); ++g2)
                        {
                            XGlyph &ag = (*g2);
                            for (auto ni = ag.contours.begin(); ni != ag.contours.end(); ++ni)
                            {
                                XContour &atoms = (*ni);
                                analyzedStartNode = atoms.nodes.begin();

                                int result = findComposition(source, sourceStartNode, atoms, analyzedStartNode, cdict, compositionFound);
                                if(result > 0)
                                {
                                    itemsCount = 0; // количество элементов! в найденной исходной комбинации
                                    int count = 0;
                                    for (auto it = sourceStartNode; it != source.nodes.end() && count < result; ++it, itemsCount++)
                                    {
                                        count += (*it).atom->rawAtomsCount;
                                    }

                                    compositionFound = true;
                                }
                            }
                        }
                    }
                }

                if(compositionFound)
                {
                    Atom c1;
                    atomsToContour(source, sourceStartNode, 2, c1);
                    Point pfirst1 = (*c1.nodes.begin()).p;
                    XNode newNode = XNode((*prev(cdict.end())), Matrix(1, 0, 0, 1, pfirst1.x, pfirst1.y), false);

                    auto sourceFinishNode = sourceStartNode;
                    getSequenceFinishNode(source, sourceFinishNode);

                    if(!replaceNodesSequenceOnNewNode(source, sourceStartNode, sourceFinishNode, newNode))
                        qDebug()<<" Error occured...";

                    sourceStartNode = source.nodes.begin();
                }
                else
                    sourceStartNode++;

            }
        }
    }

    deleteUnUsed(aglyphs, cdict);
    return cdict.size();
}

int AtomLib::deleteUnUsed(XGlyphs &agdict, Atoms &dict)
{    
    int j = 0;
    for (Atoms::iterator it = dict.begin(); it != dict.end(); j++)
    {
        Atom &a = (*it);
        Ints currentUsedIn = getUsedIn(agdict, a);
        a.usedIn = currentUsedIn;

        if(currentUsedIn.size() == 0)
        {
            dict.erase(it++);
        }
        else
        {
            ++it;
        }
    }
    return 1;
}

Ints AtomLib::getUsedIn(XGlyphs &agdict, Atom &a)
{
    Ints result;
    for(auto currentXGlyph = agdict.begin(); currentXGlyph != agdict.end(); currentXGlyph++)
    {
        XGlyph &xg = (*currentXGlyph);
        for(auto currentContour = xg.contours.begin(); currentContour != xg.contours.end(); currentContour++)
        {
            XContour &xc = (*currentContour);
            for(auto cnit = xc.nodes.begin(); cnit != xc.nodes.end(); cnit++)
            {
                XNode &xn = (*cnit);
                if(xn.atom == &a)
                    result.push_back(xg.index);
                //if(xn.contourIndex != a.contourIndex)
                //  qDebug() << "contourIndexes are different for &a: " << &a ;
            }
        }
    }

    return result;
}

int AtomLib::compress(fg::Font* font,  XGlyphs &aglyphs, Atoms &cdict)
{    
    decompose(font, aglyphs, cdict);
    compressXGlyphs(aglyphs, cdict);
    return 1;
}

int AtomLib::stats(fg::Font* font, XGlyphs &aglyphs, Atoms &regularDict, Atoms &statDict)
{	    
    decompose(font, aglyphs, regularDict);
    compressXGlyphs(aglyphs, regularDict);
    statDict = regularDict;
    removeExcessAtoms(statDict, minAtomsCount);
    return 1;
}

int AtomLib::stats2(fg::Font* font, XGlyphs &aglyphs, Atoms &regularDict, list<Atom*> &statLinks)
{
    decompose(font, aglyphs, regularDict);
    compressXGlyphs(aglyphs, regularDict);
    statLinks = suitableAtomsIndexes(regularDict, minAtomsCount);
    fillInsertPoints(aglyphs, statLinks);
    //statDict = regularDict;
    //removeExcessAtoms(statDict, minAtomsCount);
    return 1;
}

int AtomLib::fillInsertPoints(XGlyphs &glyphs, list<Atom*> &dict)
{
    int ind = 0;
    for(auto it = dict.begin(); it != dict.end(); it++, ind++)
    {
        Atom *a = (*it);

//        qDebug() << "b: " << intsToStr(a->usedIn).str().c_str();

        sort(a->usedIn.begin(),a->usedIn.end());
//        qDebug() << "a: " << intsToStr(a->usedIn).str().c_str();

        qDebug() << "link index: " << ind;
        int prev_i = -1;
        InsertPoints ips;
        for(auto it2 = a->usedIn.begin(); it2 != a->usedIn.end(); it2++)
        {
            int i = (*it2);
            if(i != prev_i)
            {
                prev_i = i;
                XGlyph *g = findGlyphByIndex(glyphs, i);
                if(g != NULL)
                {
//                    ips.push_back(findIn(*g, a));
                    InsertPoints cr = findIn(*g, a);
                    ips.insert(ips.end(), cr.begin(), cr.end());


//                    qDebug() << ips.size();
//                    printips(ips);
                    a->insertPoints = ips;
                }
                else
                    qDebug() << "NULL";
            }
        }
        printips(ips);
    }
    return 1;
}

XGlyph* AtomLib::findGlyphByIndex(XGlyphs &glyphs, int index)
{
    for(auto it = glyphs.begin(); it != glyphs.end(); it++)
    {
        XGlyph *g = &(*it);
        if(g->index == index)
            return g;
    }

    return NULL;
}

void AtomLib::printips(InsertPoints &ips)
{
    for(auto it = ips.begin(); it != ips.end(); it++)
    {
        InsertPoint &ip = (*it);
        qDebug() << "g: " << ip.glyphName.c_str() << " cnum: " << ip.contourNum << " snode: " << ip.startNodeNum;
    }
}


InsertPoints AtomLib::findIn(XGlyph &g, Atom *a)
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

/*
XGlyph* AtomLib::findGlyphByIndex(XGlyphs &glyphs, int index)
{

}


InsertPoints AtomLib::findIn(XGlyph &g, Atom *a)
{
    InsertPoints ip;
    for(auto it = g.contours.begin(); it != g.contours.end(); it++)
    {
        XContour &c = (*it);
        // by nodes
        for(auto it2 = c.nodes.begin(); it2 != c.nodes.end(); it2++)
        {
            XNode &n = (*it2);


//            qDebug() << a;//&n.atom;
            if(n.atom == a)
                ip.push_back({distance(g.contours.begin(), it),
                              distance(c.nodes.begin(), it2)});


            // тут достаточно просто сравнить атомы..
            // ну т.е. если мы нашли что вот тут стоит такой то атом то все
            // это означает что мы нашли где он используется..
            // но... как нам тогда найти узел, ну стартовый узел в исходном глифе
            // в котором у нас он не состоит из атомов

            // тогда нам нужно посчитать количество узлов..
            // тогда надо взять от той точки в которой мы сейчас находимся
            // и.. собрать все xnodes в контур и посчитать количество узлов в нем..

             // ок, ладно для начала просто найдем точку в XGlyph..
        }
    }
    return ip;
}
*/

int AtomLib::removeExcessAtoms(Atoms &dict, int maCount)
{
    auto it = std::remove_if(dict.begin(), dict.end(), [maCount](Atom &a)
    {
            if (a.rawAtomsCount < maCount)
                return true;
            return false;
    });

    dict.erase(it, dict.end());

    return 1;
}

list<Atom*> AtomLib::suitableAtomsIndexes(Atoms &dict, int maCount)
{
    list<Atom*> result;
    for(auto it  = dict.begin(); it != dict.end(); it++)
    {
        Atom &a = (*it);
        if((*it).rawAtomsCount >= maCount)
            result.push_back(&a);
    }

    return result;
}

int AtomLib::removeExcessAtoms2(Atoms &dict, int value, int type)
{
    if (type == MIN_ATOMS)
    {
        auto it = std::remove_if(dict.begin(), dict.end(), [value](Atom &a)
        {
                if(a.rawAtomsCount < value)
                return true;
                return false;
        });
        dict.erase(it, dict.end());
    }
    else if(type == MIN_USES_IN_GLYPHS)
    {
        auto it = std::remove_if(dict.begin(), dict.end(), [value](Atom &a)
        {
                if(a.usedIn.size() < value)
                return true;
                return false;
        });
        dict.erase(it, dict.end());
    }
    else if(type == MIN_USES_IN_DIFFERENT_GLYPHS)
    {
        auto it = std::remove_if(dict.begin(), dict.end(), [value](Atom &a)
        {
                if(a.rawAtomsCount < value)
                return true;
                return false;
        });
        dict.erase(it, dict.end());
    }

    return 1;
}

int AtomLib::substituteSimpleNodes(XContour &xc)
{
    for(auto it = xc.nodes.begin(); it != xc.nodes.end(); )
    {
        XNode &currentNode = (*it);
        Atom &a = *(currentNode.atom);


        if(a.rawAtomsCount == 1) // atoms didnt use in compression
        {
            // 1. remove currentNode from xc
            xc.nodes.erase(it++);

            // 2. insert atom's nodes
            //xc.nodes.insert(it, a.nodes.begin(), a.nodes.end());
            xc.nodes.insert(it, xc.nodes.begin(), xc.nodes.end());
        }
        else
            it++;
    }

    return 1;
}

Point AtomLib::getFirstJoiningPointOfXNode(XNode &xn)
{
    Point p;

    if(xn.reverse)
        p = (*prev(xn.atom->nodes.end())).p;
    else
        p = (*(xn.atom->nodes.begin())).p;

    p.transform(xn.m);
    return p;
}


