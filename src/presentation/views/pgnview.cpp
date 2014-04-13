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

#include "pgnview.h"
#include "gkchess_pgn_parser.h"
#include <QMouseEvent>
#include <QTextCursor>
#include <QCursor>
#include <QToolTip>

NAMESPACE_GKCHESS1(UI);


PGNView::PGNView(QWidget *p)
    :QTextBrowser(p),
      m_isMousePressed(false)
{
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse);
    setPlainText("Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
}

PGNView::~PGNView()
{}

static bool __is_valid_selection(const QString &s)
{
    bool ret = true;
    if(s.length() <= 1)
        ret = false;
    return ret;
}

void PGNView::_highlight_text_for_position(const QPoint &p)
{
    QTextCursor cur = cursorForPosition(p);
    cur.select(QTextCursor::WordUnderCursor);
    
    // Keep moving the cursor to the right until we get a valid word
    while(!__is_valid_selection(cur.selectedText()) && !cur.atEnd()){
        cur.movePosition(QTextCursor::WordRight);
        cur.select(QTextCursor::WordUnderCursor);
    }
    
    if(!cur.atEnd())
        setTextCursor(cur);
}

void PGNView::mousePressEvent(QMouseEvent *me)
{
    me->accept();
    _highlight_text_for_position(me->pos());
    
    m_isMousePressed = true;
}

void PGNView::mouseReleaseEvent(QMouseEvent *me)
{
    // This indicates that a new word was selected
    QToolTip::showText(QCursor::pos(), textCursor().selectedText());
    
    m_isMousePressed = false;
    me->accept();
}

void PGNView::mouseMoveEvent(QMouseEvent *me)
{
    if(m_isMousePressed)
    {
        me->accept();
        _highlight_text_for_position(me->pos());
    }
}

void PGNView::mouseDoubleClickEvent(QMouseEvent *me)
{
    //QTextBrowser::mouseDoubleClickEvent(me);
    me->accept();
}


END_NAMESPACE_GKCHESS1;
