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

#include "boardview_p.h"
#include "gkchess_piece.h"
#include "gkchess_boardmodel.h"
#include "gkchess_board.h"
#include "gkchess_abstractboard.h"
#include "gkchess_ifactory_pieceicon.h"
#include "gkchess_isquare.h"
#include "gkchess_uiglobals.h"
#include "gutil_paintutils.h"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QRubberBand>
#include <QScrollBar>
#include <QApplication>
#include <QRubberBand>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QAbstractItemView>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(QT);
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GKCHESS1(UI);


// The following defines are used to parameterize the look and behavior
//  of the board view

#define DEFAULT_SQUARE_SIZE 75

/** The percent margin to use for selecting squares. You need a margin to prevent
 *  the interface being overly sensitive to misdrops. If the user drops a piece right on
 *  the edge between squares we'll call it a misdrop.
 *
 *  For example, a value of 0 would select a square if you clicked anywhere in it,
 *  but a value of 1 would not allow you to select a square at all. So this should be
 *  a small value close to 0 for best results.
*/
#define MARGIN_SQUARE_SELECTION 0.07

/** The margin of empty space to leave around all sides of the board. */
#define MARGIN_OUTER   12

/** The size of the region used to draw the board indices. */
#define MARGIN_INDICES 28

/** The drawing thickness of the board outline. */
#define BOARD_OUTLINE_THICKNESS 3

/** The thickness of the square highlight, given as a percentage of the square size. */
#define HIGHLIGHT_THICKNESS 0.12

/** Defines the distance the "current turn" arrow is from the board, as a
 *  factor of the square size.
*/
#define CURRENT_TURN_ARROW_OFFSET   0.4


/** Defines the duration of move animation, in seconds */
#define ANIM_SNAPBACKDURATION 0.25

#define ANIM_MOVE_DURATION 0.75

/** The size of a rect that determines when to start dragging a piece (as a factor of square size). */
#define DRAG_START_RECT_SIZE 0.35

#define SCROLL_SPEED_FACTOR 0.05

/** Piece icons will appear with a size of the square multiplied by this factor.
 *  \note this should be between 0 and 1
*/
#define PIECE_SIZE_FACTOR  0.825

/** The default cursor to use. */
#define CURSOR_DEFAULT Qt::ArrowCursor


/** Returns a rect with the same center but shrunken by the given factor */
static QRectF __get_shrunken_rect(const QRectF &r, double factor)
{
    return QRect(r.x() + r.width()*(1.0-factor)/2,
                 r.y() + r.height()*(1.0-factor)/2,
                 factor*r.width(),
                 factor*r.height());
}


BoardView_p::BoardView_p(QWidget *parent)
    :QAbstractItemView(parent),
      m_squareSize(DEFAULT_SQUARE_SIZE),
      m_editable(true),
      m_darkSquareColor(Qt::gray),
      m_lightSquareColor(Qt::white),
      m_activeSquareHighlightColor(Qt::yellow),
      i_factory(0),
      m_dragging(false),
      m_wasSquareActiveWhenPressed(false),
      m_selectionBand(new QRubberBand(QRubberBand::Rectangle, this)),
      m_animationGroup(new QSequentialAnimationGroup)
{
    setMouseTracking(true);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(_update_rubber_band()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(_update_rubber_band()));

    // Make sure the application resources were initialized
    InitializeApplicationResources();
}

BoardView_p::~BoardView_p()
{}


BoardModel *BoardView_p::GetBoardModel() const
{
    // We use static cast because we already validated that it's a BoardModel
    //  when they set the model.
    return static_cast<BoardModel *>(model());
}

void BoardView_p::SetIconFactory(IFactory_PieceIcon *i)
{
    if(i_factory)
        disconnect(i_factory, SIGNAL(NotifyIconsUpdated()),
                   viewport(), SLOT(update()));

    i_factory = i;
    connect(i_factory, SIGNAL(NotifyIconsUpdated()),
            viewport(), SLOT(update()));

    viewport()->update();
}


QRect BoardView_p::visualRect(const QModelIndex &index) const
{
    QRect ret;
    if(index.isValid())
        ret = visual_rectf(index.column(), index.row()).toAlignedRect();
    return ret;
}

QRectF BoardView_p::visual_rectf(int col, int row) const
{
    QRectF ret = item_rect(col, row);
    ret.translate(-horizontalOffset(),
                  -verticalOffset());
    return ret;
}

QRectF BoardView_p::item_rect(int col, int row) const
{
    return QRectF(get_board_rect().x() + col * GetSquareSize(),
                  get_board_rect().y() + get_board_rect().height() - (GetSquareSize() * (1 + row)),
                  GetSquareSize(), GetSquareSize());
}

void BoardView_p::scrollTo(const QModelIndex &index, ScrollHint)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if (rect.left() < area.left())
        horizontalScrollBar()->setValue(
            horizontalOffset() + rect.left() - area.left());
    else if (rect.right() > area.right())
        horizontalScrollBar()->setValue(
            horizontalOffset() + qMin(
                rect.right() - area.right(), rect.left() - area.left()));

    if (rect.top() < area.top())
        verticalScrollBar()->setValue(
            verticalOffset() + rect.top() - area.top());
    else if (rect.bottom() > area.bottom())
        verticalScrollBar()->setValue(
            verticalOffset() + qMin(
                rect.bottom() - area.bottom(), rect.top() - area.top()));
}

void BoardView_p::updateGeometries()
{
    QAbstractItemView::updateGeometries();

    // Update the scrollbars whenever our geometry changes
    horizontalScrollBar()->setRange(0, Max(0.0, get_board_rect().width() + 2*MARGIN_OUTER + MARGIN_INDICES + (CURRENT_TURN_ARROW_OFFSET + 1)*GetSquareSize() - viewport()->width()));
    verticalScrollBar()->setRange(0, Max(0.0, get_board_rect().height() + 2*MARGIN_OUTER + MARGIN_INDICES - viewport()->height()));
}

void BoardView_p::currentChanged(const QModelIndex &, const QModelIndex &)
{
    _update_rubber_band();
}

void BoardView_p::_animation_finished()
{
    piece_animation_p *anim = qobject_cast<piece_animation_p *>(sender());
    if(NULL != anim)
    {
        // Stop hiding the piece that we were animating
        //hide_piece_at_index();

        // Remove the animation from the group and delete it
        m_animationGroup->removeAnimation(anim);
        anim->deleteLater();
    }
}

void BoardView_p::_update_rubber_band()
{
    // Update the rubber band
    QModelIndex cur = currentIndex();
    m_selectionBand->setVisible(cur.isValid());
    if(cur.isValid()){
        m_selectionBand->setGeometry(visualRect(cur));
    }
}

QModelIndex BoardView_p::indexAt(const QPoint &p) const
{
    QModelIndex ret;
    QPointF p_t(p.x() + horizontalOffset(),
                p.y() + verticalOffset());
    if(get_board_rect().contains(p_t))
    {
        float x = p_t.x() - get_board_rect().x();
        float y = get_board_rect().y() + get_board_rect().height() - p_t.y();

        GASSERT(model());
        GASSERT(0 < x && 0 < y);

        // We want to have a margin within each square that doesn't select it
        int row = y/GetSquareSize();
        int col = x/GetSquareSize();
        QRectF selection_rect = __get_shrunken_rect(visual_rectf(col, row), 1.0-MARGIN_SQUARE_SELECTION);
        if(selection_rect.contains(p))
            ret = model()->index(row, col);
    }
    return ret;
}

QModelIndex BoardView_p::moveCursor(CursorAction ca, Qt::KeyboardModifiers modifiers)
{
    GUTIL_UNUSED(modifiers);
    QModelIndex ret;
    if(model())
    {
        QModelIndex cur = selectionModel()->currentIndex();
        if(cur.isValid())
        {
            switch(ca)
            {
            case MoveLeft:
                if(0 <= cur.column() - 1)
                    ret = model()->index(cur.row(), cur.column() - 1);
                break;
            case MoveUp:
                if(model()->rowCount() > cur.row() + 1)
                    ret = model()->index(cur.row() + 1, cur.column());
                break;
            case MoveRight:
                if(model()->columnCount() > cur.column() + 1)
                    ret = model()->index(cur.row(), cur.column() + 1);
                break;
            case MoveDown:
                if(0 <= cur.row() - 1)
                    ret = model()->index(cur.row() - 1, cur.column());
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

int BoardView_p::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int BoardView_p::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool BoardView_p::isIndexHidden(const QModelIndex &index) const
{
    // There are no hidden indices on a chess board
    return false;
}

void BoardView_p::setSelection(const QRect &r, QItemSelectionModel::SelectionFlags cmd)
{
    GUTIL_UNUSED(cmd);
    selectionModel()->select(indexAt(QPoint(r.center().x(), r.center().y())),
                             QItemSelectionModel::ClearAndSelect);
}

QRegion BoardView_p::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion ret;
    QModelIndexList il = selection.indexes();
    if(1 == il.length()){
        ret = QRegion(visualRect(il[0]));
    }
    return ret;
}

void BoardView_p::paintEvent(QPaintEvent *ev)
{
    ev->accept();
    QPainter painter(viewport());

    paint_board(painter, ev->rect());
}

void BoardView_p::resizeEvent(QResizeEvent *)
{
    updateGeometries();
}

void BoardView_p::paint_piece_at(const Piece &piece, const QRectF &r, QPainter &p)
{
    GASSERT(ind.isValid());

    if(piece.IsNull())
        return;

    QRectF dest_rect(r);
    QIcon ico;

    // First see if we have an icon factory
    if(i_factory)
    {
        ico = i_factory->GetIcon(piece);
    }

    p.save();

    // If we didn't find an icon for the piece, default to the unicode characters
    if(ico.isNull())
    {
        QFont font_pieces = p.font();
        font_pieces.setPixelSize(PIECE_SIZE_FACTOR * dest_rect.width());
        p.setFont(font_pieces);
        p.drawText(dest_rect, Qt::AlignHCenter|Qt::AlignBottom, QChar(piece.UnicodeValue()));
    }
    else
    {
        // Modify the dest rect by the piece size factor
        dest_rect = __get_shrunken_rect(dest_rect, PIECE_SIZE_FACTOR);

        // Paint the icon
        ico.paint(&p, dest_rect.toAlignedRect());
    }
    p.restore();
}

void BoardView_p::paint_board(QPainter &painter, const QRect &update_rect)
{
    GUTIL_UNUSED(update_rect);

    if(!model())
        return;

    QModelIndex cur_indx = currentIndex();
    QStyleOptionViewItem option = viewOptions();
    QStyle::State state = option.state;
    piece_animation_p *anim = 0;

    QBrush background = option.palette.base();
    QPen textPen(option.palette.color(QPalette::Text));
    QPen outline_pen(Qt::black);
    outline_pen.setWidth(BOARD_OUTLINE_THICKNESS);

    // Set up the painter
    painter.translate(-horizontalOffset(), -verticalOffset());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(viewport()->rect(), background);

    // Draw the "current turn" indicator
    {
        QRectF indicator_rect(get_board_rect().topRight().x() + GetSquareSize()*CURRENT_TURN_ARROW_OFFSET,
                              get_board_rect().topRight().y() + get_board_rect().height()/2 - (GetSquareSize()/2),
                              GetSquareSize(),
                              GetSquareSize());
        painter.fillRect(indicator_rect,
                         Piece::White == GetBoardModel()->GetBoard()->GetWhoseTurn() ?
                         m_lightSquareColor : m_darkSquareColor);
        painter.setPen(outline_pen);
        painter.drawRect(indicator_rect);
    }

    // Shade the squares and paint the pieces
    QFont font_indices = painter.font();
    font_indices.setPixelSize(GetSquareSize() * 0.25);

    painter.setFont(font_indices);
    for(int c = 0; c < model()->columnCount(); ++c)
    {
        for(int r = 0; r < model()->rowCount(); ++r)
        {
            QModelIndex ind = model()->index(r, c);
            QRectF tmp = item_rect(c, r);

            if((c & 0x1) == (r & 0x1))
            {
                // Color the dark squares
                painter.fillRect(tmp, m_darkSquareColor);
            }
            else
            {
                // Color the light squares
                painter.fillRect(tmp, m_lightSquareColor);
            }

            // Paint the pieces
            Piece pc = ind.data(BoardModel::PieceRole).value<Piece>();
            if(!pc.IsNull())
            {
                paint_piece_at(pc, tmp, painter);
            }
        }
    }

    // paint the vertical borders and indices
    {
        float file_width = get_board_rect().width() / GetBoardModel()->columnCount();
        QPointF p1(get_board_rect().topLeft());
        QPointF p2(p1.x(), p1.y() + get_board_rect().height());
        for(int i = 0; i < GetBoardModel()->columnCount(); ++i)
        {
            // Draw the line
            if(0 != i){
                painter.setPen(outline_pen);
                painter.drawLine(p1, p2);
            }

            // Draw the index
            QRectF text_rect(p2, QPointF(p2.x() + file_width, p2.y() + MARGIN_INDICES));
            painter.setPen(textPen);
            painter.drawText(text_rect, Qt::AlignCenter,  QString('a' + i));

            p1.setX(p1.x() + file_width);
            p2.setX(p2.x() + file_width);
        }
    }

    // paint the horizontal borders and indices
    {
        float rank_height = get_board_rect().height() / GetBoardModel()->rowCount();
        QPointF p1(get_board_rect().topLeft());
        QPointF p2(QPoint(p1.x() + get_board_rect().width(), p1.y()));
        for(int i = 0; i < GetBoardModel()->rowCount(); ++i)
        {
            // Draw the line
            if(0 != i){
                painter.setPen(outline_pen);
                painter.drawLine(p1, p2);
            }

            // Draw the index
            QRectF text_rect(QPointF(p1.x() - MARGIN_INDICES, p1.y()),
                             QPointF(p1.x(), p1.y() + rank_height));
            painter.setPen(textPen);
            painter.drawText(text_rect, Qt::AlignCenter,  QString('8' - i));

            p1.setY(p1.y() + rank_height);
            p2.setY(p2.y() + rank_height);
        }
    }

    // Draw the board outline (after shading the squares)
    painter.setPen(outline_pen);
    painter.drawRect(get_board_rect());

    // Apply any square highlighting
    painter.save();
    for(typename Map<QModelIndex, SquareFormatOptions>::const_iterator iter = m_formatOpts.begin();
        iter != m_formatOpts.end();
        ++iter)
    {
        ISquare const *sqr = GetBoardModel()->ConvertIndexToSquare(iter->Key());

        QRectF cur_rect = item_rect(sqr->GetColumn(), sqr->GetRow());
        QPainterPath path;
        QPainterPath subtracted;
        path.addRect(cur_rect);
        subtracted.addRect(__get_shrunken_rect(cur_rect, 1.0-HIGHLIGHT_THICKNESS));
        painter.fillPath(path.subtracted(subtracted), iter->Value().HighlightColor);
    }
    painter.restore();

    // If we're animating a move, paint that now
    if(anim && !anim->piece.IsNull())
    {
        QPointF v = anim->currentValue().value<QPointF>();
        if(!v.isNull()){
            paint_piece_at(anim->piece,
                           QRectF(v.x()-GetSquareSize()/2, v.y()-GetSquareSize()/2, GetSquareSize(), GetSquareSize()),
                           painter);
        }
    }


    // If we're dragging then paint the piece being dragged (always paint at the end so it's on top)
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

    // Any debug drawing?
//    painter.setPen(Qt::red);
//    painter.drawPoint(temp_point);
//    painter.drawRect(temp_rect);
}

void BoardView_p::SetBoardModel(BoardModel *bm)
{
    ClearSquareHighlighting();

    // Disconnect the old model
    if(model()){
        disconnect(GetBoardModel()->GetBoard(), SIGNAL(NotifyPieceAboutToBeMoved(const GKChess::MoveData &)),
                   this, SLOT(_piece_about_to_move(const GKChess::MoveData &)));
        disconnect(GetBoardModel()->GetBoard(), SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
                   this, SLOT(_piece_moved(const GKChess::MoveData &)));
    }

    setModel(bm);

    connect(bm->GetBoard(), SIGNAL(NotifyPieceAboutToBeMoved(const GKChess::MoveData &)),
            this, SLOT(_piece_about_to_move(const GKChess::MoveData &)));
    connect(bm->GetBoard(), SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
            this, SLOT(_piece_moved(const GKChess::MoveData &)));

    updateGeometries();
}

void BoardView_p::SetDarkSquareColor(const QColor &c)
{
    m_darkSquareColor = c;
    viewport()->update();
}

void BoardView_p::SetLightSquareColor(const QColor &c)
{
    m_lightSquareColor = c;
    viewport()->update();
}

void BoardView_p::SetActiveSquareHighlightColor(const QColor &c)
{
    m_activeSquareHighlightColor = c;
    viewport()->update();
}

void BoardView_p::SetSquareSize(float s)
{
    m_squareSize = s;
    updateGeometries();
    viewport()->update();
    _update_rubber_band();
}


void BoardView_p::wheelEvent(QWheelEvent *ev)
{
    // Control-scroll changes the board size
    if(QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
    {
        float tmp = GetSquareSize() + SCROLL_SPEED_FACTOR * ev->delta();
        if(0.0 < tmp)
            SetSquareSize(tmp);
        ev->accept();
    }

    QAbstractItemView::wheelEvent(ev);

    // update the rubber band in case we scrolled
    _update_rubber_band();
}

void BoardView_p::attempt_move(const QModelIndex &s, const QModelIndex &d)
{
    GetBoardModel()->Move(s, d);
}

void BoardView_p::animate_snapback(const QPointF &from, const QModelIndex &s)
{
    //hide_piece_at_index(s);
    animate_move(*GetBoardModel()->ConvertIndexToSquare(s)->GetPiece(),
                  from,
                  item_rect(s.column(), s.row()).center(),
                  ANIM_SNAPBACKDURATION * 1000,
                  //QEasingCurve::InOutQuad
                  //QEasingCurve::InOutCubic
                  //QEasingCurve::InOutQuart
                  //QEasingCurve::InOutQuint
                  //QEasingCurve::InOutCirc
                  //QEasingCurve::InOutSine

                  //QEasingCurve::OutQuad
                  //QEasingCurve::OutCubic
                  //QEasingCurve::OutQuart
                  QEasingCurve::OutQuint
                  //QEasingCurve::OutExpo
                  );
}

void BoardView_p::animate_move(const Piece &p, const QPointF &source, const QPointF &dest, int dur, int easing_curve)
{
    //if(QVariantAnimation::Running != m_animationGroup->state())
    {
        piece_animation_p *anim = new piece_animation_p;
        connect(anim, SIGNAL(valueChanged(const QVariant &)), viewport(), SLOT(update()));
        connect(anim, SIGNAL(finished()), this, SLOT(_animation_finished()));

        anim->piece = p;
        anim->setStartValue(source);
        anim->setEndValue(dest);
        anim->setEasingCurve((QEasingCurve::Type)easing_curve);
        anim->setDuration(dur);

        m_animationGroup->addAnimation(anim);
        m_animationGroup->start();
    }
}

void BoardView_p::HighlightSquare(const QModelIndex &i, const QColor &c)
{
    if(i.isValid())
    {
        SquareFormatOptions sfo;
        sfo.HighlightColor = c;
        m_formatOpts[i] = sfo;
        viewport()->update();
    }
}

void BoardView_p::HighlightSquares(const QModelIndexList &il, const QColor &c)
{
    SquareFormatOptions sfo;
    sfo.HighlightColor = c;
    foreach(const QModelIndex &i, il)
    {
        if(i.isValid())
            m_formatOpts[i] = sfo;
    }
    viewport()->update();
}

void BoardView_p::ClearSquareHighlighting()
{
    m_formatOpts.Clear();
    viewport()->update();
}

QRectF BoardView_p::get_board_rect() const
{
    QRectF ret;
    if(model())
    {
        ret = QRectF(MARGIN_OUTER + MARGIN_INDICES,
                     MARGIN_OUTER,
                     GetSquareSize() * model()->columnCount(),
                     GetSquareSize() * model()->rowCount());
    }
    return ret;
}

void BoardView_p::mousePressEvent(QMouseEvent *ev)
{
    GASSERT(m_dragOffset.isNull());

    if(!Editable() ||
            QAbstractAnimation::Running == get_animation()->state())
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

void BoardView_p::mouseReleaseEvent(QMouseEvent *ev)
{
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

void BoardView_p::mouseMoveEvent(QMouseEvent *ev)
{
    QAbstractItemView::mouseMoveEvent(ev);

    if(get_board_rect().translated(-horizontalOffset(), -verticalOffset()).contains(ev->posF()))
    {
        setCurrentIndex(indexAt(ev->pos()));
    }

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
            HighlightSquare(ind,
                            AbstractBoard::ValidMove == GetBoardModel()->ValidateMove(m_activeSquare, ind) ?
                                Qt::green : Qt::red);
    }

    if(Editable() && !m_mousePressLoc.isNull())
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

void BoardView_p::mouseDoubleClickEvent(QMouseEvent *ev)
{
    // Only allow this event to our our subclass if we are editable
    if(Editable())
        QAbstractItemView::mouseDoubleClickEvent(ev);
}

void BoardView_p::_piece_about_to_move(const MoveData &md)
{

}

void BoardView_p::_piece_moved(const MoveData &md)
{
    // Animate the piece moving
    if(QAnimationGroup::Running == m_animationGroup->state())
    {
        // Queue the animation after the others are finished
        //m_animationGroup->addAnimation();
    }
    else
    {
        // Since we're not running, start a new animation
//        piece_animation_p *anim(new piece_animation_p);
//        anim->hidden_index = s;
//        anim->piece = p;
//        m_animationGroup->addAnimation(anim);

//        hide_piece_at_index(anim->hidden_index);
//        m_animationGroup->start();
    }
}

void BoardView_p::_update_cursor_at_point(const QPointF &pt)
{
    if(!Editable())
        setCursor(CURSOR_DEFAULT);
    else if(m_dragging)
        setCursor(Qt::ClosedHandCursor);
    else if(get_board_rect().contains(QPoint(pt.x()+horizontalOffset(), pt.y()+verticalOffset())))
    {
        QModelIndex ind = indexAt(pt.toPoint());
        if(ind.isValid() && !ind.data(BoardModel::PieceRole).isNull())
            setCursor(Qt::OpenHandCursor);
        else
            setCursor(CURSOR_DEFAULT);
    }
    else
        setCursor(CURSOR_DEFAULT);
}
