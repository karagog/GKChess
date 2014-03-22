/*Copyright 2014 George Karagoulis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef GKCHESS_BOARDVIEW_H
#define GKCHESS_BOARDVIEW_H

#include "gkchess_board.h"
#include "gutil_smartpointer.h"
#include <QAbstractItemView>
#include <QRubberBand>

namespace GKChess{ namespace UI{

class BoardModel;

/** A viewer you can use with the chess board model to display
 *  a chess position in a Qt application.
*/
class BoardView :
        public QAbstractItemView
{
    Q_OBJECT

    float m_squareSize;
    QRectF m_boardRect;
    QColor m_darkSquareColor;
    QColor m_lightSquareColor;
    QColor m_pieceColor;

    QRubberBand m_selectionBand;
public:

    /** Constructs a board view with default options. */
    explicit BoardView(QWidget *parent = 0);
    ~BoardView();

    BoardModel *GetBoardModel() const;

    float GetSquareSize() const{ return m_squareSize; }
    void SetSquareSize(float);

    QColor GetDarkSquareColor() const{ return m_darkSquareColor; }
    void SetDarkSquareColor(const QColor &);

    QColor GetLightSquareColor() const{ return m_lightSquareColor; }
    void SetLightSquareColor(const QColor &);

    QColor GetPieceColor() const{ return m_pieceColor; }
    void SetPieceColor(const QColor &);


    /** \name QAbstractItemView interface
     *  \{
    */
    virtual QRect visualRect(const QModelIndex &index) const;
    virtual void scrollTo(const QModelIndex &, ScrollHint);
    virtual QModelIndex indexAt(const QPoint &point) const;
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    virtual int horizontalOffset() const;
    virtual int verticalOffset() const;
    virtual bool isIndexHidden(const QModelIndex &index) const;
    virtual void setSelection(const QRect &, QItemSelectionModel::SelectionFlags);
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const;

    /** You are only allowed to pass board models */
    virtual void setModel(QAbstractItemModel *);
    /** /} */



    /** Describes the available options for the board as an html table. */
    struct HtmlFormattingOptions
    {
        /** Controls whether the html will be formatted prettily. */
        bool HumanReadable;

        /** Controls the color of the pieces. */
        QColor PieceColor;

        /** Controls the color of the light squares. */
        QColor LightSquareColor;

        /** Controls the color of the dark squares. */
        QColor DarkSquareColor;

        /** The number of html pts for the size of a square. */
        int SquareSize;

        /** The thickness of the border between squares. */
        int BorderSize;

        /** The size of the pieces. */
        int PieceSize;

        /** The size of the index characters for the rows and columns (1-8, a-h). */
        int IndexSize;

        /** Initializes some useful defaults. */
        HtmlFormattingOptions();
    };

    /** Returns an HTML table of the chess board with the given formatting options.
     *  You can then display this in a web browser.
    */
    static QString GenerateHtml(const Board &,
                                const HtmlFormattingOptions & = HtmlFormattingOptions());


protected:

    /** \name QAbstractItemView interface
     *  \{
    */
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    /** \} */


protected slots:

    /** \name QAbstractItemView interface
     *  \{
    */
    virtual void updateGeometries();
    /** \} */


private:

    // paints the board
    void _paint_board();

    void _update_board_rect();
    QRectF _get_rect_for_index(const QModelIndex &) const;

};


}}

#endif // GKCHESS_BOARDVIEW_H
