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
#include "boardview_p.h"
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

NAMESPACE_GKCHESS1(UI);


#define v  reinterpret_cast<BoardView_p *>(ptr)

// Converts a square to a QModelIndex
#define stoi(square_ptr)    v->model() ? \
    v->model()->index(square_ptr->GetRow(), square_ptr->GetColumn()) : \
    QModelIndex()


BoardView::BoardView(QWidget *parent)
    :QWidget(parent),
      ptr(new BoardView_p(this))
{
    new QVBoxLayout(this);
    layout()->addWidget(v);
    setContentsMargins(0,0,0,0);
    v->show();
}

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

bool BoardView::Editable() const
{
    return v->Editable();
}
void BoardView::SetEditable(bool b)
{
    v->SetEditable(b);
}


END_NAMESPACE_GKCHESS1;
