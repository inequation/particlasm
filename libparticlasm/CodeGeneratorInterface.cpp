/*
Particlasm code generator interface
Copyright (C) 2012, Leszek Godlewski <github@inequation.org>
*/

#include "CodeGeneratorInterface.h"

#if (defined(WIN32) || defined(__WIN32__))
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
#if (defined(WIN32) || defined(__WIN32__))
	#error TODO
	/*
	CreateProcess
	WaitForSingleObject
	GetExitCodeProcess
	CloseHandle
	*/
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
