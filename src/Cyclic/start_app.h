/*
 * tutorial_5/start_app.h
 *
 * Copyright (c) 2001 Cradle Technologies
 */

#if !defined(_START_APP_H)
#define _START_APP_H

  /*
   * start_app_on_pe()
   *
   * It takes two arguments.
   * 1- the absolute PE number that has to be started.
   * 2- a char pointer to the name of the application that
   *    the PE should run. The name of the application means
   *    file that contains the main() function
   *    the application name should be in CAPS.
   */
  extern void start_app_on_pe(int pe_number, char *name);

  /*
   * stop_pe()
   */
  extern void stop_pe(void);

#endif  // _START_APP_H
