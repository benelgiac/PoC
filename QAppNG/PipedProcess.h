#ifndef INCLUDE_PIPEDPROC_NG
#define INCLUDE_PIPEDPROC_NG
// ===========================================================================
/// @file
/// @brief This file contains ...
///
/// Useful for ...
///
/// @copyright
/// @history
/// REF#        Who              When        What
/// 1399        Andrea Bigagli   16-Nov-2007 Original development
/// 9996        Brian Molina     17-Jan-2014 Return an error if child process does not start correctly.
///                                          Save child pid. Implement IsProcessOk() to use waitpid to 
///                                          determine if child process has died.
/// @endhistory
///
// ===========================================================================

#ifdef WIN32
#include <windows.h>
#else
#define HANDLE int
#endif


#include <stdio.h>

namespace QAppNG
{
    class CPipedProcess
    {
    public:
        CPipedProcess(int mixstderr, int binary,  const char * cmd, const char * cmdline, int * rv, char * env[]);
        bool IsProcessOk();
        ~CPipedProcess(void);
        //FILE * GetInStream() {return m_fin;} TODO: seems not working, use Write !
        FILE * GetOutStream() {return m_fout;}
        //void FlushInStream (); TODO: Not useful if stream not working
        void Write(const char * cmd);
        size_t Write(const char * buffer, int len);
        size_t Read (char *buffer, int len);

    protected:
#ifdef WIN32
        int create_child_process(int mixstderr, int binary, const char * cmd, const char * cmdline);
#endif
        HANDLE *m_processor; 

        HANDLE m_hChildStdinRd;
        HANDLE m_hChildStdinWr;
        HANDLE m_hChildStdinWrDup;
        HANDLE m_hChildStdoutRd;
        int m_hChildStdoutRdFd;
        HANDLE m_hChildStdoutWr; 
        HANDLE m_hChildStdoutRdDup;

        FILE * m_fin;
        FILE * m_fout;
        bool   m_processok;

    private:
#ifndef WIN32
        pid_t m_childPid;
        
        template <typename IOFUNC>
        size_t atomicio(IOFUNC f, int fd, void const *addr, size_t n);
#endif

    };
}
#endif
