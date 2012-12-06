/*
Particlasm code generator interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "CodeGeneratorInterface.h"

#if defined(WIN32) || defined(__WIN32__)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else // WIN32
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <unistd.h>
	#include <cstring>
	#include <cerrno>
	#include <cstdlib>
	#include <cstdio>
#endif

bool CodeGeneratorInterface::RunProcess(const char *CommandLine,
	int& OutExitCode, char *StdoutBuffer, size_t StdoutBufferSize,
	char *StderrBuffer, size_t StderrBufferSize) const
{
#if defined(WIN32) || defined(__WIN32__)
	// create the pipes
	SECURITY_ATTRIBUTES SecAttr;
	SecAttr.nLength = sizeof(SecAttr);
	SecAttr.bInheritHandle = TRUE;
	SecAttr.lpSecurityDescriptor = NULL;
	HANDLE outfd[2], errfd[2];
	CreatePipe(&outfd[0], &outfd[1], &SecAttr, 0);
	SetHandleInformation(&outfd[0], HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&errfd[0], &errfd[1], &SecAttr, 0);
	SetHandleInformation(&errfd[0], HANDLE_FLAG_INHERIT, 0);

	// set up the control structs
	PROCESS_INFORMATION ProcInfo;
	STARTUPINFO StartupInfo;
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.hStdError = errfd[1];
	StartupInfo.hStdOutput = outfd[1];
	StartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

#if _UNICODE
	TCHAR CommandLineTCHAR[strlen(CommandLine) + 1];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, CommandLine, -1,
		CommandLineTCHAR, sizeof(CommandLineTCHAR) / sizeof(CommandLineTCHAR[0]));
#else
	TCHAR *CommandLineTCHAR = (TCHAR *)CommandLine;
#endif // _UNICODE

	// fire!
	if (!CreateProcess(NULL,
		CommandLineTCHAR,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&StartupInfo,
		&ProcInfo))
	{
		return false;
	}

	// wait for the process to finish and retrieve its exit code
	char *OutBuf = StdoutBuffer;
	char *ErrBuf = StderrBuffer;
	DWORD WaitResult;
	do
	{
		// if buffers are provided, perform the reads
		// TODO: switch to named pipes with overlapped I/O and events so that we
		// don't keep spinning while waiting for the pipes to be filled
		DWORD BytesAvail;
		if (OutBuf && PeekNamedPipe(outfd[0], NULL, 0, NULL, &BytesAvail, NULL)
			&& BytesAvail > 0)
		{
			DWORD BytesRead;
			ReadFile(outfd[0], OutBuf, StdoutBufferSize - 1, &BytesRead,
				NULL);
			OutBuf += BytesRead;
			*OutBuf = 0;
			StdoutBufferSize -= BytesRead;
		}
		if (ErrBuf && PeekNamedPipe(errfd[0], NULL, 0, NULL, &BytesAvail, NULL)
			&& BytesAvail > 0)
		{
			// we may have been asked to write both streams into the same buffer
			// if that's the case, concatenate them
			if (StderrBuffer == StdoutBuffer)
			{
				DWORD BytesRead;
				ReadFile(errfd[0], OutBuf, StdoutBufferSize - 1, &BytesRead,
					NULL);
				OutBuf += BytesRead;
				*OutBuf = 0;
				StdoutBufferSize -= BytesRead;
			}
			else
			{
				DWORD BytesRead;
				ReadFile(errfd[0], ErrBuf, StderrBufferSize - 1, &BytesRead,
					NULL);
				ErrBuf += BytesRead;
				*ErrBuf = 0;
				StderrBufferSize -= BytesRead;
			}
		}
	}
	while ((WaitResult = WaitForSingleObject(ProcInfo.hProcess, INFINITE))
		== WAIT_TIMEOUT);
	if (WaitResult != WAIT_OBJECT_0)
		return false;

	DWORD ExitCode;
	if (!GetExitCodeProcess(ProcInfo.hProcess, &ExitCode))
		return false;
	OutExitCode = (int)ExitCode;

	CloseHandle(ProcInfo.hProcess);
	CloseHandle(ProcInfo.hThread);
	CloseHandle(outfd[0]);
	CloseHandle(outfd[1]);
	CloseHandle(errfd[0]);
	CloseHandle(errfd[1]);

	return true;
#else // WIN32
	int outfd[2], errfd[2], oldout, olderr;
	pipe(outfd);
	pipe(errfd);
	oldout = dup(1);
	olderr = dup(2);
	close(1);
	close(2);
	dup2(outfd[1], 1);
	dup2(errfd[1], 2);

	pid_t ChildPID = fork();
	if (!ChildPID)
	{
		// child
		// close unnecessary FDs
		close(outfd[0]);
		close(outfd[1]);
		close(errfd[0]);
		close(errfd[1]);

		// tokenize the command line first
		// create a local copy of it on the stack
		size_t CmdLineLen = strlen(CommandLine);
		char TokenizedLine[CmdLineLen + 1];
		strncpy(TokenizedLine, CommandLine, CmdLineLen);
		TokenizedLine[CmdLineLen] = 0;

		// go over the line, place NULL terminators on whitespaces
		bool Quote = false, Escape = false;
		size_t TokenCount = 1;
		for (char *p = TokenizedLine; *p; ++p)
		{
			if (Quote)
			{
				if (Escape)
					Escape = false;
				else if (*p == '\\')
					Escape = true;
				else if (*p == '\"')
					Quote = false;
			}
			else if (*p == '\"')
				Quote = true;
			else if (*p <= ' ')
			{
				++TokenCount;
				*p = 0;
			}
		}

		// create the pointer array
		char *Argv[TokenCount + 1];
		Argv[0] = TokenizedLine;
		Argv[TokenCount] = NULL;
		size_t TokenIndex = 1;
		for (char *p = TokenizedLine;
			(p - TokenizedLine < (ptrdiff_t)CmdLineLen)
				&& (TokenIndex < TokenCount); ++p)
		{
			if (*p == 0)
			{
				//printf("Argv[%d] = %s\n", TokenIndex, p + 1);
				Argv[TokenIndex++] = p + 1;
			}
		}

		// clear errno so that we may catch errors
		errno = 0;

		execvp(Argv[0], Argv);

		// we only get here if the execvp() call fails
		assert(errno == 0);
		exit(1);
	}
	else
	{
		// parent
		// revert to actual stdout and stderr
		close(1);
		close(2);
		dup2(oldout, 1);
		dup2(olderr, 2);
		// close unnecessary FDs
		close(outfd[1]);
		close(errfd[1]);

		// FIXME: waiting for the process to finish before emptying the pipes
		// may result in a deadlock if the process overflows the buffers!

		// wait for the process to finish and retrieve its exit code
		int ExitCode;
		waitpid(ChildPID, &ExitCode, 0);
		OutExitCode = WEXITSTATUS(ExitCode);

		// if buffers are provided, perform the reads
		if (StdoutBuffer)
			StdoutBuffer[read(outfd[0], StdoutBuffer, StdoutBufferSize - 1)]
				= 0;
		if (StderrBuffer)
		{
			// we may have been asked to write both streams into the same buffer
			// if that's the case, concatenate them
			if (StderrBuffer == StdoutBuffer)
			{
				const size_t StdoutLen = strlen(StdoutBuffer);
				StderrBuffer += StdoutLen;
				if (StdoutBufferSize - StdoutLen > 0)
				{
					*StderrBuffer = 0;
					StderrBufferSize = StdoutBufferSize - StdoutLen;
				}
				else
					StderrBuffer = NULL;
			}
			if (StderrBuffer)
				StderrBuffer[read(errfd[0], StderrBuffer, StderrBufferSize - 1)]
					= 0;
		}

		return WIFEXITED(ExitCode);
	}
#endif // WIN32
}
