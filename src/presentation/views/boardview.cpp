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


struct g_d_t
{
    BoardView_p BoardView;
};

// Converts a square to a QModelIndex
#define stoi(square_ptr)    d->BoardView.model() ? \
    d->BoardView.model()->index(square_ptr->GetRow(), square_ptr->GetColumn()) : \
    QModelIndex()


BoardView::BoardView(QWidget *parent)
    :QWidget(parent)
{
    G_D_INIT();
    G_D;

    new QVBoxLayout(this);
    layout()->addWidget(&d->BoardView);
    setContentsMargins(0,0,0,0);
    d->BoardView.show();
}

BoardView::~BoardView()
{
    G_D_UNINIT();
}

void BoardView::SetIconFactory(IFactory_PieceIcon *f)
{
    G_D;
    d->BoardView.SetIconFactory(f);
}

IFactory_PieceIcon *BoardView::GetIconFactory() const
{
    G_D;
    return d->BoardView.GetIconFactory();
}

float BoardView::GetSquareSize() const
{
    G_D;
    return d->BoardView.GetSquareSize();
}
void BoardView::SetSquareSize(float s)
{
    G_D;
    d->BoardView.SetSquareSize(s);
}

QColor BoardView::GetDarkSquareColor() const
{
    G_D;
    return d->BoardView.GetDarkSquareColor();
}
void BoardView::SetDarkSquareColor(const QColor &c)
{
    G_D;
    d->BoardView.SetDarkSquareColor(c);
}

QColor BoardView::GetLightSquareColor() const
{
    G_D;
    return d->BoardView.GetLightSquareColor();
}
void BoardView::SetLightSquareColor(const QColor &c)
{
    G_D;
    d->BoardView.SetLightSquareColor(c);
}

QColor BoardView::GetActiveSquareHighlightColor() const
{
    G_D;
    return d->BoardView.GetActiveSquareHighlightColor();
}
void BoardView::SetActiveSquareHighlightColor(const QColor &c)
{
    G_D;
    d->BoardView.SetActiveSquareHighlightColor(c);
}

void BoardView::HighlightSquare(const ISquare &s, const QColor &c)
{
    G_D;
    d->BoardView.HighlightSquare(stoi((&s)), c);
}
void BoardView::HighlightSquares(const Vector<ISquare const *> &vec, const QColor &c)
{
    G_D;
    QModelIndexList il;
    G_FOREACH_CONST(ISquare const *s, vec)
        il.append(stoi(s));
    d->BoardView.HighlightSquares(il, c);
}

void BoardView::ClearSquareHighlighting()
{
    G_D;
    d->BoardView.ClearSquareHighlighting();
}

BoardModel *BoardView::GetBoardModel() const
{
    G_D;
    return d->BoardView.GetBoardModel();
}

void BoardView::SetBoardModel(BoardModel *bm)
{
    G_D;
    d->BoardView.SetBoardModel(bm);
}

bool BoardView::Editable() const
{
    G_D;
    return d->BoardView.Editable();
}
void BoardView::SetEditable(bool b)
{
    G_D;
    d->BoardView.SetEditable(b);
}


END_NAMESPACE_GKCHESS1;
