//pppBuffer.hpp

/*
Ping-Pong-Pung buffer implementation
aka:
Tick-Tack-Tock or Flip-Flap-Flop buffer

//Code by Samuel Petrocelli
//July 2011
*/

#ifndef __MIO_PPP_BUFFER_H__
#define __MIO_PPP_BUFFER_H__

#include <iostream>
#include <pthread.h>
#include <stdio.h>


namespace mio{

#define DEFAULT_PPP_BUF_SIZE 3

class pppBuffer{
  private:
    int m_nCurrentRead,
        m_nCurrentWrite,
        m_nPreviousWrite,
        m_nReadCount;

    pthread_mutex_t m_buffer_mutex;
    pthread_mutex_t m_cond_mutex;
    pthread_cond_t  m_condition_var;

    int nNext(int nCurIdx, int nNotAvail){
      if( ( (++nCurIdx) >= m_nSize) || (nCurIdx < 0) ) //reached end of ring buffer, reset to start index
        nCurIdx = 0;
      if(nCurIdx == nNotAvail)
        nCurIdx++;
      if( (nCurIdx >= m_nSize) || (nCurIdx < 0) )
        nCurIdx = 0;
      return(nCurIdx);
    }


  public:
    const int m_nSize; //zero inclusive
    /*
      Constructor of the Ringbuffer
      nSize determines how many elements are stored in it.

      Note that nSize needs to be at least 2
    */
    pppBuffer() : m_nCurrentRead(0), m_nPreviousWrite(0), m_nCurrentWrite(1), 
      m_nSize(DEFAULT_PPP_BUF_SIZE), m_nReadCount(0){}

    ~pppBuffer(){}


    void PrintParams(){
      std::cout << "Current Read:   " << m_nCurrentRead << std::endl;
      std::cout << "Current Write:  " << m_nCurrentWrite << std::endl;
      std::cout << "Previous Write: " << m_nPreviousWrite << std::endl;
      std::cout << "Read Count:     " << m_nReadCount << std::endl << std::endl;
    }

    int Init(){
      if( pthread_mutex_init(&m_buffer_mutex, NULL) != 0 ){
        perror("pppBuffer: nInit() - pthread_mutex_init(m_buffer_mutex) error");
        return(-1);
      }
      if( pthread_mutex_init(&m_cond_mutex, NULL) != 0 ){
        perror("pppBuffer: nInit() - pthread_mutex_init(m_cond_mutex) error");
        return(-1);
      }
      if( pthread_cond_init(&m_condition_var, NULL) != 0 ){
        perror("pppBuffer: nInit() - pthread_cond_init(m_condition_var) error");
        return(-1);
      }
      return(0);
    }

    /*
      gets the index to the next write-bin
    */
    int GetNextWrite(){
      int nResult;
      //std::cout << "called a write" << std::endl;

      pthread_mutex_lock(&m_buffer_mutex);
      {
        m_nPreviousWrite = m_nCurrentWrite; //the buffer just written to is now the previous write
        m_nCurrentWrite = nNext(m_nCurrentWrite, m_nCurrentRead); //m_nCurrentWrite the index that can-be/has-currently-been written to
        nResult = m_nCurrentWrite;

        //signal nGetNextRead incase it is waiting for a new buffer frame
        pthread_mutex_lock(&m_cond_mutex);
        {
          pthread_cond_signal(&m_condition_var);
        }
        pthread_mutex_unlock(&m_cond_mutex);

        m_nReadCount = (m_nReadCount <= 0) ? 0 : (m_nReadCount - 1);
      }
      pthread_mutex_unlock(&m_buffer_mutex);

      return(nResult);
    }


    /*
      gets the index to the next read-bin

      If bMostRecentReadable is true then this will return the bin that was most
      recently written. In other words, we always jump directly ahead to the most
      recent data available.
      If bMostRecentReadable is false then we only move to the next read-bin in order
      (we will not jump ahead).
    */
    int GetNextRead(bool bMostRecentReadable){
      int nResult;
      //std::cout << "called a read" << std::endl;

      m_nReadCount = (m_nReadCount >= m_nSize) ? m_nSize : (m_nReadCount + 1);

      if(m_nReadCount == m_nSize){
        //std::cout << "Blocking read..." << std::endl;
        pthread_mutex_lock(&m_cond_mutex);
        {
          pthread_cond_wait(&m_condition_var, &m_cond_mutex);
        }
        pthread_mutex_unlock(&m_cond_mutex);
      }

      pthread_mutex_lock(&m_buffer_mutex);
      {
        m_nCurrentRead = bMostRecentReadable ? m_nPreviousWrite : nNext(m_nCurrentRead, m_nCurrentWrite);
        nResult = m_nCurrentRead;
      }
      pthread_mutex_unlock(&m_buffer_mutex);

      return(nResult);
    }


    /*
      returns the index of the current write-bin
    */
    int CurWrite(){
      int nResult;
      pthread_mutex_lock(&m_buffer_mutex);
        nResult = m_nCurrentWrite;
      pthread_mutex_unlock(&m_buffer_mutex);
      return(nResult);
    }


    /*
      returns the index of the current read-bin
    */
    int CurRead(){
      int nResult;
      pthread_mutex_lock(&m_buffer_mutex);
        nResult = m_nCurrentRead;
      pthread_mutex_unlock(&m_buffer_mutex);
      return(nResult);
    }
};

} //namespace mio

#endif /*__MIO_PPP_BUFFER_H__*/
