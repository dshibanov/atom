#ifndef UTILS_H
#define UTILS_H

#include <QDebug>
#include <QPainterPath>

#include "basics.h"
#include "glyph.h"
#include "options.h"
#include "font.h"
#include "curve.h"
#include "blend.h"
#include "parser.h"

#include <sstream>
#include <string>


#include "atomlib.h"
#include "composedglyph.h"
#include "aglyph.h"


int indexIn(Atom *atom, Atoms &dict);
InsertPoints getIpsForGlyph(InsertPoints &source, string glyphName);
InsertPoints getIpsForContourNum(InsertPoints &source, int cnum);
XGlyph* findGlyphByIndex(XGlyphs &glyphs, int index);
InsertPoints findIn(XGlyph &g, Atom *a);
inline double splinesSize(const Splines &s, double len);
int getRepresentationsFromContour(const Contour &c, Representations &rs, int len, bool reverse);
std::stringstream intsToStr(const Ints &ints);
double compareSplines(const Splines &splines1, const Splines &splines2, int len);
Splines contourToSplines(const Contour &contour, int len, Matrix m = Matrix(1,0,0,1), bool reverse = false);
QPainterPath contourToPath(const fg::Contour &contour, const fg::Matrix &layerMatrix);
//int readFontFile(QString path, QString outPath);
int findGlyphByIndex(fg::Package *package, Glyph &g, int index);
int getContourFromPositionedAtoms(XContour &positionedAtoms, Contour &contour, Atom *useOnly = NULL);
int getContoursOnlyWithAtom(XGlyph &g, Atom *atom, Contours &result);
int getContoursFromXGlyph(XGlyph &g, Contours &c);
Contours getContoursOfGlyph(const Glyph &g);
void printNodes(fg::Contour &c);
string getName(const XGlyphs &xg, int i);

string namesFromUsedIn(const XGlyphs &xglyphs, const Integers &usedIn);
bool compareNodes(Node &n1, Node &n2);

bool compareContoursByNodes(Contour &c1, Contour &c2); // true - the same, false - different
bool compareGlyphs(Glyph &g, XGlyph &xg); // 1 - the same
void findGlyphByName(Glyphs &gs, Glyph &g, string &name);

void getAtoms(XContour &c);

int nodesToContourWithBreaks(Nodes &nodes, Contour &contour);
int nodesToContour(Nodes &nodes, Contour &contour);
int getSubNodesOfXNode(XNode &xn, Nodes &nodes);
int getContourFromPositionedAtoms(XContour &positionedAtoms, Contour &contour, Atom *useOnly);
string nodeKind(int k);
void printNodes(fg::Contour &c);


#endif // UTILS_H


//void contourToDebug(const Contour &c) const;
/*
void contoursToDraw(Contours cs, Point tr, QPainterPath &path);	
void coutRect(const string &header, const Rect &r);
void coutMatrix(const string &header, const Matrix &m);
void coutSplines(const Splines &s);
std::stringstream  ctrToString(const Contour &contour);
void glyphToDraw(fg::Glyph &g, const Point &tr = Point (0,0));
void addToDraw(Contour &c, const fg::Point &translation, int index);
void addToDraw(fg::Contour &c, const fg::Matrix &m, int index);
QPainterPath contourToPath(const fg::Contour &contour, const fg::Matrix &layerMatrix);
void drawNodes(QPainterPath p, Point shift = Point(0,0));  
*/
