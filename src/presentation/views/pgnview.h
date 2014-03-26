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

#ifndef GKCHESS_PGNVIEW_H
#define GKCHESS_PGNVIEW_H

#include <QTextBrowser>

namespace GKChess{ namespace UI{


/** A view that displays PGN data */
class PGNView :
        public QTextBrowser
{
    Q_OBJECT
    
    bool m_isMousePressed;
public:

    /** Constructs a board view with default options. */
    explicit PGNView(QWidget *parent = 0);
    ~PGNView();

    
protected:

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);

    
private:

    void _highlight_text_for_position(const QPoint &);
    
};


}}

#endif // GKCHESS_PGNVIEW_H
