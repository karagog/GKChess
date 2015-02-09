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
#include "boardmodel_p.h"
#include "gkchess_piece.h"
#include "gkchess_board.h"
#include "gkchess_board.h"
#include "gkchess_ifactory_pieceicon.h"
#include "gkchess_square.h"
#include "gkchess_uiglobals.h"
#include <gutil/paintutils.h>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QRubberBand>
#include <QScrollBar>
#include <QApplication>
#include <QRubberBand>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAbstractItemView>
#include <QDialog>
#include <QPushButton>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(Qt);
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

#define ANIM_MOVE_DURATION 0.35

/** Defines the easing function we use to animate piece movements. */
#define ANIM_MOVE_EASING   QEasingCurve::OutQuad

/** The size of a rect that determines when to start dragging a piece (as a factor of square size). */
#define DRAG_START_RECT_SIZE 0.35

#define SCROLL_SPEED_FACTOR 0.05

/** Piece icons will appear with a size of the square multiplied by this factor.
 *  \note this should be between 0 and 1
*/
#define PIECE_SIZE_FACTOR  0.825

/** The threat count indicators will be this factor of the square size. */
#define THREAT_COUNT_SIZE_FACTOR  0.2

/** The distance between the edge of the square and the threat count text, as a factor of square size. */
#define THREAT_COUNT_MARGIN_FACTOR  0.025

/** The default cursor to use. */
#define CURSOR_DEFAULT ::Qt::ArrowCursor


/** Returns a rect with the same center but shrunken by the given factor */
static QRectF __get_shrunken_rect(const QRectF &r, double factor)
{
    return QRect(r.x() + r.width()*(1.0-factor)/2,
                 r.y() + r.height()*(1.0-factor)/2,
                 factor*r.width(),
                 factor*r.height());
}

/** Returns a rect centered at the point with the given size. */
static QRectF __rect_centered_at(const QPointF &p, double s)
{
    return QRectF(p.x()-s/2, p.y()-s/2, s, s);
}



struct piece_animation_t :
    public QVariantAnimation
{
    Piece piece;

    void updateCurrentValue(const QVariant &){}

    piece_animation_t(Piece const &p, QObject *par = 0)
        :QVariantAnimation(par),piece(p)
    {}
};





BoardView_p::BoardView_p(QWidget *parent)
    :QAbstractItemView(parent),
      m_squareSize(DEFAULT_SQUARE_SIZE),
      m_orientation(NormalOrientation),
      m_editable(true),
      m_showThreatCounts(false),
      m_darkSquareColor(::Qt::gray),
      m_lightSquareColor(::Qt::white),
      m_activeSquareHighlightColor(::Qt::yellow),
      i_factory(0),
      m_dragging(false),
      m_wasSquareActiveWhenPressed(false),
      m_selectionBand(new QRubberBand(QRubberBand::Rectangle, this)),
      m_animation(0)
{
    setMouseTracking(true);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(_update_rubber_band()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(_update_rubber_band()));

    // Make sure the application resources were initialized
    InitializeApplicationResources();
}

BoardView_p::~BoardView_p()
{}


BoardModel_p *BoardView_p::GetBoardModel() const
{
    // We use static cast because we already validated that it's a BoardModel_p
    //  when they set the model.
    return static_cast<BoardModel_p *>(model());
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
    QRectF ret;
    if(0 <= col && col < (int)GetBoardModel()->GetBoard().ColumnCount() &&
            0 <= row && row < (int)GetBoardModel()->GetBoard().RowCount())
    {
        // Map the indices to account for the orientation
        switch(GetOrientation())
        {
        case BoardView_p::Rotated_180:
            row = GetBoardModel()->GetBoard().RowCount() - row - 1;
            col = GetBoardModel()->GetBoard().ColumnCount() - col - 1;
            break;
        case BoardView_p::NormalOrientation:
        default: break;
        }

        ret = QRectF(get_board_rect().x() + col * GetSquareSize(),
                     get_board_rect().y() + get_board_rect().height() - (GetSquareSize() * (1 + row)),
                     GetSquareSize(), GetSquareSize());
    }
    return ret;
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
    m_animationBoard.Clear();

    m_animation->deleteLater();
    m_animation = 0;
}

void BoardView_p::_update_rubber_band()
{
    // Update the rubber band
    QModelIndex cur = currentIndex();
    m_selectionBand->setVisible(cur.isValid());
    if(cur.isValid())
        m_selectionBand->setGeometry(visualRect(cur));
}

QModelIndex BoardView_p::indexAt(const QPoint &p) const
{
    QModelIndex ret;
    QPointF p_t(p.x() + horizontalOffset(),
                p.y() + verticalOffset());
    if(get_board_rect().contains(p_t))
    {
        // Map the point to account for the board's orientation
        QRectF board_rect = get_board_rect();
        switch(GetOrientation())
        {
        case BoardView_p::Rotated_180:
            p_t.setX(board_rect.left() + (board_rect.right() - p_t.x()));
            p_t.setY(board_rect.top() + (board_rect.bottom() - p_t.y()));
            break;
        case BoardView_p::NormalOrientation:
        default: break;
        }

        float x = p_t.x() - get_board_rect().x();
        float y = get_board_rect().y() + get_board_rect().height() - p_t.y();

        GASSERT(model());

        // We want to have a margin within each square that doesn't select it
        int row = y/GetSquareSize();
        int col = x/GetSquareSize();
        QRectF selection_rect = __get_shrunken_rect(visual_rectf(col, row), 1.0-MARGIN_SQUARE_SELECTION);
        if(selection_rect.contains(p))
            ret = model()->index(row, col);
    }
    return ret;
}

QModelIndex BoardView_p::moveCursor(CursorAction ca,::Qt::KeyboardModifiers modifiers)
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

bool BoardView_p::isIndexHidden(const QModelIndex &) const
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

void BoardView_p::paint_piece_at(Piece const &piece, const QRectF &r, QPainter &p)
{
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
        p.drawText(dest_rect, ::Qt::AlignHCenter|::Qt::AlignBottom, QChar(piece.UnicodeValue()));
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

    QStyleOptionViewItem option = viewOptions();
    //QStyle::State state = option.state;

    QBrush background = option.palette.base();
    QPen textPen(option.palette.color(QPalette::Text));
    QPen outline_pen(::Qt::black);
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
                         Piece::White == GetBoardModel()->GetBoard().GetWhoseTurn() ?
                         m_lightSquareColor : m_darkSquareColor);
        painter.setPen(outline_pen);
        painter.drawRect(indicator_rect);
    }

    // Shade the squares and paint the pieces
    QFont font_indices = painter.font();
    font_indices.setPixelSize(GetSquareSize() * 0.25);

    QFont font_threatCount = painter.font();
    font_threatCount.setPixelSize(GetSquareSize() * THREAT_COUNT_SIZE_FACTOR);

    painter.setFont(font_threatCount);

    // If we are animating, then we paint the animation board
    Board const *board = m_animationBoard.IsNull() ? &GetBoardModel()->GetBoard() : m_animationBoard.Data();
    int num_cols = board->ColumnCount();
    int num_rows = board->RowCount();

    for(int c = 0; c < num_cols; ++c)
    {
        for(int r = 0; r < num_rows; ++r)
        {
            QRectF tmp = item_rect(c, r);
            Square const &cur_sqr(board->SquareAt(c, r));

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

            // If we are showing threat counts, paint that now:
            if(GetShowThreatCounts())
            {
                QRectF threat_rect(tmp.x(), tmp.y(),
                                   tmp.width() * THREAT_COUNT_SIZE_FACTOR,
                                   tmp.height() * THREAT_COUNT_SIZE_FACTOR);
                painter.drawText(threat_rect.translated(THREAT_COUNT_MARGIN_FACTOR*GetSquareSize(),
                                                        THREAT_COUNT_MARGIN_FACTOR*GetSquareSize()),
                                ::Qt::AlignCenter,
                                 QString("%1").arg(cur_sqr.GetThreatCount(Piece::White)));
                painter.drawText(threat_rect.translated(tmp.width()*(1 - THREAT_COUNT_SIZE_FACTOR)-THREAT_COUNT_MARGIN_FACTOR*GetSquareSize(),
                                                        THREAT_COUNT_MARGIN_FACTOR*GetSquareSize()),
                                ::Qt::AlignCenter,
                                 QString("%1").arg(cur_sqr.GetThreatCount(Piece::Black)));
            }

            // Paint the pieces
            Piece const &pc = cur_sqr.GetPiece();
            if(!pc.IsNull() && (!m_dragging || (m_activeSquare.column() != c ||
                                      m_activeSquare.row() != r)))
            {
                paint_piece_at(pc, tmp, painter);
            }
        }
    }

    // paint the vertical borders and indices
    {
        painter.setFont(font_indices);
        float file_width = get_board_rect().width() / num_cols;
        QPointF p1(get_board_rect().topLeft());
        QPointF p2(p1.x(), p1.y() + get_board_rect().height());
        for(int i = 0; i < num_cols; ++i)
        {
            // Draw the line
            if(0 != i){
                painter.setPen(outline_pen);
                painter.drawLine(p1, p2);
            }

            // Draw the index
            QString txt;
            QRectF text_rect(p2, QPointF(p2.x() + file_width, p2.y() + MARGIN_INDICES));
            painter.setPen(textPen);
            switch(GetOrientation())
            {
            case NormalOrientation:
                txt = QString('a' + i);
                break;
            case Rotated_180:
                txt = QString('h' - i);
                break;
            default:
                GASSERT(false);
            }
            painter.drawText(text_rect,::Qt::AlignCenter,  txt);

            p1.setX(p1.x() + file_width);
            p2.setX(p2.x() + file_width);
        }
    }

    // paint the horizontal borders and indices
    {
        float rank_height = get_board_rect().height() / num_rows;
        QPointF p1(get_board_rect().topLeft());
        QPointF p2(QPoint(p1.x() + get_board_rect().width(), p1.y()));
        for(int i = 0; i < num_rows; ++i)
        {
            // Draw the line
            if(0 != i){
                painter.setPen(outline_pen);
                painter.drawLine(p1, p2);
            }

            // Draw the index
            QString txt;
            QRectF text_rect(QPointF(p1.x() - MARGIN_INDICES, p1.y()),
                             QPointF(p1.x(), p1.y() + rank_height));
            painter.setPen(textPen);
            switch(GetOrientation())
            {
            case NormalOrientation:
                txt = QString('8' - i);
                break;
            case Rotated_180:
                txt = QString('1' + i);
                break;
            default:
                GASSERT(false);
            }
            painter.drawText(text_rect,::Qt::AlignCenter,  txt);

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
        Square const *sqr = GetBoardModel()->ConvertIndexToSquare(iter->Key());
        QRectF cur_rect = item_rect(sqr->GetColumn(), sqr->GetRow());
        QPainterPath path;
        QPainterPath subtracted;
        path.addRect(cur_rect);
        subtracted.addRect(__get_shrunken_rect(cur_rect, 1.0-HIGHLIGHT_THICKNESS));
        painter.fillPath(path.subtracted(subtracted), iter->Value().HighlightColor);
    }
    painter.restore();

    // If we're animating a move, paint that now
    if(NULL != m_animation)
    {
        for(int a = 0; a < m_animation->animationCount(); ++a)
        {
            piece_animation_t *anim = static_cast<piece_animation_t *>(m_animation->animationAt(a));
            QPointF v = anim->currentValue().value<QPointF>();
            if(!v.isNull())
                paint_piece_at(anim->piece, __rect_centered_at(v, GetSquareSize()), painter);
        }
    }


    // If we're dragging then paint the piece being dragged (always paint at the end so it's on top)
    if(m_dragging && m_activeSquare.isValid())
    {
        QPoint cur_pos = mapFromGlobal(QCursor::pos());
        Piece const &active_piece = GetBoardModel()->ConvertIndexToSquare(m_activeSquare)->GetPiece();

        QRectF r(cur_pos.x() - GetSquareSize()/2 + horizontalOffset(),
                 cur_pos.y() - GetSquareSize()/2 + verticalOffset(),
                 GetSquareSize(), GetSquareSize());

        if(!active_piece.IsNull())
            paint_piece_at(active_piece, r, painter);
    }

    // Any debug drawing?
//    painter.setPen(::Qt::red);
//    painter.drawPoint(temp_point);
//    painter.drawRect(temp_rect);
}

void BoardView_p::SetBoardModel(BoardModel_p *bm)
{
    ClearSquareHighlighting();

    // Disconnect the old model
    if(model()){
        disconnect(&GetBoardModel()->GetBoard(), SIGNAL(NotifyPieceAboutToBeMoved(const GKChess::MoveData &)),
                   this, SLOT(_piece_about_to_move(const GKChess::MoveData &)));
        disconnect(&GetBoardModel()->GetBoard(), SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
                   this, SLOT(_piece_moved(const GKChess::MoveData &)));
    }

    setModel(bm);

    connect(&bm->GetBoard(), SIGNAL(NotifyPieceAboutToBeMoved(const GKChess::MoveData &)),
            this, SLOT(_piece_about_to_move(const GKChess::MoveData &)));
    connect(&bm->GetBoard(), SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
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
    if(QApplication::keyboardModifiers().testFlag(::Qt::ControlModifier))
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

bool BoardView_p::attempt_move(const QModelIndex &s, const QModelIndex &d)
{
    return Board::ValidMove == GetBoardModel()->Move(s, d, this);
}

void BoardView_p::animate_snapback(const QPointF &from, const QModelIndex &s)
{
    MoveData md;
    Square const *sqr = GetBoardModel()->ConvertIndexToSquare(s);
    md.PieceMoved = sqr->GetPiece();
    animate_move(md,
                 from,
                 item_rect(s.column(), s.row()).center(),
                 *sqr,
                 1000 * ANIM_SNAPBACKDURATION,
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

void BoardView_p::animate_move(const MoveData &md,
                               const QPointF &source, const QPointF &dest,
                               Square const &sqr_source,
                               int dur, QEasingCurve::Type easing_curve)
{
    // Make a copy of the board and populate it with the correct position information
    m_animationBoard = new Board(GetBoardModel()->GetBoard());

    // Remove the source piece from the board because it is being animated
    m_animationBoard->SetPiece(Piece(), sqr_source);

    piece_animation_t *anim = new piece_animation_t(md.PieceMoved, this);
    m_animation = new QSequentialAnimationGroup(this);
    m_animation->addAnimation(anim);

    connect(anim, SIGNAL(valueChanged(const QVariant &)), viewport(), SLOT(update()));
    connect(m_animation, SIGNAL(finished()), this, SLOT(_animation_finished()));

    anim->setStartValue(source);
    anim->setEndValue(dest);
    anim->setEasingCurve((QEasingCurve::Type)easing_curve);
    anim->setDuration(dur);

    m_animation->start();
}

void BoardView_p::animate_castle(const MoveData &md,
                                 const Square &king_src, const Square &king_dest,
                                 const Square &rook_src, const Square &rook_dest,
                                 int dur,
                                 QEasingCurve::Type easing_curve)
{
    Piece::AllegienceEnum a = md.PieceMoved.GetAllegience();

    // Make a copy of the board and populate it with the correct position information
    m_animationBoard = new Board(GetBoardModel()->GetBoard());

    // Remove the rook and king from the board because they're being animated
    m_animationBoard->SetPiece(Piece(), king_src);
    m_animationBoard->SetPiece(Piece(), rook_src);

    m_animation = new QParallelAnimationGroup(this);
    connect(m_animation, SIGNAL(finished()), this, SLOT(_animation_finished()));

    piece_animation_t *anim;

    if(m_dragging)
    {
        // If they dropped the king, put him at the dest while we animate only the rook
        m_animationBoard->SetPiece(Piece(Piece::King, a), king_dest);
    }
    else
    {
        // Only animate the king moving if they didn't drag-drop him
        anim = new piece_animation_t(Piece(Piece::King, a), this);
        anim->setStartValue(item_rect(king_src.GetColumn(), king_src.GetRow()).center());
        anim->setEndValue(item_rect(king_dest.GetColumn(), king_dest.GetRow()).center());
        anim->setEasingCurve(easing_curve);
        anim->setDuration(dur);
        m_animation->addAnimation(anim);
    }

    anim = new piece_animation_t(Piece(Piece::Rook, a), this);
    connect(anim, SIGNAL(valueChanged(const QVariant &)), viewport(), SLOT(update()));
    anim->setStartValue(item_rect(rook_src.GetColumn(), rook_src.GetRow()).center());
    anim->setEndValue(item_rect(rook_dest.GetColumn(), rook_dest.GetRow()).center());
    anim->setEasingCurve(easing_curve);
    anim->setDuration(dur);
    m_animation->addAnimation(anim);

    m_animation->start();
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
    if(!Editable() ||  NULL != m_animation)
        return;

    QModelIndex ind = indexAt(ev->pos());
    if(ind.isValid() && ev->button() ==::Qt::LeftButton)
    {
        Piece target_piece = ind.data(BoardModel_p::PieceRole).value<Piece>();
        if(target_piece.IsNull())
            return;

        if(m_activeSquare.isValid())
        {
            Piece active_piece = m_activeSquare.data(BoardModel_p::PieceRole).value<Piece>();
            m_wasSquareActiveWhenPressed = m_activeSquare == ind;

            if(target_piece.GetAllegience() != active_piece.GetAllegience() ||
                    (active_piece.GetType() == Piece::King &&
                     target_piece.GetType() == Piece::Rook))
            {
                return;
            }
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

    _update_cursor_at_point(ev->localPos());
}

void BoardView_p::mouseReleaseEvent(QMouseEvent *ev)
{
    if(m_activeSquare.isValid())
    {
        QPointF pt_translated(ev->pos().x() + horizontalOffset(), ev->pos().y() + verticalOffset());
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
                if(!attempt_move(ind_active, ind_released))
                {
                    // If the move was invalid, then snap the piece back to the source if they were dragging
                    if(m_dragging)
                        animate_snapback(pt_translated, ind_active);
                }
            }
        }
        else
        {
            // If they dropped the piece off the board, snap it back to the start location
            if(m_dragging)
                animate_snapback(pt_translated, ind_active);
        }
    }

    m_dragging = false;
    m_mousePressLoc = QPoint();
    m_wasSquareActiveWhenPressed = false;

    if(!m_activeSquare.isValid())
        ClearSquareHighlighting();
    viewport()->update();

    _update_cursor_at_point(ev->localPos());
}

void BoardView_p::mouseMoveEvent(QMouseEvent *ev)
{
    QAbstractItemView::mouseMoveEvent(ev);

    if(get_board_rect().translated(-horizontalOffset(), -verticalOffset()).contains(ev->localPos()))
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
                            Board::ValidMove == GetBoardModel()->ValidateMove(m_activeSquare, ind) ?
                               ::Qt::green : ::Qt::red);
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

    _update_cursor_at_point(ev->localPos());
}

void BoardView_p::mouseDoubleClickEvent(QMouseEvent *ev)
{
    // Only allow this event to our our subclass if we are editable
    if(Editable())
        QAbstractItemView::mouseDoubleClickEvent(ev);
}

void BoardView_p::_piece_about_to_move(const MoveData &md)
{
    if(NULL != m_animation)
    {
        // An animation is currently running, so cancel it and start a new one
        QAbstractAnimation *anim = m_animation;
        m_animation = 0;
        anim->deleteLater();
        m_animationBoard.Clear();
    }

    if(md.NoCastle != md.CastleType)
    {
        // Animate castling
        Board const &board( GetBoardModel()->GetBoard() );
        Piece::AllegienceEnum allegience = md.Whose();
        Square const *king_dest;
        Square const *rook_src, *rook_dest;
        switch(md.CastleType)
        {
        case MoveData::CastleHSide:
            if(Piece::White == allegience){
                king_dest = &board.SquareAt(6, 0);
                rook_src = &board.SquareAt(board.GetCastleWhiteH(), 0);
                rook_dest = &board.SquareAt(5, 0);
            }
            else{
                king_dest = &board.SquareAt(6, 7);
                rook_src = &board.SquareAt(board.GetCastleBlackH(), 7);
                rook_dest = &board.SquareAt(5, 7);
            }
            break;
        case MoveData::CastleASide:
            if(Piece::White == allegience){
                king_dest = &board.SquareAt(2, 0);
                rook_src = &board.SquareAt(board.GetCastleWhiteA(), 0);
                rook_dest = &board.SquareAt(3, 0);
            }
            else{
                king_dest = &board.SquareAt(2, 7);
                rook_src = &board.SquareAt(board.GetCastleBlackA(), 7);
                rook_dest = &board.SquareAt(3, 7);
            }
            break;
        default: break;
        }

        animate_castle(md, md.Source, *king_dest, *rook_src, *rook_dest,
                       ANIM_MOVE_DURATION *1000,
                       ANIM_MOVE_EASING);
    }
    else if(!md.PiecePromoted.IsNull())
    {
        // TODO: Animate pawn promotion?
    }
    else
    {
        // Animate regular moving, unless they're dragging
        if(!m_dragging)
        {
            animate_move(md,
                         item_rect(md.Source.GetColumn(), md.Source.GetRow()).center(),
                         item_rect(md.Destination.GetColumn(), md.Destination.GetRow()).center(),
                         md.Source,
                         ANIM_MOVE_DURATION * 1000,
                         ANIM_MOVE_EASING);
        }
    }
}

void BoardView_p::_piece_moved(const MoveData &)
{
    // Do nothing after a piece moves
}

void BoardView_p::_update_cursor_at_point(const QPointF &pt)
{
    if(!Editable())
        setCursor(CURSOR_DEFAULT);
    else if(m_dragging)
        setCursor(::Qt::ClosedHandCursor);
    else if(get_board_rect().contains(QPoint(pt.x()+horizontalOffset(), pt.y()+verticalOffset())))
    {
        QModelIndex ind = indexAt(pt.toPoint());
        if(ind.isValid() && !ind.data(BoardModel_p::PieceRole).isNull())
            setCursor(::Qt::OpenHandCursor);
        else
            setCursor(CURSOR_DEFAULT);
    }
    else
        setCursor(CURSOR_DEFAULT);
}

void BoardView_p::SetShowThreatCounts(bool b)
{
    m_showThreatCounts = b;
    viewport()->update();
}

void BoardView_p::SetOrientation(OrientationEnum o)
{
    m_orientation = o;
    m_activeSquare = QModelIndex();
    ClearSquareHighlighting();
    viewport()->update();
}


#define PROMOTED_WIDGET_WIDTH 400
#define PROMOTED_WIDGET_HEIGHT 100
#define PROMOTED_BUTTON_SIZE 100

class __promoted_piece_selector : public QDialog
{
    Q_OBJECT
public:

    __promoted_piece_selector(Piece::AllegienceEnum a, IFactory_PieceIcon *fac, QWidget *p = 0)
        :QDialog(p),
          _p_SelectedPiece(Piece::NoPiece, a)
    {
        setWindowTitle(tr("Select Promoted Piece"));
        resize(PROMOTED_WIDGET_WIDTH, PROMOTED_WIDGET_HEIGHT);
        new QHBoxLayout(this);

        setWindowFlags(::Qt::Dialog |
                       ::Qt::CustomizeWindowHint |
                       ::Qt::WindowTitleHint |
                       ::Qt::WindowStaysOnTopHint);
        setWindowModality(::Qt::WindowModal);

        QPushButton *btn_queen = new QPushButton(fac->GetIcon(Piece(Piece::Queen, a)), QString(), this);
        QPushButton *btn_rook = new QPushButton(fac->GetIcon(Piece(Piece::Rook, a)), QString(), this);
        QPushButton *btn_bishop = new QPushButton(fac->GetIcon(Piece(Piece::Bishop, a)), QString(), this);
        QPushButton *btn_knight = new QPushButton(fac->GetIcon(Piece(Piece::Knight, a)), QString(), this);

        QSizePolicy btn_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn_policy.setHorizontalStretch(1);
        btn_policy.setVerticalStretch(1);
        btn_queen->setSizePolicy(btn_policy);
        btn_rook->setSizePolicy(btn_policy);
        btn_bishop->setSizePolicy(btn_policy);
        btn_knight->setSizePolicy(btn_policy);

        btn_queen->resize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE);
        btn_rook->resize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE);
        btn_bishop->resize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE);
        btn_knight->resize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE);

        btn_queen->setIconSize(QSize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE));
        btn_rook->setIconSize(QSize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE));
        btn_bishop->setIconSize(QSize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE));
        btn_knight->setIconSize(QSize(PROMOTED_BUTTON_SIZE, PROMOTED_BUTTON_SIZE));

        layout()->addWidget(btn_queen);
        layout()->addWidget(btn_rook);
        layout()->addWidget(btn_bishop);
        layout()->addWidget(btn_knight);

        connect(btn_queen, SIGNAL(released()), this, SLOT(_queen_pressed()));
        connect(btn_rook, SIGNAL(released()), this, SLOT(_rook_pressed()));
        connect(btn_bishop, SIGNAL(released()), this, SLOT(_bishop_pressed()));
        connect(btn_knight, SIGNAL(released()), this, SLOT(_knight_pressed()));
    }

    PROPERTY(SelectedPiece, Piece);

private slots:

    void _queen_pressed(){
        _select_piece(Piece(Piece::Queen, GetSelectedPiece().GetAllegience()));
    }
    void _rook_pressed(){
        _select_piece(Piece(Piece::Rook, GetSelectedPiece().GetAllegience()));
    }
    void _bishop_pressed(){
        _select_piece(Piece(Piece::Bishop, GetSelectedPiece().GetAllegience()));
    }
    void _knight_pressed(){
        _select_piece(Piece(Piece::Knight, GetSelectedPiece().GetAllegience()));
    }

private:

    void _select_piece(const Piece &p){
        SetSelectedPiece(p);
        accept();
    }

};

Piece BoardView_p::ChoosePromotedPiece(Piece::AllegienceEnum a)
{
    Piece ret(Piece::Queen, a);
    __promoted_piece_selector dlg(a, i_factory);

    if(QDialog::Accepted == dlg.exec())
    {
        ret = dlg.GetSelectedPiece();
    }
    return ret;
}

#include "boardview_p.moc"
