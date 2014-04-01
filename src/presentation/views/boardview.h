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

#include "gkchess_piece.h"
#include "gutil_map.h"
#include "gutil_smartpointer.h"
#include <QAbstractItemView>

class QSequentialAnimationGroup;
class QRubberBand;

namespace GKChess{
    class ISquare;
    class AbstractBoard;
namespace UI{


class BoardModel;
class IFactory_PieceIcon;


/** A viewer you can use with the chess board model to display
 *  a chess position in a Qt application.
 * 
 *  The base boardview class takes care of all the basic functions of displaying
 *  a board, but does not allow you to edit it or move pieces (if you want editing then you
 *  need the BoardEdit, and if you want to play a game you need the GameView).
 * 
 *  By default the view first tries to paint the pieces using the icon factory, if given.
 *  If no factory was given then it tries to paint icons provided by the model in the "Qt::DecorationRole".
 *  If there are no icons produced by the model then the view simply paints the unicode chess characters.
*/
class BoardView :
        public QAbstractItemView
{
    Q_OBJECT

    struct SquareFormatOptions
    {
        QColor HighlightColor;
    };

    // Dimensional parameters
    float m_squareSize;

    // for painting
    QColor m_darkSquareColor;
    QColor m_lightSquareColor;
    QColor m_activeSquareHighlightColor;
    IFactory_PieceIcon *i_factory;

    GUtil::SmartPointer<QRubberBand> m_selectionBand;

    // Our animation objects
    QModelIndex m_hiddenIndex;
    GUtil::SmartPointer<QSequentialAnimationGroup> m_animationGroup;

    // Keeps track of our per-square format options
    GUtil::Map<ISquare const *, SquareFormatOptions> m_formatOpts;

public:

    /** Constructs a board view with default options. */
    explicit BoardView(QWidget *parent = 0);
    ~BoardView();

    BoardModel *GetBoardModel() const;
    
    /** Tells the view to use the given icon factory, which overrides
        icons returned by the model.  Pass NULL to clear the icon factory.
    */
    void SetIconFactory(IFactory_PieceIcon *);
    IFactory_PieceIcon *SetIconFactory(IFactory_PieceIcon *) const{ return i_factory; }

    float GetSquareSize() const{ return m_squareSize; }
    void SetSquareSize(float);

    QColor GetDarkSquareColor() const{ return m_darkSquareColor; }
    void SetDarkSquareColor(const QColor &);

    QColor GetLightSquareColor() const{ return m_lightSquareColor; }
    void SetLightSquareColor(const QColor &);

    QColor GetActiveSquareHighlightColor() const{ return m_activeSquareHighlightColor; }
    void SetActiveSquareHighlightColor(const QColor &);

    /** Causes the square to be highlighted. */
    void HighlightSquare(const QModelIndex &, const QColor &);

    /** Causes all the squares to be highlighted. */
    void HighlightSquares(const QModelIndexList &, const QColor &);

    /** Removes all highlighting from the board. */
    void ClearSquareHighlighting();


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
    static QString GenerateHtml(const AbstractBoard &,
                                const HtmlFormattingOptions & = HtmlFormattingOptions());

protected:

    /** Returns the rect for the entire board. */
    QRectF get_board_rect() const;

    /** This function returns the rect for the given index.
     *  This will not compensate for the scrollbars.
    */
    QRectF ind_2_rect(int col, int row) const;

    /** This function is called whenever the user attempts to move a piece
        from one index to another.  In the base view this function does nothing,
        but you can override it to do something interesting (like moving a piece!)
    */
    virtual void attempt_move(const QModelIndex &source, const QModelIndex &dest);

    /** Hides any piece at the given index. It will remain hidden until you call this function
     *  again with an invalid QModelIndex().  This is useful for animating piece movements.
    */
    void hide_piece_at_index(const QModelIndex & = QModelIndex());

    /** This is the function you should override when doing your own painting.
     *  Be sure to call the base implementation!
     *  \param painter The painter object.
     *  \param update_rect The rect that needs to be repainted.
    */
    virtual void paint_board(QPainter &painter, const QRect &update_rect);

    /** Paints the piece within the rect. */
    virtual void paint_piece_at(const Piece &, const QRectF &, QPainter &);

    /** Starts an animation of the piece moving from the source point to the dest point
     *  with the given easing curve.  The easing curve responds to the Type enum of
     *  the QEasingCurve.
    */
    void animate_move(const Piece &,
                      const QPointF &source, const QPointF &dest,
                      int duration_ms,
                      int easing_curve);

    /** A function that animates a piece snapping back from any point to the source square.
     *  This is really just a wrapper around animate_move, so it's here for convenience and
     *  consistency of animations.
    */
    void animate_snapback(const QPointF &from, const QModelIndex &back_to);

    /** Returns the animation object for direct use by the subclass. */
    QSequentialAnimationGroup *get_animation() const{ return m_animationGroup; }

    
    /** \name QAbstractItemView interface
     *  \{
    */
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    /** \} */


protected slots:

    /** \name QAbstractItemView interface
     *  \{
    */
    virtual void updateGeometries();
    virtual void currentChanged(const QModelIndex &, const QModelIndex &);
    /** \} */


private slots:

    void _animation_finished();
    void _update_rubber_band();

    void _piece_moved(const Piece &, const QModelIndex &, const QModelIndex &);

};


}}

#endif // GKCHESS_BOARDVIEW_H
