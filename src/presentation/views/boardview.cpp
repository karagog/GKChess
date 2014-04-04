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

#include "boardview.h"
#include "gkchess_piece.h"
#include "gkchess_boardmodel.h"
#include "gkchess_abstractboard.h"
#include "gkchess_ifactory_pieceicon.h"
#include "gkchess_isquare.h"
#include "gkchess_uiglobals.h"
#include "gutil_map.h"
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

/** The number of refreshes per second while animating. */
#define ANIM_REFRESH_FREQUENCY 30

#define SCROLL_SPEED_FACTOR 0.05

/** Piece icons will appear with a size of the square multiplied by this factor.
 *  \note this should be between 0 and 1
*/
#define PIECE_SIZE_FACTOR  0.825



/** A private class to implement the board view, because we don't want to expose the
 *  QAbstractItemView interface. This gives us better encapsulation.
*/
class board_view_p :
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
    explicit board_view_p(QWidget *parent = 0);
    ~board_view_p();

    BoardModel *GetBoardModel() const;
    void SetBoardModel(BoardModel *);
    void SetIconFactory(IFactory_PieceIcon *);
    IFactory_PieceIcon *GetIconFactory() const{ return i_factory; }
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


/** A class that remembers our current animation. */
class piece_animation_p :
        public QVariantAnimation
{
    Q_OBJECT
public:

    QModelIndex hidden_index;
    Piece piece;

    virtual void updateCurrentValue(const QVariant &){}

};

// Because we are declaring QObjects in this source file:
#include "boardview.moc"


/** Returns a rect with the same center but shrunken by the given factor */
static QRectF __get_shrunken_rect(const QRectF &r, double factor)
{
    return QRect(r.x() + r.width()*(1.0-factor)/2,
                 r.y() + r.height()*(1.0-factor)/2,
                 factor*r.width(),
                 factor*r.height());
}


board_view_p::board_view_p(QWidget *parent)
    :QAbstractItemView(parent),
      m_squareSize(DEFAULT_SQUARE_SIZE),
      m_darkSquareColor(Qt::gray),
      m_lightSquareColor(Qt::white),
      m_activeSquareHighlightColor(Qt::yellow),
      i_factory(0),
      m_selectionBand(new QRubberBand(QRubberBand::Rectangle, this)),
      m_animationGroup(new QSequentialAnimationGroup)
{
    setMouseTracking(true);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(_update_rubber_band()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(_update_rubber_band()));

    // Make sure the application resources were initialized
    InitializeApplicationResources();
}

board_view_p::~board_view_p()
{}


BoardModel *board_view_p::GetBoardModel() const
{
    // We use static cast because we already validated that it's a BoardModel
    //  when they set the model.
    return static_cast<BoardModel *>(model());
}

void board_view_p::SetIconFactory(IFactory_PieceIcon *i)
{
    if(i_factory)
        disconnect(i_factory, SIGNAL(NotifyIconsUpdated()),
                   viewport(), SLOT(update()));

    i_factory = i;
    connect(i_factory, SIGNAL(NotifyIconsUpdated()),
            viewport(), SLOT(update()));

    viewport()->update();
}


QRect board_view_p::visualRect(const QModelIndex &index) const
{
    QRect ret;
    if(index.isValid())
        ret = visual_rectf(index.column(), index.row()).toAlignedRect();
    return ret;
}

QRectF board_view_p::visual_rectf(int col, int row) const
{
    QRectF ret = item_rect(col, row);
    ret.translate(-horizontalOffset(),
                  -verticalOffset());
    return ret;
}

QRectF board_view_p::item_rect(int col, int row) const
{
    return QRectF(get_board_rect().x() + col * GetSquareSize(),
                  get_board_rect().y() + get_board_rect().height() - (GetSquareSize() * (1 + row)),
                  GetSquareSize(), GetSquareSize());
}

void board_view_p::scrollTo(const QModelIndex &index, ScrollHint)
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

void board_view_p::updateGeometries()
{
    QAbstractItemView::updateGeometries();

    // Update the scrollbars whenever our geometry changes
    horizontalScrollBar()->setRange(0, Max(0.0, get_board_rect().width() + 2*MARGIN_OUTER + MARGIN_INDICES + (CURRENT_TURN_ARROW_OFFSET + 1)*GetSquareSize() - viewport()->width()));
    verticalScrollBar()->setRange(0, Max(0.0, get_board_rect().height() + 2*MARGIN_OUTER + MARGIN_INDICES - viewport()->height()));
}

void board_view_p::currentChanged(const QModelIndex &, const QModelIndex &)
{
    _update_rubber_band();
}

void board_view_p::_animation_finished()
{
    piece_animation_p *anim = qobject_cast<piece_animation_p *>(sender());
    if(NULL != anim)
    {
        // Stop hiding the piece that we were animating
        hide_piece_at_index();

        // Remove the animation from the group and delete it
        m_animationGroup->removeAnimation(anim);
        anim->deleteLater();
    }
}

void board_view_p::_update_rubber_band()
{
    // Update the rubber band
    QModelIndex cur = currentIndex();
    m_selectionBand->setVisible(cur.isValid());
    if(cur.isValid()){
        m_selectionBand->setGeometry(visualRect(cur));
    }
}

QModelIndex board_view_p::indexAt(const QPoint &p) const
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

QModelIndex board_view_p::moveCursor(CursorAction ca, Qt::KeyboardModifiers modifiers)
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

int board_view_p::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int board_view_p::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool board_view_p::isIndexHidden(const QModelIndex &index) const
{
    // There are no hidden indices on a chess board
    return false;
}

void board_view_p::setSelection(const QRect &r, QItemSelectionModel::SelectionFlags cmd)
{
    GUTIL_UNUSED(cmd);
    selectionModel()->select(indexAt(QPoint(r.center().x(), r.center().y())),
                             QItemSelectionModel::ClearAndSelect);
}

QRegion board_view_p::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion ret;
    QModelIndexList il = selection.indexes();
    if(1 == il.length()){
        ret = QRegion(visualRect(il[0]));
    }
    return ret;
}

void board_view_p::paintEvent(QPaintEvent *ev)
{
    ev->accept();
    QPainter painter(viewport());

    paint_board(painter, ev->rect());
}

void board_view_p::resizeEvent(QResizeEvent *)
{
    updateGeometries();
}

void board_view_p::paint_piece_at(const Piece &piece, const QRectF &r, QPainter &p)
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

void board_view_p::paint_board(QPainter &painter, const QRect &update_rect)
{
    GUTIL_UNUSED(update_rect);

    if(!model())
        return;

    QModelIndex cur_indx = currentIndex();
    QStyleOptionViewItem option = viewOptions();
    QStyle::State state = option.state;
    piece_animation_p *anim = qobject_cast<piece_animation_p *>(m_animationGroup->currentAnimation());

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
            if(m_hiddenIndex != ind && !pc.IsNull())
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
    for(typename Map<ISquare const *, SquareFormatOptions>::const_iterator iter = m_formatOpts.begin();
        iter != m_formatOpts.end();
        ++iter)
    {
        ISquare const *sqr = iter->Key();

        QRectF cur_rect = item_rect(sqr->GetColumn(), sqr->GetRow());
        QPainterPath path;
        QPainterPath subtracted;
        path.addRect(cur_rect);
        subtracted.addRect(__get_shrunken_rect(cur_rect, 1.0-HIGHLIGHT_THICKNESS));
        painter.fillPath(path.subtracted(subtracted), m_formatOpts[sqr].HighlightColor);
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

    // Any debug drawing?
//    painter.setPen(Qt::red);
//    painter.drawPoint(temp_point);
//    painter.drawRect(temp_rect);
}

void board_view_p::SetBoardModel(BoardModel *bm)
{
    ClearSquareHighlighting();

    // Disconnect the old model
    if(model())
        disconnect(GetBoardModel(), SIGNAL(NotifyPieceMoved(const Piece &, const QModelIndex &, const QModelIndex &)),
                   this, SLOT(_piece_moved(const Piece &, const QModelIndex &, const QModelIndex &)));

    setModel(bm);

    connect(bm, SIGNAL(NotifyPieceMoved(const Piece &, const QModelIndex &, const QModelIndex &)),
            this, SLOT(_piece_moved(const Piece &, const QModelIndex &, const QModelIndex &)));

    updateGeometries();
}

void board_view_p::SetDarkSquareColor(const QColor &c)
{
    m_darkSquareColor = c;
    viewport()->update();
}

void board_view_p::SetLightSquareColor(const QColor &c)
{
    m_lightSquareColor = c;
    viewport()->update();
}

void board_view_p::SetActiveSquareHighlightColor(const QColor &c)
{
    m_activeSquareHighlightColor = c;
    viewport()->update();
}

void board_view_p::SetSquareSize(float s)
{
    m_squareSize = s;
    updateGeometries();
    viewport()->update();
    _update_rubber_band();
}


void board_view_p::wheelEvent(QWheelEvent *ev)
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

void board_view_p::attempt_move(const QModelIndex &, const QModelIndex &)
{
    // Since this is a readonly model, we don't actually let them move a piece, but
    //  if you override this method you can actually move
}

void board_view_p::animate_snapback(const QPointF &from, const QModelIndex &s)
{
    hide_piece_at_index(s);
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

void board_view_p::hide_piece_at_index(const QModelIndex &ind)
{
    m_hiddenIndex = ind;
    viewport()->update();
}

void board_view_p::animate_move(const Piece &p, const QPointF &source, const QPointF &dest, int dur, int easing_curve)
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

void board_view_p::HighlightSquare(const QModelIndex &i, const QColor &c)
{
    ISquare const *s = GetBoardModel()->ConvertIndexToSquare(i);
    if(s)
    {
        SquareFormatOptions sfo;
        sfo.HighlightColor = c;
        m_formatOpts[s] = sfo;
        viewport()->update();
    }
}

void board_view_p::HighlightSquares(const QModelIndexList &il, const QColor &c)
{
    SquareFormatOptions sfo;
    sfo.HighlightColor = c;
    foreach(const QModelIndex &i, il)
    {
        ISquare const *s = GetBoardModel()->ConvertIndexToSquare(i);
        if(s)
            m_formatOpts[s] = sfo;
    }
    viewport()->update();
}

void board_view_p::ClearSquareHighlighting()
{
    m_formatOpts.Clear();
    viewport()->update();
}

QRectF board_view_p::get_board_rect() const
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

void board_view_p::mouseMoveEvent(QMouseEvent *ev)
{
    QAbstractItemView::mouseMoveEvent(ev);

    if(get_board_rect().contains(ev->posF()))
    {
        setCurrentIndex(indexAt(ev->pos()));
    }
}

void board_view_p::mouseDoubleClickEvent(QMouseEvent *)
{
    // Suppress this event, because we don't want to open an editor
}

void board_view_p::_piece_moved(const Piece &p, const QModelIndex &s, const QModelIndex &d)
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
        piece_animation_p *anim(new piece_animation_p);
        anim->hidden_index = s;
        anim->piece = p;
        m_animationGroup->addAnimation(anim);

        hide_piece_at_index(anim->hidden_index);
        m_animationGroup->start();
    }
}




NAMESPACE_GKCHESS1(UI);

#define v  reinterpret_cast<board_view_p *>(ptr)

// Converts a square to a QModelIndex
#define stoi(square_ptr)    v->model() ? \
    v->model()->index(square_ptr->GetRow(), square_ptr->GetColumn()) : \
    QModelIndex()


BoardView::BoardView(QWidget *parent)
    :QWidget(parent),
      ptr(new board_view_p(this))
{
    setContentsMargins(0,0,0,0);
    new QVBoxLayout(this);
    layout()->addWidget(v);
    v->show();
}

BoardView::~BoardView()
{}

void BoardView::SetIconFactory(IFactory_PieceIcon *f)
{
    v->SetIconFactory(f);
}

IFactory_PieceIcon *BoardView::GetIconFactory() const
{
    return v->GetIconFactory();
}

float BoardView::GetSquareSize() const
{
    return v->GetSquareSize();
}
void BoardView::SetSquareSize(float s)
{
    v->SetSquareSize(s);
}

QColor BoardView::GetDarkSquareColor() const
{
    return v->GetDarkSquareColor();
}
void BoardView::SetDarkSquareColor(const QColor &c)
{
    v->SetDarkSquareColor(c);
}

QColor BoardView::GetLightSquareColor() const
{
    return v->GetLightSquareColor();
}
void BoardView::SetLightSquareColor(const QColor &c)
{
    v->SetLightSquareColor(c);
}

QColor BoardView::GetActiveSquareHighlightColor() const
{
    return v->GetActiveSquareHighlightColor();
}
void BoardView::SetActiveSquareHighlightColor(const QColor &c)
{
    v->SetActiveSquareHighlightColor(c);
}

void BoardView::HighlightSquare(const ISquare &s, const QColor &c)
{
    v->HighlightSquare(stoi((&s)), c);
}
void BoardView::HighlightSquares(const Vector<ISquare const *> &vec, const QColor &c)
{
    QModelIndexList il;
    G_FOREACH_CONST(ISquare const *s, vec)
        il.append(stoi(s));
    v->HighlightSquares(il, c);
}

void BoardView::ClearSquareHighlighting()
{
    v->ClearSquareHighlighting();
}

BoardModel *BoardView::GetBoardModel() const
{
    return v->GetBoardModel();
}

void BoardView::SetBoardModel(BoardModel *bm)
{
    v->SetBoardModel(bm);
}


END_NAMESPACE_GKCHESS1;
