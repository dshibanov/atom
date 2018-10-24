#ifndef COMPOSEDGLYPH_H
#define COMPOSEDGLYPH_H


typedef unsigned int uint;

class ContourData {
public:
  ContourData(const fg::Contour &contour);

  fg::Contour contour;
  fg::Rect boundingBox;
  Splines splines;

  void updateBoundingBox();
  const Splines& getSplines(int len, bool remade);
  void getTransformedSplines(Splines &tr_splines, fg::Matrix m, int len);
};

typedef std::list<ContourData> ContoursData;

class Grapheme {
public:
  int graphemeIndex;
  ContoursData cdata;
  int glyphIndex;
  vector<int> oldGraphemes;
  vector<fg::Point> oldPositions;	// тут наверное нужна матрица

  Grapheme();
  Grapheme(int index);

  fg::Rect& bbox(bool remade = true);

  int size() const
  {
    return cdata.size();
  }

  bool empty() const
  {
    return cdata.empty();
  }

  void sortContours();
  void normalize();
//  Splines getSplines(int cdataIndex, int len, bool remade = false);
  void addContours(const fg::Contours &contours);
	void printBBox();

  static bool cCompare(ContourData d1, ContourData d2);
  static bool contourToCurves(list<fg::Curve> &curves, const fg::Contour &contour);
  static void curvesToSplines(const list<fg::Curve> &curves, Splines &contour, fg::Integers &indexes, int len);

private:
  fg::Rect bboxRect;
};

class PositionedGrapheme {
public:
  int index;
  Grapheme* grapheme;
  //fg::Point position;
  fg::Matrix  matrix;

  PositionedGrapheme(Grapheme* g, fg::Point _pos, int index);
  PositionedGrapheme(Grapheme* g, fg::Matrix  _matrix, int index);
  PositionedGrapheme();
};

typedef std::vector<PositionedGrapheme> PositionedGraphemes;

class ComposedGlyph {
public:
  PositionedGraphemes graphems;
  string name;
  int originalIndex;
  ComposedGlyph();
};

typedef std::vector<Grapheme*> Graphemes;
typedef std::vector<ComposedGlyph> ComposedGlyphs;

#endif // COMPOSEDGLYPH_H
