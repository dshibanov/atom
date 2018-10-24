#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    drawDecomposedGlyph = true;

    qDebug() << " start! ";
    drawedGlyphPointer = NULL;

  //  mainWindow = this;
  //  gCounter = 0;
  //  maxGraphemesCount = 0;
  //  bool ok;
  //  ui->progressBar->hide();
  //  show();

    // now will open hardcoded path...
    file = new QFile(QFileDialog::getOpenFileName());
    //file = new QFile(QString("/data/work/projects/fontlab/atom/unittest/MinionPro-Regular.otf"));
    //file = new QFile(QString("/data/work/projects/fontlab/atom/unittest/TimesNewRoman.ttf"));

    ui->fontPath->setText("   " + file->fileName());

    qDebug() << " here fname: " << file->fileName();
    readFontFile(file->fileName(), QString("D://"));

      //len = ui->lenEdit->text().toInt();

//      ui->comboBox->addItem("simple");
//      ui->comboBox->addItem("end-begin");
//      ui->comboBox->addItem("end-begin averaging");
//      ui->comboBox->setCurrentIndex(0);



      //globalCtr = 0;

    this->update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_RunButton_clicked()
{
    ui->textBrowser->append("RUN button pressed..");
    toLog("LOH");
    AtomLib *atomLib = new AtomLib();
    setUpAtomLib(*atomLib);
    aGlyphs.clear();
    dict.clear();
    statDict.clear();
    ui->textBrowser->clear();

    //atomLib->stats(package->font, aGlyphs, dict, statDict);
    atomLib->stats2(package->font, aGlyphs, dict, statsLinks);
    fillStatDict();
    refreshAtomsList(dict, (*ui->atomsList));
    refreshAtomsList(statDict, (*ui->statAtomsList));
    refreshGlyphsList(aGlyphs, (*ui->glyphsList));

    qDebug() << dict.size() << " " << statDict.size() << " " << aGlyphs.size();
}

void MainWindow::toLog(string message)
{
    ui->textBrowser->append(message.c_str());
}

void MainWindow::fillStatDict()
{
    for(auto it = statsLinks.begin(); it != statsLinks.end(); it++)
    {
        statDict.push_back(*(*it));
    }
}

// -- on_...
void MainWindow::on_glyphsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{

    clear();
    if(current == NULL)
        return;

    qDebug()<<" name: " << current->text();
    Point topRightPoint;
    Glyph g;

    XGlyphs::iterator aGlyphIt = aGlyphs.begin();
    std::advance(aGlyphIt, current->listWidget()->currentRow());
    drawedGlyphPointer = &(*aGlyphIt);

    qDebug()<<" #: " << drawedGlyphPointer->index;

    //qDebug()<<" consists: " << DebugUtils::atomsToString(drawedGlyphPointer->contours).str().c_str();

    if(findGlyphByIndex(package, g, (*aGlyphIt).index) == 0)
        return;

    qDebug() << g.name().c_str();


    qDebug() << " source glyph's drawing contours.. ";
    topRightPoint = getTopRightPointOfGlyph(g);

    contoursToDraw(getContoursOfGlyph(g), Point(-700,0), globalPath);

    /*
    if(drawDecomposedGlyph)
    {
        // getting AGlyph contours and drawing it
        Contours cs;
        getContoursFromXGlyph((*aGlyphIt), cs);



//        compareGlyphs(g, (*aGlyphIt));


        contoursToDraw(cs, Point(-700 + 20 + (int)topRightPoint.x,0), globalPath);

        // вот тут выведем для исходного глифа
        // получим контура исходного глифа
        // найдем глиф... он уже найден это g
        // тогда получим его контура
        Contours sourceCs = getContoursOfGlyph(g);
        int i = 0;
        for(auto it = next(sourceCs.begin()); it != sourceCs.end() && i < 1; it++, i++)
        {
            qDebug() << (((*it).open) ? "OPENED" : "CLOSED");
            printNodes((*it));
        }

    }

    */
    this->update();

}

// REFACTOR ME!!
void MainWindow::on_atomsList_currentItemChanged(QListWidgetItem *item)
{

    qDebug()<<" ------  on_atomsList_currentItemChanged... ------";
    uint itemNumber = item->listWidget()->currentRow();


//    len = ui->lenEdit->text().toInt();
    globalAtoms = QPainterPath();
    redPath = QPainterPath();
    this->update();

    //qDebug()<<" dict.size " << dict.size() <<  " itemNumber " << itemNumber;
    if(item != NULL && dict.size() > itemNumber)
    {
        Atoms::iterator current = dict.begin();
        std::advance (current,itemNumber);

        Atom & currentAtom = (*current);
        qDebug() << "index is: " << indexIn(&currentAtom, dict);
        Point translation = getAtomTranslation(currentAtom);
        AtomDrawer atomDrawer(translation, fg::Matrix(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0));
        atomDrawer.drawAtomIntoPath(currentAtom, globalAtoms);
        qDebug()<<"atom #"<<itemNumber << "\n  consisted from " << currentAtom.rawAtomsCount << " raw atoms \n used in glyphs: " << intsToStr(currentAtom.usedIn).str().c_str();
//        qDebug() <<
                    printNodes(currentAtom) ;



        // here drawing current atom in the glyph canvas
        if(ui->ShowSampleCheckBox->isChecked() && drawedGlyphPointer != NULL)
        {
            Contours cs;
            XGlyph &agl = (*aGlyphs.begin());
            XGlyph *aglPointer = &(*aGlyphs.begin());


            int result = getContoursOnlyWithAtom(*drawedGlyphPointer, &currentAtom, cs);
            //int result = drawedGlyphPointer->getContoursOnlyWithAtom(&currentAtom, cs);
            int nodesCount = 0;
            // тут мы считаем количество узлов
            for (Contours::iterator it = cs.begin(); it != cs.end(); ++it)
            {
                Contour &c = (*it);
                nodesCount += c.nodes.size();
            }

            if(result == 1 && nodesCount > 0)
            {
                for (Contours::iterator it2 = cs.begin(); it2 != cs.end(); ++it2)
                {
                    (*it2).open = true;
                }
                contoursToDraw(cs, Point(-700,0), redPath);
            }
        }

        // here comparation with control atom
        if(ui->drawComparingLabel->isChecked())
        {
            // second atom
            Atoms::iterator comparing = dict.begin();
            std::advance (comparing, ui->compareEdit->text().toInt());

            if(ui->compareEdit->text().toInt() != item->listWidget()->currentRow())
            {
                qDebug() << "Compare atom " << ui->compareEdit->text().toInt() << " with atom " << item->listWidget()->currentRow();// << endl;
                atomDrawer.drawAtomIntoPath((*comparing), redPath);
            }

            vector<double> errors;
            vector<double> endPointsErrors;
            errors.reserve(8);
            Representations rs = Representations();
            getRepresentationsFromContour((*comparing), rs, ui->lenEdit->text().toInt(), false);
            getRepresentationsFromContour((*comparing), rs, ui->lenEdit->text().toInt(), true);
            Splines s2 = contourToSplines(currentAtom, ui->lenEdit->text().toInt());

            for (Representations::iterator si = rs.begin(); si != rs.end(); ++si)
            {
                endPointsErrors.push_back((*prev(currentAtom.nodes.end())).p.dist((*prev((*si).s.end()))));
                errors.push_back(compareSplines((*si).s, s2, ui->lenEdit->text().toInt()));
            }

            int minErrori = 0;
            for (uint i = 0; i < errors.size(); ++i)
            {
                if(errors[minErrori] > errors[i])
                {
                    minErrori = i;
                }
            }

            if(errors[minErrori] > ui->toleranceEdit->text().toFloat())
                qDebug()<<">    Atoms are different because of splinesError too high: " << errors[minErrori] ;
            else if(errors[minErrori] < ui->toleranceEdit->text().toFloat() && endPointsErrors[minErrori] > ui->toleranceEndPoints->text().toInt())
                qDebug()<<">    Atoms are different because of endPointError too high: " <<  endPointsErrors[minErrori];
        }

        this->update();
    }
    else
    qDebug()<<"ERROR: item index too high...";

}

string xContourToAtomIndexes(XContour &c, Atoms &dict)
{
    stringstream ss;
    for(auto it = c.nodes.begin(); it != c.nodes.end(); it++)
    {
        ss << indexIn((*it).atom, dict) << ",";
    }

    return ss.str();
}

void printips(InsertPoints &ips)
{
    for(auto it = ips.begin(); it != ips.end(); it++)
    {
        InsertPoint &ip = (*it);
        qDebug() << "g: " << ip.glyphName.c_str() << " cnum: " << ip.contourNum << " snode: " << ip.startNodeNum;
    }
}

void MainWindow::on_statAtomsList_currentItemChanged(QListWidgetItem *item, QListWidgetItem *previous)
{
    qDebug()<<" on_statAtomsList_currentItemChanged... ";
    int itemNumber = item->listWidget()->currentRow();

    globalAtoms = QPainterPath();
    redPath = QPainterPath();
    this->update();

    if(item != NULL && statDict.size() > itemNumber)
    {
        Atoms::iterator current = statDict.begin();
        std::advance (current,itemNumber);
        Atom & currentAtom = (*current);

        // second atom
        Atoms::iterator comparing = statDict.begin();
        std::advance (comparing, ui->compareEdit->text().toInt());
        Point translation;
        fg::Point center;
        QPoint pp = QPoint(700,0);
        Rect bbox = (*current).transformed(Matrix()).boundingBox(Matrix(), false);
        Point dCenter = Point(pp.x() + ui->canvasRight->size().width()/2, ui->canvasRight->y() + ui->canvasRight->size().height()/2);
        center = fg::Point(pp.x() + ui->canvasRight->geometry().width()/2, ui->canvasRight->geometry().y() + ui->canvasRight->geometry().height()/2);
        Point glyphC = Point(bbox.left() + bbox.width() / 2, bbox.top() + bbox.height() / 2);
        translation.x = -(glyphC.x*2.5*global_scale - dCenter.x);
        int bottom_line = center.y + 100*global_scale;        

        fg::Matrix mtx(2.5*global_scale, 0, 0, -2.5*global_scale, 0, 0);
        QPainterPath p = contourToPath((*current), mtx).translated(QPointF(translation.x,bottom_line + translation.y));
        p.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 2,2);
        globalAtoms.addPath(p);
        qDebug()<<" usedIn: " << intsToStr((*current).usedIn).str().c_str() << " components: " << intsToStr((*current).components).str().c_str();
        qDebug()<< " names: " << namesFromUsedIn(aGlyphs, (*current).usedIn).c_str();
        qDebug()<< " glyphsFoundIn(): " << namesFromUsedIn(aGlyphs, (*current).glyphsFoundIn()).c_str();


        ui->textBrowser->clear();
        toLog("found in: " + namesFromUsedIn(aGlyphs, (*current).usedIn));
        toLog("consisted from: " + to_string((*current).rawAtomsCount) + " raw atoms");

        if(ui->ShowSampleCheckBox->isChecked() && drawedGlyphPointer != NULL)
        {            
            Contours cs;
            XGlyph &agl = (*aGlyphs.begin());
            XGlyph *aglPointer = &(*aGlyphs.begin());

            XGlyphs::iterator aGlyphIt = aGlyphs.begin();
            std::advance(aGlyphIt, ui->glyphsList->currentRow());
            drawedGlyphPointer = &(*aGlyphIt);
            qDebug()<<" #: " << drawedGlyphPointer->index;

            Atom *statAtom  (*next(statsLinks.begin(), itemNumber));

            qDebug() << " statAtom->rawAtomsCount " << statAtom->rawAtomsCount;
            qDebug() << "components: " << intsToStr(statAtom->components).str().c_str();

            Atom & currentAtom = *statAtom;
            int result = getContoursOnlyWithAtom(*drawedGlyphPointer, &currentAtom, cs);
            int nodesCount = 0;
            for (Contours::iterator it = cs.begin(); it != cs.end(); ++it)
            {
                Contour &c = (*it);
                nodesCount += c.nodes.size();
            }

            if(result == 1 && nodesCount > 0)
            {
                for (Contours::iterator it2 = cs.begin(); it2 != cs.end(); ++it2)
                {
                    (*it2).open = true;
                }
                contoursToDraw(cs, Point(-700,0), redPath);
            }

        }

        this->update();
    }
    else
        qDebug()<<"ERROR: item index too high...";
}

int MainWindow::setUpAtomLib(AtomLib &atomlib)
{

    //atomlib.rCaching = ui->Rcaching->isChecked();
    atomlib.compareBBox = ui->compareBBox->isChecked();
    atomlib.from = ui->fromEdit->text().toInt();
    atomlib.to = ui->toEdit->text().toInt();
    atomlib.splineLength = ui->lenEdit->text().toInt();
    atomlib.tolerance = ui->toleranceEdit->text().toFloat();
    atomlib.toleranceBBox =  ui->toleranceEditBBox->text().toFloat();
    atomlib.toleranceEndPoints = ui->toleranceEndPoints->text().toInt();
    atomlib.minAtomsCount = ui->MinAtomsCountEdit->text().toInt();
    atomlib.minUsedCount = ui->MinUsedCountEdit->text().toInt();
    //atomlib.joiningType = ui->comboBox->currentIndex();

    if(atomlib.from == 0 && atomlib.to == 0)
        atomlib.allGlyphs = true;
    else
        atomlib.allGlyphs = false;

    return 1;
}

void MainWindow::refreshAtomsList(const Atoms &dict, QListWidget &qlist)
{

    //	ui->atomsList->blockSignals(true);
    //	ui->atomsList->clear();
    //	ui->atomsList->blockSignals(false);

    qlist.blockSignals(true);
    qlist.clear();
    qlist.blockSignals(false);

    // add atoms to list
    int atomCount = 0;//dict.size()-1;
    for (Atoms::const_iterator it = dict.begin(); it != dict.end(); ++it, atomCount++)
    {
        qlist.addItem(QString("#%1 ").arg(atomCount));
    }
}

void MainWindow::refreshGlyphsList(const XGlyphs &dict, QListWidget &qlist)
{
    qlist.blockSignals(true);
    qlist.clear();
    qlist.blockSignals(false);

    // add atoms to list
    /*
    int counter = 0;
    for (Atoms::const_iterator it = dict.begin(); it != dict.end(); ++it, counter++)
    {
        qlist.addItem(QString("#%1 ").arg(counter));
    }
    */

    for (auto it = dict.begin(); it != dict.end(); ++it)
    {
        qlist.addItem(QString("%1 %2").arg((*it).index).arg((*it).name.c_str()));
    }
}

int MainWindow::readFontFile(QString path, QString outPath)
{
    //ui->progressBar->show();

    qDebug() << " filename: " << path ;

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
            //			ui->glyphsList->addItem(QString("%1").arg((*it)->name.c_str()));

            //			if((*it)->name.compare("I") == 0)
            //				qDebug()<<"I found gi = " << glyphIndex;
        }
    }


    return 1;

}

void MainWindow::clear()
{
  paths.clear();
  globalPath = QPainterPath();
  globalBBoxes = QPainterPath();
  this->update();
}

Point MainWindow::getTopRightPointOfGlyph(const Glyph &g)
{
    Point p;
    fg::GlyphsR grs;
    Rect bbox = g.boundingBox(grs, fg::Matrix(1, 0, 0, -1, 0, 0),false);
    p = Point((int)bbox.width(), 0);
    return p;
}

void MainWindow::contoursToDraw(Contours cs, Point tr, QPainterPath &path)
{
    //qDebug()<<"contoursToDraw ";
    //qDebug()<<" cs.size() " << cs.size();

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

void MainWindow::drawNodes(QPainterPath p, Point shift)
{
    if(p.isEmpty())
    return;

    globalBBoxes.addEllipse(QPointF(p.elementAt(0).x, p.elementAt(0).y), 3,3);
    for(int i = 1; i < p.elementCount(); ++i)
    {
        globalBBoxes.addEllipse(QPointF(p.elementAt(i).x + shift.x, p.elementAt(i).y + shift.y), 1,1);
    }
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

Point MainWindow::getAtomTranslation(Atom &atom)
{
    QPoint leftTop = QPoint(700,0);
    Rect bbox = atom.transformed(Matrix()).boundingBox(Matrix(), false);
    Point dCenter = Point(leftTop.x() + ui->canvasRight->size().width()/2,
                                                ui->canvasRight->y() + ui->canvasRight->size().height()/2);
    fg::Point center = fg::Point(leftTop.x() + ui->canvasRight->geometry().width()/2,
                                                             ui->canvasRight->geometry().y() + ui->canvasRight->geometry().height()/2);
    Point atomCenter = Point(bbox.left() + bbox.width() / 2, bbox.top() + bbox.height() / 2);
    Point translation = Point(dCenter.x - atomCenter.x*2.5*global_scale, center.y + 100*global_scale);
    return translation;
}
