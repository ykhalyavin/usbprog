#ifndef _XSVFEXEC_H_
#define _XSVFEXEC_H_

/*
 * $Log$
 */

/*!
 * \file xsvfexec.h
 * \brief Executor header file.
 */

extern void XsvfInit(void);
extern void XsvfClose(void);
extern int XsvfExec(char *buf, int size);


#endif
