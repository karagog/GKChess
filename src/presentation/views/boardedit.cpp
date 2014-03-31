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


NAMESPACE_GKCHESS1(UI);


BoardEdit::BoardEdit(QWidget *p)
    :BoardView(p),
      m_dragging(false)
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

    if(!m_activeSquare.isValid() && get_board_rect().contains(ev->pos()))
    {
        QPoint p(ev->pos());
        m_activeSquare = indexAt(ev->pos());

        QPointF center = ind_2_rect(m_activeSquare.column(), m_activeSquare.row()).center();
        m_dragging = true;

        // Add highlighting to the active square
        HighlightSquare(m_activeSquare, GetActiveSquareHighlightColor());

        viewport()->update();

        GASSERT(m_activeSquare.isValid());
    }

    _update_cursor_at_point(ev->posF());
}

void BoardEdit::mouseReleaseEvent(QMouseEvent *ev)
{
    BoardView::mousePressEvent(ev);

    if(m_activeSquare.isValid())
    {
        if(get_board_rect().contains(ev->pos()))
            attempt_move(m_activeSquare, indexAt(ev->pos()));
        else
        {
            // If they dropped the piece off the board, snap it back to the start location
            animate_snapback(ev->pos(), m_activeSquare);
        }
    }

    m_activeSquare = QModelIndex();
    m_dragging = false;

    ClearSquareHighlighting();
    viewport()->update();

    _update_cursor_at_point(ev->posF());
}

void BoardEdit::mouseMoveEvent(QMouseEvent *ev)
{
    BoardView::mouseMoveEvent(ev);

    if(m_dragging)
    {
        viewport()->update();
    }

    _update_cursor_at_point(ev->posF());
}

void BoardEdit::mouseDoubleClickEvent(QMouseEvent *ev)
{
    BoardView::mouseDoubleClickEvent(ev);

    // Open an editor so they can modify pieces
    QModelIndex ind = indexAt(ev->pos());
    if(ind.isValid())
    {
        edit(ind);
        ev->accept();
    }
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

        if(active_piece && update_rect.intersects(r.toAlignedRect()))
            paint_piece_at(*active_piece, r, painter);
    }
}

void BoardEdit::_update_cursor_at_point(const QPointF &pt)
{
    if(m_dragging)
        setCursor(Qt::ClosedHandCursor);
    else if(get_board_rect().contains(QPoint(pt.x()+horizontalOffset(), pt.y()+verticalOffset())))
        setCursor(Qt::OpenHandCursor);
    else
        setCursor(Qt::ArrowCursor);
}


END_NAMESPACE_GKCHESS1;
