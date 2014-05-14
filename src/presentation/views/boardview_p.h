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

#ifndef GKCHESS_BOARDVIEW_P_H
#define GKCHESS_BOARDVIEW_P_H

#include "gkchess_board.h"
#include "gutil_smartpointer.h"
#include "gutil_vector.h"
#include "gutil_map.h"
#include <QAbstractItemView>
#include <QVariantAnimation>

class BoardModel_p;

class QRubberBand;
class QAnimationGroup;

namespace GKChess{
    class Piece;
namespace UI{
    class IFactory_PieceIcon;
}}


/** A private class to implement the board view, because we don't want to expose the
 *  QAbstractItemView interface. This gives us better encapsulation.
*/
class BoardView_p :
        public QAbstractItemView,
        public GKChess::IPlayerResponse
{
    Q_OBJECT

    struct SquareFormatOptions
    {
        QColor HighlightColor;
    };

    // Dimensional parameters
    float m_squareSize;

    // Behavioral parameters
    bool m_editable;
    bool m_showThreatCounts;

    // for painting
    QColor m_darkSquareColor;
    QColor m_lightSquareColor;
    QColor m_activeSquareHighlightColor;
    GKChess::UI::IFactory_PieceIcon *i_factory;

    // Our current state for user interaction
    QModelIndex m_activeSquare;
    bool m_dragging;
    QPoint m_mousePressLoc;
    bool m_wasSquareActiveWhenPressed;
    GUtil::SmartPointer<QRubberBand> m_selectionBand;

    // Our animation objects
    QAnimationGroup *m_animation;
    GUtil::SmartPointer<GKChess::Board> m_animationBoard;

    // Keeps track of our per-square format options
    GUtil::Map<QModelIndex, SquareFormatOptions> m_formatOpts;

public:

    /** Constructs a board view with default options. */
    explicit BoardView_p(QWidget *parent = 0);
    ~BoardView_p();

    BoardModel_p *GetBoardModel() const;
    void SetBoardModel(BoardModel_p *);
    void SetIconFactory(GKChess::UI::IFactory_PieceIcon *);
    GKChess::UI::IFactory_PieceIcon *GetIconFactory() const{ return i_factory; }
    float GetSquareSize() const{ return m_squareSize; }
    void SetSquareSize(float);
    QColor GetDarkSquareColor() const{ return m_darkSquareColor; }
    void SetDarkSquareColor(const QColor &);
    QColor GetLightSquareColor() const{ return m_lightSquareColor; }
    void SetLightSquareColor(const QColor &);
    QColor GetActiveSquareHighlightColor() const{ return m_activeSquareHighlightColor; }
    void SetActiveSquareHighlightColor(const QColor &);
    void HighlightSquare(const QModelIndex &, const QColor &);
    void HighlightSquares(const QModelIndexList &, const QColor &);
    void ClearSquareHighlighting();
    bool Editable() const{ return m_editable; }
    void SetEditable(bool b){ m_editable = b; }
    void SetShowThreatCounts(bool);
    bool GetShowThreatCounts() const{ return m_showThreatCounts; }

    /** \name IPlayerResponse interface
     *  \{
    */
    virtual GKChess::Piece ChoosePromotedPiece(GKChess::Piece::AllegienceEnum);
    /** \} */


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
    /** /} */


protected:

    /** Returns the rect for the entire board. */
    QRectF get_board_rect() const;

    /** This function returns the rect for the given index.
     *  This will not compensate for the scrollbars.
    */
    QRectF item_rect(int col, int row) const;

    /** Returns a floating point rect for the given index, with compensation for
     *  scrollbars.
    */
    QRectF visual_rectf(int col, int row) const;

    /** This function is called whenever the user attempts to move a piece
        from one index to another.  In the base view this function does nothing,
        but you can override it to do something interesting (like moving a piece!)

        \returns True if the move was successful
    */
    virtual bool attempt_move(const QModelIndex &source, const QModelIndex &dest);

    /** This is the function you should override when doing your own painting.
     *  Be sure to call the base implementation!
     *  \param painter The painter object.
     *  \param update_rect The rect that needs to be repainted.
    */
    virtual void paint_board(QPainter &painter, const QRect &update_rect);

    /** Paints the piece within the rect. */
    virtual void paint_piece_at(const GKChess::Piece &, const QRectF &, QPainter &);

    /** Starts an animation of the piece moving from the source point to the dest point
     *  with the given easing curve.  The easing curve responds to the Type enum of
     *  the QEasingCurve.
    */
    void animate_move(const GKChess::MoveData &,
                      const QPointF &source, const QPointF &dest,
                      GKChess::Square const &sqr_source,
                      int duration_ms,
                      QEasingCurve::Type);

    void animate_castle(const GKChess::MoveData &,
                        const GKChess::Square &king_src, const GKChess::Square &king_dest,
                        const GKChess::Square &rook_src, const GKChess::Square &rook_dest,
                        int duration_ms,
                        QEasingCurve::Type easing_curve);

    /** A function that animates a piece snapping back from any point to the source square.
     *  This is really just a wrapper around animate_move, so it's here for convenience and
     *  consistency of animations.
    */
    void animate_snapback(const QPointF &from, const QModelIndex &back_to);


    /** \name QAbstractItemView interface
     *  \{
    */
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
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

    void _piece_about_to_move(const GKChess::MoveData &);
    void _piece_moved(const GKChess::MoveData &);


private:

    void _update_cursor_at_point(const QPointF &);

};

#endif // GKCHESS_BOARDVIEW_P_H
