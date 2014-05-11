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

#include "moverecorderplayer.h"
#include "gkchess_board.h"
USING_NAMESPACE_GUTIL;
NAMESPACE_GKCHESS;


MoveRecorderPlayer::MoveRecorderPlayer(ObservableBoard &b, QObject *p)
    :QObject(p),
      m_board(b),
      m_currentLine(&m_moveHistory),
      m_index(-1),
      m_suppressUpdates(false)
{
    connect(&b, SIGNAL(NotifyPieceMoved(GKChess::MoveData)),
            this, SLOT(_piece_moved(GKChess::MoveData)));
    connect(&b, SIGNAL(NotifyBoardReset()),
            this, SLOT(_board_reset()));
}

void MoveRecorderPlayer::_piece_moved(const MoveData &md)
{
    if(m_suppressUpdates)
        return;

    if(m_index < m_currentLine->Length() - 1){
        // If we are somewhere in the middle of a line, then
        //  fork a new variant from the last move
        MoveData &last_move = m_currentLine->operator [](m_index + 1);
        last_move.Variants.Append(List<MoveData>());
        m_currentLine = &last_move.Variants.Back();
        m_index = -1;
    }

    // Append to the end of the current line
    m_currentLine->Append(md);
    ++m_index;

    emit NotifyHistoryUpdated();
}

void MoveRecorderPlayer::_board_reset()
{
    if(m_suppressUpdates)
        return;
    Clear();
    emit NotifyHistoryUpdated();
}

void MoveRecorderPlayer::Clear()
{
    // Clear the history
    m_index = -1;
    m_moveHistory.Empty();
    m_currentLine = &m_moveHistory;
}

void MoveRecorderPlayer::NavigateForward()
{
    if(m_index + 1 < m_currentLine->Length())
    {
        ++m_index;
        m_suppressUpdates = true;
        m_board.Move(m_currentLine->operator [](m_index));
        m_suppressUpdates = false;
    }
}

void MoveRecorderPlayer::NavigateBackward()
{
    if(0 <= m_index - 1)
    {
        --m_index;
        m_suppressUpdates = true;
        m_board.FromFEN(m_currentLine->operator [](m_index).Position);
        m_suppressUpdates = false;
    }
}


END_NAMESPACE_GKCHESS;
