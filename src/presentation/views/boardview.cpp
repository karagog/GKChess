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
#include <QXmlStreamWriter>
#include <QVBoxLayout>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QRubberBand>
#include <QScrollBar>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GKCHESS;

NAMESPACE_GKCHESS1(UI);


#define DEFAULT_SQUARE_SIZE 75

#define MARGIN_OUTER   12
#define MARGIN_INDICES 28
#define FONT_SIZE_INDICES  13
#define BOARD_OUTLINE_THICKNESS 3


BoardView::BoardView(QWidget *parent)
    :QAbstractItemView(parent),
      m_squareSize(DEFAULT_SQUARE_SIZE),
      m_darkSquareColor(Qt::gray),
      m_lightSquareColor(Qt::white),
      m_pieceColor(Qt::black),
      m_selectionBand(QRubberBand::Rectangle, this)
{

}

BoardView::~BoardView()
{}


BoardModel *BoardView::GetBoardModel() const
{
    // We use static cast because we already validated that it's a BoardModel
    //  when they set the model.
    return static_cast<BoardModel *>(model());
}


QRect BoardView::visualRect(const QModelIndex &index) const
{
    QRect ret;
    if(index.isValid())
        ret = _get_rect_for_index(index).toAlignedRect();
    return ret;
}

void BoardView::scrollTo(const QModelIndex &, ScrollHint)
{

}

void BoardView::updateGeometries()
{
    QAbstractItemView::updateGeometries();
    horizontalScrollBar()->setRange(0, m_boardRect.width() + 2*MARGIN_OUTER + MARGIN_INDICES);
    verticalScrollBar()->setRange(0, m_boardRect.height() + 2*MARGIN_OUTER + MARGIN_INDICES);
}

QModelIndex BoardView::indexAt(const QPoint &point) const
{
    QModelIndex ret;
    if(m_boardRect.contains(point))
    {
        float x = point.x() - m_boardRect.x();
        float y = m_boardRect.y() + m_boardRect.height() - point.y();

        GASSERT(model());
        GASSERT(0 < x && 0 < y);

        ret = model()->index(y / m_squareSize, x / m_squareSize);
    }
    return ret;
}

QModelIndex BoardView::moveCursor(CursorAction ca, Qt::KeyboardModifiers modifiers)
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

int BoardView::horizontalOffset() const
{
    //return MARGIN_OUTER + MARGIN_INDICES;
    return 0;
}

int BoardView::verticalOffset() const
{
    //return MARGIN_OUTER;
    return 0;
}

bool BoardView::isIndexHidden(const QModelIndex &index) const
{
    // There are no hidden indices on a chess board
    return false;
}

void BoardView::setSelection(const QRect &r, QItemSelectionModel::SelectionFlags cmd)
{
    QModelIndex ind;
    if(m_boardRect.contains(r.topLeft())){
        ind = indexAt(r.topLeft());
    }
    selectionModel()->select(ind, cmd);
    m_selectionBand.setVisible(ind.isValid());
    if(ind.isValid()){
        m_selectionBand.setGeometry(_get_rect_for_index(ind).toAlignedRect());
    }
}

QRegion BoardView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion ret;
    QModelIndexList il = selection.indexes();
    if(1 == il.length()){
        ret = QRegion(_get_rect_for_index(il[0]).toAlignedRect());
    }
    return ret;
}

void BoardView::paintEvent(QPaintEvent *ev)
{
    _paint_board();
}

void BoardView::resizeEvent(QResizeEvent *)
{

}

void BoardView::_paint_board()
{
    if(!model())
        return;

    QStyleOptionViewItem option = viewOptions();
    QStyle::State state = option.state;

    QBrush background = option.palette.base();
    QPen textPen(option.palette.color(QPalette::Text));
    QPen outline_pen(Qt::black);
    outline_pen.setWidth(BOARD_OUTLINE_THICKNESS);

    QPen piece_pen(m_pieceColor);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(viewport()->rect(), background);


    // Shade the squares and paint the pieces
    QFont font_indices = painter.font();
    font_indices.setPointSize(FONT_SIZE_INDICES);

    QFont font_pieces = painter.font();
    font_pieces.setPixelSize(0.825 * m_squareSize);

    painter.setPen(piece_pen);
    painter.setFont(font_indices);
    for(int c = 0; c < model()->columnCount(); ++c)
    {
        for(int r = 0; r < model()->rowCount(); ++r)
        {
            QModelIndex ind = model()->index(r, c);
            QRectF tmp = _get_rect_for_index(ind);

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
            QVariant data = model()->data(ind, Qt::DisplayRole);
            if(!data.isNull())
            {
                if(QVariant::String == data.type()){
                    painter.setFont(font_pieces);
                    painter.drawText(tmp, Qt::AlignCenter, data.toString());
                    painter.setFont(font_indices);
                }
            }
        }
    }

    // paint the vertical borders and indices
    {
        float file_width = m_boardRect.width() / GetBoardModel()->columnCount();
        QPointF p1(m_boardRect.topLeft());
        QPointF p2(p1.x(), p1.y() + m_boardRect.height());
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
        float rank_height = m_boardRect.height() / GetBoardModel()->rowCount();
        QPointF p1(m_boardRect.topLeft());
        QPointF p2(QPoint(p1.x() + m_boardRect.width(), p1.y()));
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
    painter.drawRect(m_boardRect);
}

QRectF BoardView::_get_rect_for_index(const QModelIndex &ind) const
{
    QRectF ret;
    if(ind.isValid())
    {
        int cols = GetBoardModel()->columnCount();
        int rows = GetBoardModel()->rowCount();
        float inc_w = m_boardRect.width() / cols;
        float inc_h = m_boardRect.height() / rows;

        ret = QRectF(m_boardRect.x() + ind.column() * inc_w,
                     m_boardRect.y() + m_boardRect.height() - (inc_h * (1 + ind.row())),
                     inc_w, inc_h);
    }
    return ret;
}

void BoardView::setModel(QAbstractItemModel *m)
{
    if(NULL == dynamic_cast<BoardModel *>(m))
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "The BoardView was designed for BoardModels only");
    QAbstractItemView::setModel(m);
    _update_board_rect();
}

void BoardView::SetDarkSquareColor(const QColor &c)
{
    m_darkSquareColor = c;
    viewport()->update();
}

void BoardView::SetLightSquareColor(const QColor &c)
{
    m_lightSquareColor = c;
    viewport()->update();
}

void BoardView::SetPieceColor(const QColor &c)
{
    m_pieceColor = c;
    viewport()->update();
}

void BoardView::_update_board_rect()
{
    if(model())
    {
        m_boardRect = QRect(QPoint(MARGIN_OUTER + MARGIN_INDICES, MARGIN_OUTER),
                            QPoint(m_squareSize * model()->columnCount() - MARGIN_OUTER,
                                   m_squareSize * model()->rowCount() - MARGIN_OUTER - MARGIN_INDICES));
    }
    else
    {
        m_boardRect = QRect();
    }
}

void BoardView::SetSquareSize(float s)
{
    m_squareSize = s;
    _update_board_rect();
    viewport()->update();
}










static QString __generate_table_style(const BoardView::HtmlFormattingOptions &f)
{
    GUTIL_UNUSED(f);
    return "text-align:center;"
            "border-spacing:1pt;"
            "font-family:'Arial Unicode MS';"
            "border-collapse:collapse;"
            "border-color:#FFFFFFFF;"
            "border-style:solid;"
            "border-width:0pt 0pt 0pt 0pt;";
}

static QString __generate_row_style(const BoardView::HtmlFormattingOptions &)
{
    return "vertical-align:bottom;";
}

static QColor __get_square_color(ISquare const &s, const BoardView::HtmlFormattingOptions &f)
{
    QColor ret;
    if((0x1 & s.GetColumn()) == (0x1 &s.GetRow()))
        ret = f.DarkSquareColor;
    else
        ret = f.LightSquareColor;
    return ret;
}


static QString __generate_cell_style(const ISquare &s, const BoardView::HtmlFormattingOptions &f)
{
    return QString("width:%1pt;"
                   "height:%2pt;"
                   "border-collapse:collapse;"
                   "border-color:#FFFFFFFF;"
                   "border-style:solid;"
                   "border-width:%3pt %3pt %3pt %3pt;"
                   "background-color:#%4;")
            .arg(f.SquareSize)
            .arg(f.SquareSize)
            .arg(f.BorderSize)
            .arg(0x00FFFFFF & __get_square_color(s, f).rgb(), 6, 16, QChar('0'));
}

static QString __generate_piece_style(const Piece &, const BoardView::HtmlFormattingOptions &f)
{
    return QString("font-size:%1pt;"
                   "color:#%2")
            .arg(f.PieceSize)
            .arg(0x00FFFFFF & f.PieceColor.rgb(), 6, 16, QChar('0'));
}

BoardView::HtmlFormattingOptions::HtmlFormattingOptions()
    :HumanReadable(true),
      PieceColor(Qt::black),
      LightSquareColor(Qt::white),
      DarkSquareColor(Qt::gray),
      SquareSize(40),
      BorderSize(1),
      PieceSize(30),
      IndexSize(15)
{}


QString BoardView::GenerateHtml(const Board &b, const HtmlFormattingOptions &f)
{
    QString html;
    if(b.ColumnCount() > 0 && b.RowCount() > 0)
    {
        QXmlStreamWriter sw(&html);
        sw.setAutoFormatting(f.HumanReadable);

        sw.writeStartElement("table");
        sw.writeAttribute("style", __generate_table_style(f));

        // Write each row to html
        for(int i = b.RowCount() - 1; 0 <= i; --i)
        {
            sw.writeStartElement("tr");
            sw.writeAttribute("style", __generate_row_style(f));

            // Write the row number:
            sw.writeStartElement("td");
            sw.writeAttribute("style",
                              QString("vertical-align:middle;width:12pt;font-size:%1pt;")
                              .arg(f.IndexSize));
            sw.writeCharacters(QString("%1").arg(i + 1));
            sw.writeEndElement(); //td

            // Iterate through the columns and write each cell
            for(int j = 0; j < b.ColumnCount(); ++j)
            {
                ISquare const &s( b.SquareAt(j, i) );

                sw.writeStartElement("td");
                sw.writeAttribute("style", __generate_cell_style(s, f));

                // Put a piece in the square if there is one
                Piece const *p = s.GetPiece();
                if(p)
                {
                    sw.writeStartElement("span");
                    sw.writeAttribute("style", __generate_piece_style(*p, f));
                    sw.writeEntityReference(QString("#%1").arg(p->UnicodeValue()));
                    sw.writeEndElement(); //span
                }
                else
                {
                    // Need to write an empty piece to force the xml stream writer to write a
                    //  close tag for the cell
                    sw.writeCharacters(" ");
                }

                sw.writeEndElement();
            }

            sw.writeEndElement(); //tr
        }

        // Write the last row which holds the column letters
        sw.writeStartElement("tr");
        {
            // Empty cell
            sw.writeStartElement("td");
            sw.writeCharacters("");
            sw.writeEndElement(); //td

            char letter = 'a';
            for(int i = 0; i < b.RowCount(); ++i, ++letter)
            {
                sw.writeStartElement("td");
                sw.writeAttribute("style", QString("font-size:%1pt;").arg(f.IndexSize));
                sw.writeCharacters(QString::fromUtf8(&letter, 1));
                sw.writeEndElement(); //td
            }
        }
        sw.writeEndElement(); //tr

        sw.writeEndElement(); //table
        sw.writeEndElement(); //html
    }
    return html;
}


END_NAMESPACE_GKCHESS1;
