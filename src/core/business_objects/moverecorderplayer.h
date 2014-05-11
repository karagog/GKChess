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

#ifndef GKCHESS_MOVERECORDERPLAYER_H
#define GKCHESS_MOVERECORDERPLAYER_H

#include "gkchess_movedata.h"
#include <QObject>

namespace GKChess{

class ObservableBoard;



/** Records all the moves done on the board and allows you to navigate through the history
 *  of moves.
*/
class MoveRecorderPlayer :
        public QObject
{
    Q_OBJECT
    ObservableBoard &m_board;
    GUtil::List<MoveData> m_moveHistory;
    GUtil::List<MoveData> *m_currentLine;
    int m_index;
    bool m_suppressUpdates;
public:

    MoveRecorderPlayer(ObservableBoard &b, QObject * = 0);

    /** Returns the move history. */
    GUtil::List<MoveData> const &GetHistory() const{ return m_moveHistory; }

    /** Returns the current line. */
    GUtil::List<MoveData> const *GetCurrentLine() const{ return m_currentLine; }

    /** Returns the index we are navigating with the board. */
    int GetCurrentIndex() const{ return m_index; }


signals:

    void NotifyHistoryUpdated();


public slots:

    /** Navigates one move forward in the history. */
    void NavigateForward();

    /** Navigates one move backward in the history. */
    void NavigateBackward();

    /** Clears the history. */
    void Clear();


private slots:

    void _piece_moved(const GKChess::MoveData &);
    void _board_reset();

};


}

#endif // GKCHESS_MOVERECORDERPLAYER_H
