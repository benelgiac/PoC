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
#include <winsock2.h>
#include <io.h>
#else
//A.B.This is a kludge: /usr/include/spawn.h uses the same _RESTRICT_KYWD for
//both pointers _and_ arrays, but _RESTRICT_KYWD is defined as __restrict which
//cannot work with arrays, requiring __restrict_arr, so the best thing other
//than modyfing a system header, is to completely switch off this possible
//optimization
#if defined (__sun__)
#   if defined _RESTRICT_KYWD
#       undef _RESTRICT_KYWD
#       define _RESTRICT_KYWD
#   endif
#endif
#include <spawn.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#endif

#include <string>
#include <cstring> //for strlen

#include <QAppNG/PipedProcess.h>

#define PIPE_SIZE        512000

namespace QAppNG
{

#ifdef WIN32
    CPipedProcess::CPipedProcess(int mixstderr, int binary,  const char * cmd, const char * cmdline, int * rv, char * env[])
    {
        m_processok = 0;
        m_hChildStdinRd = NULL;
        m_hChildStdinWr = NULL;
        m_hChildStdinWrDup = NULL;
        m_hChildStdoutRd = NULL;
        m_hChildStdoutWr = NULL; 
        m_hChildStdoutRdDup = NULL;
        
        SECURITY_ATTRIBUTES saAttr; 
        int fSuccess; 
        // Set the bInheritHandle flag so pipe handles are inherited. 
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
        saAttr.bInheritHandle = TRUE; 
        saAttr.lpSecurityDescriptor = NULL; 

        // Create a pipe for the child process's STDOUT. 
        if (!CreatePipe(&m_hChildStdoutRd, &m_hChildStdoutWr, &saAttr, PIPE_SIZE)) {
            *rv = -1;
            return;
        }

        // Create noninheritable read handle and close the inheritable read 
        // handle. 
        fSuccess = DuplicateHandle(GetCurrentProcess(), m_hChildStdoutRd,
            GetCurrentProcess(), &m_hChildStdoutRdDup , 0,
            FALSE, //NOT INHERITABLE
            DUPLICATE_SAME_ACCESS);

        if( !fSuccess ) {
            *rv = -1;
            return;
        }

        CloseHandle(m_hChildStdoutRd);
        m_hChildStdoutRd=NULL;

        // Create a pipe for the child process's STDIN. 
        if (! CreatePipe(&m_hChildStdinRd, &m_hChildStdinWr, &saAttr, PIPE_SIZE))  {
            *rv = -1;
            return;
        }

        // Duplicate the write handle to the pipe so it is not inherited. 
        fSuccess = DuplicateHandle(GetCurrentProcess(), m_hChildStdinWr, 
            GetCurrentProcess(), &m_hChildStdinWrDup, 0, 
            FALSE,   //NOT INHERITABLE 
            DUPLICATE_SAME_ACCESS); 

        if (!fSuccess)  {
            *rv = -1;
            return;
        }
        CloseHandle(m_hChildStdinWr); 
        m_hChildStdinWr=NULL;

        // Now create the child process. 
        fSuccess = create_child_process(mixstderr, binary, cmd, cmdline);
        if (fSuccess != 0)  {
            *rv = -1;
            return;
        }

        //And now close all handle to pipes that are needed only by child, so we don't need anymore after child has born...
        CloseHandle(m_hChildStdoutRd);
        m_hChildStdoutRd=NULL;
        CloseHandle(m_hChildStdoutWr);
        m_hChildStdoutWr=NULL;


        *rv = 0;
        m_processok = 1;
    }


    CPipedProcess::~CPipedProcess(void)
    {
        if (m_hChildStdinRd)
            CloseHandle (m_hChildStdinRd);
        if (m_hChildStdinWr)
            CloseHandle (m_hChildStdinWr);
        if (m_hChildStdinWrDup)
            CloseHandle (m_hChildStdinWrDup);

        if (m_hChildStdoutRd)
            CloseHandle (m_hChildStdoutRd);
        if (m_hChildStdoutWr)
            CloseHandle (m_hChildStdoutWr);
        if (m_hChildStdoutRdDup)
            CloseHandle (m_hChildStdoutRdDup);
    }

    // BDM: don't really care about the windows version just now...
    //
    bool CPipedProcess::IsProcessOk()
    {
        return true;
    }

    int CPipedProcess::create_child_process(int mixstderr, int binary, const char * cmd, const char * args)
    {
        PROCESS_INFORMATION piProcInfo; 
        STARTUPINFO siStartInfo;
        int bFuncRetn = 0; 

        // Set up members of the PROCESS_INFORMATION structure. 
        ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

        // Set up members of the STARTUPINFO structure. 
        ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
        siStartInfo.cb = sizeof(STARTUPINFO); 
        if (mixstderr)
            siStartInfo.hStdError = m_hChildStdoutWr;
        else
            siStartInfo.hStdError = (void *)STD_ERROR_HANDLE;

        /*CECCO CLOSE HANDLE TO STDERROR TO AVOID PROBLEM WITH TSHARK IN CASE OF STD ERROR PRINT(&2)*/
        CloseHandle(siStartInfo.hStdError);

        siStartInfo.hStdOutput = m_hChildStdoutWr;
        siStartInfo.hStdInput = m_hChildStdinRd;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process. 
        std::string c(cmd);
        c += " ";
        c += args;
    
        char * p = (char * )malloc (c.size()+1);
        memcpy (p, c.c_str(), c.size()+1);

        bFuncRetn = CreateProcess(NULL, 
            p,       // command line 
            NULL,          // process security attributes 
            NULL,          // primary thread security attributes 
            TRUE,          // handles are inherited 
            DETACHED_PROCESS, // creation flags 
            NULL,          // use parent's environment 
            NULL,          // use parent's current directory 
            &siStartInfo,  // STARTUPINFO pointer 
            &piProcInfo);  // receives PROCESS_INFORMATION 

        free (p);
        if (bFuncRetn == 0) 
            return -1;

        m_hChildStdoutRdFd =_open_osfhandle ((long)m_hChildStdoutRdDup,0);
        if (m_hChildStdoutRdFd<0)
            return -1;
        if (binary)
            m_fout = _fdopen (m_hChildStdoutRdFd, "rb");
        else 
            m_fout = _fdopen (m_hChildStdoutRdFd, "r");

    /*

        fd=_open_osfhandle ((long)m_hChildStdinWrDup,0);
        if (fd<0)
            return -1;
        m_fin = _fdopen (fd, "wb");
    */
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);


        return 0;
    }
    /*
    void CPipedProcess::FlushInStream()
    {
        FlushFileBuffers(m_hChildStdinWrDup);    
    }
    */
    void CPipedProcess::Write( const char * cmd )
    {
        unsigned long written;

        WriteFile (m_hChildStdinWrDup, cmd, strlen(cmd), &written, NULL);
        FlushFileBuffers(m_hChildStdinWrDup);    


    }
    size_t CPipedProcess::Write( const char * buffer, int len )
    {
        unsigned long written;
        WriteFile (m_hChildStdinWrDup, buffer, len, &written, NULL);
        FlushFileBuffers(m_hChildStdinWrDup);    
        return static_cast<size_t>(written);
    }

    size_t CPipedProcess::Read (char *buffer, int len)
    {
        unsigned long bytesRead;
        ReadFile (m_hChildStdoutRdDup, buffer, len, &bytesRead, NULL);
        return static_cast<size_t>(bytesRead);
    }
#else
    CPipedProcess::CPipedProcess(int mixstderr, int binary, const char * cmd, const char * args, int * rv, char * env[])
        : m_childPid(0)
    {
        sigignore(SIGCHLD);

        int fildes[2];
    
        char * argv[64];
        std::string empty;
        argv[0]=const_cast<char *>(empty.c_str());
        char * ar = (char *)malloc(strlen(args)+1);
        strcpy (ar, args);
    
        int start = 0;
        int cnt = 1;
        for (char * p = ar; *p != 0; p++) {
            if (*p == ' ') {
                *p = 0;
                if (start) {
                    cnt++;
                    start = 0;
                }
            }
            else if (start == 0) {
                start = 1;
                argv[cnt] = p;
            }
        }
        cnt++;
        argv[cnt]=NULL;
    
        pipe (fildes);
    
        m_hChildStdinRd = fildes[0];
        m_hChildStdinWr = fildes[1];

        pipe (fildes);
        m_hChildStdoutRd = fildes[0];
        m_hChildStdoutWr = fildes[1];

        posix_spawn_file_actions_t acts;
   
        int r;
        r = posix_spawn_file_actions_init(&acts);
        r = posix_spawn_file_actions_adddup2(&acts, m_hChildStdinRd,STDIN_FILENO);
        r = posix_spawn_file_actions_adddup2(&acts, m_hChildStdoutWr,STDOUT_FILENO);
        if (mixstderr)
            r = posix_spawn_file_actions_adddup2(&acts, m_hChildStdoutWr,STDERR_FILENO);
        r = posix_spawn_file_actions_addclose(&acts,m_hChildStdinWr);
        r = posix_spawn_file_actions_addclose(&acts,m_hChildStdoutRd);
   
        r = posix_spawnp (&m_childPid, cmd, &acts, NULL, argv,env);
        if ( (r != 0) || (m_childPid == 0) || !IsProcessOk() )
        {
            *rv = -1;
            return;
        }
        
        r = posix_spawn_file_actions_destroy (&acts);
    
        if (binary)
            m_fout = fdopen (m_hChildStdoutRd, "rb");
        else
            m_fout = fdopen (m_hChildStdoutRd, "r");

        free (ar);
    
        close (m_hChildStdinRd);
        close (m_hChildStdoutWr);
        m_hChildStdoutRdFd = m_hChildStdoutRd;
        *rv = 0;
    }

    CPipedProcess::~CPipedProcess(void)
    {
        fclose (m_fout);
        close (m_hChildStdinWr);
        close (m_hChildStdoutRd);
    }

    bool CPipedProcess::IsProcessOk()
    {
        pid_t   wpid;
        int     status;
        
        wpid = waitpid(m_childPid, &status, WNOHANG );
        
        // BDM
        // waitpid is returning an error code of -1 because the action for SIGCHLD is set to SIG_IGN in the parent.
        // Ignoring this signal prevents the child process from sending a SIGCHLD signal to the parent and also
        // prevents the child from becoming a zombie process. Because of this, waitpid is returning an error
        // and errno should be equal to ECHILD.
        //
        if ( wpid == m_childPid )
        {
            return false;
        }
        
        // BDM
        // Ignoring your own children? BAD parent!
        // It's supposed to set errno to ECHILD, but it doesn't seem to be setting errno at all.
        // Assume that an error means the child is dead :(
        //
        if ( wpid == -1 ) // && errno == ECHILD )
        {
            return false;
        }
        
        return true;
    }

    void CPipedProcess::Write( const char * cmd )
    {
        write (m_hChildStdinWr, cmd, strlen(cmd));
    }

    size_t CPipedProcess::Write( const char * buffer, int len )
    {
        return atomicio (::write, m_hChildStdinWr, buffer, len);
    }

    size_t CPipedProcess::Read ( char * buffer, int len )
    {
        return atomicio (::read, m_hChildStdoutRdFd, buffer, len);
    }


    template <typename IOFUNC>
    size_t CPipedProcess::atomicio(IOFUNC f, int fd, void const *addr, size_t n)
    {
        char *buf = static_cast<char *>(const_cast<void *>(addr));
        size_t pos=0;
        ssize_t res;
    
        while (pos < n)
        {
            res = (f) (fd, buf + pos, n - pos);
#ifdef DEBUG
            if (static_cast<size_t>(res) < n)
                std::cerr << "Partial IO: Asked " << n << ", received " << res << std::endl;
#endif
            switch (res)
            {
                case -1:
                    if (errno == EINTR || errno == EAGAIN)
                        continue;
                    return 0;
                case 0:
                    errno = EPIPE;
                    return pos;
                default:
                    pos += res;
            }
        }
        return pos;
    }
#endif
}
