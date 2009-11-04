/*************************************************************
 *
 * File:        key_handler.h
 * Author:      David Kelley
 * Description: Defines and function prototypes related to
 *              user input
 *
 *************************************************************/

#ifndef __KEY_HANDLER_H__
#define __KEY_HANDLER_H__

#define CR      '\r'
#define NL      '\n'
#define ESC     27
#define INS     331
#define TAB     9
#define BVICTRL(n)    (n&0x1f)


void handle_key(int c);
int is_hex(int c);

#endif /* __KEY_HANDLER_H__ */
