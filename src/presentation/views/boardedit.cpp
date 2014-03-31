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

#include "boardedit.h"
#include "gkchess_editableboardmodel.h"
#include "gkchess_isquare.h"
#include <QMouseEvent>
#include <QEasingCurve>
#include <QVariantAnimation>
USING_NAMESPACE_GUTIL;


#define ANIM_MOVE_DURATION 0.75

/** The size of a rect that determines when to start dragging a piece (as a factor of square size). */
#define DRAG_START_RECT_SIZE 0.35


NAMESPACE_GKCHESS1(UI);


BoardEdit::BoardEdit(QWidget *p)
    :BoardView(p),
      m_dragging(false),
      m_wasSquareActiveWhenPressed(false)
{}

void BoardEdit::setModel(QAbstractItemModel *m)
{
    if(NULL == dynamic_cast<EditableBoardModel *>(m))
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Editable board models only");

    BoardView::setModel(m);
}

EditableBoardModel *BoardEdit::GetBoardModel() const
{
    return static_cast<EditableBoardModel *>(model());
}

bool BoardEdit::validate_move(const QModelIndex &, const QModelIndex &)
{
    return true;
}

void BoardEdit::attempt_move(const QModelIndex &s, const QModelIndex &d)
{
    // As long as both indices are valid, we will move the piece
    if(s != d && s.isValid() && d.isValid())
    {
        hide_piece_at_index(s);
        animate_move(*GetBoardModel()->ConvertIndexToSquare(s)->GetPiece(),
                      ind_2_rect(s.column(), s.row()).center(),
                      ind_2_rect(d.column(), d.row()).center(),
                      ANIM_MOVE_DURATION * 1000,
                      //QEasingCurve::InOutQuad
                      QEasingCurve::InOutCubic
                      //QEasingCurve::InOutQuart
                      //QEasingCurve::InOutQuint
                      //QEasingCurve::InOutCirc
                      //QEasingCurve::InOutSine
                      );
    }
}

void BoardEdit::mousePressEvent(QMouseEvent *ev)
{
    GASSERT(m_dragOffset.isNull());

    BoardView::mousePressEvent(ev);

    if(QAbstractAnimation::Running == get_animation()->state())
        return;

    QModelIndex ind = indexAt(ev->pos());
    if(ind.isValid())
    {
        QPoint p(ev->pos());
        Piece target_piece = ind.data(BoardModel::PieceRole).value<Piece>();
        if(target_piece.IsNull())
            return;

        if(m_activeSquare.isValid())
        {
            Piece active_piece = m_activeSquare.data(BoardModel::PieceRole).value<Piece>();
            m_wasSquareActiveWhenPressed = m_activeSquare == ind;

            if(target_piece.GetAllegience() != active_piece.GetAllegience())
                return;
        }

        m_activeSquare = ind;

        // Remember where they clicked, so we can start dragging if they pull away from this spot
        m_mousePressLoc = ev->pos();

        // Add highlighting to the active square
        ClearSquareHighlighting();
        HighlightSquare(m_activeSquare, GetActiveSquareHighlightColor());

        viewport()->update();

        GASSERT(m_activeSquare.isValid());
    }

    _update_cursor_at_point(ev->posF());
}

void BoardEdit::mouseReleaseEvent(QMouseEvent *ev)
{
    BoardView::mouseReleaseEvent(ev);

    if(m_activeSquare.isValid())
    {
        QModelIndex ind_active = m_activeSquare;
        QModelIndex ind_released = indexAt(ev->pos());
        m_activeSquare = QModelIndex();

        if(ind_released.isValid())
        {
            if(ind_active == ind_released)
            {
                // If they released on the square they started on, then it leaves that square activated
                //  But if the square was already active, then we make it inactive if they click it.
                if(!m_wasSquareActiveWhenPressed)
                    m_activeSquare = ind_active;
            }
            else
            {
                // If they released on a different square, then execute a move
                attempt_move(ind_active, ind_released);
            }
        }
        else
        {
            // If they dropped the piece off the board, snap it back to the start location
            if(m_dragging)
                animate_snapback(ev->pos(), ind_active);
        }
    }

    m_dragging = false;
    m_mousePressLoc = QPoint();
    m_wasSquareActiveWhenPressed = false;

    if(!m_activeSquare.isValid())
        ClearSquareHighlighting();
    viewport()->update();

    _update_cursor_at_point(ev->posF());
}

void BoardEdit::mouseMoveEvent(QMouseEvent *ev)
{
    BoardView::mouseMoveEvent(ev);

    if(m_activeSquare.isValid())
    {
        ClearSquareHighlighting();

        // Highlight the valid squares for moving
        QModelIndexList valid_moves;
        valid_moves.append(m_activeSquare);

        // Todo:  Generate a list of valid moves for the piece

        HighlightSquares(valid_moves, GetActiveSquareHighlightColor());


        // Next we want to highlight the square the user is hovering over
        QModelIndex ind = indexAt(ev->pos());
        if(ind.isValid() && ind != m_activeSquare)
            HighlightSquare(ind, validate_move(m_activeSquare, ind) ? Qt::green : Qt::red);
    }

    if(!m_mousePressLoc.isNull())
    {
        double side = DRAG_START_RECT_SIZE*GetSquareSize();
        if(!QRect(m_mousePressLoc.x()-side/2, m_mousePressLoc.y()-side/2, side, side)
                .contains(ev->pos()))
        {
            m_dragging = true;
        }
        viewport()->update();
    }

    _update_cursor_at_point(ev->posF());
}

void BoardEdit::mouseDoubleClickEvent(QMouseEvent *ev)
{
    // We bypass the BoardView implementation, which does nothing, and
    //  allow the abstract item model class to edit the item, if necessary.
    QAbstractItemView::mouseDoubleClickEvent(ev);
}

void BoardEdit::paint_board(QPainter &painter, const QRect &update_rect)
{
    // First let the base class paint the board
    BoardView::paint_board(painter, update_rect);


    // If we're dragging then paint the piece being dragged
    if(m_dragging && m_activeSquare.isValid())
    {
        QPoint cur_pos = mapFromGlobal(QCursor::pos());
        Piece const *active_piece = GetBoardModel()->ConvertIndexToSquare(m_activeSquare)->GetPiece();

        QRectF r(cur_pos.x() - GetSquareSize()/2 + horizontalOffset(),
                 cur_pos.y() - GetSquareSize()/2 + verticalOffset(),
                 GetSquareSize(), GetSquareSize());

        if(active_piece)
            paint_piece_at(*active_piece, r, painter);
    }
}

void BoardEdit::_update_cursor_at_point(const QPointF &pt)
{
    if(m_dragging)
        setCursor(Qt::ClosedHandCursor);
    else if(get_board_rect().contains(QPoint(pt.x()+horizontalOffset(), pt.y()+verticalOffset())))
    {
        QModelIndex ind = indexAt(pt.toPoint());
        if(ind.isValid() && !ind.data(BoardModel::PieceRole).isNull())
            setCursor(Qt::OpenHandCursor);
        else
            setCursor(Qt::ArrowCursor);
    }
    else
        setCursor(Qt::ArrowCursor);
}


END_NAMESPACE_GKCHESS1;
